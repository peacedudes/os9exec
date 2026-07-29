# Host Terminal: Write Backpressure, and an Opt-In `OS9T` Wildcard

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:subagent-driven-development or superpowers:executing-plans.
>
> **REQUIRED DOMAIN SKILL:** Invoke **`os9-systems-dev`** (os9exec's kernel-interface layer, the cooperative scheduler, process states) and **`os9-dev`** before reading any code. We own both and are dogfooding them: **report anything wrong, missing or misleading**, especially about process states (`pWaitRead`/`pWaitWrite`) and device semantics.

**Goal:** Two changes to the shipped host-terminal feature. Task 1 stops output being silently dropped when a host endpoint's buffer is full. Task 2 adds an opt-in `OS9T=pty` wildcard so unconfigured `/tN` devices allocate a terminal on demand.

**Order is deliberate:** Task 1 first. Task 2 makes unattached ptys far more likely, so the drop must be fixed before the thing that provokes it is added.

**Tech Stack:** C, POSIX termios/pty, Swift for the integration suite.

---

## Global Constraints

- **`make` warning-clean**; **`make warnings` 0/0 on all four legs, no `NOT BUILT`.**
- **Suite green — judge on `0 failed`, never a total.** Other sessions add tests to the same file.
- **Make every new test fail once before believing it.** Five vacuous checks have been caught in this feature's development, most authored by the controller. Paste both outputs.
- **Test filtering is a BARE POSITIONAL**: `swift run --package-path test OS9Tests hostterm`. Not `--filter`.
- **One focused commit per task.** `Fix:` for Task 1, `Core:` for Task 2. One line, <=80 chars.
- **Never `pkill -f os9exec`** — match a full path or a PID.
- **Several sessions share this repo and commit concurrently.** Only `git add` the listed files, **never `git add -A`**.
- **`ROADMAP*.md` and `DECISIONS*.md` are gitignored** — never commit or modify them.
- **Do not touch `Source/OS9AppEmu/UAE68emulator/`.** Keep the mingw arm compiling.
- **Do not spawn subagents.**
- **`timeout` does not exist on macOS.** Piping into `os9exec` without a trailing `printf '...\n\033\n\004\n'` terminator hangs by design.
- `make warnings` takes minutes — run it in the FOREGROUND, or poll `until ! pgrep -f 'make warnings'; do sleep 15; done`. Do not set up a monitor and end your turn.
- Traps: do not call `cfmakeraw()` (undeclared on the Linux leg); `EWOULDBLOCK` is `#undef`'d, use `EAGAIN`; `host2os9err` takes the POSIX return code `-1`; the OS-9 shell echoes each input line, so never assert on a string that appears in the command text; `makePTY()` returns THREE values and holds a spare slave fd — read its current signature.

---

### Task 1: A full host endpoint blocks the writer instead of losing bytes

**Files:**
- Modify: `Source/OS9exec_core/consio.c` (`ConsoleOut`'s unpaced branch)
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_put` — drop the retry loop)
- Test: `test/Sources/OS9Tests/main.swift`

**The defect.** `ConsPutc` calls `hostterm_put()` and **ignores its return value** (`consio.c:303`). `hostterm_put` was written deliberately never to discard — bounded retry, then `-1` — because silent discard is the exact bug that shipped once before (`75a8ea8`: a best-effort auto-LF push dropped under FIFO saturation, leaving bare-CR lines). That contract is defeated at the call site: on a full endpoint the write stalls ~100ms and the byte is then quietly lost.

Reachable today: bind `/t1` to a pty, attach nothing, write more than the pty buffer (~8KB on macOS).

**The fix, and why it is the right one.** The paced path already solves exactly this problem three lines away (`consio.c:1077`): when the FIFO has no room it saves `cnt`/state, sets `pWaitWrite`, sets `arbitrate`, and breaks. `ConsoleOut`'s entry already restores `saved_cnt` for **any** `pWaitWrite` resume, regardless of which branch parked. And a `pWaitWrite` process is **rescheduled periodically** rather than blocked on an event (`procstuff.c:1129`, `procstuff.c:1294`), so it re-enters, retries, and proceeds once the far end drains. All the machinery exists; use it rather than inventing anything.

- [ ] **Step 1: Write the failing test**

```swift
if let (master, slaveName, spare) = makePTY() {
    // Deliberately DO NOT drain the master. `dir /dd/CMDS` is far larger than a
    // pty buffer (~8KB), so the endpoint fills mid-write. Correct behaviour is
    // that the writer waits and every byte eventually lands; the bug silently
    // discards whatever did not fit after a ~100ms stall.
    //
    // Read the master only AFTER the run, so it is genuinely backed up during
    // it. Assert on total bytes, not on any single string: this is about loss,
    // not content.
    _ = os9(["dir /dd/CMDS >/t1"], env: ["OS9T1": slaveName], timeout: 90)

    var total = 0
    var buf = [UInt8](repeating: 0, count: 65536)
    while true {
        let n = read( master, &buf, buf.count )
        if n <= 0 { break }
        total += n
    }
    close( master ); close( spare )

    // A CMDS listing is comfortably over a pty buffer. Pick the threshold from
    // what a WORKING run actually produces, not from a guess -- measure once,
    // then assert somewhat under it so the test is not brittle.
    if total > 12000 {
        print("PASS: hostterm: a full endpoint blocks the writer, losing nothing")
        passed += 1
    } else {
        print("FAIL: hostterm: a full endpoint blocks the writer, losing nothing")
        print("      [only \(total) bytes survived; output was discarded]")
        failed += 1
    }
}
```

**Measure before asserting.** Run `dir /dd/CMDS` to a drained pty first and record the true byte count; set the threshold below that. If the listing turns out to be under ~8KB it cannot provoke the bug at all — use a bigger producer (`dump` of a large file) and say which you used.

- [ ] **Step 2: Run it and watch it fail**

```sh
swift run --package-path test OS9Tests hostterm
```
Expected: FAIL, with a byte count short of the full listing. **If it passes, STOP AND REPORT** — the drop is then not reachable the way I think it is, and this task needs rethinking rather than implementing.

- [ ] **Step 3: Make `hostterm_put` purely non-blocking**

Remove the retry loop. A 100ms stall inside a cooperative scheduler halts every other OS-9 process, and it is no longer needed once the caller parks properly.

```c
int hostterm_put( int term_id, const char* buffer, int n )
{
    hostterm_typ* h;
    ssize_t       w;

    hostterm_init();
    if (!hostterm_bound( term_id )) return -1;
    h= &hostterms[ term_id ];

    do { w= write( h->fd, buffer, (size_t)n ); } while (w<0 && errno==EINTR);

    /* 0 means "would block": the far end has not drained. NOT an error and NOT
       a licence to discard -- the caller parks the writing process instead.
       Retrying here would stall every other OS-9 process, because the
       scheduler is cooperative. */
    if (w<0 && errno==EAGAIN) return 0;
    if (w<0)                  return -1;

    return (int)w;
} /* hostterm_put */
```

- [ ] **Step 4: Park the writer in `ConsoleOut`**

In `ConsoleOut`'s unpaced `else` branch (currently `ConsPutc( c ); if (needsLF) ConsPutc( LF );`), give a host-bound device its own arm that mirrors the paced one immediately above it:

```c
              else if (hostterm_bound( gConsoleID )) {
                  /* CR and its auto-LF must reach the endpoint TOGETHER or not
                     at all -- a lone CR leaves an unterminated line and the
                     next output lands on top of it (the 75a8ea8 bug). Build
                     the pair, then write it as one unit. */
                  char pair[2];
                  int  len= 0;
                  int  w;

                  pair[len++]= c;
                  if (needsLF) pair[len++]= LF;

                  w= hostterm_put( gConsoleID, pair,len );

                  if (w<len) {
                      /* Would block, or a short write. Park exactly as the
                         paced branch does and retry from this same character
                         on resume -- never advance cnt past a byte the
                         endpoint did not take. A pWaitWrite process is
                         rescheduled periodically (procstuff.c), so it comes
                         back here and proceeds once the far end drains. */
                      cp->saved_cnt  = cnt;
                      cp->saved_state= cp->state;
                      set_os9_state( pid, pWaitWrite, "ConsoleOut" );
                      arbitrate= true;
                      break;
                  }
              }
              else {
                                ConsPutc( c  );
                  if (needsLF)  ConsPutc( LF );
              }
```

Two things to get right, and to state in your report that you checked:
1. `w<len` covers both the would-block case (`0`) and a genuine short write. A hard error (`-1`) also parks — acceptable, and better than dropping; note it in a comment.
2. The `pid>0 && pid<MAXPROCESSES` guard the paced branch relies on must hold here too. Read how `paced` is computed and make sure a system-process write (`pSysTask`, or the `MAXPROCESSES` sentinel used for banner output) cannot park. **If it can, that is a hang — guard it and say so.**

Leave `ConsPutc`'s own host branch alone. It still serves the echo path, `baud_drain_due` and `debug.c`, all single characters at low volume; document in a comment that the bulk path no longer goes through it and those callers remain best-effort.

- [ ] **Step 5: Verify, gate, commit**

```sh
make
swift run --package-path test OS9Tests
make warnings
```
The whole suite matters: `ConsoleOut` is the main output path for every console, so a mistake here breaks everything, not just `/tN`. The existing `baud pacing: paced dump matches unpaced dump byte-for-byte` check is the direct regression guard.

```sh
git add Source/OS9exec_core/consio.c Source/OS9exec_core/hostterm.c \
        test/Sources/OS9Tests/main.swift
git commit -m "Fix: a full /tN blocks the writer instead of losing output"
```

---

### Task 2: `OS9T=pty` — opt-in on-demand terminals

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_spec`)
- Modify: `README.md`
- Test: `test/Sources/OS9Tests/main.swift`

**What and why.** Today an unconfigured `/tN` returns `E_UNIT`, deliberately: on real OS-9 a device without a descriptor does not exist, and a typo (`>/t5` for `>/t4`) should say so rather than silently swallow output. This adds a wildcard so that, **when explicitly asked for**, any unconfigured `/tN` allocates a pty on demand:

```sh
OS9T=pty ./os9exec shell                            # /t1../t49 spring up as used
OS9T=pty OS9T1=/dev/cu.usbserial-1420 ./os9exec shell  # real serial on /t1, ptys elsewhere
```

The default stays `E_UNIT`. **Do not change that**, and do not make the wildcard implicit.

- [ ] **Step 1: Write the failing tests**

Two assertions: the wildcard works, and the default is still a refusal.

```swift
// The wildcard makes an unconfigured /t5 allocate on demand.
let wild = os9(["echo x >/t5"], env: ["OS9T": "pty"])
if wild.contains("attach with: screen") && !wild.contains("Error #000:240") {
    print("PASS: hostterm: OS9T=pty serves an unconfigured /tN"); passed += 1
} else {
    print("FAIL: hostterm: OS9T=pty serves an unconfigured /tN"); failed += 1
}

// Without it, the refusal must stand. This guards the decision, not the code:
// a later change that made auto-allocation implicit would break this test,
// which is the point.
let strict = os9(["echo x >/t5"])
if strict.contains("Error #000:240 (E_UNIT)") {
    print("PASS: hostterm: an unconfigured /tN still refuses by default"); passed += 1
} else {
    print("FAIL: hostterm: an unconfigured /tN still refuses by default"); failed += 1
}
```

- [ ] **Step 2: Run them and watch the first fail**

The second should already pass (it guards existing behaviour); the first must fail. Report both.

- [ ] **Step 3: Fall back to the wildcard**

In `hostterm_spec`, after the per-device lookup:

```c
    snprintf( name,sizeof(name), "OS9T%d", term_id );
    v= getenv( name );
    if (v!=NULL && *v==NUL) v= NULL; /* set-but-empty means unset */

    /* Fall back to the bare OS9T wildcard: "any /tN not named individually".
       Deliberately opt-in. Auto-allocating by default would turn a typo
       (">/t5" for ">/t4") into a terminal nobody is attached to, silently --
       and unlike /hx's fallback, which DISCOVERS a directory that already
       exists, this CREATES a resource, so it can never fail to find anything
       and can never tell you that you were wrong. Real OS-9 has no device
       without a descriptor; E_UNIT stays the default because it is the
       faithful answer. */
    if (v==NULL) {
        v= getenv( "OS9T" );
        if (v!=NULL && *v==NUL) v= NULL;
    }

    return v;
```

Check whether a per-device value that fails to open should fall through to the wildcard. **It should not** — if someone names `/dev/cu.usbserial-1420` explicitly and it is missing, they want the error, not a surprise pty. Confirm the code does not do that and say so.

- [ ] **Step 4: Document it**

Extend the `README.md` device-table row and the terminals paragraph added in `9c5761c`. Match the file's existing voice — it uses em-dashes and prose, so write prose, not a changelog line. State that the wildcard is opt-in and that the default refusal is deliberate.

- [ ] **Step 5: Verify, gate, commit**

```sh
make
swift run --package-path test OS9Tests
make warnings
git add Source/OS9exec_core/hostterm.c README.md test/Sources/OS9Tests/main.swift
git commit -m "Core: OS9T=pty opts in to on-demand terminals for any /tN"
```

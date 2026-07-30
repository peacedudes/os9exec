# `ConsPutcTo`: an explicit destination for console output

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:subagent-driven-development or superpowers:executing-plans.
>
> **REQUIRED DOMAIN SKILL:** Invoke **`os9-systems-dev`** (os9exec's console layer, the cooperative scheduler) and **`os9-dev`** before reading any code. We own both and are dogfooding them: report anything wrong, missing or misleading in a **"Skill feedback"** section.

**Goal:** Give console output an explicit destination parameter, so the one output path that cannot inherit an ambient one stops guessing.

**Architecture:** `ConsPutcTo( int term_id, char c )` does what `ConsPutc` does today, taking its destination as an argument. `ConsPutc( char c )` becomes a one-line wrapper passing `gConsoleID`. `baud_drain_due` — the only console-output site that is not an fmgr entry point — calls `ConsPutcTo` with the device whose FIFO it is draining.

---

## Why, precisely

`ConsPutc( char c )` picks its destination from the global `gConsoleID`. Every console entry point sets that global from the path on the way in, and they are consistent about it: `pConsIn` (`consio.c:745`), `pConsInLn` (`:760`), `ConsoleOut` (`:1010`), `pCready` (`:1228`), `pCclose` (`:691`) all do `gConsoleID = spP->term_id`.

`baud_drain_due` is the sole exception, and structurally so, not carelessly: **it is the only console-output path that is not an fmgr entry point.** It runs from the scheduler (`DoWait`/`do_arbitrate`) with no `spP` in hand, because that is what a FIFO does — it decouples output from the call that produced it. An API meaning "write to whichever console we are currently on" has no answer there, because there is no current console. So it drains every device's backlog to whatever `gConsoleID` was last left holding.

Harmless while `/term` was the only destination. Reachable now that `/tN` exists, and baud pacing is **on by default**.

An earlier fix that saved and restored `gConsoleID` around the loops was written and **dropped** (2026-07-29) because the obvious test passed without it. This version is preferred because it makes the mistake *structurally impossible at that site* rather than corrected by assignment.

## Global Constraints

- **`make` warning-clean**; **`make warnings` 0/0 on all four legs, no `NOT BUILT`.**
- **Suite green — judge on `0 failed`, never a total.** It read 180/0 before this work.
- **One focused commit**, `Fix:` prefix, one line, <=80 chars.
- **Never `pkill -f os9exec`** — match a full path or a PID.
- **Several sessions share this repo and commit concurrently.** Only `git add` the listed files, **never `git add -A`**.
- **`ROADMAP*.md` and `DECISIONS*.md` are gitignored** — never commit or modify them.
- **Do not touch `Source/OS9AppEmu/UAE68emulator/`.** Keep the mingw arm compiling.
- **Do not spawn subagents.** **No non-ASCII in source.**
- **`timeout` does not exist on macOS.** Piping into `os9exec` without a trailing `printf '...\n\033\n\004\n'` terminator hangs by design.
- Test filtering is a BARE POSITIONAL: `swift run --package-path test OS9Tests hostterm`. Not `--filter`.
- `make warnings` takes minutes — foreground it, or poll `until ! pgrep -f 'make warnings'; do sleep 15; done`. Do NOT set up a monitor and end your turn.
- `makePTY()` returns three values, convention `(master, slave, slaveName)` — read its current signature.

## Facts already established — do not re-derive

- **Only non-TTY devices are ever paced.** `ConsoleOut`'s structure is `if (gConsoleID>=TTY_Base) { ...WriteCharsToPTY... } else { ...paced logic... }`, so `baud_devices[]` only ever holds `term_id < TTY_Base`. You therefore do **not** need to worry about `WriteCharsToPTY`'s own separate dependence on the `g_spP` global — that path is unreachable from `baud_drain_due`. Note it in a comment so the next reader does not have to work it out.
- `gLastwritten_pid = proc_slot( currentpid )` inside `ConsPutc` records whoever is *running*, not whoever queued the bytes. Draining from the scheduler therefore attributes bytes to an unrelated process. **Pre-existing, out of scope, do not fix** — but mention it in your report so it is on the record.
- macOS pty output queue is 1024 bytes, not 8K.

---

### Task 1: Add `ConsPutcTo` and use it where there is no ambient destination

**Files:**
- Modify: `Source/OS9exec_core/consio.c` (`ConsPutc`, `baud_drain_due`)
- Modify: `Source/OS9exec_core/consio.h` (declare `ConsPutcTo`)
- Test: `test/Sources/OS9Tests/main.swift` (see Step 1 — read it before writing code)

- [ ] **Step 1: Try once, honestly, to make the old behaviour fail**

This is a structural fix, and a test that provokes the bug is genuinely hard — an earlier attempt is why the assignment-based fix was dropped. **Make one serious attempt, then report the outcome truthfully either way.**

The attempt worth making needs two paced devices with concurrent writers producing enough output to span many drain calls:

```swift
// Two devices, two CONCURRENT writers, distinct content, PACED (no -r).
// `dir /dd/CMDS` is ~3226 bytes; at the default 19200 that is ~1.7s of paced
// output, so both backlogs are live across many baud_drain_due calls. If the
// drain ignores which device queued a byte, content crosses over.
//   /t1 gets a CMDS listing; /t2 gets a DEFS listing.
// Assert each pty saw ONLY its own listing's marker. Neither marker appears in
// any command text, so the shell's command echo cannot satisfy either.
_ = os9(["dir /dd/CMDS >/t1 &", "dir /dd/DEFS >/t2"],
        env: ["OS9T1": slave1Name, "OS9T2": slave2Name],
        paced: true, timeout: 120)
```

Pick markers by looking at what those two directories actually contain — a filename unique to each. **Verify your markers exist first**; a marker that never appears makes the test pass vacuously in the "no crossover" direction.

- [ ] **Step 2: Run it against the CURRENT code and report what happened**

```sh
swift run --package-path test OS9Tests hostterm
```

Two acceptable outcomes, and you must say which:
- **It FAILS** — you have a real repro. Keep the test. Proceed.
- **It PASSES** — the bug is not provokable this way either. **Say so plainly, keep the test anyway** (it is a legitimate multi-device pacing regression guard even if it cannot catch this particular defect), and record in your report that this fix ships as a structural change whose guard is the existing `baud pacing: paced dump matches unpaced dump byte-for-byte` check plus the full suite.

**Do not report a failure you did not observe**, and do not tune the test until it fails — a test bent into failing proves nothing.

- [ ] **Step 3: Add `ConsPutcTo`, make `ConsPutc` a wrapper**

In `consio.c`, parameterise the existing body. Every use of `gConsoleID` inside it becomes the parameter:

```c
  /* Put one character to a NAMED console. Split out from ConsPutc because the
     baud FIFO drains from the scheduler, where there is no "current" console
     to inherit -- see baud_drain_due. Every other caller legitimately runs
     inside an fmgr entry point that has just set gConsoleID from its path, and
     keeps using the ConsPutc wrapper below. */
  void ConsPutcTo( int term_id, char c )
  {
      gLastwritten_pid= proc_slot( currentpid );

      if (term_id>=TTY_Base) {
          #ifdef PIP_SUPPORT
            WriteCharsToPTY( &c,1, term_id, false );
          #endif
          return;
      }

      if (hostterm_bound( term_id )) {
          hostterm_put( term_id, &c,1 );
          return;
      }

      write( 1,&c,1 );

      #ifdef win_unix
        lw_pid( &main_mco );
      #endif
  } /* ConsPutcTo */

  void ConsPutc( char c )
  {
      ConsPutcTo( gConsoleID, c );
  } /* ConsPutc */
```

Preserve the existing comments on `gLastwritten_pid` and the PTY/hostterm branches — move them onto `ConsPutcTo`, do not delete them.

Declare `ConsPutcTo` in `consio.h` next to the existing console declarations, inside the same `#ifdef TERMINAL_CONSOLE` guard `ConsPutc` lives under. **Check how `ConsPutc` is declared there and match it** — if it is not declared in the header at all, keep `ConsPutcTo` file-static instead and say so.

- [ ] **Step 4: Use it in `baud_drain_due`**

Both loops. The unpaced-drain loop:

```c
        if (d->us_per_char==0) {
            /* Name the device explicitly: this function runs from the
               scheduler, not from an fmgr entry point, so there is no current
               console for ConsPutc to inherit -- it would send every device's
               backlog to whichever console was touched last. Only non-TTY
               devices are ever paced (ConsoleOut routes TTY_Base ids away from
               the FIFO entirely), so WriteCharsToPTY's own g_spP dependence
               cannot be reached from here. */
            while (fifo_pop(d,&c)) ConsPutcTo( d->term_id, c );
        }
```

and the paced-drain loop:

```c
        while (d->count>0 && d->next_due_us<=now) {
            fifo_pop( d,&c );
            ConsPutcTo( d->term_id, c );
            d->next_due_us += d->us_per_char;
        }
```

`baud_drain_due` must now leave `gConsoleID` completely untouched. If you find yourself adding a save/restore, stop — that is the fix this one replaces.

- [ ] **Step 5: Verify**

```sh
make
swift run --package-path test OS9Tests
make warnings
```

**The whole suite is the point here**, not the `hostterm` subset: `ConsPutc` is on the output path of every console, so a mistake in the split shows up everywhere. `baud pacing: paced dump matches unpaced dump byte-for-byte` is the direct guard on `baud_drain_due`.

Also confirm interactively that ordinary paced output still looks right — boot `tools/os9repl.sh` (unique `OS9REPL_SESSION`) **without** `-r` so pacing is live, log in, run `dir`, and check the output is intact and correctly ordered rather than garbled. Paste it.

- [ ] **Step 6: Commit**

```sh
git add Source/OS9exec_core/consio.c Source/OS9exec_core/consio.h \
        test/Sources/OS9Tests/main.swift
git commit -m "Fix: paced output names its destination instead of inheriting one"
```

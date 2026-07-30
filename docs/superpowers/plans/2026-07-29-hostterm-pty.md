# Host-Backed Terminal Devices (`/t1`..`/t49`) — Phase 1: PTY Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **REQUIRED DOMAIN SKILL:** Invoke the **`os9-systems-dev`** skill before reading any code — this is os9exec's own kernel-interface layer, which that skill explicitly covers. Also invoke **`os9-dev`** for anything application-level (shell syntax, `kermit`, error codes, SCF path options). We own both skills and are dogfooding them: **report anything in them that is wrong, missing, or misleading** as part of your final report.

**Goal:** Bind OS-9 devices `/t1`..`/t49` to real host terminal endpoints via `OS9Tn` environment variables, so each becomes an independent terminal instead of a silent alias for the main console.

**Architecture:** A new `hostterm.c` owns an array of per-device records, each holding a host fd plus the existing `ttydev_typ` input buffer. `consio.c`'s four console entry points (`ConsPutc`, `ConsGetc`, `pCready`, `pCopen`/`pCclose`) each grow a branch for host-backed devices, exactly mirroring the `gConsoleID>=TTY_Base` branch that already serves the internal PTY backend. Endpoint selection is by value shape: a path opens that device, the literal `pty` allocates a new one.

**Tech Stack:** C89-ish (matches the tree), POSIX `termios`/`posix_openpt`, GNU make, Swift for the integration suite.

---

## Global Constraints

Copied verbatim from `.claude/CLAUDE.md` and project memory. **Every task's requirements implicitly include this section.**

- **`make` must be warning-clean** — "warning-clean, or it is a bug".
- **`make warnings` must be 0/0 on all four toolchains** (clang + gcc-in-docker + mingw). It prints `NOT BUILT` if a leg produced no binary — **a 0/0 score from a build that never ran is not a pass**.
- **`swift run --package-path test OS9Tests` must be green.** The count moves: other sessions add tests to this same file, and it read 168/0 after Task 1. **Judge on `0 failed`, not on a total** — and never "fix" a failure you did not cause; stop and report it.
- **Filtering is a bare positional argument**, not a flag: `swift run --package-path test OS9Tests hostterm`. Passing `--filter hostterm` sets the filter to the literal string `--filter`, matches nothing, and silently runs zero targeted tests while still looking like a clean run. (Found by Task 1's implementer.)
- **Known harness quirk, pre-existing, do not fix here:** the `baud pacing` check (`main.swift` ~line 441) is a bare `do { }` block with no `filter` guard, so it runs on every filtered invocation and costs two emulator runs. Ignore it in filtered output; it is not your test.
- **Make every new check fail once before believing it.** Three checks in this codebase were found unable to fail. This is a hard rule, not advice.
- **Never `pkill -f os9exec`** — other sessions and the suite match that pattern. Match a path.
- **One focused commit per task.** Prefix format: `Core:` `Fix:` `Tests:` `Docs:`. One line, <=80 chars, no body, no footer.
- **Run heavy work sequentially.** One build / one suite / one emulator run at a time. Never overlap them.
- **Do not touch the UAE core** (`Source/OS9AppEmu/UAE68emulator/`).
- **No non-ASCII anywhere in the repo.** No secrets or keys.
- **Do not spawn subagents.** Do the work directly. (A nested agent once outlived its parent and raced the controller.)
- **Several Claude sessions share this repo.** Expect commits and modified files you did not make. Leave them alone; never revert or "fix" them.
- **Interactive verification goes through `tools/os9repl.sh`**, never raw `printf | ./os9exec` pipes. Use `OS9REPL_SESSION=<unique>` to avoid colliding with another session's tmux.
- **`ROADMAP.md`, `ROADMAP-68k.md` and `ROADMAP-6809.md` are gitignored** — never commit them.

## Reference: verified facts this plan is built on

Measured live on 2026-07-29, not inferred. Do not re-derive:

- `/t1` today opens as `fCons` with `term_id`=1 and writes to the **same host stdout** as `/term`. `echo x >/t1` interleaves with the shell prompt character-by-character.
- `tsmon /t1` fails today: `E_NOTRDY` on `I$SetStt`, then `F$Wait`/`E_NOCHLD`.
- `kermit` (`h0/CMDS/kermit`, "OS-9 Kermit V1 R6 (pcb)") runs, takes a *line* argument, and under `-d 0x0001` (dbgAnomaly) issues **no unimplemented/dummy syscalls**. Its device usage is already inside what os9exec implements.
- A pty in raw mode is **8-bit clean both directions** with the slave opened by name from a second process: NUL, CR, LF, XON(0x11), XOFF(0x13), ESC, DEL, 0x80, 0xFF all survive. Without `cfmakeraw` on the slave the same bytes come back mangled (XON/XOFF swallowed, CR->CRLF, DEL->BS-space-BS).
- `egetenv()` (`os9main.c:229`) path-munges **only** `OS9DISK`/`OS9CMDS`/`OS9MDIR` (gated at `os9main.c:302`); every other name falls through to plain `getenv()` and is returned verbatim.
- The `/hx` device-discovery cascade lives in `TwoCharDev()` (`filestuff.c:514`) and is **not** on this code path. There is no "magic" to opt out of.
- Constants (`os9exec_nt.h`): `Main_ID`=0, `VModBase`=50, `TTY_Base`=100, `MAX_CONSOLE`=100, `MAXTTYDEV`=100, `INBUFSIZE`=256.
- `IO_Type()` (`utilstuff.c:3220`) gates `/t0`..`/t49` -> `fCons` behind `#ifndef linux`, so `/t1` is a console on macOS and **not** on Linux. This is a latent bug fixed in Task 1.
- `KeyToBuffer()` (`utilstuff.c:1032`) is already per-device (reads `mco->spP->opt`) and already delivers `SS_SSig` to every syspath matching `term_id`.
- Nothing in the repo currently depends on `/t1` — verified by grep across `test/`, `h0/startup`, and the suite.
- Error codes: `E_UNIT`=240, `E_DEVBSY`=250, `E_PNNF`=216 (`os9defs/os9errno.h`). The shell's live error format is `shell: can't open "/x" - Error #000:221 (E_MNF) Module Not Found`.
- **Do NOT call `cfmakeraw()`.** It is undeclared on the Linux leg: `os9_ll.h:58` does `#undef __USE_MISC` on purpose (to stop `module.h`'s `ulong` clashing with glibc's), and that is the same macro glibc's `termios.h` uses to gate `cfmakeraw`. Task 2 therefore open-codes the canonical `termios(3)` flag recipe in `hostterm_raw()`. Leave it that way. (Diagnosed by Task 2's implementer; `_GNU_SOURCE` does not help.)
- **`host2os9err( hosterr, suggestion )` wants the POSIX RETURN CODE as its first argument, not an errno.** Its UNIX arm opens with `if (hosterr==0) return 0;` and then reads `errno` itself, so passing `0` silently reports SUCCESS. Pass the failing call's `-1`. It maps ENOENT->E_PNNF, EACCES->E_FNA, EBUSY->E_SHARE, ENXIO/ENODEV->E_UNIT, and falls back to your suggestion otherwise (`utilstuff.c:568`).
- **The OS-9 shell echoes each input line to its console before running it.** So a `check(..., contains: "foo", "echo foo")` passes on the echoed *command text* alone and can never fail. Never assert on a string that appears in the command. Assert on something only the code under test can produce (an error code, a byte read back from the far end of a pty). This killed the plan's original Task 2 test — found by its implementer.
- **The Swift harness DISCARDS the emulator's host stderr** — it is drained only to avoid a pipe deadlock (`main.swift`, `_ = stderrPipe...readDataToEndOfFile()`), never merged into the returned string. This does not break `uphe_printf`: OS-9's stderr user path maps to the console, and `ConsPutc` writes to host fd **1**, so emulator `# ...` messages do arrive on stdout and are captured. Do not "fix" the harness to merge stderr as part of this plan.
- **`tsmon /t1` WORKS on a host-backed terminal — verified independently by the controller, 2026-07-29.** With `OS9T1=<pty slave>` and `-r`, `tsmon /t1` serves the full banner and `User name?:` prompt onto the pty, and a login proceeds to a working shell. An earlier agent reported this as failing; that was its test rig, not the emulator (it hit the two pty traps below). **Do not re-investigate this as a bug.** Reproduction script pattern: raw-mode the master BEFORE writing, hold a spare slave fd, `tsmon /t1`, then send `\r` on the master.
- **`-d 0x0002` return lines are MISLABELLED** — `debug_return()` re-derives the call name from `cp->func`, a single scalar, so a nested call renames the outer call's return. Do not trust the name on a `<<< ... returns:` line. Logged on the roadmap.
- Machine tooling: `screen`, `cu`, `nc`, `tmux` present. **No `socat`** — and not needed.

## Scope

**In scope (Phase 1):** open-by-name and self-allocated pty, output path, input path, `E_UNIT` when unconfigured, kermit end-to-end.

**Explicitly deferred to later plans** — do not build these:
- **TCP endpoints** (`OS9T1=tcp:2301`) — sockets, listen/accept, no termios. Separate plan.
- **Baud/parity/flow-control from `PD_BAU`** — meaningless for a pty; belongs with the real-serial arm.
- **Windows** — no `termios`, no pty. `hostterm.c` compiles to a stub there (Task 1).
- **`pWaitWrite` parking on a full host buffer** — see Task 3's documented limitation.

## File Structure

| File | Responsibility |
|---|---|
| `Source/OS9exec_core/hostterm.c` (**create**) | All host-fd ownership: config lookup, open/close, raw-mode setup, read/write, poll. The only file that knows what an endpoint spelling means. |
| `Source/OS9exec_core/hostterm.h` (**create**) | Its public interface, consumed by `consio.c` and `telnetaccess.c`. |
| `Source/OS9exec_core/consio.c` (**modify**) | Four branch points routing console I/O to `hostterm` when a device is bound. |
| `Source/OS9exec_core/telnetaccess.c` (**modify**) | One call adding host terminals to the existing input-poll sweep. |
| `Source/OS9exec_core/utilstuff.c` (**modify**) | `IO_Type()` `#ifndef linux` fix. |
| `GNUmakefile` (**modify**) | Add `hostterm.c` to `SRCS`. |
| `test/Sources/OS9Tests/main.swift` (**modify**) | Env passthrough plus the new tests. |
| `README.md` (**modify**) | One row in the device table, one paragraph. |

---

### Task 1: Skeleton, config lookup, and the `E_UNIT` gate

**Files:**
- Create: `Source/OS9exec_core/hostterm.h`
- Create: `Source/OS9exec_core/hostterm.c`
- Modify: `GNUmakefile` (SRCS list, after `$(CORE)/funcdispatch.c`)
- Modify: `Source/OS9exec_core/consio.c` (`pCopen`, around line 578)
- Modify: `Source/OS9exec_core/utilstuff.c` (`IO_Type`, lines 3219-3224)
- Modify: `Source/OS9exec_core/os9exec_incl.h` (add the include)
- Test: `test/Sources/OS9Tests/main.swift`

**Interfaces:**
- Consumes: nothing (first task).
- Produces: `hostterm.h`, whose full contents are given below. Later tasks call `hostterm_configured`, `hostterm_open`, `hostterm_close`, `hostterm_bound`, `hostterm_put`, `hostterm_get`, `hostterm_ready`, `hostterm_poll`. Also produces the Swift harness's `env:` parameter on `os9()`, `run()` and the new `checkEnv()`.

- [ ] **Step 1: Write the failing test**

In `test/Sources/OS9Tests/main.swift`, first add env passthrough. Change the `os9` signature (line 139) and the `run` signature (line 311), and the local-run environment (line 254):

```swift
// line 139 - add the env parameter
func os9(_ commands: [String], timeout: TimeInterval = defaultTimeout, paced: Bool = false,
         disk: String = diskPath, env: [String: String] = [:]) -> String {
```

```swift
// line 254 - merge the caller's env over the defaults
        process.environment  = ["OS9DISK": disk,
                                "OS9H\(scratchDev.dropFirst())": resolvedScratchDisk]
                               .merging(env) { _, caller in caller }
```

```swift
// line 311 - thread env through run()
func run(_ name: String, expectation: String, commands: [String], disk: String = diskPath,
         timeout: TimeInterval = defaultTimeout, env: [String: String] = [:],
         check: (String) -> Bool) {
    guard filter.isEmpty || name.localizedCaseInsensitiveContains(filter) else { return }
    let output = os9(commands, timeout: timeout, disk: disk, env: env)
```

Then add a helper next to `check` (after line 344) and the new test. Put the test at the end of the file, before the summary print:

```swift
func checkEnv(_ name: String, contains pattern: String,
              env: [String: String], _ commands: String...) {
    run(name, expectation: "contains: \(pattern)", commands: commands, env: env) {
        $0.contains(pattern)
    }
}

// -- host-backed terminals -------------------------------------------------
// An unconfigured /tN is not a device. It used to silently alias the main
// console, so `echo` to it appeared to work while interleaving its bytes into
// the shell's own prompt -- and `tsmon /t1` failed with a misleading E_NOTRDY
// much later instead of an honest "no such unit" at open time.
check("hostterm: unconfigured /t1 is E_UNIT",
      contains: "Error #000:240 (E_UNIT)",
      "echo hello >/t1")
```

- [ ] **Step 2: Run the test to verify it fails**

```sh
swift run --package-path test OS9Tests hostterm
```

Expected: `FAIL: hostterm: unconfigured /t1 is E_UNIT`. Today `/t1` opens successfully and writes to stdout, so no error appears at all.

**The expected string is already verified — do not re-derive it.** `E_UNIT` is 240 (`os9defs/os9errno.h:178`), `E_DEVBSY` is 250 (`:188`), and the shell's live message format was captured on 2026-07-29 as:

```
shell: can't open "/nosuchdev" - Error #000:221 (E_MNF) Module Not Found
```

so a refused `/t1` will read `... - Error #000:240 (E_UNIT) Illegal Unit (drive)`.

- [ ] **Step 3: Create `hostterm.h`**

```c
//
//    OS9exec, OS-9 emulator for Mac OS, Windows and Linux
//
//    Host-backed terminal devices: /t1../t49 bound to a real host endpoint.
//

#ifndef HOSTTERM_H
#define HOSTTERM_H

/* /t0 is the main console (term_id 0); /t50 and up are VMod territory
   (VModBase). So the host-backed range is 1..VModBase-1. */
#define HOSTTERM_MIN  1
#define HOSTTERM_MAX  (VModBase-1)

/* Is <term_id> inside the host-backed range at all? */
Boolean hostterm_in_range ( int term_id );

/* Is an OS9T<n> environment variable set for this device?
   Says nothing about whether it is currently open. */
Boolean hostterm_configured( int term_id );

/* Is this device currently open on a host fd? */
Boolean hostterm_bound    ( int term_id );

/* Open the configured endpoint and bind it to <spP>.
   Returns 0, or an os9err (E_UNIT if unconfigured or the spelling is
   unsupported, E_DEVBSY if the host refuses). */
os9err  hostterm_open     ( int term_id, syspath_typ* spP );

/* Close and release the host fd. Safe on an unbound device. */
void    hostterm_close    ( int term_id );

/* Write <n> bytes. Returns bytes written, or -1 on a genuine error. */
int     hostterm_put      ( int term_id, const char* buffer, int n );

/* Take one buffered byte. Returns 1 if <c> was filled, 0 if none ready. */
int     hostterm_get      ( int term_id, char* c );

/* How many bytes are buffered for this device? */
Boolean hostterm_ready    ( int term_id, long* cnt );

/* Drain every bound host fd into its input buffer. Called from
   CheckInputBuffers(), alongside the tty and main-console sweeps. */
void    hostterm_poll     ( void );

#endif /* HOSTTERM_H */

/* eof */
```

- [ ] **Step 4: Create `hostterm.c` with config lookup only**

Everything except `hostterm_configured` / `hostterm_in_range` is a stub in this task; later tasks fill them in. Stubs must be honest (return an error), never silently succeed.

```c
//
//    OS9exec, OS-9 emulator for Mac OS, Windows and Linux
//
//    Host-backed terminal devices: /t1../t49 bound to a real host endpoint.
//
//    Selected per device by an environment variable, mirroring OS9DISK and
//    OS9Hx: OS9T1, OS9T2, ... The VALUE picks the backend, exactly as
//    OS9DISK's value already picks host-directory vs RBF image:
//
//        OS9T1=pty              allocate a new pty, report the slave name
//        OS9T1=/dev/ttys004     open an existing tty device by name
//
//    Deliberately NOT the /hx discovery cascade (TwoCharDev, filestuff.c):
//    there is nothing to search for. Set = bound, unset = E_UNIT.
//

#include "os9exec_incl.h"

#if defined UNIX && !defined MINGW
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>
#endif

typedef struct {
    Boolean     open;       /* host fd is live */
    int         fd;         /* -1 when not open */
    ttydev_typ  dev;        /* per-device input buffer; KeyToBuffer target */
    char        endpoint[OS9PATHLEN]; /* what we actually opened, for messages */
} hostterm_typ;

/* Indexed by term_id directly; slot 0 is unused so the arithmetic stays
   obvious at every call site. */
static hostterm_typ hostterms[ HOSTTERM_MAX+1 ];
static Boolean      initialised= false;

static void hostterm_init( void )
{
    int k;
    if (initialised) return;

    for (k=0; k<=HOSTTERM_MAX; k++) {
        hostterms[k].open       = false;
        hostterms[k].fd         =    -1;
        hostterms[k].endpoint[0]=   NUL;
        hostterms[k].dev.installed = false;
        hostterms[k].dev.inBufUsed =     0;
        hostterms[k].dev.holdScreen= false;
        hostterms[k].dev.pid       =     0;
        hostterms[k].dev.spP       =  NULL;
    }

    initialised= true;
} /* hostterm_init */

Boolean hostterm_in_range( int term_id )
{
    return term_id>=HOSTTERM_MIN && term_id<=HOSTTERM_MAX;
} /* hostterm_in_range */

/* The OS9T<n> value for this device, or NULL.
   Plain getenv, not egetenv: egetenv resolves relative values against
   startPath, which is right for a DISK and wrong for an endpoint -- "pty" is
   not a path and must not become "<startPath>/pty". egetenv happens to pass
   non-OS9DISK names through verbatim today (os9main.c:302), but depending on
   that would silently couple us to a disk-path decision. */
static char* hostterm_spec( int term_id )
{
    char  name[16];
    char* v;

    if (!hostterm_in_range( term_id )) return NULL;

    snprintf( name,sizeof(name), "OS9T%d", term_id );
    v= getenv( name );

    if (v!=NULL && *v==NUL) v= NULL; /* set-but-empty means unset */
    return v;
} /* hostterm_spec */

Boolean hostterm_configured( int term_id )
{
    return hostterm_spec( term_id )!=NULL;
} /* hostterm_configured */

Boolean hostterm_bound( int term_id )
{
    hostterm_init();
    if (!hostterm_in_range( term_id )) return false;
    return hostterms[ term_id ].open;
} /* hostterm_bound */

os9err hostterm_open( int term_id, syspath_typ* spP )
{
    #ifndef __GNUC__
    #pragma unused( term_id,spP )
    #endif
    (void)spP;
    hostterm_init();
    return os9error(E_UNIT); /* filled in by Task 2 */
} /* hostterm_open */

void hostterm_close( int term_id )
{
    #ifndef __GNUC__
    #pragma unused( term_id )
    #endif
    (void)term_id;
} /* hostterm_close */

int hostterm_put( int term_id, const char* buffer, int n )
{
    #ifndef __GNUC__
    #pragma unused( term_id,buffer,n )
    #endif
    (void)buffer; (void)n; (void)term_id;
    return -1; /* filled in by Task 3 */
} /* hostterm_put */

int hostterm_get( int term_id, char* c )
{
    #ifndef __GNUC__
    #pragma unused( term_id,c )
    #endif
    (void)c; (void)term_id;
    return 0; /* filled in by Task 4 */
} /* hostterm_get */

Boolean hostterm_ready( int term_id, long* cnt )
{
    #ifndef __GNUC__
    #pragma unused( term_id )
    #endif
    (void)term_id;
    *cnt= 0;
    return false; /* filled in by Task 4 */
} /* hostterm_ready */

void hostterm_poll( void )
{
    /* filled in by Task 4 */
} /* hostterm_poll */

/* eof */
```

**On Windows** (`MINGW`/`windows32`) there is no `termios` and no pty. The file above still compiles there — the includes are gated and no stub touches a POSIX call. Task 2 must keep it that way: put every fd operation behind `#if defined UNIX && !defined MINGW`, with the non-UNIX arm returning `os9error(E_UNIT)`. Do not add a Windows implementation; it is explicitly out of scope.

- [ ] **Step 5: Wire the include and the makefile**

In `Source/OS9exec_core/os9exec_incl.h`, add `#include "hostterm.h"` next to the other core includes — **after** whichever header defines `VModBase`, `ttydev_typ`, `syspath_typ`, `Boolean`, `os9err` and `OS9PATHLEN` (that is `os9exec_nt.h`). Verify by reading the include list rather than guessing the position.

In `GNUmakefile`, add to `SRCS` in alphabetical position (after `$(CORE)/funcdispatch.c`, before `$(CORE)/icalls.c`):

```make
    $(CORE)/hostterm.c \
```

- [ ] **Step 6: Add the `E_UNIT` gate in `pCopen`**

In `Source/OS9exec_core/consio.c`, immediately after the matcher `while` loop ends and before `spP->term_id= id;` (currently line 578):

```c
    /* A /tN in the host-backed range is only a device if OS9T<n> names an
       endpoint. Unconfigured, it used to fall through and share the MAIN
       console's stdin/stdout -- so `echo x >/t1` interleaved its bytes into
       the shell's own prompt, and `tsmon /t1` failed with a misleading
       E_NOTRDY much later instead of an honest refusal here. Real OS-9 has no
       device without a descriptor; E_UNIT is what it says for a bad unit. */
    if (hostterm_in_range( id ) && !hostterm_configured( id ))
        return os9error(E_UNIT);
```

- [ ] **Step 7: Fix the `IO_Type` Linux inconsistency**

In `Source/OS9exec_core/utilstuff.c`, lines 3219-3224 currently read:

```c
            #ifndef linux
           || (ustrcmp (os9path,"/t0") == 0)
           || (os9path[0]==PSEP &&
               os9path[1]=='t'  && atoi(&os9path[2])>= 1 && /* /t1 ../t49 */
                                   atoi(&os9path[2])<VModBase)
            #endif
```

Remove the `#ifndef linux` / `#endif` guard, leaving the three conditions unconditional:

```c
           /* No longer #ifndef linux. /t1../t49 classified as fCons on macOS
              and as a FILE on Linux, so the same path was a console on one
              host and not on another -- an accident of the old Mac-only
              serial code, not a deliberate platform difference. Nothing in
              the tree depended on the Linux behaviour (checked). */
           || (ustrcmp (os9path,"/t0") == 0)
           || (os9path[0]==PSEP &&
               os9path[1]=='t'  && atoi(&os9path[2])>= 1 && /* /t1 ../t49 */
                                   atoi(&os9path[2])<VModBase)
```

- [ ] **Step 8: Build and run the test to verify it passes**

```sh
make
swift run --package-path test OS9Tests hostterm
```

Expected: `make` warning-clean; `PASS: hostterm: unconfigured /t1 is E_UNIT`.

- [ ] **Step 9: Run the full suite and the toolchain sweep**

```sh
swift run --package-path test OS9Tests
```
Expected: 148/0 (or 149/0 with the new test). **Any pre-existing failure is a stop-and-report, not something to fix here.**

```sh
make warnings
```
Expected: 0/0 on all legs, and **no `NOT BUILT`**. If a leg says `NOT BUILT`, that leg did not run and is not a pass — report it.

- [ ] **Step 10: Commit**

```sh
git add Source/OS9exec_core/hostterm.c Source/OS9exec_core/hostterm.h \
        Source/OS9exec_core/consio.c Source/OS9exec_core/utilstuff.c \
        Source/OS9exec_core/os9exec_incl.h GNUmakefile \
        test/Sources/OS9Tests/main.swift
git commit -m "Core: unconfigured /t1../t49 report E_UNIT, not the main console"
```

---

### Task 2: Open an endpoint by name, in raw mode

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_open`, `hostterm_close`)
- Test: `test/Sources/OS9Tests/main.swift`

**Interfaces:**
- Consumes: `hostterm_in_range`, `hostterm_spec`, the `hostterms[]` array, `hostterm_init` — all from Task 1.
- Produces: a working `os9err hostterm_open( int term_id, syspath_typ* spP )` that leaves `hostterms[term_id].open==true` and `.fd` a raw-mode fd; `void hostterm_close( int term_id )`. Tasks 3 and 4 read `.fd` and `.dev`.

Open-by-name comes **before** self-allocated pty deliberately: it is simpler, it is what the serial arm will need unchanged, and it lets the Swift suite own the pty pair, so every later test is deterministic with no output parsing.

- [ ] **Step 1: Write the failing test**

Add to `main.swift`, next to the Task 1 test. This helper creates a pty pair the test owns; the emulator gets the slave name.

```swift
// Creates a pty pair. Returns (masterFD, slaveDeviceName). The TEST owns the
// master, so it can drive /tN from outside the emulator with no parsing of
// emulator output. A pty slave is an ordinary tty device, so this exercises
// exactly the code path a real /dev/cu.usbserial-* will take.
func makePTY() -> (Int32, String)? {
    var master: Int32 = 0
    var slave:  Int32 = 0
    guard openpty(&master, &slave, nil, nil, nil) == 0 else { return nil }
    let name = String(cString: ttyname(slave))
    close(slave)                       // the emulator opens it by name
    var raw = termios()
    tcgetattr(master, &raw)
    cfmakeraw(&raw)
    tcsetattr(master, TCSANOW, &raw)
    return (master, name)
}

if let (master, slaveName) = makePTY() {
    checkEnv("hostterm: /t1 opens a named endpoint",
             contains: "opened",
             env: ["OS9T1": slaveName],
             "echo opened")
    close(master)
} else {
    print("FAIL: hostterm: could not create a pty pair")
    failed += 1
}
```

**Verified 2026-07-29 on this machine: `openpty` compiles and runs from Swift via plain `import Foundation`** — no extra import, no `linkedLibrary`. Do not add either. (If a future Linux Swift run fails to link, `linkedLibrary("util")` on the `OS9Tests` target guarded by `#if os(Linux)` is the fix — but do not add it speculatively.)

- [ ] **Step 2: Run the test to verify it fails**

```sh
swift run --package-path test OS9Tests hostterm
```

Expected: FAIL. `hostterm_open` is still Task 1's stub returning `E_UNIT`, so the shell reports an error rather than reaching `echo`.

Note this test asserts only that the emulator **starts and runs a command** with `OS9T1` set — a weak but genuine gate. It fails today because a failing `hostterm_open` aborts the open. Tasks 3 and 4 assert the actual byte flow.

- [ ] **Step 3: Implement `hostterm_open` and `hostterm_close`**

Replace Task 1's stubs in `hostterm.c`:

```c
#if defined UNIX && !defined MINGW

/* Put a tty into 8-bit-transparent raw mode.
   Verified necessary, not assumed: without cfmakeraw a round-trip through a
   pty mangles exactly the bytes a file transfer depends on -- XON/XOFF are
   swallowed, CR becomes CRLF, DEL becomes BS-space-BS. */
static Boolean hostterm_raw( int fd )
{
    struct termios t;

    if (tcgetattr( fd,&t )!=0) return false;
    cfmakeraw( &t );

    /* CLOCAL: do not wait on carrier detect. CREAD: enable the receiver.
       Both are no-ops for a pty and both matter for a real serial port, so
       they are set here rather than in a later serial-only change. */
    t.c_cflag |= CLOCAL | CREAD;

    /* Return immediately with whatever is there. The emulator polls; it must
       never block inside a read, because the scheduler is cooperative and a
       blocked read stalls every other OS-9 process. */
    t.c_cc[VMIN ]= 0;
    t.c_cc[VTIME]= 0;

    return tcsetattr( fd,TCSANOW, &t )==0;
} /* hostterm_raw */

os9err hostterm_open( int term_id, syspath_typ* spP )
{
    hostterm_typ* h;
    char*         spec;
    int           fd;

    hostterm_init();
    if (!hostterm_in_range( term_id )) return os9error(E_UNIT);

    h= &hostterms[ term_id ];
    if (h->open) { h->dev.spP= spP; return 0; } /* already open: share it */

    spec= hostterm_spec( term_id );
    if (spec==NULL) return os9error(E_UNIT);

    if (*spec!=PATHDELIM) {
        /* "pty" arrives in Task 5; anything else is simply not an endpoint.
           Refuse loudly -- a mistyped OS9T1 must not look like it worked. */
        uphe_printf( "OS9T%d: unsupported endpoint '%s'\n", term_id, spec );
        return os9error(E_UNIT);
    }

    /* O_NONBLOCK at open, and kept: it skips the carrier-detect wait a real
       serial port would otherwise impose, and it is what keeps every later
       read non-blocking. O_NOCTTY: this must never become our controlling
       terminal, which would route the host's job-control signals here. */
    fd= open( spec, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0) {
        uphe_printf( "OS9T%d: cannot open '%s'\n", term_id, spec );
        return os9error(E_DEVBSY);
    }

    if (!hostterm_raw( fd )) {
        uphe_printf( "OS9T%d: '%s' is not a terminal\n", term_id, spec );
        close( fd );
        return os9error(E_DEVBSY);
    }

    h->fd  = fd;
    h->open= true;
    strncpy( h->endpoint,spec, OS9PATHLEN-1 );
             h->endpoint[      OS9PATHLEN-1 ]= NUL;

    h->dev.installed = true;
    h->dev.inBufUsed =     0;
    h->dev.holdScreen= false;
    h->dev.pid       =     0;
    h->dev.spP       =   spP;

    debugprintf( dbgTerminal,dbgNorm,
                 ( "# hostterm: /t%d -> %s (fd %d)\n", term_id, spec, fd ) );
    return 0;
} /* hostterm_open */

void hostterm_close( int term_id )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_in_range( term_id )) return;

    h= &hostterms[ term_id ];
    if (!h->open) return;

    close( h->fd );
    h->fd          =    -1;
    h->open        = false;
    h->dev.installed= false;
    h->dev.spP     =  NULL;
} /* hostterm_close */

#else /* not UNIX, or MINGW: no termios, no pty */

os9err hostterm_open( int term_id, syspath_typ* spP )
{
    (void)spP;
    hostterm_init();
    (void)term_id;
    return os9error(E_UNIT);
} /* hostterm_open */

void hostterm_close( int term_id ) { (void)term_id; }

#endif
```

`E_DEVBSY` (250, `os9defs/os9errno.h:188`, "Device Busy") is already verified — use it as written.

- [ ] **Step 4: Call it from `pCopen` and `pCclose`**

In `consio.c` `pCopen`, replace Task 1's gate (the `hostterm_in_range && !hostterm_configured` block) with the full version, placed **after** `spP->term_id= id;` and `strcpy( spP->name,&name[1] );` so `spP` is populated before binding:

```c
    if (hostterm_in_range( id )) {
        os9err herr= hostterm_open( id, spP );
        if    (herr) return herr;
    }
```

`hostterm_open` already returns `E_UNIT` when unconfigured, so the Task 1 behaviour is preserved by this single call — do not keep both.

In `pCclose`, at the very top (before the existing `if (spP->type!=fTTY)` early return, which would otherwise skip it):

```c
    if (hostterm_in_range( spP->term_id )) hostterm_close( spP->term_id );
```

- [ ] **Step 5: Build and run the test to verify it passes**

```sh
make
swift run --package-path test OS9Tests hostterm
```

Expected: both hostterm tests PASS.

- [ ] **Step 6: Prove the refusal path still fails**

Confirm the Task 1 test is not now passing vacuously:

```sh
OS9T1=/dev/definitely-not-a-tty ./os9exec -r shell
```
Type `echo hi >/t1`, expect the `cannot open` message and an OS-9 error, then `logout`/ESC. This must be run through `tools/os9repl.sh`, not a raw pipe:

```sh
OS9REPL_SESSION=ht2 ./tools/os9repl.sh start
OS9REPL_SESSION=ht2 ./tools/os9repl.sh send dog
OS9REPL_SESSION=ht2 ./tools/os9repl.sh stop
```
(The env var needs to be set on the emulator; edit the `start` line's environment or export `OS9T1` before `start`.)

- [ ] **Step 7: Full suite, toolchain sweep, commit**

```sh
swift run --package-path test OS9Tests
make warnings
```
Expected: suite green, 0/0 with no `NOT BUILT`.

```sh
git add Source/OS9exec_core/hostterm.c Source/OS9exec_core/consio.c \
        test/Sources/OS9Tests/main.swift test/Package.swift
git commit -m "Core: /tN binds to a host terminal named by OS9Tn"
```

---

### Task 2a: Report the real reason an endpoint could not be opened

A correction to Task 2's shipped code, caused by an error in this plan. **Do this first, as its own commit, before starting Task 3.**

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_open`, the `open()` failure path)
- Test: `test/Sources/OS9Tests/main.swift` (the existing "refuses a host path that does not exist" test)

**The defect.** This plan told Task 2 to return a blanket `os9error(E_DEVBSY)` whenever `open()` fails, so a path that does not exist reports "Device Busy". That is precisely the mistake commit `572d4fb` ("UNIX host errors report their real cause, not the caller's guess") was written to stamp out, and `2ceec83` before it. The tree already has the fix: `host2os9err()`.

- [ ] **Step 1: Change the failing test's expectation**

The existing test asserts `Error #000:250 (E_DEVBSY)` for a nonexistent path. A nonexistent path is `ENOENT`, which `host2os9err` maps to `E_PNNF` (216). Change the expected string to `Error #000:216 (E_PNNF)`.

- [ ] **Step 2: Run it and watch it fail**

```sh
swift run --package-path test OS9Tests hostterm
```
Expected: FAIL, reporting 250 where 216 is now wanted. This confirms the test still discriminates.

- [ ] **Step 3: Use the real cause**

In `hostterm_open`, the `open()` failure path currently reads `return os9error(E_DEVBSY);`. Replace with:

```c
    fd= open( spec, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0) {
        uphe_printf( "OS9T%d: cannot open '%s'\n", term_id, spec );
        /* <fd> is -1: host2os9err's UNIX arm wants the POSIX RETURN CODE, not
           an errno -- it reads errno itself, and returns SUCCESS if handed 0.
           E_DEVBSY stays as the fallback for an errno it does not map. */
        return host2os9err( fd, E_DEVBSY );
    }
```

Leave the `tcsetattr`/not-a-terminal failure below it returning `os9error(E_DEVBSY)` unchanged — that one is not an `open()` failure and has no fresh `errno` to consult.

- [ ] **Step 4: Verify it passes, then check a second errno really differs**

```sh
swift run --package-path test OS9Tests hostterm
```
Then confirm the mapping is live rather than coincidental: point `OS9T1` at a path that exists but is not a terminal (`/etc/hosts`) via `tools/os9repl.sh`, and confirm the reported error is NOT 216 — proving the code reads `errno` rather than returning one constant. Report the raw output.

- [ ] **Step 5: Commit**

```sh
git add Source/OS9exec_core/hostterm.c test/Sources/OS9Tests/main.swift
git commit -m "Fix: /tN reports why an endpoint would not open, not a guess"
```

---

### Task 3: Output path — OS-9 writes reach the host endpoint

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_put`)
- Modify: `Source/OS9exec_core/consio.c` (`ConsPutc` ~line 290, `ConsoleOut` ~line 972)
- Test: `test/Sources/OS9Tests/main.swift`

**Interfaces:**
- Consumes: `hostterm_bound`, `hostterms[].fd` (Task 2).
- Produces: `int hostterm_put( int term_id, const char* buffer, int n )` returning bytes written or -1.

- [ ] **Step 1: Write the failing test**

```swift
if let (master, slaveName) = makePTY() {
    // Drain the master on a background queue while the emulator runs, so a
    // full pty buffer can never stall the guest.
    let seen = NSMutableData()
    let lock = NSLock()
    let source = DispatchSource.makeReadSource(fileDescriptor: master,
                                               queue: .global())
    source.setEventHandler {
        var buf = [UInt8](repeating: 0, count: 1024)
        let n = read(master, &buf, buf.count)
        if n > 0 { lock.lock(); seen.append(buf, length: n); lock.unlock() }
    }
    source.resume()

    _ = os9(["echo host-terminal-works >/t1"], env: ["OS9T1": slaveName])
    usleep(200_000)
    source.cancel()
    close(master)

    lock.lock()
    let text = String(data: seen as Data, encoding: .utf8) ?? ""
    lock.unlock()

    if text.contains("host-terminal-works") {
        print("PASS: hostterm: output reaches the host endpoint"); passed += 1
    } else {
        print("FAIL: hostterm: output reaches the host endpoint")
        print("      [pty master should have received the echoed text]")
        print("      got: \(text.debugDescription.prefix(200))")
        failed += 1
    }
}
```

- [ ] **Step 2: Run the test to verify it fails**

```sh
swift run --package-path test OS9Tests hostterm
```
Expected: FAIL — `hostterm_put` is still the stub, and `ConsPutc` still writes to fd 1, so the pty master receives nothing.

- [ ] **Step 3: Implement `hostterm_put`**

> **The bounded-retry design below was SUPERSEDED by `5e45bb8`.** It shipped,
> then proved to be only half a fix: `ConsPutc` ignores this function's return
> value, so once the retries were exhausted the byte was silently dropped
> anyway — measured at a stable 15 bytes lost out of 3226. `hostterm_put` is now
> purely non-blocking (single `write`, 0 on `EAGAIN`, no `usleep`), and
> `ConsoleOut` parks the writing process in `pWaitWrite` exactly as its paced
> branch does. A 100ms sleep inside a cooperative scheduler was always wrong.
> See `2026-07-29-hostterm-backpressure-and-wildcard.md`, Task 1.

Add inside the `#if defined UNIX && !defined MINGW` block in `hostterm.c`:

```c
int hostterm_put( int term_id, const char* buffer, int n )
{
    hostterm_typ* h;
    int           done= 0;
    int           tries;

    hostterm_init();
    if (!hostterm_bound( term_id )) return -1;
    h= &hostterms[ term_id ];

    while (done<n) {
        ssize_t w= write( h->fd, buffer+done, (size_t)(n-done) );

        if (w>0) { done+= (int)w; tries= 0; continue; }

        if (w<0 && (errno==EAGAIN || errno==EWOULDBLOCK)) {
            /* The far end is not draining. The fd is deliberately
               O_NONBLOCK -- blocking here would stall every OS-9 process,
               because the scheduler is cooperative -- so retry briefly and
               then report a write error rather than DROP the bytes.
               Silently dropping is the specific bug that shipped once
               already: a best-effort auto-LF push was discarded whenever the
               output FIFO was full, leaving bare-CR lines (fixed in 75a8ea8).
               Do not "improve" this into a discard.
               A proper fix parks the process in pWaitWrite exactly as the
               baud FIFO does (consio.c). Deferred: it needs the saved_cnt /
               saved_state resume dance, which is its own change. */
            if (++tries>10) break;
            usleep( 10000 ); /* 10ms x 10 = 100ms ceiling */
            continue;
        }

        if (w<0 && errno==EINTR) continue;
        return -1; /* genuine error: EIO on a hung-up pty, etc. */
    }

    return done<n ? -1 : done;
} /* hostterm_put */
```

Add `#include <errno.h>` to the POSIX include block if it is not already reachable via `os9exec_incl.h` (check before adding — a duplicate include is harmless but the tree may already have it).

Initialise `tries= 0` at declaration, not only in the success branch, or the first EAGAIN reads an uninitialised value. Write it as `int tries= 0;`.

- [ ] **Step 4: Branch `ConsPutc` and `ConsoleOut`**

In `consio.c` `ConsPutc`, after the existing `TTY_Base` block and **before** the bare `write( 1,&c,1 );`:

```c
      if (hostterm_bound( gConsoleID )) {
          hostterm_put( gConsoleID, &c,1 );
          return;
      }
```

In `ConsoleOut`, the existing structure is `if (gConsoleID>=TTY_Base) {...} else { ...console... }`. The console arm's per-character loop already calls `ConsPutc`, which now routes correctly — so **no change is needed there**. Verify this by reading the loop rather than assuming: confirm every output byte in the non-paced path goes through `ConsPutc`/`ConsPutcEdit`.

Baud pacing: `baud_dev_for()` is keyed by `term_id`, so a host terminal gets its own FIFO for free and the paced path also ends at `ConsPutc` via the FIFO consumer. Confirm the consumer calls `ConsPutc` with `gConsoleID` set to the right device; if it uses a stale global, note it in your report and **do not fix it here** — that is a separate defect.

- [ ] **Step 5: Run the test to verify it passes**

```sh
make
swift run --package-path test OS9Tests hostterm
```
Expected: all three hostterm tests PASS.

- [ ] **Step 6: Prove 8-bit transparency, and prove the check can fail**

Add a second assertion in the same block, sending bytes a cooked tty would mangle:

```swift
    // 0x11/0x13 are XON/XOFF and 0x7f is DEL -- a cooked tty eats or rewrites
    // all three. This is the assertion that would catch a missing cfmakeraw.
    _ = os9(["echo \u{11}\u{13}\u{7f} >/t1"], env: ["OS9T1": slaveName])
```
and assert the three bytes appear in `seen`. Then **temporarily** comment out the `cfmakeraw( &t );` line in `hostterm_raw`, rebuild, and confirm this test FAILS. Restore the line, rebuild, confirm it passes. Report both results — a check that has never failed is not a check.

- [ ] **Step 7: Full suite, toolchain sweep, commit**

```sh
swift run --package-path test OS9Tests
make warnings
git add Source/OS9exec_core/hostterm.c Source/OS9exec_core/consio.c \
        test/Sources/OS9Tests/main.swift
git commit -m "Core: /tN output goes to its own host endpoint"
```

---

### Task 3a: Paced output must go to the device that queued it

> **DROPPED, then SUPERSEDED — do not implement this task.** Its test passed
> against unfixed code (a single writer to a single device never contends
> `gConsoleID` under a cooperative scheduler), so the fix was reverted rather
> than landed behind a check that could not fail. The defect was real, and was
> fixed properly in **`045da4b`** by a different route: `ConsPutcTo( term_id, c )`
> gives console output an explicit destination, so `baud_drain_due` names its
> device instead of inheriting an ambient global — the mistake becomes
> structurally impossible at that site rather than corrected by assignment.
> A working repro was then found (two paced devices, two CONCURRENT writers)
> and the symptom was worse than described here: byte-level interleaving of the
> two streams, not whole-backlog misattribution. See
> `2026-07-29-conspuctto-explicit-destination.md`. The save/restore approach
> below is kept only as the record of what was tried.

Found by Task 3's implementer while reading `ConsoleOut`, and confirmed by the controller. **Do this before Task 4.**

**Files:**
- Modify: `Source/OS9exec_core/consio.c` (`baud_drain_due`, ~lines 866-896)
- Test: `test/Sources/OS9Tests/main.swift`

**The defect.** `baud_drain_due()` walks `baud_devices[]` and calls `ConsPutc(c)` for each queued byte. But `ConsPutc` chooses its destination from the **global** `gConsoleID`, and `baud_drain_due` never sets it — so every device's backlog drains to whatever `gConsoleID` was last left holding by some unrelated `ConsoleOut` call. Both loops in the function have this bug.

This was harmless while `/term` was the only destination: `gConsoleID` was always 0 and always right. Binding `/tN` to real endpoints makes it reachable, and it defeats the entire point of the feature — with the main console and a host terminal both paced, device B's bytes surface on device A's terminal. Baud pacing is **on by default**, so this is the default configuration, not an edge case. The suite runs `-r` (unpaced), which is exactly why no existing test catches it.

- [ ] **Step 1: Write the failing test**

Reuse `makePTY()` from Task 3, and note the deliberate absence of `-r`: this bug only exists on the paced path, so the test must run paced or it cannot fail.

```swift
if let (master, slaveName, spare) = makePTY() {
    let seen = drainNonBlocking( master )

    // Two consoles active at once, both paced. The listing is redirected to
    // /t1; the shell's own prompts and echo go to /term. If baud_drain_due
    // ignores which device queued a byte, the two interleave onto whichever
    // console was last touched.
    //
    // "DEFS" is on the system disk and appears in NO command text here, so the
    // shell's command echo cannot satisfy this assertion (see Global Constraints).
    _ = os9(["dir /dd >/t1"], env: ["OS9T1": slaveName], paced: true, timeout: 60)

    let got = seen()
    close( master ); close( spare )

    if got.contains("DEFS") {
        print("PASS: hostterm: paced output reaches the device that queued it")
        passed += 1
    } else {
        print("FAIL: hostterm: paced output reaches the device that queued it")
        print("      [the listing was queued for /t1 but never arrived there]")
        failed += 1
    }
}
```

Adapt the `makePTY()`/`drainNonBlocking()` call shapes to whatever Task 3 actually landed — read them first, do not assume the tuple above matches.

- [ ] **Step 2: Run it and watch it fail**

```sh
swift run --package-path test OS9Tests hostterm
```
Expected: FAIL. The bytes are queued on `/t1`'s FIFO but drained to whatever `gConsoleID` held, which is the main console.

If it PASSES before the fix, stop and report — that means the routing is already correct by some path I have not accounted for, and this task should be dropped rather than "fixed".

- [ ] **Step 3: Route each byte to its own device**

In both loops of `baud_drain_due`, set the destination before draining, and restore it afterwards so the function has no side effect on a global other code reads:

```c
void baud_drain_due( void )
{
    int   i;
    byte  c;
    ulong now;
    int   saved_id= gConsoleID; /* leave the global as we found it */

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0) continue;

        if (d->us_per_char==0) {
            /* ConsPutc picks its destination from gConsoleID, so it must name
               the device whose FIFO we are draining. Without this every
               device's backlog went wherever the last ConsoleOut happened to
               leave the global -- invisible while /term was the only
               destination, wrong the moment a second terminal exists. */
            gConsoleID= d->term_id;
            while (fifo_pop(d,&c)) ConsPutc(c);
        }
    }

    if (g_next_wake_us==0) { gConsoleID= saved_id; return; }
    now= host_micros();
    if (now<g_next_wake_us) { gConsoleID= saved_id; return; }

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0 || d->us_per_char==0) continue;

        gConsoleID= d->term_id; /* same reason as above */
        while (d->count>0 && d->next_due_us<=now) {
            fifo_pop( d,&c );
            ConsPutc( c );
            d->next_due_us += d->us_per_char;
        }
    }

    recompute_next_wake();
    gConsoleID= saved_id;
} /* baud_drain_due */
```

Note both early returns restore the global too — an early return that skips the restore is the classic way this kind of fix goes wrong.

- [ ] **Step 4: Verify it passes, and confirm the main console still works**

```sh
swift run --package-path test OS9Tests
```
The whole suite matters here, not just the `hostterm` subset: this function drains the **main** console's paced output as well, so a mistake shows up as garbled or missing output anywhere. In particular the existing `baud pacing: paced dump matches unpaced dump byte-for-byte` check must still pass — it is the direct regression guard for this function.

- [ ] **Step 5: Commit**

```sh
git add Source/OS9exec_core/consio.c test/Sources/OS9Tests/main.swift
git commit -m "Fix: paced output goes to the device that queued it"
```

---

### Task 4: Input path — the host endpoint reaches OS-9 reads

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_poll`, `hostterm_get`, `hostterm_ready`)
- Modify: `Source/OS9exec_core/consio.c` (`ConsGetc` ~line 320, `pCready` ~line 1131)
- Modify: `Source/OS9exec_core/telnetaccess.c` (`CheckInputBuffers` ~line 397)
- Test: `test/Sources/OS9Tests/main.swift`

**Interfaces:**
- Consumes: `hostterm_bound`, `hostterms[].fd`, `hostterms[].dev` (Task 2).
- Produces: `void hostterm_poll( void )`, `int hostterm_get( int, char* )`, `Boolean hostterm_ready( int, long* )`.

- [ ] **Step 1: Write the failing test**

```swift
if let (master, slaveName) = makePTY() {
    // Feed a whole COMMAND LINE into the pty before the emulator reads, so the
    // bytes are already waiting -- no race with the guest's scheduler.
    //
    // `shell </t1` makes an OS-9 shell take its input from the host endpoint.
    // Whatever we write is read, echoed and executed by that shell, and its
    // output goes to the console, which the harness captures. So the assertion
    // string is one only a byte that travelled host -> pty -> OS-9 can produce:
    // it appears NOWHERE in the command text we send on stdin, which is the
    // trap that made the original Task 2 test vacuous.
    //
    // Trailing ESC is deliberate and load-bearing. 0x1B is PD_EOF's default in
    // the console option table (`init_consoleopts`, utilstuff.c), so it makes
    // the inner shell's read return E_EOF and the shell exit cleanly. Without
    // it that shell reads a pty that never reaches EOF and the test can only
    // end by timing out.
    let line = "echo GOT-IT-FROM-HOST\r\u{1B}\r"
    _ = line.withCString { write(master, $0, strlen($0)) }

    let out = os9(["shell </t1"], env: ["OS9T1": slaveName], timeout: 30)
    close(master)

    if out.contains("GOT-IT-FROM-HOST") {
        print("PASS: hostterm: input arrives from the host endpoint"); passed += 1
    } else {
        print("FAIL: hostterm: input arrives from the host endpoint")
        print("      [a command written into the pty should have run inside OS-9]")
        print("      got: \(out.suffix(200).debugDescription)")
        failed += 1
    }
}
```

`shell`, `tee`, `dump` and `echo` are all confirmed present in `h0/CMDS`. This test deliberately does not use `tee`: `tee </t1` reads until EOF and a pty does not deliver one on its own, so it could only ever end by timing out.

Bonus assertion this gives you for free: the CR written by the host stays a CR. `ConsGetc`'s host-terminal branch returns before the LF<->CR swap, so if that early return were ever removed the line would arrive LF-terminated, the shell would never see a completed record, and this test would fail.

- [ ] **Step 2: Run the test to verify it fails**

Expected: FAIL. `hostterm_get` is the stub, `ConsGetc` still reads the main console's `main_mco`, so nothing written to the pty is visible.

- [ ] **Step 3: Implement poll / get / ready**

Add inside the `#if defined UNIX && !defined MINGW` block:

```c
void hostterm_poll( void )
{
    int id;

    hostterm_init();

    for (id=HOSTTERM_MIN; id<=HOSTTERM_MAX; id++) {
        hostterm_typ* h= &hostterms[ id ];
        int           room;

        if (!h->open) continue;

        /* Never read more than inBuf has guaranteed space for: KeyToBuffer
           silently drops once it is full, and a byte already taken off the fd
           cannot be pushed back. Same conservative floor HandleEvent() uses
           for stdin. Special chars do not consume inBuf space at all, so the
           worst case is deferring a few plain bytes to the next poll. */
        room= INBUFSIZE-1 - h->dev.inBufUsed;

        while (room-->0) {
            char c;
            if (read( h->fd,&c,1 )!=1) break; /* EAGAIN: nothing more today */
            KeyToBuffer( &h->dev, c );
        }
    }
} /* hostterm_poll */

int hostterm_get( int term_id, char* c )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_bound( term_id )) return 0;
    h= &hostterms[ term_id ];

    hostterm_poll();
    if (h->dev.inBufUsed==0) return 0;

    *c= h->dev.inBuf[ 0 ];
    h->dev.inBufUsed--;
    if (h->dev.inBufUsed>0)
        MoveBlk( h->dev.inBuf, h->dev.inBuf+1, h->dev.inBufUsed );

    return 1;
} /* hostterm_get */

Boolean hostterm_ready( int term_id, long* cnt )
{
    hostterm_typ* h;

    hostterm_init();
    *cnt= 0;
    if (!hostterm_bound( term_id )) return false;
    h= &hostterms[ term_id ];

    hostterm_poll();
    if (h->dev.inBufUsed==0) return false;

    *cnt= h->dev.inBufUsed;
    return true;
} /* hostterm_ready */
```

The non-UNIX arm needs matching no-op definitions so the mingw leg links:

```c
void    hostterm_poll ( void ) { }
int     hostterm_get  ( int term_id, char* c ) { (void)term_id; (void)c; return 0; }
Boolean hostterm_ready( int term_id, long* cnt ) { (void)term_id; *cnt= 0; return false; }
```

- [ ] **Step 4: Branch `ConsGetc` and `pCready`, and add the poll hook**

In `consio.c` `ConsGetc`, after the existing `TTY_Base` block and **before** the `#if defined windows32 || defined MINGW` chain:

```c
    if (hostterm_bound( gConsoleID )) {
        /* Returns before the LF<->CR swap below on purpose. That swap exists
           for a genuine Unix terminal in cooked mode; a host endpoint is raw
           and 8-bit transparent, and rewriting CR would corrupt every binary
           transfer -- which is the entire point of this device. The existing
           PTY branch above returns early for the same reason. */
        int n= hostterm_get( gConsoleID, c );
        devIsReady= n>0;
        return devIsReady;
    }
```

In `pCready`, the current shape is `if (gConsoleID>=TTY_Base) {...} else { ...DevReady... }`. Insert a host-terminal arm between them:

```c
    if (gConsoleID>=TTY_Base) {
        ...unchanged...
    }
    else if (hostterm_bound( gConsoleID )) {
        if (hostterm_ready( gConsoleID, &cnt )) {
            *n = (uint32_t)cnt;
            return 0;
        }
    }
    else {
        ...unchanged...
    }
```

In `telnetaccess.c` `CheckInputBuffers()`, add one line alongside the existing sweeps:

```c
    CheckInputBuffersTTY();      /*   "        "       "  ttys         */
    hostterm_poll();             /* and every host-backed /tN          */
    HandleEvent();               /* and check also the Mac events      */
```

- [ ] **Step 5: Run the test to verify it passes**

```sh
make
swift run --package-path test OS9Tests hostterm
```
Expected: all hostterm tests PASS.

- [ ] **Step 6: Verify `tsmon /t1` now works**

This is the payoff check and it must be done live through the REPL. In one terminal, create a pty and attach:

```sh
OS9REPL_SESSION=ht4 ./tools/os9repl.sh start
```
with `OS9T1` exported to a pty slave you created (a three-line Python `pty.openpty()` script in the scratchpad is fine). Then from the OS-9 side run `tsmon /t1 &`, attach `screen <slave>` in another window, and confirm you get a `User name?:` prompt on it and can log in.

Report the raw captured output, not a summary. If it does not work, **stop and report** — do not start fixing; the cause may be outside this task's scope.

- [ ] **Step 7: Full suite, toolchain sweep, commit**

```sh
swift run --package-path test OS9Tests
make warnings
git add Source/OS9exec_core/hostterm.c Source/OS9exec_core/consio.c \
        Source/OS9exec_core/telnetaccess.c test/Sources/OS9Tests/main.swift
git commit -m "Core: /tN reads its own host endpoint, so tsmon works on it"
```

---

### Task 5: `OS9Tn=pty` — self-allocated pty with a reported slave name

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_open`)
- Test: `test/Sources/OS9Tests/main.swift`

**Interfaces:**
- Consumes: everything from Tasks 2-4.
- Produces: no new symbols — `hostterm_open` gains a second endpoint spelling.

This is the convenience layer for the interactive case: you cannot name a pty that does not exist yet, so the emulator allocates one and tells you what to attach to.

- [ ] **Step 1: Write the failing test**

Two assertions. The first checks the name is reported at all; the second checks the device actually carries output away from the console.

```swift
// 1. The slave name must be reported, or there is no way to attach.
//    NOTE: do NOT "strengthen" this by stat()ing the reported path. On macOS
//    /dev/ttysNNN are STATIC device nodes that exist whether or not anything
//    allocated them, so a stat() check passes unconditionally -- vacuous.
//    Liveness is proved by assertion 2 and by the interactive step below.
let ptyOut = os9(["dir /dd >/t1"], env: ["OS9T1": "pty"])

if ptyOut.range(of: "/dev/[a-z]*tty[a-zA-Z0-9/]+", options: .regularExpression) != nil {
    print("PASS: hostterm: OS9T1=pty reports a slave device to attach to")
    passed += 1
} else {
    print("FAIL: hostterm: OS9T1=pty reports a slave device to attach to")
    print("      [no /dev/...tty... name found in emulator output]")
    failed += 1
}

// 2. `dir /dd` output must go to the pty, NOT to the console.
//    "DEFS" is a directory on the system disk, so it appears in the listing --
//    and it appears in NO command this harness sends (the setup lines are
//    `chx /dd/CMDS` and `load math cio`). That matters: the OS-9 shell echoes
//    every command it runs, so asserting on a string that occurs in the
//    command text is satisfied by the echo alone and can never fail.
//    Before Task 5 this fails honestly: OS9T1=pty is refused, the redirection
//    never takes effect, and the listing lands on the console.
if !ptyOut.contains("DEFS") {
    print("PASS: hostterm: OS9T1=pty carries output off the console")
    passed += 1
} else {
    print("FAIL: hostterm: OS9T1=pty carries output off the console")
    print("      [listing reached the console, so /t1 was not the pty]")
    failed += 1
}
```

Confirm `DEFS` really is in `dir /dd` output on the disk you are testing against before trusting assertion 2 — if the disk differs, pick another entry that appears in the listing and in no command text, and say which you used.

- [ ] **Step 2: Run the test to verify it fails**

Expected: FAIL — `pty` currently hits the "unsupported endpoint" refusal from Task 2.

- [ ] **Step 3: Implement the `pty` spelling**

In `hostterm_open`, replace the `if (*spec!=PATHDELIM) {...refuse...}` block with:

```c
    if (ustrcmp( spec,"pty" )==0) {
        char* slave;

        fd= posix_openpt( O_RDWR | O_NOCTTY );
        if (fd<0 || grantpt( fd )!=0 || unlockpt( fd )!=0) {
            uphe_printf( "OS9T%d: cannot allocate a pty\n", term_id );
            if (fd>=0) close( fd );
            return os9error(E_DEVBSY);
        }

        slave= ptsname( fd );
        if (slave==NULL) {
            uphe_printf( "OS9T%d: pty has no slave name\n", term_id );
            close( fd );
            return os9error(E_DEVBSY);
        }

        /* The name is the whole point -- without it there is no way to
           attach. stderr via uphe_printf, so it survives stdout redirection
           and matches the emulator's other "# ..." startup messages. */
        uphe_printf( "/t%d is %s   (attach with: screen %s)\n",
                     term_id, slave, slave );

        /* We hold the MASTER. O_NONBLOCK is set here rather than at
           posix_openpt because the grant/unlock dance wants the plain fd. */
        fcntl( fd, F_SETFL, fcntl( fd,F_GETFL,0 ) | O_NONBLOCK );
        strncpy( h->endpoint,slave, OS9PATHLEN-1 );
                 h->endpoint[       OS9PATHLEN-1 ]= NUL;
    }
    else if (*spec==PATHDELIM) {
        fd= open( spec, O_RDWR | O_NOCTTY | O_NONBLOCK );
        if (fd<0) {
            uphe_printf( "OS9T%d: cannot open '%s'\n", term_id, spec );
            return os9error(E_DEVBSY);
        }
        strncpy( h->endpoint,spec, OS9PATHLEN-1 );
                 h->endpoint[      OS9PATHLEN-1 ]= NUL;
    }
    else {
        uphe_printf( "OS9T%d: unsupported endpoint '%s'\n", term_id, spec );
        return os9error(E_UNIT);
    }
```

The `hostterm_raw( fd )` call and the `h->fd`/`h->open`/`h->dev` assignments that followed the old `open()` now run after this `if`/`else` chain for both spellings — restructure so they are not duplicated. Remove the now-dead `strncpy` that followed the old single `open()`.

Add `<stdlib.h>` (for `posix_openpt`, `grantpt`, `unlockpt`, `ptsname`) and `<fcntl.h>` to the POSIX include block if not already present.

- [ ] **Step 4: Run the test to verify it passes**

```sh
make
swift run --package-path test OS9Tests hostterm
```
Expected: all hostterm tests PASS.

- [ ] **Step 5: Verify interactively**

```sh
OS9T1=pty OS9DISK=$PWD/h0 ./os9exec -r shell
```
Note the reported `/dev/ttysNNN`, attach `screen /dev/ttysNNN` from another terminal, and from the OS-9 shell run `tsmon /t1 &`. Confirm the login prompt appears in the `screen` window and you can log in and run `dir`. Paste raw captured output in your report.

- [ ] **Step 6: Full suite, toolchain sweep, commit**

```sh
swift run --package-path test OS9Tests
make warnings
git add Source/OS9exec_core/hostterm.c test/Sources/OS9Tests/main.swift
git commit -m "Core: OS9Tn=pty allocates a terminal and reports its device"
```

---

### Task 6: kermit end-to-end between two emulator instances

**Files:**
- Create: `test/68k-live-verification/hostterm-kermit-report-2026-07-29.md`
- Modify: none (verification only, unless a defect is found)

**Interfaces:**
- Consumes: everything. No new symbols.

This is the acceptance test for the whole feature. It is **live verification**, so it must not be delegated to a weak model and must not be reported as done without raw captured output.

- [ ] **Step 1: Build the two-instance rig**

Now that Task 5 has landed, the rig is two emulator instances and no external plumbing at all:

- Run **instance A** with `OS9T1=pty`. It allocates the pty, holds the master, and prints the slave device name `S`.
- Run **instance B** with `OS9T1=S`. It opens that slave by name.
- Bytes written by A's `/t1` surface on B's `/t1`, and vice versa.

Both ends are already in raw 8-bit mode, so nothing in the host path rewrites a byte.

Note A also holds a spare read reference on the slave (Task 5) which it never reads from — that does not steal data from B; an unread fd does not consume the queue.

Drive the two instances however you can do so reliably and reproducibly. Two `tools/os9repl.sh` sessions with distinct `OS9REPL_SESSION` names (e.g. `kermA`, `kermB`) is the intended route; a scripted Python driver playing both terminals is equally acceptable and has proved more reliable than tmux for this work — earlier tasks hit repeated tmux/env friction. **Say which you used.** What matters is that the bytes really cross between two independent emulator processes.

- [ ] **Step 2: Transfer a text file A -> B**

On B: `kermit r /t1`. On A: `kermit s /t1 /dd/echo` (or any small file that exists — check with `dir /dd` first).

Confirm on B that the file arrived and compare it byte-for-byte against the original using `dump` on both sides. **Do not compare by eye or by size alone** — the failure mode this whole feature guards against is a handful of rewritten bytes, which a size check cannot see.

- [ ] **Step 3: Transfer a binary file B -> A**

Repeat in the other direction with a file containing bytes above 0x7F and control characters — an OS-9 module from `/dd/CMDS` is ideal, since a single flipped byte breaks its CRC and `ident` will say so. Run `ident` on the received copy: a valid CRC is a strong, self-checking assertion.

- [ ] **Step 4: Record the result**

Write `test/68k-live-verification/hostterm-kermit-report-2026-07-29.md` containing: the exact commands run on both sides, raw captured terminal output, the `dump`/`ident` comparison results, and an explicit PASS or FAIL line. Follow the existing reports in that directory for format.

If anything fails, **stop and report with the raw output**. Do not fix defects inside this task — a kermit-level failure could be in any of Tasks 2-5 and needs the controller to decide scope.

- [ ] **Step 5: Commit**

```sh
git add test/68k-live-verification/hostterm-kermit-report-2026-07-29.md
git commit -m "Tests: kermit transfers between two instances over /t1"
```

---

### Task 6a: A bound terminal must outlive the path that opened it

Found by the controller while reviewing Task 6's FAIL. **Do this before re-running Task 6.**

**Files:**
- Modify: `Source/OS9exec_core/hostterm.c` (`hostterm_open`, `hostterm_close`, `hostterm_poll`, the `hostterm_typ` struct)
- Test: `test/Sources/OS9Tests/main.swift`

**The defect.** `hostterm_open` says *"already open: share it"* and returns without counting the extra holder. `hostterm_close` then tears the whole device down — `close(fd)`, `close(spareFd)`, `open=false` — on **any** path close. Two consequences, both real:

1. When two OS-9 paths are open on `/t1` (ordinary: a process with stdin *and* stdout on it, or `tsmon` plus the `login` it spawns), the first close destroys the device under the second holder.
2. With `OS9Tn=pty`, tearing down and reopening allocates a **different** pty. The name printed at startup — the one a user ran `screen` against — goes dead as soon as the opening path closes. That makes the headline workflow unusable across more than one command.

This is very likely why Task 6's kermit run lost its final handshake ACK whenever the pty-master-holding instance was the receiver.

**The decision: a binding persists for the emulator's lifetime once established.** That matches how a real OS-9 device behaves — a device descriptor is not destroyed because one path to it closed — and it keeps a self-allocated pty's name stable, which is the whole point of printing it.

- [ ] **Step 1: Write the failing test**

Count how many times a pty is **allocated**, not how many distinct names appear.

**Do not assert on distinct name strings — that was tried and is vacuous on macOS.** `posix_openpt`/`ptsname` deterministically hand back the same lowest-free slot, so a torn-down-and-reallocated pty reappears under the *identical* name (`/dev/ttys002`, even the same fd). Proven live: the fresh-allocation debug line fires twice across two commands while the name never changes.

`hostterm_open` prints its `attach with: screen` line only on the fresh-allocation path, never on the "already open" shortcut — so counting that line counts allocations directly.

```swift
// Two commands, each opening and closing /t1. With OS9T1=pty the emulator
// announces the slave name once per ALLOCATION. If the binding is torn down
// when the first command's path closes, the second command allocates again and
// the announcement appears TWICE -- and any `screen` attached to the first is
// already dead, even though the recycled pty happens to carry the same name.
let twice = os9(["echo one >/t1", "echo two >/t1"], env: ["OS9T1": "pty"])
let allocations = twice.components(separatedBy: "attach with: screen").count - 1

if allocations == 1 {
    print("PASS: hostterm: a pty binding survives the path that opened it")
    passed += 1
} else {
    print("FAIL: hostterm: a pty binding survives the path that opened it")
    print("      [expected exactly 1 pty allocation across two commands, saw \(allocations)]")
    failed += 1
}
```

If you change `hostterm_open`'s announcement wording, change this assertion with it.

- [ ] **Step 2: Run it and watch it fail**

```sh
swift run --package-path test OS9Tests hostterm
```
Expected: FAIL reporting 2 distinct names. If it reports 1 already, STOP AND REPORT — the teardown is not happening where I think it is, and this task needs rethinking rather than implementing.

- [ ] **Step 3: Count holders, and stop closing the endpoint**

Add to `hostterm_typ`:

```c
    int  openCount;  /* live OS-9 paths on this device; the endpoint itself
                        is never closed once bound -- see hostterm_close */
```

Initialise it to `0` alongside the other fields in `hostterm_init`.

In `hostterm_open`, replace the share-and-return line:

```c
    h= &hostterms[ term_id ];
    if (h->open) { h->openCount++; h->dev.spP= spP; return 0; } /* another holder */
```

and set `h->openCount= 1;` in the success tail, next to `h->open= true;`.

Rewrite `hostterm_close` so it releases the *path*, never the endpoint:

```c
void hostterm_close( int term_id )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_in_range( term_id )) return;

    h= &hostterms[ term_id ];
    if (!h->open) return;

    if (h->openCount>0) h->openCount--;
    if (h->openCount>0) return;          /* another path still holds it */

    /* Deliberately does NOT close h->fd or h->spareFd. A bound terminal is a
       DEVICE, and a device does not cease to exist because the last path to it
       closed -- real OS-9 keeps its descriptor. Two concrete reasons here:
       a self-allocated pty would otherwise hand out a DIFFERENT name on the
       next open, killing whatever `screen` was attached to the old one; and
       closing the master discards bytes the peer has not read yet (the mirror
       of the slave-side discard already documented in this file's plan).
       The fd is released when the emulator exits, which is when the device
       genuinely goes away. */
    h->dev.spP= NULL; /* the syspath is going away; do not keep a stale pointer */
} /* hostterm_close */
```

- [ ] **Step 4: Stop polling a device with no path open**

`hostterm_poll` calls `KeyToBuffer( &h->dev, c )`, which dereferences `h->dev.spP` for the path options. That is now NULL between opens, so skip those devices:

```c
        if (!h->open || h->dev.spP==NULL) continue;
```

Make the same guard in `hostterm_get` and `hostterm_ready` if they can be reached with no path open — read them and decide; say in your report what you found. Missing this is a NULL dereference, so check it rather than assuming.

- [ ] **Step 5: Verify, including that the endpoint still works after a reopen**

```sh
swift run --package-path test OS9Tests
```
All hostterm tests must pass, not just the new one — several of them open and close `/t1` repeatedly, so they are the regression guard for this change.

- [ ] **Step 6: Commit**

```sh
git add Source/OS9exec_core/hostterm.c test/Sources/OS9Tests/main.swift
git commit -m "Fix: a bound /tN outlives the path that opened it"
```

- [ ] **Step 7: Re-run Task 6's acceptance test**

Re-run the kermit rig from Task 6 (its report file records the exact commands, including the `l` and `i` flags it discovered are required). Determine whether the handshake now completes in **both** directions.

Then **update** `test/68k-live-verification/hostterm-kermit-report-2026-07-29.md` with the new result and commit it:

```sh
git add test/68k-live-verification/hostterm-kermit-report-2026-07-29.md
git commit -m "Tests: record kermit result after the binding-lifetime fix"
```

If it still fails, say so plainly and leave the FAIL recorded — do not keep patching toward a pass. Report what changed and what did not.

---

### Task 7: Documentation and the final gate

**Files:**
- Modify: `README.md` (device table ~line 108, plus one paragraph)
- Modify: `ROADMAP-68k.md` — **NO. This file is gitignored. Do not touch it.**

**Interfaces:**
- Consumes: everything. No new symbols.

- [ ] **Step 1: Add the device-table row**

In `README.md`, the table around line 108 lists `/dd` and `/h0`-`/hz`. Add:

```markdown
| `/t1`–`/t49` | `OS9T1=…` through `OS9T49=…` | Terminals. `pty` allocates one and prints the device to attach to; a `/dev/…` path opens that terminal or serial port. Unset means the device does not exist. |
```

- [ ] **Step 2: Add the explanatory paragraph**

Place it near the existing device-resolution discussion (around line 173), and state plainly that this is **not** the `/hX` cascade:

```markdown
**Terminals resolve directly, with no search.** Unlike `/hX` — which falls back
through `OS9Hx`, then a file next to the binary, then one directory up — a
terminal is bound only by its `OS9Tn` variable. Set means bound; unset means the
device does not exist and opening it returns `E_UNIT`. `OS9T1=pty` makes
`os9exec` allocate a terminal and print the device to attach to (`screen
/dev/ttys004`); `OS9T1=/dev/cu.usbserial-1420` opens a real serial port. With
`tsmon /t1` running inside OS-9, that terminal gets its own login prompt — one
emulator, several independent sessions.

A binding lasts for the life of the emulator, not the life of the path that
opened it — so the device you attached to stays the same device between
commands, and the terminal is 8-bit transparent, which is what lets `kermit`
move a binary across it intact.
```

Verify the claim about `E_UNIT` against the code before writing it, and check the surrounding README voice so this reads like the rest of the file.

- [ ] **Step 3: The final gate — all four legs**

```sh
make
swift run --package-path test OS9Tests
make test-notick
make warnings
```

Expected: build warning-clean; suite green both with and without the system tick; `make warnings` 0/0 on every leg with **no `NOT BUILT`**.

Report the actual numeric results. A summary like "all green" without the counts is not acceptable.

- [ ] **Step 4: Commit**

```sh
git add README.md
git commit -m "Docs: document OS9Tn host-backed terminal devices"
```

---

## Self-Review

**Spec coverage.** Every design decision settled in conversation maps to a task: `OS9Tn` env var (T1), `E_UNIT` when unset (T1), no `/hx` cascade (T1, documented T7), the four `consio.c` seams (T2 open/close, T3 output, T4 input+ready), the `CheckInputBuffers` hook (T4), the `#ifndef linux` bug (T1), pty allocation (T5), the two-instance kermit rig proved on this machine (T6), README (T7). TCP, baud-from-`PD_BAU`, Windows and `pWaitWrite` parking are listed as explicitly out of scope with reasons.

**Placeholder scan.** No TBDs. Three places deliberately instruct *verify before writing* rather than asserting a value — the `E_UNIT`/`E_DEVBSY` numeric codes, the `os9exec_incl.h` include position, and whether `tee` exists on the disk. Each says what to do if the guess is wrong, which is a real instruction, not a placeholder.

**Type consistency.** `hostterm_*` names are identical in `hostterm.h` (T1) and every call site (T2-T5). `hostterm_put` returns `int` throughout. `hostterm_ready` takes `long*` to match `pCready`'s existing local `cnt`. `hostterm_get` returns `int` (1/0), matching how `ConsGetc` consumes it. The non-UNIX stubs in T4 have the same signatures as the UNIX versions.

**Known weak point, stated rather than hidden.** Task 2's test asserts only that the emulator runs a command with `OS9T1` set — it cannot assert byte flow, because nothing writes to the endpoint until Task 3. It genuinely fails before Task 2's change and passes after, so it is a real gate, just a shallow one. Tasks 3 and 4 carry the substantive assertions.

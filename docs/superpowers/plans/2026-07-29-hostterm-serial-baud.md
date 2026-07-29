# Host Terminal Serial Speed (`PD_BAU` -> termios) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:subagent-driven-development or superpowers:executing-plans.
>
> **REQUIRED DOMAIN SKILL:** Invoke **`os9-systems-dev`** before reading any code (os9exec's kernel-interface layer, SCF path options) and **`os9-dev`** (`tmode`, `PD_BAU`, SCF option semantics). We own both and are dogfooding them: **report anything wrong, missing, or misleading** in your final report.

**Goal:** Drive a bound `/tN`'s host serial speed from the path's OS-9 `PD_BAU` option, so a real USB-serial adapter runs at the rate OS-9 thinks it is running at.

**Architecture:** One new function in `hostterm.c` maps bits-per-second to a `speed_t` and applies it with `cfsetispeed`/`cfsetospeed`. `consio.c` calls it from the two places the path's baud becomes known or changes: after `pSCFopt` in `pCopen`, and after the option propagation in `pCsetopt`.

**Tech Stack:** C, POSIX termios, Swift for the integration suite.

---

## Context: this is a small delta, not a new subsystem

The host-terminal feature shipped on 2026-07-29 (`docs/superpowers/plans/2026-07-29-hostterm-pty.md`). **A real serial port already opens and works** — `OS9T1=/dev/cu.usbserial-1420` takes the identical code path as a pty slave name, there is no pty-vs-device branch anywhere in `hostterm.c`, and `hostterm_raw()` already sets `CLOCAL|CREAD` and opens `O_NONBLOCK` specifically so real hardware behaves.

The single missing piece is speed. `hostterm_raw()` never calls `cfsetispeed`/`cfsetospeed`, so a real port runs at whatever rate it was last left at.

## Global Constraints

- **`make` warning-clean**; **`make warnings` 0/0 on all four legs, no `NOT BUILT`** (a score from a build that never ran is not a pass; start Docker if the Linux leg says so).
- **`swift run --package-path test OS9Tests` green.** Judge on `0 failed`, never a total — other sessions add tests to the same file.
- **Make the new test fail once before believing it.** Non-negotiable here: four vacuous checks were caught during the parent feature, three of them authored by the controller.
- **Test filtering is a BARE POSITIONAL**: `swift run --package-path test OS9Tests hostterm`. Not `--filter`.
- **One focused commit.** `Core:` prefix, one line, <=80 chars.
- **Never `pkill -f os9exec`** — match a full path or a PID.
- **Several sessions share this repo and commit to it concurrently.** Leave their work alone; only `git add` the listed files, **never `git add -A`**.
- **`ROADMAP*.md` and `DECISIONS*.md` are gitignored** — never commit or modify them.
- **Do not touch `Source/OS9AppEmu/UAE68emulator/`.** Keep the mingw arm compiling (stub it).
- **Do not spawn subagents.**
- **`timeout` does not exist on macOS.** Piping into `os9exec` without a trailing `printf '...\n\033\n\004\n'` terminator hangs by design.
- `make warnings` takes minutes — run it in the FOREGROUND, or poll `until ! pgrep -f 'make warnings'; do sleep 15; done`. Do not set up a monitor and end your turn.

## Reference: verified facts (measured 2026-07-29 — do not re-derive)

- **`hostterm_open` runs BEFORE the option table is populated.** In `pCopen`: `hostterm_open` is at `consio.c:605`, `pSCFopt( pid,spP, (byte*)&spP->opt )` at `consio.c:618`. Reading `_sgs_bau` inside `hostterm_open` would read uninitialised data. **This is why the speed call goes after `pSCFopt`, not inside the open.**
- `_sgs_bau` is a single byte at offset 0x15 of the SCF option table (`os9defs/sgstat_from_book.h:65`). It is a **code, not a rate**.
- `baud_bps( byte code )` (`consio.c:967`) already maps that code to bits per second. It is `static` — both new call sites are inside `consio.c`, so it stays static; pass the resolved `ulong` into `hostterm.c`.
- Codes map to: 50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200, 9600, 19200, 38400, 57600, 115200. **2000, 3600 and 7200 have no POSIX `B` constant.**
- **Default `PD_BAU` is `0x0F` = code 15 = 19200** (`init_consoleopts`, `utilstuff.c`).
- A pty ignores speed for timing but **stores** it, and its termios state is **shared between master and slave** (proven during the parent feature). That is what makes this testable without hardware.
- `EWOULDBLOCK` is `#undef`'d by `os9exec_incl.h:129`. `cfmakeraw()` is undeclared on the Linux leg (`os9_ll.h:58` `#undef __USE_MISC`) — `hostterm.c` already defines `_GNU_SOURCE` before its includes, which is what makes `posix_openpt` and friends visible; `cfsetispeed`/`cfsetospeed`/`speed_t` are plain POSIX and need nothing extra, but **confirm on the Linux leg rather than assuming**.

## Scope

**In:** speed applied from `PD_BAU` at open, and re-applied when `SS_Opt` changes it (so `tmode baud=` retunes a live port).

**Out — do not build:** parity and stop bits (OS-9's `PD_PAR` is a separate option); `CRTSCTS` hardware flow control; Windows; TCP endpoints. And **hardware verification**, which is impossible here — see "What this plan cannot prove".

---

### Task 1: Drive the host port's speed from `PD_BAU`

**Files:**
- Modify: `Source/OS9exec_core/hostterm.h` (declare the new function)
- Modify: `Source/OS9exec_core/hostterm.c` (implement it, both UNIX and non-UNIX arms)
- Modify: `Source/OS9exec_core/consio.c` (`pCopen` after `pSCFopt`; `pCsetopt` after the propagation loop)
- Test: `test/Sources/OS9Tests/main.swift`

- [ ] **Step 1: Write the failing test**

Reuse `makePTY()` as it actually exists — **read its current signature first**, it returns a spare fd as well as master and name.

```swift
if let (master, slaveName, spare) = makePTY() {
    // The emulator must push the path's PD_BAU onto the host port. A pty
    // ignores speed for timing but STORES it, and its termios state is shared
    // between both ends -- so we can read back exactly what the emulator set.
    // Default PD_BAU is 0x0F = 19200 (init_consoleopts, utilstuff.c).
    _ = os9(["echo x >/t1"], env: ["OS9T1": slaveName])

    var t = termios()
    tcgetattr( master, &t )
    let got = cfgetospeed( &t )
    close( master ); close( spare )

    if got == speed_t(B19200) {
        print("PASS: hostterm: host port speed follows the path's PD_BAU")
        passed += 1
    } else {
        print("FAIL: hostterm: host port speed follows the path's PD_BAU")
        print("      [expected B19200 (\(speed_t(B19200))), got \(got)]")
        failed += 1
    }
}
```

**Before trusting this, check what a fresh pty's default speed actually is on this machine** — print `cfgetospeed` on a brand-new pty. If macOS happens to default to 19200, this assertion cannot fail and is worthless; in that case make the *second* test below the primary one and pick a non-default rate. Say in your report which you did and what the pty default was.

- [ ] **Step 2: Run it and watch it fail**

```sh
swift run --package-path test OS9Tests hostterm
```
Expected: FAIL, reporting the pty's untouched default speed.

- [ ] **Step 3: Map bits-per-second to `speed_t`**

In `hostterm.c`, inside the `#if defined UNIX && !defined MINGW` block:

```c
/* OS-9 rates that have a POSIX B-constant. 2000, 3600 and 7200 appear in
   OS-9's own table (baud_bps, consio.c) and have no POSIX equivalent, so they
   land in the default arm: leave the port at whatever it was rather than
   silently substituting a different speed, which would be worse than not
   setting one at all. */
static speed_t hostterm_speed( ulong bps )
{
    switch (bps) {
        case     50: return B50;
        case     75: return B75;
        case    110: return B110;
        case    134: return B134;
        case    150: return B150;
        case    300: return B300;
        case    600: return B600;
        case   1200: return B1200;
        case   1800: return B1800;
        case   2400: return B2400;
        case   4800: return B4800;
        case   9600: return B9600;
        case  19200: return B19200;
        case  38400: return B38400;
        case  57600: return B57600;
        case 115200: return B115200;
        default    : return 0;
    }
} /* hostterm_speed */

void hostterm_setspeed( int term_id, ulong bps )
{
    hostterm_typ*  h;
    struct termios t;
    speed_t        sp;

    hostterm_init();
    if (!hostterm_bound( term_id )) return;

    sp= hostterm_speed( bps );
    if (sp==0) return; /* no POSIX equivalent: leave the port alone */

    h= &hostterms[ term_id ];
    if (tcgetattr  ( h->fd,&t   )!=0) return;
    if (cfsetispeed( &t,   sp   )!=0) return;
    if (cfsetospeed( &t,   sp   )!=0) return;

    tcsetattr( h->fd,TCSANOW, &t );

    debugprintf( dbgTerminal,dbgNorm,
                 ( "# hostterm: /t%d speed %lu\n", term_id, bps ) );
} /* hostterm_setspeed */
```

Matching no-op in the non-UNIX arm:

```c
void hostterm_setspeed( int term_id, ulong bps ) { (void)term_id; (void)bps; }
```

Declare it in `hostterm.h` with a doc comment saying it is a no-op for a rate with no POSIX constant, and that a pty stores the speed without honouring it.

- [ ] **Step 4: Call it where the baud becomes known, and where it changes**

In `consio.c` `pCopen`, immediately **after** `pSCFopt( pid,spP, (byte*)&spP->opt );` — not earlier; the option table does not exist before that line:

```c
    /* Not inside hostterm_open: that runs before pSCFopt above, so the option
       table -- and _sgs_bau with it -- is not populated yet there. */
    if (hostterm_bound( id )) {
        struct _sgs* ot= (struct _sgs*)&spP->opt;
        hostterm_setspeed( id, baud_bps( ot->_sgs_bau ) );
    }
```

In `consio.c` `pCsetopt`, after the loop that propagates options to every path on the same device:

```c
    /* SS_Opt is how `tmode baud=` reaches us, so a live port retunes. */
    if (hostterm_bound( spP->term_id )) {
        struct _sgs* ot= (struct _sgs*)&spP->opt;
        hostterm_setspeed( spP->term_id, baud_bps( ot->_sgs_bau ) );
    }
```

`baud_bps` is `static` in `consio.c` and both call sites are in that file, so it stays static. Do not un-static it.

- [ ] **Step 5: Add the `tmode` test — this is the one that proves the `SS_Opt` hook**

```swift
if let (master, slaveName, spare) = makePTY() {
    // `tmode` issues I$SetStt SS_Opt on its stdin path, so redirecting stdin
    // from /t1 retunes THAT device. 2400 is deliberately not the default.
    _ = os9(["tmode </t1 baud=2400"], env: ["OS9T1": slaveName])

    var t = termios()
    tcgetattr( master, &t )
    let got = cfgetospeed( &t )
    close( master ); close( spare )

    if got == speed_t(B2400) {
        print("PASS: hostterm: tmode baud= retunes a live host port")
        passed += 1
    } else {
        print("FAIL: hostterm: tmode baud= retunes a live host port")
        print("      [expected B2400 (\(speed_t(B2400))), got \(got)]")
        failed += 1
    }
}
```

**Verify the `tmode </t1 baud=2400` invocation interactively first** via `tools/os9repl.sh` (unique `OS9REPL_SESSION`), because `tmode`'s argument handling is worth confirming rather than assuming — check `tmode </t1` alone reports that device's settings. If that spelling does not work, find one that does and say which you used. Note project memory records that `tmode baud=<n>` takes the raw bits-per-second value, not the code index.

- [ ] **Step 6: Verify, gate, commit**

```sh
make
swift run --package-path test OS9Tests
make warnings
```
All hostterm tests pass; suite `0 failed`; `make warnings` 0/0 on four legs with no `NOT BUILT`.

```sh
git add Source/OS9exec_core/hostterm.c Source/OS9exec_core/hostterm.h \
        Source/OS9exec_core/consio.c test/Sources/OS9Tests/main.swift
git commit -m "Core: a bound /tN runs at the speed OS-9 thinks it does"
```

---

## What this plan cannot prove

**No real serial hardware exists on this machine**, and no Mac of any era has RS-232. Everything above is verified against a pty, which stores a speed but does not honour it — so this proves the *plumbing* (the right rate reaches the right port at the right time) and **not** that bytes actually move at that rate on a wire.

Genuinely closing it needs a USB-serial adapter: one with pins 2-3 jumpered gives a loopback; two plus a null-modem cable let the Mac mini and the iMac talk. FTDI chipset has the best record with Apple's built-in driver.

**Say so plainly in your report.** Do not describe this task as "serial ports working" — describe it as "the speed the port is set to now follows OS-9's, verified against a pty, unverified against a wire."

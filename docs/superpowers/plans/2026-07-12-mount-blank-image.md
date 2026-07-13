# `mount -k` Blank RBF Image Creation — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **This project has its own REPL-driving skill.** Any step below that runs `tools/os9repl.sh` is driving `os9exec` interactively. Before doing that, read the `os9-dev` skill's `references/common/using-os9exec-repl.md` (gated "send" mode vs. raw "key" mode, one command at a time, restart on a hang rather than poking at it). Skipping this is the most common way to waste time on this project — the REPL has real, non-obvious gotchas.

**Goal:** Add a `-k=<size>` option to `os9exec`'s built-in `mount` command that creates a new, ready-to-use RBF disk image (or, with `-k=0`, a plain host-native directory) at the conventional `hX` device-slot location next to the emulator binary — usable from inside a running OS-9 session with no separate `format` step and no host-side pre-setup.

**Architecture:** `mount`'s existing RAM-disk path (`-r=<size>`) already builds a complete, working RBF filesystem in memory (`PrepareRAM` in `file_rbf.c`) — allocation bitmap, root directory, everything — with no `format` call involved. This plan extracts that filesystem-building logic into a reusable helper, then adds a new code path that writes the same kind of buffer to a host file at `<dir-holding-the-binary>/hX` instead of installing it as RAM. That location is the *already-existing, already-documented* "sibling file next to the binary" convention that `os9exec` auto-detects on first access (`README.md`, "Devices and host filesystem") — so the new device is usable immediately, in the same session, without going anywhere near `mount`'s separate (and currently broken, for our purposes, out of scope to fix here) arbitrary-path-attachment logic.

**Tech Stack:** C (K&R-ish style, matching this file's existing code — this is *not* the Swift-project CLAUDE.md's domain), built via the repo's `GNUmakefile`; verified via `tools/os9repl.sh` (tmux-driven interactive REPL) and the Swift integration-test suite (`test/Sources/OS9Tests/main.swift`).

## Global Constraints

- Match the existing code style in `Source/OS9exec_core/file_rbf.c` exactly: K&R-ish C, no `const`-heavy modern idioms, trailing-comment alignment style (`// comment` after code, aligned), `NUL`/`PSEP`/`PATHDELIM` macros already in use, `Boolean`/`os9err`/`byte`/`ulong` project typedefs — not `bool`/`int`/`uint8_t`/`unsigned long`.
- **Don't invent a new size-parsing convention** — `Source/OS9exec_core/os9main.c:754-791` already parses `-m`/`-mm`/`-M`/`-x`/`-y`/`-w`/`-p`'s size arguments with a `sscanf("%lu%c",...)` + `switch(tolower(modifier))` + case-fallthrough idiom (bare number = base unit, `k` suffix ×1024, `M` suffix ×1024²). Task 1 mirrors that exact idiom (extended with a `g`/`G` case) rather than writing a new one from scratch with different semantics.
- `-b` is already used by `mount` (Windows-only: SCSI bus number). **Do not reuse or overload it.** This plan uses `-k` instead, which is free on every platform — no platform `#ifdef` split needed for the new option at all.
- No new third-party dependencies. Only standard C library additions if not already included in this file (`<sys/stat.h>`/`<direct.h>` for `mkdir` — verify against existing includes before adding).
- Every C change must build clean with `make` (see `GNUmakefile`, targets `all`/`test`) with **zero new warnings** (existing `-Wall` build already used).
- `make test` must stay green (currently 86/0 — see `test/Sources/OS9Tests/main.swift`) after every task that touches test code.
- Preserve `PrepareRAM`'s existing behavior byte-for-byte during the refactor (Task 2) — including its one non-obvious quirk: after the RBF-descriptor-module override branch fires, `totBits` is deliberately *not* recomputed against the overridden `totScts`. Don't "fix" this while refactoring; just preserve the existing order of operations.
- Valid `mount -k` targets are `h0`–`hz` only (never `dd`/`xx`) — `/dd` is fixed at process boot via `OS9DISK` before any OS-9 code runs, so there is no meaningful "create it while running" case for it.
- **Verifying a freshly created device:** don't stop at `touch`+`dir`. `dsave -ive <target>` (from a small source directory) generates *and runs* a restore script with `cmp`-based verification — confirmed live during this plan's design: it genuinely populates and verifies a target device, not just prints a script. Prefer it over a bare `touch` wherever this plan asks you to confirm a new device is really usable.
- **`RAM_SUPPORT` must be defined for `mount -r=<size>` to work at all.** Discovered during Task 2's execution: `GNUmakefile` never defines this macro anywhere (checked the old Xcode project too — also nowhere), so `OS9_Device()`'s only check for a RAM-disk path (`RAM_Device()`, gated behind a real, uncommented `#ifdef RAM_SUPPORT`) is compiled out of every binary this project's actual build produces. `PrepareRAM`/`RoundSectorCount`/`BuildBlankImage` are NOT gated by this — they're already fully compiled and correct — only the path-classification step that recognizes a not-yet-installed absolute path as RAM-disk-eligible is missing. Confirmed live: rebuilding with `-DRAM_SUPPORT` added to `CFLAGS` makes `mount -r=<size> /name` work cleanly (`dir`/`free` both succeed on the result) on an otherwise-unmodified checkout. **Task 2 must add `-DRAM_SUPPORT` to `GNUmakefile`'s `CFLAGS`** (both the default target's flags and the `prod` target's override — they're currently two separate lists, keep them in sync) before its RAM-disk verification step can pass.
- **`mount -r=<size> <name>` requires `<name>` to be an absolute path** (`/ram1`, not `ram1`) — also discovered during Task 2's execution, unrelated to the `RAM_SUPPORT` gap above. A bare relative name never reaches device classification at all: `IO_Type()` short-circuits on any non-absolute path straight to "type of the current directory," before the RBF/RAM classification chain ever runs. This is a real, pre-existing usage requirement of `mount -r=`, not a bug — every `mount -r=<size> <name>` example anywhere in this plan uses a leading `/` on `<name>` accordingly. (`unmount <name>` is unaffected either way — it matches by bare name against the already-installed device table, not by path classification, so `unmount ram1` and `unmount /ram1` both work.)

---

### Reference: current code being modified

- `Source/OS9exec_core/file_rbf.c:934-1077` — `PrepareRAM` (RAM-disk filesystem builder)
- `Source/OS9exec_core/file_rbf.c:1441-1462` — `mount_usage`
- `Source/OS9exec_core/file_rbf.c:1542-1681` — `int_mount`
- `Source/OS9exec_core/file_rbf.c:208-225` — `RAM_zero[]` template sector (has the `"Cruz"` stamp at the right offset already)
- `Source/OS9exec_core/os9main.c:754-791` — the existing `-m`/`-mm` size-suffix parsing idiom to mirror in Task 1
- `Source/OS9exec_core/os9exec_nt.h:651-659` — `TOT_POS`/`TRK_POS`/`MAP_POS`/`BIT_POS`/`DIR_POS`/`CRUZ_POS`/`SECT_POS`/`Cruz_Str`
- `Source/OS9exec_core/os9exec_nt.h:345` — `KByte` (`1024`)
- `Source/OS9exec_core/os9exec_nt.h:369` — `DD__MINALLOC` (`32`)
- `Source/OS9exec_core/os9exec_nt.h:869` — `STD_SECTSIZE` (`256`)
- `Source/OS9exec_core/os9exec_nt.h:1354` — `extern char startPath[OS9PATHLEN]` (host directory holding the `os9exec` binary)
- `Source/OS9exec_core/os9_ll.h:175-178` — `GET_OS9L`/`SET_OS9L`/`GET_OS9W`/`SET_OS9W`
- `Source/OS9exec_core/memstuff.h:102-103` — `get_mem`/`release_mem`
- `Source/OS9exec_core/fileaccess.c:2714-2733` — existing platform-conditional host directory creation (`CreateDirectory` on Windows, `mkdir` elsewhere) — mirror this pattern, don't invent a new one

---

## Task 1: Size-string parser (`ParseDiskSize`)

**Files:**
- Modify: `Source/OS9exec_core/file_rbf.c` (add a new static function near the top of the file, just above `PrepareRAM` at line 934 — i.e. insert before line 934)

**Interfaces:**
- Produces: `static Boolean ParseDiskSize( const char* s, uint32_t* sizeKBOut )` — parses a size string into kilobytes, in the *same unit `mnt_ramSize`/`-r=<size>` already uses*. Consumed by Task 3.

- [ ] **Step 1: Add the parser**

Insert immediately above `PrepareRAM` (before line 934 in the current file). This deliberately mirrors `os9main.c:754-791`'s existing `-m`/`-mm` idiom — same `sscanf`+modifier+`switch`-with-fallthrough shape, same semantics (bare number = base unit, `k` scales ×1024, `M` ×1024², here extended with `g` ×1024³) — rather than inventing a different convention:

```c
static Boolean ParseDiskSize( const char* s, uint32_t* sizeKBOut )
/* Parses a size string the same way os9main.c's -m/-mm option does
 * (os9main.c:754-791): a bare number is bytes; a trailing g/M/k
 * (case-insensitive) scales it up by 1024^3/1024^2/1024 first. Returns
 * the result in kBytes -- the same unit -r=<size> already uses. False on
 * malformed input (nothing parsed, or an unrecognized modifier letter). */
{
    unsigned long val;
    char          modifier= 0;

    if (sscanf( s,"%lu%c", &val,&modifier )<1) return false;

    switch (tolower(modifier)) {
        case 'g' : val*= 1024; /* fall into M */
        case 'm' : val*= 1024; /* fall into k */
        case 'k' : val*= 1024;
        case  0  : break;
        default  : return false;
    } // switch

    *sizeKBOut= (uint32_t)(val / KByte);
    return true;
} /* ParseDiskSize */
```

Note (matches the existing `os9main.c` idiom, not a new gap): like the code it mirrors, this doesn't reject trailing garbage after the modifier letter (e.g. `"20Mx"` parses the same as `"20M"`, silently ignoring the `x`) — that's the established, accepted behavior of the pattern being reused here, not an oversight.

- [ ] **Step 2: Verify includes**

Check the top of `Source/OS9exec_core/file_rbf.c` for `#include <stdio.h>` (needed for `sscanf`) and `#include <ctype.h>` (needed for `tolower`) — both almost certainly already included, since `int_mount` already calls `sscanf`/`tolower` directly (e.g. line 1568, line 1598). No new include should be needed for this step; confirm rather than assume.

- [ ] **Step 3: Build**

Run: `make` from the repo root.
Expected: clean build, no warnings, no errors. (`ParseDiskSize` is unused so far — some compilers warn on unused static functions; if you see `-Wunused-function`, that's expected until Task 3 wires it in. Don't suppress it — just proceed to Task 3 in the same work session so it's short-lived, or verify the warning disappears once Task 3 lands.)

- [ ] **Step 4: Commit**

```bash
git add Source/OS9exec_core/file_rbf.c
git commit -m "Core: add ParseDiskSize size-string helper (K/M/G suffixes)"
```

---

## Task 2: Extract `PrepareRAM`'s filesystem-builder into shared helpers

This is a pure refactor — RAM-disk behavior must not change at all. Two helpers come out of `PrepareRAM`:

- `RoundSectorCount` — the KB→sector-count conversion + track/cluster granulation (the exact math the user asked to see reused for `-k=<size>`'s rounding).
- `BuildBlankImage` — given an already-resolved sector count, builds the complete filesystem buffer (allocation bitmap, root directory FD, root directory entry) that both a RAM disk and a new file-backed image need.

**Files:**
- Modify: `Source/OS9exec_core/file_rbf.c:934-1077` (`PrepareRAM`, replace the body of the non-`devCopy` branch)

**Interfaces:**
- Produces: `static Boolean RoundSectorCount( uint32_t ramSizeKB, uint32_t sctSize, int clu, uint32_t* totSctsOut, uint32_t* totBitsOut )` — returns false (after printing the reason) if `clu` isn't a power of 2; callers must check the return value before using the output. **Signature note:** an earlier draft of this task had this function return `void` and validate `clu` only inside `BuildBlankImage`, called afterward — that ordering divides by `clu` (`totBits = (totScts-1) / clu + 1`) before the power-of-2 check ever runs, which is a real, live-confirmed bug (wrong error code for an invalid cluster size, and undefined-behavior divide-by-zero for `clu==0` — didn't crash on the arm64 host used for testing, since AArch64 `UDIV` returns 0 rather than trapping, but this project ships x86/Windows builds where integer division by zero does trap). The code below has already been corrected; validate `clu` inside `RoundSectorCount`, at the very top, before any division.
- Produces: `static Boolean BuildBlankImage( uint32_t totScts, uint32_t totBits, uint32_t sctSize, int clu, byte** bufOut )` — consumed by Task 3. No longer re-validates `clu`'s power-of-2-ness (that moved to `RoundSectorCount`, its caller); still validates the allocation-bitmap-fits-in-`mapSize` condition, which has no divide-before-check hazard.
- Consumes: `RAM_zero[]` (file_rbf.c:208), `get_mem`/`release_mem`, `SET_OS9L`/`SET_OS9W`, `TOT_POS`/`TRK_POS`/`MAP_POS`/`BIT_POS`/`DIR_POS`/`SECT_POS`, `BpB` (already defined at `os9exec_nt.h:346`).

- [ ] **Step 1: Add the two helpers, just above `PrepareRAM`** (i.e. right after `ParseDiskSize` from Task 1)

```c
#define SectsPerTrack 0x20
#define DefaultScts   8192
#define MaxKB         0x001ffffe // 2097151 kB = 2047.999 MB -- shared cap for -r=<size> and -k=<size>

static Boolean RoundSectorCount( uint32_t ramSizeKB, uint32_t sctSize, int clu,
                                  uint32_t* totSctsOut, uint32_t* totBitsOut )
/* Converts a kBytes request into a valid sector count: rounds up to a whole
 * track, then to a whole allocation cluster. Falls back to DefaultScts if
 * the request rounds down to zero (e.g. ramSizeKB==0). Identical math to
 * PrepareRAM's original inline computation -- moved, not changed. Returns
 * false (after printing the reason) if clu isn't a power of 2 -- checked
 * HERE, before the division below that uses clu as a divisor, not in
 * BuildBlankImage (which runs after this and would divide by an invalid
 * or zero clu first if the check lived there instead). */
{
    uint32_t totScts, tracks, totBits;
    Boolean  ok= false;
    int      ii;

    for (ii=0; ii<31; ii++) { if (1<<ii==clu) { ok= true; break; } }
    if (!ok) {
      upe_printf( "mount: cluster size must be a power of 2\n" );
      return false;
    } // if

              totScts= ramSizeKB*KByte/sctSize; /* adapt to KBytes */
    tracks  = (totScts-1) / SectsPerTrack + 1;
              totScts=      SectsPerTrack * tracks;  /* granulate to tracks */
    totBits = (totScts-1) / clu + 1;
              totScts=      clu * totBits;           /* granulate to clusters */
    if        (totScts==0)  totScts= DefaultScts;

    *totSctsOut= totScts;
    *totBitsOut= totBits;
    return true;
} /* RoundSectorCount */

static Boolean BuildBlankImage( uint32_t totScts, uint32_t totBits, uint32_t sctSize, int clu,
                                 byte** bufOut )
/* Builds a complete, ready-to-use RBF filesystem image in a freshly
 * allocated buffer: identification sector (Cruz-stamped, via RAM_zero),
 * allocation bitmap, root directory FD sector, root directory entry.
 * Returns false (after printing the reason) if the allocation bitmap
 * doesn't fit in the available map size -- the caller owns *bufOut only on
 * true. Caller must already have validated clu (via RoundSectorCount) --
 * this function trusts it's a valid power of 2. */
{
    ulong   allocSize, allocN, mapSize, f, r, fN, rN, cluRest, ii;
    byte*   b;
    int     v;
    byte    pt;
    byte*   base;

             mapSize= (totBits-1)/BpB + 1; // rounding up
    if      (mapSize>0xffff) {
      while (mapSize>0xffff) { mapSize= mapSize/2; clu= clu*2; }
      upe_printf( "mount: error - cluster size is too small for this device.\n" );
      upe_printf( "cluster size must be at least %d.\n", clu );
      return false;
    } // if

    allocSize= (totBits-1)/(sctSize*BpB) + 1; // nr of allocation sectors, rounded up
    allocN   =  allocSize * sctSize*BpB;      // nr of allocation bits

            base= get_mem( sctSize*totScts );
    if    ( base==NULL ) return false;
    memset( base,          sctSize*totScts, 0 ); // clear all
    memcpy( base,RAM_zero, sctSize );

    f= allocSize + 1; fN= f*sctSize; // root dir fd sector position
    r=         f + 1; rN= r*sctSize;

    cluRest=       r/clu + 1;
    cluRest= cluRest*clu - r;

    /* Reserve all these bits in the allocation map */
    pt= 0x80;
    for (ii=0; ii<allocN; ii++) {
      if  (ii<=r/clu || ii>=totBits) { // including fd + dir
        v= ii/BpB;
        b= &base[ sctSize + v ]; *b |= pt;
      } // if
      pt= pt/2; if (pt==0) pt= 0x80; /* prepare the next pattern */
    } // for

    SET_OS9L(base, TOT_POS,  totScts << BpB); /* 0x03 overwritten, is 0 anyway */
             base[ TRK_POS ]= SectsPerTrack;   /* number of sectors per track */
    SET_OS9W(base, MAP_POS,  mapSize);
    SET_OS9W(base, BIT_POS,  clu);
    SET_OS9L(base, DIR_POS,  f << BpB);       /* 0x0b overwritten, is 0 anyway */
    SET_OS9W(base, SECT_POS, sctSize);

                 base[ fN      ]= 0xbf; /* prepare the fd sector */
                 base[ fN+0x08 ]= 0x01;
                 base[ fN+0x0C ]= 0x40;
    SET_OS9L(base, fN+0x10,  r << BpB);       /* fN+0x14 overwritten, is 0 anyway */
                 base[ fN+0x14 ]= cluRest;

                 base[ rN      ]= 0x2e; /* prepare the directory entry */
                 base[ rN+0x01 ]= 0xae;
    SET_OS9W(base, rN+0x1e,  f);
                 base[ rN+0x20 ]= 0xae;
    SET_OS9W(base, rN+0x3e,  f);

    *bufOut= base;
    return true;
} /* BuildBlankImage */
```

- [ ] **Step 2: Replace all of `PrepareRAM` (lines 934-1077) with this complete function**

This is the entire function, verbatim — declarations included — replacing the whole current `Source/OS9exec_core/file_rbf.c:934-1077`. The `devCopy` branch (a pre-existing, unrelated feature: "create RAM disk as a copy of `<device>`") is untouched; only the branch below it changes, to call the two new helpers instead of doing the work inline:

```c
static os9err PrepareRAM( ushort pid, rbfdev_typ* dev, char* cmp )
{
    os9err    err, cErr;
    uint32_t  iSize;
    uint32_t  totBits;
    int       clu= mnt_cluSize;
    mod_dev*  mod;
    char*     p;
    ptype_typ type;
    ushort    sp;

    if (strcmp( mnt_devCopy,""  )!=0) {
      strcat  ( mnt_devCopy,"@" );
      type= IO_Type        ( pid,            mnt_devCopy, poDir );   
      err = syspath_open   ( pid, &sp, type, mnt_devCopy, poDir ); if (err) return err;
      err = syspath_gs_size( pid,  sp, &iSize );
      
      if (!err && iSize>0) {
             dev->ramBase= get_mem( iSize );
        if ( dev->ramBase==NULL ) return E_NORAM;
        err= syspath_read( pid, sp, &iSize, dev->ramBase, false );
      } // if
      
      cErr= syspath_close( pid, sp ); if (!err) err= cErr;
      if (err) return err;
      
      dev->totScts    = GET_OS9L(dev->ramBase, TOT_POS) >> BpB;
                        dev->imgScts    = dev->totScts;
      dev->clusterSize= GET_OS9W(dev->ramBase, BIT_POS);
      dev->sctSize    = GET_OS9W(dev->ramBase, SECT_POS);
                                            dev->sas        = DD__MINALLOC;
      return 0;
    } // if

    if (mnt_ramSize>MaxKB) {
      upe_printf( "mount: error - size is too large for this device.\n" );
      return 1;
    } // if

    if (mnt_sctSize>0) { dev->sctSize    = mnt_sctSize; }
                         dev->clusterSize= clu;
                         dev->sas        = DD__MINALLOC;

    if (!RoundSectorCount( mnt_ramSize, dev->sctSize, clu, &dev->totScts, &totBits ))
      return E_NORAM; /* RoundSectorCount already printed the specific reason */

    if ( mnt_ramSize==0
      && IsDesc( cmp, &mod, &p )
      && ustrcmp( p,"RBF" )==0 ) {
        p= (char*)mod + os9_word(mod->_mpdev);
        if (ustrcmp( p,"ram" )==0) {
                dev->totScts= GET_OS9W((byte*)(&mod->_mdtype + PD_SCT), 0);
        } // if
    } // if

    dev->imgScts= dev->totScts;

    if (!BuildBlankImage( dev->totScts, totBits, dev->sctSize, clu, &dev->ramBase ))
      return E_NORAM;

    strcpy( dev->img_name,cmp );
    return 0;
} /* PrepareRAM */
```

Compared to the original: `#define DefaultScts`/`#define SectsPerTrack`/`#define MaxKB` are all gone from inside this function — they now live at file scope above `RoundSectorCount` (added in Step 1), and `CreateBlankDevice` (Task 3) reuses the same `MaxKB`. Leaving a second copy of any of these here would be a duplicate-macro warning. Declarations drop everything that moved into the two new helpers (`allocSize, allocN, mapSize, f, r, fN, rN, tracks, cluRest, ii, b, v, pt, ok` are gone), and gain `uint32_t totBits;` for the new `RoundSectorCount` call. The `RoundSectorCount` call site now checks its (`Boolean`) return value and returns `E_NORAM` on failure — see the note on `RoundSectorCount`'s signature above; this replaces the standalone power-of-2 check the original inline code ran before any division, now folded into `RoundSectorCount` itself instead of duplicated here. Everything else — the `devCopy` branch, the descriptor-module override, the field-assignment order — is character-for-character the same as before.

- [ ] **Step 3: Build**

Run: `make`
Expected: clean build, zero warnings (in particular, no "unused variable" warnings — this is the signal that Step 2's variable cleanup was done correctly).

- [ ] **Step 4: Enable `RAM_SUPPORT` in the build**

`mount -r=<size>` (RAM disks) cannot work at all on a standard build of this project today — `GNUmakefile` never defines `RAM_SUPPORT`, which is required for `OS9_Device()` to recognize a not-yet-installed absolute path as RAM-disk-eligible (see the Global Constraints note above; this is pre-existing and unrelated to this task's refactor, but it must be fixed here since the next step needs a working `mount -r=` to verify against). In `GNUmakefile`, add `-DRAM_SUPPORT` to both CFLAGS lists — the default target's `CFLAGS` (near the top of the file, alongside `-DTERMINAL_CONSOLE -DINT_CMD`) and the `prod` target's separate override (which repeats the same flag list with `-O2` instead of `-g`). Keep both lists in sync — search for `-DINT_CMD` to find both occurrences.

Run `make clean && make` (a plain `make` won't detect the CFLAGS change since `make` doesn't track compiler flags, only file mtimes — `make clean` first is required to force a full rebuild here). Expected: clean build, zero warnings.

- [ ] **Step 5: Verify RAM-disk behavior is unchanged, thoroughly — this is the "test RAM disk works as expected" the user asked for**

Read the `os9-dev` skill's `using-os9exec-repl.md` first if you haven't already this session (see the note at the top of this plan). Then:

```bash
tools/os9repl.sh start
tools/os9repl.sh send "mount -r=200 /ram1"
tools/os9repl.sh send "dir /ram1"
tools/os9repl.sh send "free /ram1"
```

Expected so far: `dir /ram1` on the fresh disk shows an empty listing (no error); `free /ram1` reports a sector count and byte total containing "sectors". If either fails, stop — either the `RAM_SUPPORT` build fix from Step 4 didn't take (confirm with `make clean && make` again) or the refactor broke something; don't proceed to the steps below until this is clean. Note the leading `/` on `/ram1` — `mount -r=<size> <name>` requires an absolute device name (see the Global Constraints note); a bare `ram1` fails with a different, unrelated error regardless of this task's changes.

Now populate and verify it for real, using `dsave -ive` (confirmed during this plan's design to genuinely copy-and-`cmp`-verify, not just print a script) rather than a bare `touch`:

```bash
tools/os9repl.sh send "makdir /dd/USR/TESTER/ramtest"
tools/os9repl.sh send "echo ramdisk verification content >/dd/USR/TESTER/ramtest/f1"
tools/os9repl.sh send "chd /dd/USR/TESTER/ramtest"
tools/os9repl.sh send "dsave -ive /ram1"
tools/os9repl.sh send "dir /ram1"
tools/os9repl.sh send "chd /dd"
tools/os9repl.sh send "del /dd/USR/TESTER/ramtest/f1"
tools/os9repl.sh send "deldir -q /dd/USR/TESTER/ramtest"
tools/os9repl.sh send "unmount ram1"
tools/os9repl.sh stop
```

Expected: `dsave -ive /ram1` prints the generated script as it runs it (`chd`, `tmode`, `load copy`, `copy`, `cmp`, `unlink`) with no error from the `cmp` step; the following `dir /ram1` shows `f1`. `unmount ram1` succeeds with no error (bare name is fine here — `unmount` isn't affected by the absolute-path requirement, see the Global Constraints note). This whole sequence proves the refactor preserved real RAM-disk behavior.

- [ ] **Step 6: Commit**

```bash
git add Source/OS9exec_core/file_rbf.c GNUmakefile
git commit -m "Core: extract PrepareRAM's filesystem builder into reusable helpers

Also enables RAM_SUPPORT in the build -- mount -r=<size> (RAM disks)
never worked on a standard build before this; the feature code itself
was already complete and correct, just never compiled in."
```

---

## Task 3: `mount -k=<size> hX` — create the file-backed (or directory) device

**Files:**
- Modify: `Source/OS9exec_core/file_rbf.c:1441-1462` (`mount_usage`)
- Modify: `Source/OS9exec_core/file_rbf.c:1542-1681` (`int_mount`)

**Interfaces:**
- Consumes: `ParseDiskSize` (Task 1), `RoundSectorCount`/`BuildBlankImage` (Task 2), `startPath` (`os9exec_nt.h:1354`), `FileFound`/`PathFound` (already used elsewhere in this file, e.g. `filestuff.c`).
- Produces: the `-k=<size>` CLI option on the `mount` internal command; no new exported symbols (this is the top of the feature — nothing later depends on it).

- [ ] **Step 1: Add `CreateBlankDevice`, just above `int_mount`** (i.e. right after `mount_usage`, before line 1542)

```c
static os9err CreateBlankDevice( ushort pid, const char* name, uint32_t sizeKB,
                                             int sctSizeArg, int cluSizeArg )
/* Creates a new hX device at <startPath>/hX -- either a fully-formatted
 * blank RBF image (sizeKB>0) or a plain host directory (sizeKB==0).
 * Refuses if a file/dir already exists at that path. <name> may be given
 * with or without a leading '/' ("h7" or "/h7"), and only h0..hz is valid
 * -- /dd is fixed at boot via OS9DISK and is never a valid target.
 * Reuses the file-scope MaxKB defined above RoundSectorCount (Task 2, Step 1). */
{
    char      hostpath[OS9PATHLEN];
    const char* p= name;
    byte*     buf;
    uint32_t  totScts, totBits;
    uint32_t  sctSize= (sctSizeArg>0) ? (uint32_t)sctSizeArg : STD_SECTSIZE;
    int       clu    = (cluSizeArg>0) ? cluSizeArg           : 1;
    FILE*     fp;

    if (*p==PSEP) p++; /* allow a leading '/' */
    if (tolower(p[0])!='h' || p[1]==NUL || p[2]!=NUL || !isalnum((unsigned char)p[1]))
      return _errmsg( E_BPNAM, "mount -k: device must be h0..hz, got \"%s\".\n", name );

    strcpy( hostpath,startPath );
    if (hostpath[strlen(hostpath)-1]!=PATHDELIM) strcat( hostpath,PATHDELIM_STR );
    strncat( hostpath,p,2 );

    if (FileFound( hostpath ) || PathFound( hostpath ))
      return _errmsg( E_CEF, "mount -k: '%s' already exists -- remove it first.\n", hostpath );

    if (sizeKB==0) { /* -k=0 : plain host directory */
      #ifdef windows32
        if (!CreateDirectory( hostpath,NULL ))
          return _errmsg( E_BPNAM, "mount -k: can't create directory '%s'.\n", hostpath );
      #else
        if (mkdir( hostpath,0x01c0 )!=0)
          return _errmsg( E_BPNAM, "mount -k: can't create directory '%s'.\n", hostpath );
      #endif
      upo_printf( "mount: created host directory '%s'\n", hostpath );
      return 0;
    } // if

    if (sizeKB>MaxKB)
      return _errmsg( E_BPNAM, "mount -k: size is too large for this device.\n" );

    if (!RoundSectorCount( sizeKB, sctSize, clu, &totScts, &totBits ))
      return E_NORAM; /* RoundSectorCount already printed the specific reason */
    if (!BuildBlankImage( totScts, totBits, sctSize, clu, &buf ))
      return E_NORAM; /* BuildBlankImage already printed the specific reason */

    fp= fopen( hostpath,"wb" );
    if (fp==NULL) { release_mem( buf ); return _errmsg( E_BPNAM, "mount -k: can't create '%s'.\n", hostpath ); }
    fwrite( buf, sctSize, totScts, fp );
    fclose( fp );
    release_mem( buf );

    upo_printf( "mount: created '%s' (%u sectors, %u bytes/sector)\n", hostpath, totScts, sctSize );
    return 0;
} /* CreateBlankDevice */
```

- [ ] **Step 2: Verify includes**

`CreateBlankDevice` uses `mkdir` (non-Windows) or `CreateDirectory` (Windows), `isalnum`, and `fopen`/`fwrite`/`fclose`. Check the top of `Source/OS9exec_core/file_rbf.c` for `#include <sys/stat.h>` (non-Windows `mkdir`) and add it if missing — `<ctype.h>` and `<stdio.h>` are almost certainly already included (this file already calls `tolower` and `sscanf`/`sprintf`). On the Windows build, `CreateDirectory` needs `<windows.h>`, which is already included wherever this file's existing `#ifdef windows32` blocks reference other Win32 calls (e.g. `scsiadaptor_help`) — no new include needed there.

- [ ] **Step 3: Wire the `-k` option into `int_mount`'s argument parsing**

Modify the local-variable block at the top of `int_mount` (around line 1545-1558) — add two new declarations after `int imgMode = Img_Unchanged;`:

```c
    Boolean   blankImage = false;
    uint32_t  blankSizeKB= 0;
```

Add a new `case 'k'` to the main option `switch` (not inside any `#ifdef` — `-k` is free on every platform, unlike `-b`). Place it right after the existing `case 'f' : imgMode = Img_FullSize; break;` line and before the `#ifdef windows32` block for `case 'a'`/`case 'b'`:

```c
                case 'k' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if

                           if (!ParseDiskSize( p,&blankSizeKB )) {
                             upe_printf( "mount: error - invalid size '%s'\n",p );
                             return 1;
                           } // if
                           blankImage= true;
                           break;
```

Everything else in the switch (`case 'a'`/`case 'b'` under `#ifdef windows32`, `case 's'`, `case 'l'`, `case 'r'`, `case 'n'`, `case 'c'`, `case 'd'`, `default`) stays exactly as it is today — untouched.

- [ ] **Step 4: Handle the `-k` path before the normal mount logic runs**

Immediately after the `for (k=1; k<argc; k++) { ... }` argument-parsing loop closes (right before the existing `if (nargc==0) { ... }` block, around line 1668), insert:

```c
    if (blankImage) {
      if (nargc!=1)
        return _errmsg( E_BPNAM, "usage: mount -k=<size> <h0..hz>\n" );
      err= CreateBlankDevice( pid, nargv[0], blankSizeKB, sctSize, cluSize );
      if (err) return err; /* CreateBlankDevice already printed the reason */
      return 0;
    } // if
```

- [ ] **Step 5: Update `mount_usage`**

In `Source/OS9exec_core/file_rbf.c:1441-1462`, add a new line after the existing `-d=<device>` line:

```c
    upe_printf( "    -d=<device>  create RAM disk as a copy of <device>\n" );
    upe_printf( "    -k=<size>    create blank hX device (K/M/G suffix; 0 = host dir)\n" );
```

(No `#ifdef` needed — unlike the old `-b` idea, `-k` means the same thing on every platform.)

- [ ] **Step 6: Build**

Run: `make`
Expected: clean build, zero warnings.

- [ ] **Step 7: Verify end-to-end via the REPL, using `dsave -ive` to prove the fresh filesystem is genuinely usable**

Read the `os9-dev` skill's `using-os9exec-repl.md` first if you haven't already this session.

```bash
ls h7 2>&1   # confirm nothing named h7 exists yet, host-side
tools/os9repl.sh start
tools/os9repl.sh send "mount -?"
tools/os9repl.sh send "mount -k=500K h7"
tools/os9repl.sh send "dir /h7"
tools/os9repl.sh send "free /h7"
tools/os9repl.sh send "dcheck /h7"
```

Expected so far: `mount -?` shows the new `-k=<size>` line. `mount -k=500K h7` prints a "created ... sectors ..." confirmation with no error. `dir /h7` on the fresh image shows an empty listing. `free /h7` reports a sector count. `dcheck /h7` reports the structure intact.

Now populate and verify it for real:

```bash
tools/os9repl.sh send "makdir /dd/USR/TESTER/h7test"
tools/os9repl.sh send "echo h7 verification content >/dd/USR/TESTER/h7test/f1"
tools/os9repl.sh send "chd /dd/USR/TESTER/h7test"
tools/os9repl.sh send "dsave -ive /h7"
tools/os9repl.sh send "dir /h7"
tools/os9repl.sh send "chd /dd"
tools/os9repl.sh send "del /dd/USR/TESTER/h7test/f1"
tools/os9repl.sh send "deldir -q /dd/USR/TESTER/h7test"
tools/os9repl.sh send "mount -k=500K h7"
tools/os9repl.sh stop
rm -f h7
```

Expected: `dsave -ive /h7` runs its generated script (`copy`, `cmp`, ...) with no error; the following `dir /h7` shows `f1`. The **second** `mount -k=500K h7` (target already exists) must fail with a clear "already exists" error, not silently overwrite `h7`.

- [ ] **Step 8: Verify the `-k=0` (host directory) variant**

```bash
ls h8 2>&1
tools/os9repl.sh start
tools/os9repl.sh send "mount -k=0 h8"
tools/os9repl.sh send "touch /h8/probe"
tools/os9repl.sh send "dir /h8"
tools/os9repl.sh stop
ls -la h8   # confirm this is a real host directory, not an RBF image file
rm -rf h8
```

Expected: `mount -k=0 h8` creates a real host directory (confirmed via `ls -la h8` showing a `d` in the permissions), `touch`+`dir` inside it behave exactly like any other host-native device (e.g. `/h0`). (A bare `touch` is fine here — `dsave` doesn't add anything new to prove for a plain host-native directory, which has no filesystem structure of its own to verify; that's only interesting for the RBF-image case above.)

- [ ] **Step 9: Commit**

```bash
git add Source/OS9exec_core/file_rbf.c
git commit -m "Core: mount -k=<size> creates a ready-to-use blank hX device"
```

---

## Task 4: README documentation

**Files:**
- Modify: `README.md:206-221` (insert a new subsection right after the existing `/h0`–`/hz` paragraph, before the `### Devices stay inside their root` heading at line 223)

**Interfaces:** none (documentation only).

- [ ] **Step 1: Insert the new subsection**

In `README.md`, immediately after this existing line (221):

```
Use `mount <image> <devname>` to attach an image under a name of your choosing.
```

insert:

```markdown

### Creating new disk images

`mount` can also create a brand-new device, instead of attaching an existing
one:

```
mount -r=<size> [<name>]      create an in-memory RAM disk, <size> in kBytes
mount -k=<size> h0..hz        create a ready-to-use blank RBF image on disk
mount -k=0      h0..hz        create a plain host directory instead
```

`<size>` accepts a bare number (bytes) or a `k`/`M`/`G` suffix (×1024/×1024²/×1024³) —
the same convention `os9exec`'s own `-m`/`-mm` command-line options use.

**RAM disk** (`-r=<size>`): fully formatted and usable immediately — no
`format` needed. Lives only in memory; gone on `unmount` or emulator exit.
`<name>` must be an absolute path (defaults to `/r0` if omitted) — it isn't
tied to the `h0`–`hz` convention since there's no host file involved, but it
does need the leading `/`.

```
mount -r=2000 /scratch    # 2000 kB RAM disk named /scratch
dir /scratch
unmount scratch           # releases the memory (bare name is fine here)
```

**Blank disk image** (`-k=<size>`): writes a fully formatted, ready-to-use
RBF image straight to `<dir-holding-the-binary>/hX` — the same place the
`/h0`–`/hz` auto-mount convention already looks (see the table above), so
the new device works immediately in the same session with no extra step.
`<size>` is rounded up to a valid sector/track/cluster boundary
automatically. The target must be `h0`–`hz` (never `dd`) and must not
already exist — `mount -k` refuses to overwrite an existing file or
directory.

```
mount -k=20M h7
dir /h7
```

To populate a freshly created image with real content (and verify the
copy), `dsave` from an existing directory works against it like any other
device — `-i` indents, `-v` verifies each file with `cmp`, `-e` executes
the generated script immediately instead of just printing it:

```
chd /dd/CMDS
dsave -ive /h7
```

**Device-resolution order, if you're layering these:** for any `/hX` path,
`os9exec` checks, in this order: (1) the `OS9Hx` environment variable, if
set; (2) a file/dir named `hX` next to the binary — what `mount -k` writes;
(3) one directory level up from the binary (a legacy fallback). Separately,
and taking priority over all three of those for as long as the current
process keeps running, an explicit `mount <file> <name>` (or `mount -r=`)
call registers a device directly in memory — if you've already mounted
something under a given name this session, a same-named `mount -k` file
created afterward is shadowed by that registration until the process
restarts.
```

- [ ] **Step 2: Proofread against the actual build**

Read the `os9-dev` skill's `using-os9exec-repl.md` first if you haven't already this session. Then re-run Task 3's Step 7 REPL session once more, this time copying the exact commands shown in the new README text (`mount -r=2000 /scratch`, `mount -k=20M h7`, `dsave -ive /h7`) rather than the plan's own test values, to confirm the documented examples work verbatim. Clean up any files/directories created (`h7`, `rm -f h7`).

- [ ] **Step 3: Commit**

```bash
git add README.md
git commit -m "Docs: document mount -k blank-image creation and -r RAM disks"
```

---

## Task 5: Self-contained RBF tests + a RAM disk test

Currently `test/Sources/OS9Tests/main.swift`'s RBF-specific tests depend on a real, pre-existing RBF image at the repo-root `h1` (passed via `OS9H1`, gated on `h1Available`). Replace that with a scratch image the test suite creates and destroys itself via the new `mount -k`, and add a RAM disk test exercising `mount -r=`. Both use `dsave -ive` to populate and verify, not just `touch`.

**Files:**
- Modify: `test/Sources/OS9Tests/main.swift` (the `// ── RBF device regression tests ──` block near the end of the file — replace it; the block currently reads `h1Path`/`h1Available` from the top-of-file configuration section, which can also be removed since nothing else uses it)

**Interfaces:**
- Consumes: `mount -k=<size> hX` and `mount -r=<size> <name>` / `unmount <name>` (Task 3, and the pre-existing `-r=`/`unmount`), `dsave -ive` (confirmed live during this plan's design — see the Global Constraints note).
- Consumes existing test helpers: `check(_:contains:_:)`, `noError(_:_:)`, `os9(_:timeout:paced:)`, `repoRoot` (all already defined earlier in this file).

- [ ] **Step 1: Remove the now-unused `h1Path`/`h1Available` configuration**

Near the top of `test/Sources/OS9Tests/main.swift`, remove these lines (added in the earlier "stop depending on test/h1" work):

```swift
// Optional RBF disk image (repo-root h1) for RBF-specific regression tests.
let h1Path      = repoRoot.appendingPathComponent("h1").path
let h1Available = FileManager.default.fileExists(atPath: h1Path)
```

And in the `os9(_:)` function, remove the `OS9H1` line from the local-run environment:

```swift
        var env = ["OS9DISK": diskPath]
        if h1Available { env["OS9H1"] = h1Path }
        process.environment  = env
```

replacing it with:

```swift
        process.environment  = ["OS9DISK": diskPath]
```

(This is a deliberate scope reduction — the test suite created its own dependency on `OS9H1` a few tasks ago specifically to reach a real RBF image; Task 5 replaces that need entirely, so the plumbing for it goes too. Don't leave dead code.)

- [ ] **Step 2: Replace the RBF test block**

Find and replace the entire `// ── RBF device regression tests ──` block (added in the earlier "stop depending on test/h1" work — it currently gates on `h1Available` and runs `free /h1`/`dcheck /h1`) with:

```swift
// ── RBF device regression tests ───────────────────────────────────────────────
// Self-contained: create our own scratch RBF image via mount -k, populate
// and verify it with dsave -ive (not just touch), then delete the host
// file -- no pre-existing disk image required.
let scratchDevice   = "h9"
let scratchHostPath = repoRoot.appendingPathComponent(scratchDevice).path
try? FileManager.default.removeItem(atPath: scratchHostPath) // in case a previous run left it behind

noError("mount -k: creates blank RBF image", "mount -k=500K \(scratchDevice)")
noError("rbf: dir /h9 on fresh image",       "dir /h9")
check  ("rbf: free /h9 reports sectors",     contains: "sectors", "free /h9")
noError("rbf: dcheck /h9 structure intact",  "dcheck /h9")
check  ("rbf: dsave -ive populates+verifies", contains: "f1",
    "echo dsave test content >/dd/t_dsavesrc",
    "makdir /dd/t_dsavedir",
    "copy /dd/t_dsavesrc /dd/t_dsavedir/f1",
    "chd /dd/t_dsavedir", "dsave -ive /h9", "dir /h9",
    "chd /dd", "del /dd/t_dsavesrc", "del /dd/t_dsavedir/f1", "deldir -q /dd/t_dsavedir")

try? FileManager.default.removeItem(atPath: scratchHostPath)

// ── RAM disk regression test ──────────────────────────────────────────────────
// mount -r=<size> builds a complete filesystem in memory -- no host file,
// released again via unmount. Also verified with dsave -ive, not just touch.
// Note the leading '/' on /ram9: mount -r=<size> <name> requires an
// absolute device name -- a bare "ram9" fails classification entirely
// (unrelated to this feature; see the plan's Global Constraints note).
noError("ramdisk: mount -r creates disk", "mount -r=200 /ram9", "dir /ram9", "unmount ram9")
check  ("ramdisk: dsave -ive populates+verifies", contains: "f1",
    "echo ramdisk test content >/dd/t_ramsrc",
    "makdir /dd/t_ramdir",
    "copy /dd/t_ramsrc /dd/t_ramdir/f1",
    "chd /dd/t_ramdir", "mount -r=200 /ram9", "dsave -ive /ram9", "dir /ram9", "unmount ram9",
    "chd /dd", "del /dd/t_ramsrc", "del /dd/t_ramdir/f1", "deldir -q /dd/t_ramdir")
```

- [ ] **Step 3: Run the full suite**

```bash
make test
```

Expected: all tests pass, ending in a line like `Results: 90 passed, 0 failed` (86 previous + 4 new RBF-scratch checks + 2 new RAM-disk checks — recount against the actual new total once you've run it, this is an estimate). If anything fails, read the printed `output:` preview for that check before changing anything — don't guess.

- [ ] **Step 4: Confirm no leftover scratch files**

```bash
ls h9 2>&1   # expect "No such file or directory" -- confirms cleanup ran
```

- [ ] **Step 5: Commit**

```bash
git add test/Sources/OS9Tests/main.swift
git commit -m "Tests: self-contained RBF scratch image via mount -k; add RAM disk test"
```

---

## Self-review notes (for whoever executes this plan)

- **Spec coverage:** K/M/G size parsing (reusing the existing `os9main.c` idiom, not a new one) + rounding → Task 1 + `RoundSectorCount` in Task 2. `mount -k` creating a ready-to-use image in one step → Task 2 (buffer builder) + Task 3 (wiring + host directory variant). No `-b` reuse/overload → Task 3 uses `-k` throughout, no platform `#ifdef` needed. Required-`hX`-naming constraint, no `dd` → Task 3 Step 1 validation. `dsave -ive` used for real populate+verify, not just `touch` → Task 2 Step 5, Task 3 Step 7, Task 5. README section for both blank images and RAM disks, plus the precedence/conflict notes and the `dsave -ive` technique → Task 4. Self-contained RBF tests + RAM disk test → Task 5.
- **Not in scope, deliberately:** fixing `mount`'s existing arbitrary-file/arbitrary-path attachment classification bug (`E_FNA`/`E_MNF` found during design exploration) — out of domain per explicit direction; `crefile` reconstruction — dropped in favor of `mount -k` per explicit direction.
- **In scope, discovered mid-execution:** `GNUmakefile` never defining `RAM_SUPPORT` (so `mount -r=<size>` never worked on any real build) and `mount -r=<size> <name>` requiring an absolute `<name>` — both found while implementing Task 2, both fixed as part of Task 2 (the Makefile fix) or documented throughout the plan (the absolute-path requirement), by explicit direction rather than deferred as out-of-scope like the `mount`-classification bug above. The distinction: that bug is genuinely unrelated machinery this feature doesn't need working; `RAM_SUPPORT` gates code this same plan explicitly relies on and reuses (`PrepareRAM`'s helpers), so leaving it broken would leave Task 2's own verification, and Task 5's RAM disk test, unable to pass.
- **Bug caught by Task 2's own task review, fixed in the plan text before re-dispatch:** the first draft of `RoundSectorCount`/`BuildBlankImage` (this plan's own Task 2 Step 1, as originally written) put the `clu`-is-a-power-of-2 validation inside `BuildBlankImage`, called *after* `RoundSectorCount` already divides by `clu`. Live-tested by the reviewer: this changed the error code returned for an invalid cluster size, and caused undefined-behavior integer division by zero for `-c=0` (silently returned 0 on the arm64 test host rather than crashing, but this project ships x86/Windows builds where that traps). Fixed by moving the validation into `RoundSectorCount` itself (now returns `Boolean`) — see the signature note under Task 2's Interfaces section. Left here as a record of a real defect in this plan's own authored code, not just the implementers' — the plan's authorship doesn't exempt it from the same review rigor as any implementation.

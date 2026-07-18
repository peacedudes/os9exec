# Native ARM64 Windows build of os9exec — design

## Goal

Today the only Windows `os9exec` is the mingw-w64 **x86_64** cross-build
(compiled on the Mac via `x86_64-w64-mingw32-gcc`, copied to the Windows
11 ARM64 guest, run through Windows' x64-on-ARM64 emulation). ROADMAP's
top-priority bug/feature item asks for a true **native ARM64** binary.

This session reframes that as two goals at once, of equal weight:

1. **Get a native ARM64 Windows build working**, using the toolchains an
   actual Windows C developer would realistically reach for — not the
   single likeliest-to-work option, but as many common ones as are
   practical, so we can say more than "the Windows build works."
2. **Use toolchain diversity as a bug-hunting oracle.** Every previous
   platform/toolchain switch on this project (mingw LLP64, Linux gcc,
   ASan/UBSan, 32-bit Linux) surfaced real bugs the others missed — most
   notably the `ulong` pointer-truncation bug on mingw's LLP64 model. A
   new architecture (ARM64) and new compilers (clang variants, MSVC) are
   expected to do the same, particularly around **endianness** and
   **int/pointer width mismatches** (32-bit int vs 64-bit pointer, or any
   toolchain-specific surprise in that family). Compiler warnings are
   data, not noise — they get investigated, not suppressed.

## Explicit non-goals / boundaries

- **Do not touch the macOS build or run the local macOS REPL/test
  suite.** A separate Claude session is using the local Mac repl; macOS
  `os9exec` must stay exactly as that session leaves it. All build/test
  work in this task happens **only** on the Windows ARM64 UTM guest.
- **Known-open Windows test failures are out of scope.** Current Windows
  baseline is **122/129** (7 failing): the `pd`/`dsave` directory-FD
  instability pair, the `/h0` device-alias resolution test, and ~4
  NTFS-owner/permission-model divergence tests (all already tracked in
  ROADMAP). These are not this task's job to fix — a new toolchain
  reproducing the *same* 7 is a clean baseline match, not a new problem.
- **CI wiring (GitHub Actions) is deferred**, even though GitHub now
  offers hosted Windows-arm64 runners. Worth a ROADMAP follow-up note,
  not this session's scope.
- Not chasing exotic/rare toolchains (e.g. Zig cc) — common, mainstream
  choices only.

## Toolchains to try, in this order

Each is installed and built **on the guest itself** (not cross-compiled
from the Mac) — the point is to reproduce what an actual Windows
developer's workflow looks like, source arriving via `scp` (no git on
the guest) standing in for a real `git clone`.

1. **MSYS2, `clangarm64` environment.** Most common way an open-source
   C project with a plain Makefile gets built on Windows today; official
   ARM64 support since 2023. `$OS=Windows_NT` is a native Windows env var
   regardless of shell, so the existing `GNUmakefile`'s Windows branch
   should apply unmodified with `CC=clang`. Bundles its own `make`.
2. **llvm-mingw** (standalone prebuilt toolchain — the one FFmpeg/mpv
   etc. use for Windows ARM64 builds). Same mingw-w64 CRT lineage as
   MSYS2's clangarm64 and the already-working x86_64 leg, different
   distribution mechanism. Needs a `make` sourced separately (not
   bundled).
3. **Visual Studio 2022 + clang-cl, arm64 target.** MSVC ABI/UCRT, not
   mingw-w64's CRT — `winfiles.c` is a byte-identical copy of
   `linuxfiles.c` leaning on POSIX dirent/stat/realpath, none of which
   exists verbatim under UCRT. Expected to need real porting work per
   ROADMAP's own assessment. Push far enough to characterize exactly
   what breaks; if it turns into a full second platform port, stop and
   document rather than force it through unscoped.
4. **cl.exe (MSVC) directly.** Baseline "no third-party tools" case,
   heaviest lift (no GNU Makefile support at all). Same stop condition
   as #3.

We are not dying on the hill of getting all four fully working — best
effort, but each one attempted and its outcome documented either way.

## Per-toolchain workflow

For each toolchain, in order, with a checkpoint back to the user after
each before moving to the next:

1. Install the toolchain on the guest.
2. Build `os9exec.exe` from the existing `GNUmakefile`, changing as
   little as possible. If a genuine platform-specific change is needed
   (new `PLAT` variant, macro guard, etc.), make it — same standard as
   the existing MINGW port work.
3. **Read every warning, not just errors.** Don't suppress; investigate.
   Specifically hunt for endianness bugs and int/pointer-width
   mismatches (the `ulong`-truncation bug family) — flag and fix real
   ones found, following the project's standing practice of fixing real
   bugs surfaced during investigation rather than just documenting them.
4. Build the Swift test harness (`OS9Tests.exe`, already native ARM64 on
   the guest via Swift+MSVC) and run the full suite against the new
   binary — full run is ~1 minute, no need to shorten or sample.
5. Compare against the 122/129 Windows baseline. New failures beyond the
   known 7 are real bugs to root-cause (same standard as prior Windows
   sessions — "every test failure has a reason"). Reproducing exactly
   the known 7 is a clean pass.
6. Checkpoint: report what worked, what didn't, what was fixed, and any
   warnings/oddities worth discussing before starting the next toolchain.

## Landing durably

For each toolchain that reaches a comparable pass rate:

- Commit whatever Makefile/build changes were needed to reproduce the
  build (aiming for as little as possible — ideally the existing
  `GNUmakefile` needs no changes beyond `CC=`).
- Write a memory file with the exact install → build → run recipe,
  following `windows-vm-access.md`'s established pattern (canonical
  how-to, not scattered across session notes).
- Update ROADMAP's "PRIORITY — native ARM64 Windows build" bullet once
  at least one native path is confirmed working and verified, rather
  than leaving it stale once this work lands.

For toolchains that don't reach a working build within reasonable
scope (expected candidates: clang-cl, cl.exe), document precisely what
broke and why as a ROADMAP entry — not a silent drop.

## Verification

- Full Swift integration suite (`OS9Tests.exe`) run natively on the
  guest per toolchain, compared against the 122/129 baseline.
- All three existing warning-clean toolchains (macOS clang, Linux gcc,
  Windows x86_64 mingw-gcc) are unaffected — this task only adds new
  build paths, doesn't modify existing ones, and never touches the
  macOS binary or local Mac REPL.

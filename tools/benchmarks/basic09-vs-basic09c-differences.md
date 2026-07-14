# Real BASIC09 vs. `basic09c` — behavioral comparison

This compares the real Microware BASIC09 compiler/runtime — as documented
in the BASIC09 Reference Manual (Rev H) and OS-9 BASIC User Manual, and
live-tested this session on the actual Microware binary running under
`os9exec` (68k) — against
[`basic09c`](https://github.com/DrPitre/basic09c), a modern, standalone,
LLVM-based BASIC09 frontend maintained separately from this project. It's
written as a handoff artifact for `basic09c`'s author, using the same
test files as `tools/benchmarks/basic09-*-test.bas` in this directory
where the two overlap.

**Method:** everything here was checked by actually running code on
both sides — the real binary via `os9exec`, and `basic09c` itself,
built from source this session (`cmake` + `ninja`, Homebrew LLVM) and
exercised with small standalone `.b09` programs, cross-checked against
its own committed test suite (`test/*.test`) and source
(`src/Basic09*.cpp`) where useful. Nothing here is guessed from the
manual alone without a live check on at least one side.

## Confirmed divergences (worth `basic09c`'s author knowing about)

### `EOF()` semantics — the single biggest behavioral gap found

Real BASIC09's `EOF(#path)` behaves like C's `feof()`: it's a **sticky
flag set only by an actual failed read**, not a live position check.
Live-verified on 68k: after reading the last record in a sequential
file, `EOF(#path)` is still `FALSE`. `SEEK`ing arbitrarily far past the
end (checked up to 1000 bytes into a 6-byte file) never sets it either —
only a further `READ` attempt that actually hits the end does, raising
the standard OS-9 `E$EOF` (error 211). This was root-caused against
`os9exec`'s own file-manager source (`Source/OS9exec_core/file_rbf.c`'s
`pReof()`) to rule out an emulator position-tracking bug — the position
math there is correct in isolation; BASIC09's `EOF()` just isn't driven
by a live call to it.

**Practical consequence**: the natural idiom
`WHILE NOT EOF(#path) DO READ #path,x ... ENDWHILE` always over-reads on
its final iteration on real BASIC09 and needs `ON ERROR GOTO` wrapped
around it (or a restructured loop that checks `EOF` after the read, not
before) to run cleanly.

**`basic09c`'s `test/fileio.test` uses exactly this idiom** in
`eofloop.b09` (`WHILE NOT(EOF(#path)) DO READ #path,line ... ENDWHILE`)
and expects it to complete with no error, printing all three lines
cleanly. That means `basic09c`'s own `EOF()` implementation does **not**
replicate the feof()-style sticky-flag behavior — it must be doing a
live position/look-ahead check instead. If full behavioral compatibility
with real BASIC09 is a goal, this is the one divergence most likely to
bite a ported program: any code written and tested against `basic09c`
using the naive `WHILE NOT EOF` loop would need an `ON ERROR GOTO` added
before it would run correctly on real hardware.

### `PRINT USING`'s `B` (boolean) format — case

The manual states `B` format converts a boolean expression to the
strings `"TRUE"`/`"FALSE"` (uppercase). Live-verified on the real 68k
binary: it prints `"True"` (mixed case, capital T only) — a real
manual-vs-implementation divergence in the Microware binary itself, not
a test artifact.

`basic09c`'s `test/print-using-formats.test` asserts uppercase
`TRUE`/`FALSE` output (`# CHECK-NEXT:{{^}}TRUE {{$}}` /
`{{^}}FALSE {{$}}`) — meaning **`basic09c` matches the manual's spec
here, while the real historical binary does not**. This is a case where
`basic09c` is arguably *more* spec-compliant than Microware's own
compiler, not a bug on either side — but worth knowing if bit-for-bit
output matching against real hardware is ever a goal, since `basic09c`
would need to deliberately reproduce the real binary's quirk to match it
exactly.

### `DATE$` format and a real Y2K-class bug in the real binary

Real BASIC09's `DATE$` uses a 2-digit year: `"YY/MM/DD HH:MM:SS"` (17
characters, confirmed via `LEN()`). Live-verified on the real 68k binary
with the actual system date at 2026-07-14: `DATE$` returned
`"<6/07/14 10:05:04"` instead of `"26/07/14 10:05:04"` — confirmed at
the byte level via `ASC(MID$(...))`: the first character is `60` (`'<'`),
not the expected `50` (`'2'`). Root-caused against `os9exec`'s own
`Get_Time()` (`Source/OS9exec_core/utilstuff.c:701,744`): the packed
date's year field correctly holds the full 4-digit year (2026) in a
16-bit slot, so `os9exec`'s own date packing is not the bug. The bug is
almost certainly baked into BASIC09's own `DATE$` formatting code, which
likely assumes a 2-digit year (0-99) and was never updated for years past
1999 — a genuine Y2K-class bug that would very likely reproduce on real
6809 hardware too, not just this emulator.

`basic09c`'s `emitDate()` (`src/Basic09IR.cpp:2560`) uses `strftime`
with format `"%Y/%m/%d %H:%M:%S"` — a **4-digit** year, giving a
19-character string, not 17. This means:
- `basic09c` does **not** inherit the Y2K bug (it computes the year
  correctly).
- But it also does not match the real `DATE$` format's length or field
  widths at all. Any real BASIC09 program that parses `DATE$` by fixed
  character position (e.g. `MID$(DATE$,1,2)` for the year, or anything
  assuming the 17-character total) would get different results — or
  outright wrong slices — under `basic09c` versus real hardware. If
  byte-for-byte compatible `DATE$` output matters, `basic09c` would need
  to switch to a 2-digit year and 17-character layout (deliberately
  *not* reproducing the Y2K bug, presumably — worth a design decision
  either way rather than an accidental format mismatch).

### `PRINT USING`'s center-justify symbol — independently confirmed

The manual's own text renders the third justify symbol as a garbled
"degree sign" (OCR corruption), leaving it ambiguous. Live-tested on the
real 68k binary to resolve it: `'^'` compiles and genuinely centers
(`"HI"` in an 8-wide `S8^` field → `"   HI   "`, 3 spaces each side);
`'%'` is a runtime syntax error. **`basic09c`'s own
`print-using-formats.test` independently uses `'^'`** for center-justify
(`"R10.2^"`, `"I4^"`, `"S8^"`) — this is strong independent corroboration
from a completely different source (a from-scratch reimplementation)
that `'^'` is indeed the real symbol the manual's OCR mangled, not a
guess either side made in isolation.

## Live-verified matches (basic09c gets these right)

These were confirmed correct on both real BASIC09 (live-tested this
session) and in `basic09c`'s test suite / source — listed because they
were genuinely uncertain or previously wrong in this project's own
documentation before being fixed, so it's worth recording that
`basic09c` already has them right:

- **`PRINT USING`'s real format-string grammar.** This is the biggest
  one: an earlier pass in this project's own BASIC09 documentation
  assumed a generic-BASIC `"###.##"` pound-sign format string — that was
  a complete fabrication, never real BASIC09 syntax. `basic09c`'s
  `print-using-formats.test` already uses the correct directive-letter
  grammar throughout (`R8.2`, `I4`, `H4`, `S8`, `E12.3`, `B5`) with no
  trace of the pound-sign style ever having been assumed — confirmed by
  building `basic09c` and running a subset of that test's own program
  directly: field widths, hex output, and center-justify all matched
  its expected output exactly.
- **`PRINT #path USING fmt, list` path-number placement** — `#path`
  right after `PRINT`, before `USING`. Confirmed in `basic09c`'s parser
  handling and matches the manual's real grammar.
- **`TYPE` records**: one-line, semicolon-separated field declarations,
  including nested `TYPE`-in-`TYPE` fields and array fields inside a
  record (`basic09c`'s `test/recordio.test` builds a `widget` record
  containing a nested `point` and a `samples(4):BYTE` array field —
  exactly the same pattern confirmed live on real BASIC09 in
  `basic09-type-test.bas`). No `ENDTYPE` block form assumed anywhere in
  `basic09c`.
- **Random-access `PUT`/`GET`/`SEEK` with `TYPE` records**, including
  round-tripping a record through a file and reading it back
  byte-identical (`basic09c`'s `test/recordio.test` explicitly checks
  `cmp` on the two written files for byte-identical output — a stronger
  check than this project's own live test, which only verified field
  values after `GET`, not the raw on-disk bytes).
- **`BYTE` is unsigned (0-255)** and widens to `INTEGER`/`REAL` by
  zero-extension, not sign-extension — matches real BASIC09 exactly.
- **`RESTORE <line_number>`** — jump to a specific numbered `DATA`
  statement, not just rewind to the first one. `basic09c`'s
  `test/restore-line.test` exercises this; this project's own live
  testing only checked bare `RESTORE` (no line-number argument), so this
  is a genuine gap in what we verified live, not a confirmed match — but
  the skill's documented grammar (`RESTORE [line_number]`) is consistent
  with `basic09c`'s behavior.
- **`ON ERROR GOTO` / bare `ON ERROR`** disarming — both sides use the
  same statement pair for arm/disarm, matching the manual.

## Structural differences (inherent, not bugs on either side)

`basic09c` compiles BASIC09 source directly to a native host executable
via LLVM. Real BASIC09 is an interactive I-code interpreter running
inside the OS-9 environment. Several differences follow directly from
that and aren't behavioral bugs to fix — they're consequences of
targeting a fundamentally different execution model:

- **No Debug Mode equivalent.** Real BASIC09 has an interactive Debug
  Mode (`PAUSE`, `TRON`/`TROFF`, `BREAK`, `STATE`, the `D:` prompt) that
  an untrapped runtime error drops into. `basic09c` compiles to a
  standalone binary with no such interactive layer — its
  `test/onerror.test` shows an untrapped error (a second `SHELL "false"`
  with no `ON ERROR` armed) simply lets execution continue to the next
  statement, since there's no debugger to fall into. This can't be made
  to match real BASIC09 without `basic09c` growing its own interactive
  debugger, which is presumably out of scope for a compiled-executable
  tool.
- **Entry-procedure `PARAM` mapped to `argv`.** `basic09c`'s
  `test/entry-procedure-param.test` shows the entry procedure's `PARAM`
  list read positionally from the command line
  (`entryparam 42 3.5 hello 200`), printing a usage message if too few
  are given. Real BASIC09 has no equivalent concept — a procedure is
  invoked interactively via `RUN`, or as an OS-9 module receiving
  OS-9-style parameters, never from a host shell's `argv`. This is
  `basic09c`'s own necessary adaptation for standalone execution, not
  something to reconcile against the manual.
- **File I/O targets host files directly** (via the host C library, not
  OS-9's RBF/file-manager path-descriptor model). Binary `PUT`/`GET`
  layout should still match since both mirror the same in-memory
  representation, but OS-9-specific file attributes (execute permission
  bits, OS-9 directory-entry format) don't apply under `basic09c`.

## `basic09c` was built and run directly to answer the rest

Everything below was checked by actually building `basic09c` from source
(`cmake` + `ninja`, Homebrew LLVM, this session) and compiling/running
small `.b09` programs through it — not inferred from reading source
alone.

### `INTEGER` is 16-bit in `basic09c` — matches the *original 6809* spec, not the 68k port

`basic09c`'s type-lowering code (`Basic09IR.cpp`, the `BasicType ==
"INTEGER"` case) maps `INTEGER` to LLVM `i16`, not `i32`. That's the
**original 6809 BASIC09 width** (confirmed via the manual and this
project's own 6809 documentation), not the 32-bit width the Microware
68k port uses (confirmed live this session: `SIZE(INTEGER)=4` on the
68k binary). This is the single most important thing to know before
comparing any numeric behavior between `basic09c` and the 68k binary
tested elsewhere in this document — they're testing genuinely different
target widths, both legitimately "real BASIC09," just different
editions. (First surfaced when `i:=100000; r:=i/3` gave a nonsensical
`-10357` under `basic09c` — `100000` silently overflows a 16-bit
INTEGER at the assignment, before division ever runs. Retested with
16-bit-safe values below.)

### Divide-by-zero — `basic09c` never crashes, on either type

Real BASIC09 (68k, `os9exec`): `INTEGER÷0` silently falls through with
no error; `REAL÷0` **crashes the entire process** via an uncatchable
68k CPU trap (`E_TRAPV`), never reaching `ON ERROR GOTO`.

`basic09c` (tested live): `INTEGER÷0` also falls through with no error,
leaving the result as `0` (real BASIC09's left-behind value in this case
was never checked, so this specific number isn't a confirmed
divergence). `REAL÷0` computes ordinary **IEEE-754 `inf`** and keeps
running normally — no crash at all. This is a real, load-bearing
divergence: a BASIC09 program that (accidentally or on purpose) divides
a REAL by zero crashes outright on real hardware but silently produces
`inf` and continues under `basic09c`. Any test or program relying on
"REAL÷0 crashes / never happens" behaves completely differently under
`basic09c`.

### `BOOLEAN` in a numeric expression — `basic09c` allows it, silently coercing `TRUE`→`1`

Real BASIC09: compile-time error (`Error #000:067 E_ILLARG`) — the
program never starts running.

`basic09c` (tested live): `n:=flag+1` with `flag:BOOLEAN=TRUE` compiles
with zero errors or warnings at any stage (`--syntax-only`,
`--analyze-only`, `--compile` all clean) and runs, printing `2` —
`TRUE` silently coerces to `1`. A real, confirmed semantic gap: code
that (by accident) mixes a BOOLEAN into arithmetic is rejected outright
on real hardware but runs to completion under `basic09c`.

### Division truncates before widening — CONFIRMED MATCH

Real BASIC09: `r = i / 3` (both effectively INTEGER) truncates first,
then widens the already-truncated result into a REAL destination.

`basic09c` (tested live, using a 16-bit-safe value given the INTEGER
width difference above): `i:=1000; r:=i/3` → `333` (truncated);
`r:=i/3.0` → `333.333` (REAL operand forces real division). Exact match
to real BASIC09's rule.

### INTEGER overflow direction — this project's own long-standing discrepancy, now resolved in one direction

The manual claims 68k INTEGER overflow wraps to a *positive* value;
live testing on the real 68k binary showed ordinary negative
two's-complement wrap instead — previously left as an "unresolved
discrepancy" in this project's own documentation.

`basic09c` (tested live, at its own 16-bit width): `i:=32767; i:=i+1` →
`-32768` — ordinary negative two's-complement wrap, matching the real
68k binary's behavior and NOT the manual's claim. Two independent
implementations (a real Microware binary and a from-scratch
reimplementation) now agree with each other and disagree with the
manual — strong evidence the manual's "positive wrap" claim is simply
wrong, though this still isn't proof of real 6809 hardware behavior (no
6809 emulator exists in this project to check directly).

### The GOTO+GOSUB+ON ERROR GOTO combined-construct compile bug does NOT reproduce in `basic09c`

Real BASIC09 hits a compile-time `Error #000:069 (Unmatched Control
Structure)` when GOTO, GOSUB, and ON ERROR GOTO with numbered-line
targets are all used together in one procedure, even though each works
individually in isolation. Never root-caused on the real binary
(proprietary, no source access).

`basic09c` (tested live): the equivalent combined construct compiles
with zero errors and runs correctly. This supports the theory that the
real binary's error is a quirk/limitation in the Microware compiler's
own internal control-structure validation, not a genuine language-level
restriction — a from-scratch reimplementation with no reason to
replicate an undocumented internal quirk simply doesn't hit it.

### `RND(n<0)` does NOT reseed in `basic09c` — and its RNG isn't seeded from entropy at all

Real BASIC09: `RND(n<0)` reseeds the generator using `ABS(n)` as the
seed; live-verified fully deterministic (same seed reproduces an
identical subsequent `RND(0)` sequence), and reseed calls return a
value `>=0`.

`basic09c`'s `RND` implementation (`Basic09IR.cpp`) calls `drand48()`
for `RND(0)` and computes `drand48() * n` for `RND(n)` with **no
special-casing for negative `n` at all** — there is no reseed logic.
Live-verified: `RND(-42)` returns `-35.3004` — a **negative** value
(since `drand48()` is always in `[0,1)`, multiplying by a negative `n`
gives a negative result), a complete divergence from the documented
reseed contract. **Separately**, `basic09c`'s RNG appears to have no
entropy-based seeding at program start at all: running the exact same
compiled program twice, as two entirely separate processes, produced
the identical "random" sequence both times (`1.98232, -35.3004,
-14.8401`, byte-for-byte the same). This looks like an unintentional
bug rather than a deliberate design choice — `drand48()` without an
explicit `srand48()` call uses a fixed default seed per the C standard,
so every run of every `basic09c`-compiled program currently produces
the same "random" numbers. Worth flagging directly: this would silently
break any program relying on `RND` for actual randomness (games,
simulations, anything non-deterministic) across runs.

## Source material

All real-BASIC09 findings above come from live testing on the actual
Microware binary via `os9exec` this session — see the individual
`tools/benchmarks/basic09-*-test.bas` files for the exact source and
captured output each finding is based on, and
`~/.claude/skills/os9-dev/references/basic09/basic09-language.md` /
`gotchas.md` for the fuller write-up of each. All `basic09c` findings
come from building `basic09c` from source this session and running
small test programs through it directly, cross-checked against its
`test/*.test` lit tests and `src/Basic09*.cpp` source (as of this
session; `basic09c` is under active development and any of this may
have already changed).

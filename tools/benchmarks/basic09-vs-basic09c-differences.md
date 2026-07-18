# BASIC09: 6809 vs. `basic09c` vs. 68k

Three implementations, tested live: real BASIC09 on 6809 (NitrOS-9 under
XRoar), `basic09c` (independent BASIC09-to-LLVM compiler), real BASIC09
on 68k (`os9exec`).

| Behavior | 6809 | `basic09c` | 68k |
|---|---|---|---|
| `EOF()` | `TRUE` right after last `READ`, or after `SEEK` past end. Loop works. | Same as 6809. | `TRUE` only after a failed `READ`. `SEEK` never sets it. Loop over-reads; needs `ON ERROR GOTO`. |
| `REAL÷0` | `Error #045`, catchable. | Returns `inf`, keeps running. | Crashes the process. Not catchable. |
| `INTEGER÷0` | `Error #045`, catchable — same as `REAL÷0`. | Undefined: `0` at `-O0`, garbage at `-O2` (C's div-by-zero UB). | No error. Saturates at `2147483647`. |
| `INTEGER` width | 16-bit: `-32768` to `32767`. | 16-bit, same as 6809. | 32-bit: `-2147483648` to `2147483647`. |
| `BOOLEAN` in numeric expr | Compile error (`#067`). | Compiles. `TRUE` → `1`. | Compile error (`E_ILLARG`). |
| `PRINT USING` boolean | `True` | `TRUE` | `True` |
| `SQR` vs `SQRT` | Same function, stored as `SQRT`. | Same function, both names work. | Same as 6809. |
| `DATE$` | Correct: `26/07/16...` | Correct, different layout (`strftime`). | Wrong leading year digit (e.g. `<6/07/16`). |
| `RND(n<0)` | Reseeds with `ABS(n)` — same call repeats. | No reseed — just `drand48()*n`, negative. | Same as 6809. |
| `IF cond THEN GOTO n`, no `ENDIF` | Rejected (`#069`). | Accepted, runs. | Rejected (`#069`), same as 6809. |
| Untrapped runtime error | Drops to Debug Mode. | Keeps running (no Debug Mode). | Same as 6809. |
| `PARAM`, bare shell call (`prog 42`) | `STRING` OK. `INTEGER` broken — gets raw bytes, not the number (`42`→`13362`). | `argv`, parsed correctly. | Parsed correctly (`42`→`42`). |
| Trailing `!` comment | Compile error (`#034`). | Compiles — breaks only right after a `DIM` type. | Compile error, same rejection. |
| Standalone `!` comment line | Fine anywhere. Shown as `REM`. | Parses, but won't compile to native code. | Same as 6809. |
| `LOAD`, comment as file's first line | Breaks the whole `LOAD` (`#043`). | No `LOAD` concept. | Comment dropped, rest loads fine. |

Also matching, all three: `RND(n>0)` — REAL in `[0,n)` everywhere
(`basic09c`'s `drand48()*n` lands in the same range); `INTEGER` overflow
— silent two's-complement wrap at each target's own width, no error.

Matching, 68k and `basic09c` only: `PRINT USING` directive letters
(`R8.2`/`I4`/`S8`/`H4`/`B8`/`E12.3`), `^` center-justify, `TYPE` I/O via
`PUT`/`GET`/`SEEK`, division truncating before REAL widening,
`RESTORE <line>`, `RND(0)` not seeding from entropy.

## Speed

Same program logic on all three (`basic09c`'s 16-bit `INTEGER` needs a
nested loop for large counts; the 6809 tests match that width).

### Trig-heavy (`SIN`+`COS`+`SQR`+`LOG`+`EXP`+`ATN` per iteration)

| Implementation | Iterations | Time | Per-iteration |
|---|---|---|---|
| 6809, throttled (default XRoar) | 500 | 45.7s avg | 91.4 ms |
| 6809, unthrottled | 5,000 | 16.6s avg | 3.32 ms |
| 68k | 50,000 | 32.7s | 654 µs |
| `basic09c`, `-O2` | 100,000,000 | 1.30s avg | 13.0 ns |

Native is ~50,300× faster than 68k, ~7,030,000× faster than 6809
throttled, ~255,700× faster than 6809 unthrottled.

### Integer-only (`LAND`/`MOD`/add)

| Implementation | Iterations | Time | Per-iteration |
|---|---|---|---|
| 6809, throttled (default XRoar) | 20,000 | 22.7s avg | 1.14 ms |
| 6809, unthrottled | 600,000 | 22.6s avg | 37.6 µs |
| 68k | 500,000 | ≤8.6s | ≤17 µs |
| `basic09c`, `-O2` | 18,000,000,000 | 2.19s avg | 0.122 ns |

Native is ≥139,300× faster than 68k, ~9,340,000× faster than 6809
throttled, ~308,200× faster than 6809 unthrottled.

Trig-vs-integer ratio (per-iteration time): 68k ≈38×, 6809 ≈80-88×,
native ≈107×. 68k has hardware multiply/divide, so its integer path is
cheap and dispatch overhead dominates; 6809 doesn't, so its integer path
stays relatively expensive, closer to native's ratio.

Both XRoar figures measured on a quiet host. Throttled paces to a
simulated clock; unthrottled runs at full host speed, same as `os9exec`
and `basic09c` — neither is a historical-hardware benchmark.

Source: `tools/benchmarks/basic09-trigbench.bas`, `basic09-intbench.bas`
(68k side, in this repo). 6809/`basic09c` sources aren't committed —
built ad hoc.

## PACK + argument passing

On 68k, both `runb <name> <args>` and bare `<name> <args>` pass
command-line args into `PARAM`, positionally — numbers and strings both
parse correctly. On 6809, bare invocation mis-parses `INTEGER` args (see
table above); `runb <name> <numeric-arg>` fails outright with a
different error (`Error #000`), not yet diagnosed.

`PACK proc1,proc2` (named list) always picks the first-listed procedure
as entry point; `PACK*` (wildcard) picks whichever is current. `PACK`/
`SAVE`'s `>pathlist` form can print a misleading error even when the
write succeeds — check the file, not the error text.

Module resolution/boot/invocation details: `os9-dev` skill's
`common/module-format.md`.

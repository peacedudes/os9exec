# Authored GOTCHA cards — the OS-9 `debug` symbolic debugger

Written directly by Claude from a live investigation (2026-07-13) using a
purpose-built test program with three named functions, compiled `cc -g` and
driven under `debug` in `os9exec`. Every address below was read off a
running debugger and cross-checked against the module header.

These cards **correct** an earlier, wrong belief held in this project's own
notes — that `b <symbolname>` was broken and one should use `sc`'s addresses
instead. The truth is the exact opposite. That inversion is itself the most
useful thing here.

`verify: VERIFIED` per the design spec's honesty ledger.

--- CARD ---
id:        debug-breakpoint-by-symbol-name-works
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     debugging
claim:     In the OS-9 `debug` symbolic debugger, setting a breakpoint by symbol name (`b main`) resolves correctly and stops exactly on the function's entry. It is the right way to set a breakpoint. What is NOT safe is taking an address out of the `sc` (list code symbols) listing and using it as a breakpoint address — `sc`'s addresses are wrong (see [[debug-sc-double-counts-the-relocation]]).
context:   Verified against three named functions, each of which stopped precisely on its `link.w a5,#0` prologue, confirmed both by the disassembler's own labelling and by register state at the stop. This is worth stating explicitly because this project previously documented the opposite advice — "b <symbolname> resolves to a wrong address, use sc's absolute address instead" — and that advice sends a breakpoint into the middle of an unrelated library function, where it can never be hit. Once `b <name>` is used, `g` (run to breakpoint) works exactly as expected too.
source:    authored
--- END ---

--- CARD ---
id:        debug-sc-double-counts-the-relocation
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     debugging
claim:     `debug`'s `sc` command prints code-symbol addresses that are wrong: it double-counts the module's load-base relocation, printing `real_address + symbol_offset` — i.e. applying the relocation to a symbol value that has already been relocated. The symbol NAMES it lists are fine; only the addresses are corrupt.
context:   Verified exactly on four symbols of a test module (base 0x1D280; entry PC 0x1D312 = base + the module header's exec offset 0x92). `sc` reported `main` at 0x1D8DC when it actually sat at 0x1D5AE; disassembling 0x1D8DC showed `sprintf+0x22E`, deep inside an unrelated library function. **The error grows the further into the module a symbol lies**, so it does not present as a constant offset and looks entirely plausible in a listing -- which is precisely how it went undiagnosed and got another feature blamed instead. Purely an arithmetic bug inside the shipped 1980s `debug` binary; the emulator cannot see or fix it, and does not need to, since breakpoints by name work.
source:    authored
--- END ---

--- CARD ---
id:        disassemble-an-address-before-trusting-it
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     debugging
claim:     Before using any address a tool hands you -- from a symbol listing, a map file, or a debugger command -- disassemble it (`di <addr>`) and check that what is there makes sense. A function entry should look like a prologue. If the disassembly lands mid-instruction, or inside a different symbol, the address is wrong and the tool is lying to you.
context:   This one habit is what cracked a bug that had been misdiagnosed for two sessions: two of a debugger's own subsystems disagreed about where a function lived, and disassembling both candidate addresses settled it in seconds -- one was a clean `link.w a5,#0` prologue, the other was the middle of `sprintf`. Symbol tables, relocation and load bases give tools many chances to be confidently wrong, and an address carries no evidence of its own correctness. The disassembly does.
source:    authored
--- END ---

--- CARD ---
id:        debug-gs-is-run-to-fallthrough-not-single-step
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     debugging
claim:     `debug`'s `gs` command is not a single-step, despite the name. It plants a temporary breakpoint at the fall-through address (`PC + length of the instruction at PC`) and lets the child run until that address is reached. So it steps *over* `bsr`/`jsr` calls, and across a *taken* branch it keeps executing until the fall-through happens to be reached anyway -- which may be a whole loop iteration later, or never.
context:   "Never" is the case that bites: if the fall-through address is not executed at all -- dead code following an unconditional `bra`, for instance -- the child simply runs on. Confirmed live: `gs` on a `bra.w` whose fall-through was an unreachable `nop` ran an entire loop iteration before an unrelated breakpoint happened to catch it. Practical guidance: use `gs` for straight-line code and do not trust it across a branch; prefer setting a breakpoint by name and using `g` to land on a chosen spot. In os9exec a runaway of this kind is always recoverable with Ctrl-C.
source:    authored
--- END ---

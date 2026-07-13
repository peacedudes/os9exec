# Exceptions, interrupts, and traps

38 raw cards in -> 14 kept. 11 dropped as mis-bucketed (BASIC-level ON ERROR GOTO,
C-library signal()/intercept() and signal-number tables belong to a separate
IPC/signals cluster; device-descriptor structure belongs to the device-driver/I-O
cluster) and 2 pruned as generic CPU-101 restatements or too vague to be useful
("interrupt-driven I/O beats polling", "atomic kernel handles thousands of
interrupts/sec"). One near-duplicate register-convention card was folded into
its more specific twin. The remaining 21 raw cards merged down into 14,
mostly by combining the same fact reported by two or three source manuals, or
by combining a table of near-identical per-vector entries (bus/address/
illegal-instruction/zero-divide) into one card. No FPU/coprocessor-exception
cards and no 6809-specific SWI/SWI2/SWI3 cards appeared in this raw bucket —
that 6809 material, if it exists, needs to come from a separate pass; nothing
here was averaged across the two architectures.

## Exception vector table and dispatch mechanism (68k)

--- CARD ---
id:        reset-vector-0-ssp
type:      FACT
target:    68k
verify:    from-manual
topic:     exception-vector-table
claim:     Exception vector 0 contains the reset initial SSP (supervisor stack pointer) value loaded at startup; there must be at least 4K of RAM below and 4K above this address for system global storage, and OS-9 uses this vector to find the base address of system global data on every exception.
context:   User programs must not use or alter this vector.
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "Reset Vectors"
--- END ---

--- CARD ---
id:        reset-vector-1-pc
type:      FACT
target:    68k
verify:    from-manual
topic:     exception-vector-table
claim:     Exception vector 1 contains the reset initial PC — the coldstart entry point into OS-9. After startup its only use is to restart the system after a catastrophic failure.
context:   User programs must not use or alter this vector.
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "Reset Vectors"
--- END ---

--- CARD ---
id:        error-exceptions-vectors-2-5
type:      FACT
target:    68k
verify:    from-manual
topic:     exception-vector-table
claim:     Vectors 2-5 are the CPU hardware error exceptions, each with a Microware trap-name macro: vector 2 = bus error (T_BUSERR, access to non-existent/privileged memory), vector 3 = address error (T_ADDERR, misaligned access e.g. a word access at an odd address), vector 4 = illegal instruction (T_ILLINS), vector 5 = zero divide (T_ZERDIV, DIV/DIVS by zero). All four dispatch through F$STrap and are normally fatal: the offending user process is unconditionally terminated.
context:   If the process was created via F$DFork, its resources remain intact and control returns to the parent debugger for post-mortem examination instead of the process simply vanishing.
source:    the independent 1985-era OS-9/68000 technical manual, §4-7 "Error Exceptions"; The OS-9 Primer, Chapter 14 "Hardware Exception Types"
--- END ---

--- CARD ---
id:        privilege-violation-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exception-vector-table
claim:     A privilege violation exception (T_PRIV) occurs when a privileged instruction is executed while the CPU is in user state.
context:   -
source:    The OS-9 Primer, Chapter 14 "Hardware Exception Types"
--- END ---

--- CARD ---
id:        trace-exception-vector-9
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exception-vector-table
claim:     Exception vector 9 is the trace exception, triggered when the status register's trace bit is set, letting the MPU single-step instructions. OS-9's F$DFork, F$DExec, and F$DExit system calls control program tracing through this exception, and it is what the OS-9 debugger uses to implement single-stepping.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "The Trace Exception"; OS-9 v2.4 Technical Reference Manual, "The Trace Exception", p. 2-32
--- END ---

## Trap instruction conventions (F$OS9 system calls / F$TLink user traps)

--- CARD ---
id:        os9-syscall-trap0
type:      FACT
target:    68k
verify:    from-manual
topic:     trap-instruction-conventions
claim:     Exception vector 32 (TRAP #0) is reserved by OS-9 for standard system-call dispatch through the F$OS9 entry point: user code loads a system-call number and executes TRAP #0, and the kernel routes to the matching kernel function. The assembler recognizes `OS9 <func>` as a macro that expands to `TRAP #0 / dc.w <func>`, which is how calls like F$Sleep are actually invoked in source.
context:   This is the primary and only sanctioned mechanism for user programs to invoke OS-9 kernel services on 68k.
source:    the independent 1985-era OS-9/68000 technical manual, §4-9 "User Traps"; OS-9 v2.4 Technical Reference Manual, Figure 2-4 & 2-5, p. 2-29 to 2-31; The OS-9 Guru, §10.3, p. 200; The OS-9 Primer, Chapter 12 "Global variable pointer"
--- END ---

--- CARD ---
id:        user-trap-vectors-1-15
type:      FACT
target:    68k
verify:    from-manual
topic:     trap-instruction-conventions
claim:     The 68000 family provides 16 software-trap exception vectors (32-47, i.e. TRAP #0-#15). Vector 32/TRAP #0 is reserved for OS-9 system calls; vectors 33-47 (TRAP #1-15) are available for user-defined trap handlers installed dynamically via F$TLink. By Microware convention, TRAP #13 is CIO and TRAP #15 is the math trap.
context:   Trap handlers reduce program size versus static linking (the handler is shared rather than duplicated per program), but calling through a trap executes slightly slower than a direct static call.
source:    OS-9 v2.4 Technical Reference Manual, "Trap Handlers", p. 5-1; the independent 1985-era OS-9/68000 technical manual, §4-9 "User Traps"
--- END ---

--- CARD ---
id:        trap-assembly-mnemonics
type:      ROSETTA
target:    68k
verify:    from-manual
topic:     trap-instruction-conventions
claim:     OS-9's relocatable assembler gives two equivalent macro forms for trap calls, both of which keep the underlying TRAP instruction visible in source rather than hiding it: `OS9 <func>` expands to `TRAP #0 / dc.w <func>` (e.g. `OS9 F$TLink` ≡ `TRAP #0 / dc.w F$TLink`), and `tcall <trapnum>,<func>` expands to `TRAP <trapnum> / dc.w <func>` (e.g. `tcall T$Math,T$DMul` ≡ `TRAP #5 / dc.w T$DMul`).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "OS9 and tcall: Equivalent Assembly Language Syntax", p. 5-3
--- END ---

--- CARD ---
id:        trap-handler-linking-performance
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     trap-instruction-conventions
claim:     Programs can link their own custom trap handlers through the OS-9 API (F$TLink); in some cases calling through a linked trap handler is measurably faster than an equivalent direct function call.
context:   -
source:    OS-9 Insights §14 "Traps"
--- END ---

## IRQ polling and device interrupt handling

--- CARD ---
id:        autovectored-interrupt-ranges
type:      FACT
target:    68k
verify:    from-manual
topic:     irq-polling
claim:     68k interrupt vector numbering: 25-31 are the seven autovectored interrupt levels (1-7) used by devices that can't generate a vectored interrupt — these are serviced through F$IRQ and handled internally identically to vectored interrupts; 57-63 are the 68070's on-chip autovectored interrupts (levels 1-7, 68070 only); 64-255 are ordinary vectored interrupts.
context:   These vector numbers are 68k-specific and appear in a device descriptor's M$Vector field.
source:    the independent 1985-era OS-9/68000 technical manual, §4-9 "AutoVectored Interrupts"; Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-10; OS-9 v2.4 Technical Reference Manual, Figure 2-4 & 2-5, p. 2-29 to 2-31
--- END ---

--- CARD ---
id:        level-7-interrupt-nonmaskable-gotcha
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     irq-polling
claim:     Level 7 (vector 31) is non-maskable and can interrupt the kernel at dangerous times, including during critical kernel sections; OS-9 I/O devices should not normally be configured at level 7. It's reserved for hardware the system doesn't need to coordinate with, e.g. dynamic-RAM refresh — and if used that way, the level-7 IRQ service routine must not call any OS-9 system call or touch system data structures.
context:   Levels 1-6 are maskable and safe for ordinary device use.
source:    the independent 1985-era OS-9/68000 technical manual, §4-9 "AutoVectored Interrupts"; Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-11; OS-9 v2.4 Technical Reference Manual, "AutoVectored Interrupts", p. 2-32
--- END ---

--- CARD ---
id:        driver-irq-register-conventions
type:      FACT
target:    68k
verify:    from-manual
topic:     irq-polling
claim:     The IRQ routine called by kernel interrupt polling receives (a2) = driver static storage, (a3) = device port address, (a6) = system global storage (a2/a3 are whatever values were supplied at F$IRQ installation). It may destroy only d0, d1, a0, a2, a3, a6, and reports whether it handled the interrupt via the carry flag: clear if serviced, set if not (the kernel continues polling the next handler).
context:   Register conventions between an ISR and ordinary C code differ, so extra care is needed whenever an ISR calls into C or C code is entered from one.
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-23; The OS-9 Primer, Chapter 15 "Register conventions between ISRs and C code"
--- END ---

--- CARD ---
id:        interrupt-masking-cpu-level
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     irq-polling
claim:     OS-9 masks interrupts to protect critical kernel operations at the CPU level, not merely via a software flag.
context:   -
source:    OS-9 Insights §18.6 "Masking Interrupts"
--- END ---

--- CARD ---
id:        interrupt-latency-factors
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     irq-polling
claim:     Interrupt latency increases when signals are masked, when a system call that masks interrupts is currently active, or while the kernel is inside a critical section.
context:   -
source:    OS-9 Insights §21.4 "Interrupt Latency"
--- END ---

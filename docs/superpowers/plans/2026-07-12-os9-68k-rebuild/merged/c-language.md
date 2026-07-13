# The OS-9 C compiler and library

38 raw cards in (35 from c-language-RAW.md + 3 pulled over from
misc-unclustered-RAW.md's 6809 data-type/storage-format cards) -> 17 kept.
19 dropped as mis-bucketed: 4 were BASIC09-language cards (RUN syntax, string
concatenation, RND, transcendental-function accuracy) and 15 were OS-9 BASIC
string/math functions and Math-module (T$xxx) system-call tables — none of
these are C-compiler or C-library facts, they were swept in by keyword match
alone (e.g. "float", "long") and belong to the BASIC09 and math-module
clusters instead. 3 raw cards were merged into 1 (the three 6809 data-size/
naming-convention cards collapsed into a single data-types card), for a net
reduction of 2. No card was pruned merely for restating generic K&R C that a
competent LLM already knows — everything that survived the mis-bucketing cut
is an OS-9- or Microware-compiler-specific delta.

**Source-material warning:** the underlying "OS-9 C Compiler manual" cited
throughout is explicitly a **6809** (1983) compiler manual. Its data-type
sizes, floating-point format, and several code-generation quirks are
hardware/compiler-vintage facts that do **not** carry over to the 68k
compiler — this project has independently verified via `sizeof()` on a real
68k toolchain that `int`, `long`, and all pointer types are 32-bit on OS-9/68k,
not the 16-bit `int` the 6809 manual describes. Cards below are tagged
`target: 6809` wherever the fact is hardware/vintage-specific, and
`target: all` or `target: 68k` where the fact is a genuine OS-9-wide
convention (calling philosophy, library gotchas, linker tricks, register
ABI) confirmed to apply regardless of, or specifically to, the 68k.

Target distribution in the original 17: 4 `all`, 7 `68k`, 6 `6809`. Three
more `target: 6809` cards were folded in afterward from `assembly-RAW.md`
(register-allocation and optimizer facts mis-bucketed there by keyword
match on "register") — final total 20 cards: 4 `all`, 7 `68k`, 9 `6809`.

## Calling conventions and register usage (68k)

--- CARD ---
id:        d1-first-parameter
type:      FACT
target:    68k
verify:    from-manual
topic:     c-interface
claim:     Register D1 is used to pass the first parameter to an assembly routine when called from C.
context:   -
source:    The OS-9 Primer, Chapter 12: "Register conventions"
--- END ---

--- CARD ---
id:        d0-d1-return-value
type:      FACT
target:    68k
verify:    from-manual
topic:     c-interface
claim:     In 68k OS-9 C code, register D0 holds the return value of a function; for 64-bit return values, D0 holds the low 32 bits and D1 holds the high 32 bits.
context:   -
source:    The OS-9 Primer, Chapter 12: "Register conventions"
--- END ---

--- CARD ---
id:        a5-frame-pointer
type:      FACT
target:    68k
verify:    from-manual
topic:     c-interface
claim:     Register A5 is the frame pointer in 68k OS-9 C code: local variables and function parameters are addressed relative to A5 set up by the link instruction.
context:   -
source:    The OS-9 Primer, Chapter 12: "Frame pointer"
--- END ---

--- CARD ---
id:        a6-global-base
type:      FACT
target:    68k
verify:    from-manual
topic:     c-interface
claim:     Register A6 is the global variable pointer in 68k OS-9: it is initialized to point to the base of global variables plus 0x8000, allowing direct access to 64k of globals using A6-based addressing.
context:   The 0x8000 offset allows use of 16-bit signed offsets to reach globals in the range 0x8000..0xffff bytes from A6.
source:    The OS-9 Primer, Chapter 12: "Global variable pointer"
--- END ---

## Data types and storage (6809 vs. 68k)

--- CARD ---
id:        c-data-type-sizes-and-naming-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     On the 1983 6809 C compiler, CHAR is 1 byte, INT and UNSIGNED are 2 bytes, LONG and FLOAT are 4 bytes, and DOUBLE is 8 bytes (CHAR/INT/LONG stored two's-complement; UNSIGNED stored plain unsigned binary; char sign-extended for comparisons, giving it range -128 to 127). Following PDP-11 convention, the compiler treats SHORT and SHORT INT as synonyms for plain INT, LONG INT as a synonym for LONG, and LONG FLOAT as meaning DOUBLE. BASIC09's INTEGER type and this compiler's int are documented as identical (both 2-byte); BASIC09's BYTE and BOOLEAN are likewise identical to char.
context:   These sizes, the naming synonyms, and the BASIC09<->C type equivalence are explicitly 6809-only facts. This project has independently confirmed int/long/pointer are all 32-bit on OS-9/68k — none of this table's figures, synonyms, or the BASIC09 equivalence carry over to a 68k C compiler.
source:    OS-9 C Compiler manual, "Data Representation and Storage Requirements" p. 1-5, and "Interfacing to BASIC09" p. C-1
--- END ---

--- CARD ---
id:        float-double-binary-format-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     The 6809 compiler's FLOAT and DOUBLE are a proprietary sign-magnitude binary floating-point format (not IEEE 754), with an exponent biased by 128. DOUBLE (8 bytes) uses a 7-byte mantissa (with an implied leading one bit) plus a 1-byte exponent; FLOAT (4 bytes) is identical but with a 3-byte mantissa, produced from a DOUBLE by truncating (FLOAT<-DOUBLE) or zero-padding (DOUBLE<-FLOAT) the least-significant mantissa bytes.
context:   A 6809-C-compiler-specific floating-point representation from 1983; do not assume it matches a 68k Microware C compiler's float format, which may instead follow 68000/68881 IEEE-754-oriented conventions.
source:    OS-9 C Compiler manual, "Data Representation and Storage Requirements", p. 1-5
--- END ---

## Dialect and code-generation differences from K&R (6809 compiler)

--- CARD ---
id:        k-and-r-deviations-1983-compiler
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This 1983 6809 C compiler deviates from the K&R specification in several ways: bit fields are not supported at all; old-style assignment operators (e.g. `=+`) are not recognized, only the modern form (`+=`); macro definitions and string literals cannot span more than one source line; and `#if <constant expression>` is not supported (only `#ifdef`/`#ifndef` ... `#else` ... `#endif`).
context:   These are this specific compiler/vintage's limitations, not general OS-9 facts; a later or 68k-targeted Microware C compiler may or may not share any of them.
source:    OS-9 C Compiler manual, "Differences From The K & R Specification", p. 1-2
--- END ---

--- CARD ---
id:        constant-expr-evaluation-limited-to-int-char
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler only performs compile-time constant folding of expressions when all operands are of type CHAR, INT, or UNSIGNED. Constant expressions involving LONG, FLOAT, or DOUBLE (beyond single constants/casts of them) are instead evaluated at runtime by the compiled program, so speed-critical code should have such values pre-computed by the programmer.
context:   A code-generation limitation of this specific compiler; not confirmed for the 68k compiler.
source:    OS-9 C Compiler manual, "Programming Considerations", p. 1-9
--- END ---

--- CARD ---
id:        struct-assignment-unsupported-strass-workaround
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler does not support direct structure assignment (`struct1 = struct2;`). The library provides a strass() function (byte-by-byte block copy) as the documented workaround for copying one structure's contents to another.
context:   A language-completeness gap specific to this early (1983) compiler; not confirmed present in a later or 68k-targeted Microware C compiler, which may support struct assignment natively.
source:    OS-9 C Compiler manual, "Strass", p. 3-41
--- END ---

## Register allocation and optimization (6809 compiler)

(3 cards folded in from `assembly-RAW.md`, mis-bucketed there by keyword
match on "register" — these are C-compiler behaviors, not raw assembly
language facts.)

--- CARD ---
id:        register-variable-single-per-function
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler permits only one `register`-class variable per function, and only for types int, unsigned, or pointer. A register declaration outside these constraints (a second register variable, or an unsupported type) is not an error — it is silently downgraded to `auto` storage.
context:   The single-register-variable limit reflects the 6809's very small general-purpose register set; an architecture with more registers (like the 68000, with 8 data + 7 address registers) would not need this restriction. Do not assume this constraint for a 68k compiler.
source:    OS-9 C Compiler manual, "Register Variables", p. 1-5
--- END ---

--- CARD ---
id:        register-variable-performance-gotcha
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     A register variable gives its biggest code-size/speed win when used as a pointer or a loop counter; when used inside a complex arithmetic expression instead, the manual states there is no saving at all from declaring it `register`.
context:   This is presented in the manual specifically in terms of the 6809's limited register file and instruction costs; the tradeoff calculus could differ substantially on a register-rich architecture like the 68000.
source:    OS-9 C Compiler manual, "Register Variables", p. 1-5
--- END ---

--- CARD ---
id:        optimizer-pass-11-percent
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     An optional post-compilation optimizer pass rewrites the generated 6809 assembly source to remove redundant code and substitute shorter/faster instruction sequences, typically shrinking object code by about 11% with a further speed increase; it can be disabled with the -O flag to speed up error-checking-only compiles.
context:   The specific 11% figure and the pass's mechanics are stated for this 6809 code generator; not necessarily representative of a 68k compiler's optimizer.
source:    OS-9 C Compiler manual, "The Optimizer Pass", p. 1-9
--- END ---

## Embedded assembly

--- CARD ---
id:        embedded-asm-pragma-convention
type:      CONCEPT
target:    all
verify:    from-manual
topic:     c-compiler
claim:     Microware's OS-9 C compiler lets a program embed raw assembly directly in C source: a line beginning "#asm" switches the compiler into pass-through mode, copying subsequent lines verbatim to the assembly output until a line beginning "#endasm" is seen.
context:   Normal compiler-generated code lives in the PSECT (code) section; if embedded assembly switches to the VSECT (variable/data) section, the programmer must emit an ENDSECT directive before "#endasm" to leave the section state correct for compiler-generated code that follows. The exact section-directive names are this compiler's assembler convention (RMA) and should be checked against the 68k toolchain, but the #asm/#endasm C-source mechanism itself is a Microware-C-family feature.
source:    OS-9 C Compiler manual, "Imbedded Assembly Language", p. 1-4
--- END ---

--- CARD ---
id:        asm-in-c-caveat
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     c-interface
claim:     The _asm() function in the Ultra C compiler (68k) embeds assembly inside C code, but the compiler does not optimize assembly; embedded assembly may be inadvertently damaged by compiler optimization of surrounding C code.
context:   Do not embed assembly directly inside C function bodies; use separate assembly files or linking instead. (Starting with Ultra C 1.2, macro-form assembly inside functions is supported.) This is a distinct mechanism from the 6809 compiler's #asm/#endasm pragma above — a different compiler generation with its own embedding facility and its own caveat.
source:    The OS-9 Primer, Chapter 12: "Placing assembly directly within a C code module"
--- END ---

## Standard library conventions

--- CARD ---
id:        system-call-name-portability-intent
type:      CONCEPT
target:    all
verify:    from-manual
topic:     c-compiler
claim:     The C library's system-call wrapper names are deliberately chosen for portability with UNIX-style code rather than to match OS-9's own assembly-language service-request names; where no UNIX call maps directly, the library simulates the behavior, and where OS-9 has no UNIX equivalent, OS-9's own naming is used. A cross-reference between C wrapper names and OS-9 assembler call names (F$xxx/I$xxx) is provided for programmers already familiar with OS-9 assembly.
context:   Programmers already fluent in OS-9 assembly should take particular care, since parameters/return values of a "familiar-looking" call may not be identical between the C wrapper and the raw OS-9 service request.
source:    OS-9 C Compiler manual, "Operating System Calls", p. 1-7
--- END ---

--- CARD ---
id:        os9-has-no-group-permission-class
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     c-library
claim:     OS-9 has only two permission classes, owner and public — there is no group class. os9lib's `<stat.h>` acknowledges this by deliberately aliasing the group macros onto the public ones: `S_IRGRP` and `S_IROTH` are both `0x0008`, `S_IWGRP` and `S_IWOTH` both `0x0010`, and so on. Unix code that extracts permission triples with the octal masks `0700`/`0070`/`0007` misses OS-9's permission bits (`0x01`-`0x20`) entirely and renders every permission as absent.
context:   The aliasing is a useful hint about intent: when rendering a Unix-style 10-character mode string, mirroring the public bits into the group position is what the header itself already implies. Symptom to recognise: an `ls -l` whose type character is right but whose permissions are all dashes means the *permission* decode is using Unix masks, independently of whatever is wrong with the type decode.
source:    authored
--- END ---

--- CARD ---
id:        os9-file-dates-are-local-time-not-utc
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     c-library
claim:     OS-9 stores file dates as *local wall-clock* time (the FD sector's 5-byte `fd_date`: year-1900, month, day, hour, minute — no seconds). A Unix `time_t` is UTC. Converting the stored fields as though they were UTC and then printing them back through `localtime()` shifts every timestamp by the local UTC offset — a flat, silent whole-timezone error.
context:   The obvious fix, `mktime()` (which interprets a `struct tm` as local time), is **not** available: os9lib's `<time.h>` declares it only in its non-GCC branch, so under gcc2 it is not declared. A zone-agnostic workaround that needs no new library symbol: convert the fields naively, pass the result to `localtime()`, see how far the returned fields moved from the originals, and add that difference back. Correct in any zone; can be an hour out for a timestamp landing inside a DST transition, which minute-resolution local dates cannot disambiguate anyway.
source:    authored
--- END ---

--- CARD ---
id:        os9lib-ctype-is-pre-ansi-no-isgraph
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     c-library
claim:     os9lib's `<ctype.h>` is K&R-era and is missing ANSI-era additions. It defines `isalpha`, `isupper`, `islower`, `isdigit`, `isxdigit`, `isspace`, `ispunct`, `isalnum`, `isprint`, `iscntrl` and `isascii` as macros over a `_chcodes[]` table — but has no `isgraph`. A call to `isgraph()` therefore degrades to an implicit function call and fails at link time as an unresolved symbol.
context:   Treat this as an instance of the general rule rather than a one-off: os9lib predates ANSI C, so assume any function added by the ANSI standard may simply be absent, and expect to discover it at link time rather than compile time. (`isgraph` is trivially supplied as `isascii(c) && isprint(c) && c != ' '` — note that os9lib's `isprint` already excludes space, since space is classified `_WHITE` rather than `_PUNCT`.)
source:    authored
--- END ---

## Linker and build conventions

--- CARD ---
id:        c-compiler-startup-file
type:      FACT
target:    68k
verify:    from-manual
topic:     compilation
claim:     C compiler adds 'LIB/cstart.r' at front of link file list; cstart.r is root psect containing initialization code that calls main() function.
context:   cstart.r created from 'C/SOURCE/cstart.a'; provides C program entry point and initialization.
source:    The OS-9 Guru, section 3.2.12, page 44
--- END ---

--- CARD ---
id:        long-float-linker-inclusion-trick
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     If a program uses printf()/fprintf()/sprintf() to output LONG integers, the source MUST also contain a call to a specific do-nothing marker function ("pflmit()" per this manual) somewhere in the program; the mere presence of that call reference tells the linker to pull in the long-output support code, which is otherwise omitted to save space. A separate marker call is likewise required to pull in FLOAT/DOUBLE printing support.
context:   This "reference a do-nothing function to make the linker include optional runtime support" trick is common across Microware C compilers in spirit, but the exact marker-function name(s) are this compiler's own and should be independently checked against whatever 68k C compiler/runtime is in use rather than assumed identical.
source:    OS-9 C Compiler manual, "The Standard Library", p. 1-7
--- END ---

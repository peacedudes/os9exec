--- CARD ---
id:        os9-cr-not-lf-newline
type:      GOTCHA
target:    all
verify:    from-manual
topic:     io
claim:     On OS-9, the C escape sequence `\n` produces the carriage-return character (hex 0D), not linefeed (hex 0A), because OS-9 uses CR as its end-of-line convention. Programs written the K&R way (using `\n` for newline) still work correctly since OS-9's own convention is CR.
context:   A separate escape, `\e`, is defined in this compiler specifically to produce true linefeed (0A) when it's needed. This CR-vs-LF convention is OS-9-wide, not tied to CPU architecture.
source:    OS-9 C Compiler manual, "Differences From The K & R Specification" / "Control Character Escape Sequences", p. 1-2, p. 1-4
--- END ---

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
id:        cmdline-argc-argv-quoting
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     c-compiler
claim:     The C startup routine converts the OS-9 parent process's parameter string into a conventional null-terminated argv array for main(argc, argv). Additionally, it runs together as a single argument any text enclosed in matching single or double quotes; if one quote character is needed literally inside the string, the other quote type should be used as the delimiter.
context:   This quote-joining behavior is a convenience added by the C runtime on top of OS-9's raw parameter string, not part of the OS-9 kernel itself.
source:    OS-9 C Compiler manual, "Access to Command Line Parameters", p. 1-6
--- END ---

--- CARD ---
id:        errno-system-call-convention
type:      FACT
target:    all
verify:    from-manual
topic:     error-handling
claim:     OS-9 C system calls signal failure by returning -1; the specific error code is left in the predefined variable `errno`, with definitions supplied by `<errno.h>`. `errno` retains the value from the most recently *failed* call — it is not reset on success.
context:   Because errno isn't cleared on success, code must check the call's own return value first, then consult errno only when an error was indicated.
source:    OS-9 C Compiler manual, "Introduction to C System Calls", p. 3-1
--- END ---

--- CARD ---
id:        path-number-vs-file-pointer-crash
type:      GOTCHA
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 C distinguishes low-level "path numbers" (returned by system calls like open()/creat()) from high-level "file pointers" (returned by fopen(), of type FILE *). Passing a file pointer where a system call expects a path number, or vice versa, is called out in the manual as a common beginner mistake that reliably crashes the program.
context:   The two are not interchangeable even though both eventually refer to the same open file; a FILE * wraps a path number plus a buffer/status structure maintained by the standard library.
source:    OS-9 C Compiler manual, "Introduction to the C Standard Library", p. 4-1
--- END ---

--- CARD ---
id:        three-standard-file-descriptors
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     Three file pointers are open automatically the moment a C program starts running: stdin (equivalent to path number 0), stdout (path number 1), and stderr (path number 2). All files are buffered by default except stderr, which is always unbuffered.
context:   This mirrors the Unix stdio convention but the underlying path numbers 0/1/2 are OS-9's own standard path numbering, established by the parent process (typically the Shell) before the program starts.
source:    OS-9 C Compiler manual, "Fopen", p. 4-7 to 4-8
--- END ---

--- CARD ---
id:        defs-directory-include-convention
type:      CONCEPT
target:    all
verify:    from-manual
topic:     c-compiler
claim:     When a C source file names an #include file with angle brackets (`<stdio.h>`) rather than double quotes, the compiler resolves it relative to the "DEFS" directory on the default system drive — e.g. `#include <stdio.h>` becomes equivalent to `#include /d0/defs/stdio.h` if /d0 is the default drive.
context:   Most standard header files are required to reside in this DEFS directory; double-quoted includes do not get this path prepended.
source:    OS-9 C Compiler manual, "Introduction to the C Standard Library", p. 4-1 to 4-2
--- END ---

--- CARD ---
id:        module-directory-link-count-model
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     modlink() searches the OS-9 module directory for a module matching a given name and, if found, increments its link count rather than reloading it. modload() instead opens a file, loads whatever modules it contains, and adds them to the module directory. munlink() decrements a module's link count and, once that count reaches zero, removes the module from the directory (freeing its memory).
context:   This link-count-based lifetime model is core to how OS-9 shares a single in-memory copy of a module (e.g. a reentrant program or a library) across multiple processes.
source:    OS-9 C Compiler manual, "Modload, Modlink" / "Munlink", p. 3-24 to 3-25
--- END ---

--- CARD ---
id:        signal-numbers-table
type:      FACT
target:    all
verify:    from-manual
topic:     signals
claim:     OS-9's standard signal numbers, as defined in the C library's <signal.h>, are: `#define SIGKILL 0` (system abort, cannot be caught or ignored), `#define SIGWAKE 1` (wake up), `#define SIGQUIT 2` (keyboard abort), `#define SIGINT 3` (keyboard interrupt). Special addresses for signal() are `#define SIG_DFL 0` (reset to default) and `#define SIG_IGN 1` (ignore).
context:   These are OS-9 kernel-level signal numbers delivered via the kill() system call, not a C-runtime invention layered on top of a different underlying mechanism.
source:    OS-9 C Compiler manual, "Kill" / "Signal", p. 3-23 to 3-24, p. 3-38 to 3-39
--- END ---

--- CARD ---
id:        signal-intercept-mutual-exclusion
type:      GOTCHA
target:    all
verify:    from-manual
topic:     signals
claim:     signal() and intercept() are two independent mechanisms for handling OS-9 signals/interrupts in a C program, and they are mutually incompatible: calls to both must not appear in the same program. The linker detects this and aborts the link with an "entry name clash _sigint" error if both are used.
context:   Pick one mechanism per program; the linker enforces the exclusivity at link time rather than at compile time or runtime.
source:    OS-9 C Compiler manual, "Intercept" / "Signal", p. 3-19, p. 3-38 to 3-39
--- END ---

--- CARD ---
id:        mknod-creates-directory-not-device
type:      ROSETTA
target:    all
verify:    from-manual
topic:     io
claim:     Unlike Unix's mknod() (which creates a general-purpose special/device file), OS-9's C library mknod() call only creates a new directory. Ordinary files are created with creat(), not mknod().
context:   A programmer coming from Unix should not expect mknod() to create device nodes on OS-9 — the name is shared but the semantics are OS-9's own (directory creation).
source:    OS-9 C Compiler manual, "Mknod", p. 3-22 to 3-23
--- END ---

--- CARD ---
id:        l3tol-ltoi3s-3byte-lsn-conversion
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     "Certain system values, such as disc addresses, are maintained in three-byte form rather than four-byte" on OS-9. The standard library provides l3tol() (3-byte integers to long) and ltoi3s() (long to 3-byte integers) specifically so C arithmetic can be performed on these values.
context:   This documents that OS-9 RBF-style disk block addresses (LSNs) are natively 3 bytes wide, a fact independent of CPU word size, requiring explicit conversion when treated as C longs.
source:    OS-9 C Compiler manual, "L3tol,Ltoi3s", p. 4-15
--- END ---

--- CARD ---
id:        read-readln-raw-vs-line-edited
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 offers two read strategies for character-oriented files/devices: read() transfers up to a requested byte count in raw ("raw mode") form with no editing of the input stream, while readln() honors device character mappings (line editing/echoing) and returns as soon as a carriage return is seen on the input. readln() is preferred for interactive terminal input; read() for other file input.
context:   The stdio getc() function auto-selects between these two underlying strategies based on a flag bit (_SCF or _RBF) set in the file structure the first time getc() is called on that stream — reflecting whether the file's device is a Sequential Character File device or a Random Block File device. The programmer can force the choice by setting the flag bits directly before first use.
source:    OS-9 C Compiler manual, "Getc,Getchar", p. 4-11 to 4-12
--- END ---

--- CARD ---
id:        write-writeln-distinction
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     Mirroring read()/readln(), OS-9 provides both write() (a raw byte transfer with no line-editing) and writeln() (which applies line-editing and stops writing after the first embedded `\n` in the buffer, even if the requested count hasn't been exhausted).
context:   Same SCF-vs-RBF device-class distinction as read()/readln() applies on the output side.
source:    OS-9 C Compiler manual, "Write,Writeln", p. 3-45
--- END ---

--- CARD ---
id:        wait-exit-implicit-status
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     process-mgmt
claim:     wait() blocks the calling task until a child task terminates, then returns the terminated task's id and places its exit status into the caller-supplied status pointer. That status is either the argument passed to the child's exit()/_exit() call, or the signal number if the child was terminated by a signal. A C program that returns normally from main() without an explicit exit() call is treated as an implicit call to exit(0).
context:   A wait() must be executed for each child task spawned, or the parent will not reap that child's status.
source:    OS-9 C Compiler manual, "Wait", p. 3-44; "Exit,_Exit", p. 3-14
--- END ---

--- CARD ---
id:        os9fork-chain-module-type-lang-check
type:      CONCEPT
target:    all
verify:    from-manual
topic:     process-mgmt
claim:     Both os9fork() and chain() take explicit "type" and "lang" arguments that must match the type and language nibble recorded in the target module's own header (normally "program" type). The OS-9 kernel uses this match as part of validating that the module being forked/executed is appropriate.
context:   This is a general OS-9 module-typing safeguard at the process-creation calls; the specific *value* used for the language nibble is architecture-dependent (see module-lang-nibble-6809-value for the 6809 C compiler's value).
source:    OS-9 C Compiler manual, "Chain", p. 3-4 to 3-5; "Os9fork", p. 3-29
--- END ---

--- CARD ---
id:        getstat-setstat-status-model
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     getstat()/setstat() take a numeric "code" plus the path number of an already-open file and operate on a path descriptor's status/option area. Code 0 reads/writes a 32-byte status-packet buffer (device- and file-manager-specific structures); code 2 gets/sets the current file size as a long integer; other codes are defined per device/file-manager class.
context:   Full semantics for each code are deferred to the OS-9 System Programmer's Manual — this compiler's manual only summarizes enough to call the C wrappers correctly.
source:    OS-9 C Compiler manual, "Getstat", p. 3-16 to 3-17; "Setstat", p. 3-37
--- END ---

--- CARD ---
id:        os9-macro-direct-syscall-access
type:      CONCEPT
target:    all
verify:    from-manual
topic:     c-compiler
claim:     The library function os9() lets a C program invoke essentially any OS-9 system call directly, given a numeric call code (from <os9.h>, covering both F$ and I$ service requests plus getstt/setstt/error codes) and a `struct registers` holding the values that would otherwise be loaded into CPU registers for the call (accumulator/data-page/index-register style fields).
context:   This is an escape hatch for calling OS-9 service requests that don't have a dedicated C wrapper function; the struct-registers layout itself mirrors the 6809's register set in this manual, but the concept of a generic direct-syscall-by-code-and-register-block C entry point is architecture-general.
source:    OS-9 C Compiler manual, "Os9", p. 3-25 to 3-26
--- END ---

--- CARD ---
id:        module-string-literal-text-section-reentrancy
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     C string literals are placed in the module's TEXT (executable) section rather than its DATA section. Because a reentrant module's TEXT is shared read-only across all concurrent invocations, this avoids duplicating string-literal storage per invocation — at the cost that programmers should not alter a string literal in place (they should copy it out first).
context:   The one documented exception is a char array explicitly initialized from a string literal (`char msg[] = "hi";`), whose backing storage lands in DATA (one copy per invocation) and is safely mutable.
source:    OS-9 C Compiler manual, "String Literals", p. 2-2 to 2-3
--- END ---

--- CARD ---
id:        edata-end-symbols
type:      FACT
target:    all
verify:    from-manual
topic:     memory-mgmt
claim:     Two linker-defined symbols, `edata` and `end`, mark addresses one byte past the end of a program's initialized data and one byte past the end of its uninitialized data, respectively. They are not variables — a C program must take their address with `&edata` / `&end` to get the values (in assembler, they're accessed as plain labels, e.g. `leax end,y`).
context:   Useful for a program that wants to know where its own static data region ends, e.g. before calling sbrk()/malloc()-style allocation.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        sbrk-ibrk-memory-request
type:      FACT
target:    all
verify:    from-manual
topic:     memory-mgmt
claim:     sbrk() requests new memory for a running program from *outside* its initial memory allocation, going back to the OS for more; ibrk() instead requests memory that still fits *inside* the process's already-granted initial allocation. Both return -1 if the requested amount of contiguous memory cannot be granted.
context:   sbrk() failing does not necessarily mean total system memory is exhausted — it can fail if OS-9 declines to grant more for any reason.
source:    OS-9 C Compiler manual, "Sbrk,Ibrk", p. 3-33; "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        memory-map-parameter-stack-data-order
type:      CONCEPT
target:    all
verify:    from-manual
topic:     memory-mgmt
claim:     A running C program's memory, from high addresses to low, is laid out as: the parameter string passed by the parent process, the (downward-growing) stack, free/heap memory available for sbrk()/malloc(), uninitialized data, then initialized data. The overall size of this region comes from the "storage size" field recorded in the program's own module header, and can be overridden at invocation time via the OS-9 Shell's memory-size ("#") option.
context:   Exact low-address layout below initialized data (e.g. any CPU-specific fast-access area) is architecture-dependent; see the 6809-specific direct-page cards for that detail on this compiler.
source:    OS-9 C Compiler manual, "Typical C Program Memory Map", p. 2-4
--- END ---

--- CARD ---
id:        module-header-fields-general
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     Every module produced by the compiler carries the standard OS-9 module header: a type/language byte, an attribute/revision byte, an execution offset (where to start running), a storage size (initial memory allocation for data/stack/parameters), and a module name. The module name is what OS-9 uses to register/find the module in the module directory.
context:   The specific byte *values* used for "C program, this architecture's object code" are architecture-dependent (see the 6809-specific card for this compiler's $4/$81 values).
source:    OS-9 C Compiler manual, "Module Header" / "Module Name", p. 2-1 to 2-2
--- END ---

--- CARD ---
id:        compiler-produces-pic-reentrant-modules
type:      CONCEPT
target:    all
verify:    from-manual
topic:     c-compiler
claim:     Microware's OS-9 C compiler generates position-independent, reentrant code packaged in the standard OS-9 memory-module format — meaning a single loaded copy of a compiled program can be shared and run concurrently by multiple processes without relocation-on-load or per-process code copies.
context:   This PIC/reentrant characteristic is a foundational property of the OS-9 module system generally, not something specific to the 6809 code generator (though how position-independence is achieved — via reference tables here — is compiler/architecture-specific; see data-text-data-data-reference-tables).
source:    OS-9 C Compiler manual, "The Object Code Module", p. 2-1
--- END ---

--- CARD ---
id:        info-directive-module-strings
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Strings preceded by the assembly-source directive "info" are placed into the module's information/description area of the header. A common use is embedding a version number and/or copyright notice into the compiled module. The `#asm` pre-compiler directive can be used from C source to emit this directive into the compiler-generated assembly.
context:   -
source:    OS-9 C Compiler manual, "Information", p. 2-2
--- END ---

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
id:        abort-core-dump
type:      FACT
target:    all
verify:    from-manual
topic:     process-mgmt
claim:     Calling abort() writes a memory image of the current process out to a file named "core" in the current data directory, then exits the program with status 1.
context:   This is OS-9's analogue to the Unix core-dump convention.
source:    OS-9 C Compiler manual, "Abort", p. 3-2
--- END ---

--- CARD ---
id:        system-call-max-string-length
type:      FACT
target:    all
verify:    from-manual
topic:     process-mgmt
claim:     system() hands its string argument to the OS-9 Shell for execution as a command line, blocking until the shell command completes and returning the shell's exit status. The maximum command-string length accepted by system() is 80 characters; longer command lines require os9fork() instead.
context:   -
source:    OS-9 C Compiler manual, "System", p. 4-31
--- END ---

--- CARD ---
id:        crc-accumulation-helper
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     The crc() library call accumulates a running CRC into a caller-supplied 3-byte array, over "count" bytes starting at a given address; the 3-byte accumulator must be initialized to 0xFFFFFF before the first call, but repeated calls can extend the same running CRC across an entire module. If the result is to be used as an OS-9 module's own CRC check field, its bytes must be complemented before insertion.
context:   This documents the general OS-9 module-CRC algorithm's calling convention (init to all-ones, complement before storing), independent of CPU architecture.
source:    OS-9 C Compiler manual, "Crc", p. 3-10
--- END ---

--- CARD ---
id:        module-header-6809-object-code-byte
type:      FACT
target:    6809
verify:    from-manual
topic:     modules
claim:     For this compiler, the module header's type/language byte is set to the value $4, documented as "C Program — 6809 Object Code," and the attribute/revision byte is set to $81 (Reentrant + revision level 1).
context:   These specific byte values are this 6809 compiler's encoding; a 68k C compiler's module header would use a different type/language byte value for its object-code type. Do not treat $4 as a general OS-9 constant.
source:    OS-9 C Compiler manual, "Module Header", p. 2-1
--- END ---

--- CARD ---
id:        module-lang-nibble-6809-value
type:      FACT
target:    6809
verify:    from-manual
topic:     modules
claim:     When calling os9fork() or chain(), the "lang" argument for a C program module must equal 1, documented as meaning "6809 machine code," matching the language nibble recorded in that compiler's module header.
context:   This specific nibble value (1) is this compiler/architecture's encoding, not a universal OS-9 constant; a 68k module would carry a different language-nibble value.
source:    OS-9 C Compiler manual, "Chain", p. 3-5; "Os9fork", p. 3-29
--- END ---

--- CARD ---
id:        direct-storage-class
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler extends K&R's storage-class specifiers with three 6809-only additions: `direct`, `extern direct`, and `static direct`, which place a variable in the 6809's "direct page" — a 256-byte memory page addressable with fast 2-byte instructions via the CPU's direct-page register. Direct-class variables cannot be used to declare function arguments, and (like other uninitialized globals/statics) default to zero if not explicitly initialized.
context:   This entire storage class exists to exploit a 6809 hardware addressing mode; the manual itself warns "direct" is unique to this compiler and programs using it may not be portable to other environments even within the 6809 world, let alone to 68k.
source:    OS-9 C Compiler manual, "The 'Direct' Storage Class", p. 1-3
--- END ---

--- CARD ---
id:        direct-page-256-byte-limit
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     Total 6809 direct-page storage is capped at 255 usable bytes (the linker itself consumes 1 of the page's 256 bytes). If a program's declared `direct`-class variables would exceed this, the linkage editor reports an error and the programmer must move some variables out of direct storage to fit.
context:   Not applicable off the 6809 — a page-relative fast-addressing hardware feature of this specific CPU.
source:    OS-9 C Compiler manual, "The 'Direct' Storage Class", p. 1-3 to 1-4
--- END ---

--- CARD ---
id:        direct-page-min-one-byte
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     Even when a program declares no `direct`-class variables at all, the compiler/linker still reserves at least 1 byte for the direct-page variable area. This is deliberate: it guarantees no pointer to a direct-page variable can ever have the value 0 (NULL).
context:   6809-direct-page-specific; not meaningful on architectures without a direct-page addressing mode.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

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
id:        data-type-sizes-6809-table
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     Data type sizes in this compiler: "CHAR 1 / INT 2 / UNSIGNED 2 / LONG 4 / FLOAT 4 / DOUBLE 8" bytes, with CHAR/INT/LONG stored two's-complement and UNSIGNED stored plain unsigned binary.
context:   These sizes are explicitly 6809-only. This project has independently confirmed int/long/pointer are all 32-bit on OS-9/68k — do not carry the INT=2-byte or LONG=4-byte figures from this table over to 68k targets.
source:    OS-9 C Compiler manual, "Data Representation and Storage Requirements", p. 1-5
--- END ---

--- CARD ---
id:        short-long-int-aliasing-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     Following PDP-11 convention, this compiler treats "SHORT" and "SHORT INT" as synonyms for plain INT, and "LONG INT" as a synonym for LONG; "LONG FLOAT" means DOUBLE.
context:   Tied to this compiler's 2-byte INT/4-byte LONG sizing; the naming convention itself may or may not be retained by a 68k Microware C compiler, so don't assume it without separate confirmation.
source:    OS-9 C Compiler manual, "Data Representation and Storage Requirements", p. 1-5
--- END ---

--- CARD ---
id:        float-double-binary-format-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     This compiler's FLOAT and DOUBLE are a proprietary sign-magnitude binary floating-point format (not IEEE 754), with an exponent biased by 128. DOUBLE (8 bytes) uses a 7-byte mantissa (with an implied leading one bit) plus a 1-byte exponent; FLOAT (4 bytes) is identical but with a 3-byte mantissa, produced from a DOUBLE by truncating (FLOAT<-DOUBLE) or zero-padding (DOUBLE<-FLOAT) the least-significant mantissa bytes.
context:   A 6809-C-compiler-specific floating point representation from 1983; do not assume it matches a 68k Microware C compiler's float format (which may use the 68000/68881's IEEE-754-oriented conventions instead).
source:    OS-9 C Compiler manual, "Data Representation and Storage Requirements", p. 1-5
--- END ---

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
id:        optimizer-pass-11-percent
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     An optional post-compilation optimizer pass rewrites the generated 6809 assembly source to remove redundant code and substitute shorter/faster instruction sequences, typically shrinking object code by about 11% with a further speed increase; it can be disabled with the -O flag to speed up error-checking-only compiles.
context:   The specific 11% figure and the pass's mechanics are stated for this 6809 code generator; not necessarily representative of a 68k compiler's optimizer.
source:    OS-9 C Compiler manual, "The Optimizer Pass", p. 1-9
--- END ---

--- CARD ---
id:        compiler-cli-flags-6809-toolchain
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler's command-line flags: -A (suppress assembly, leave .a output), -E=<n> (set module edition byte), -O (inhibit optimizer pass), -P (invoke function-call profiler), -R (suppress linking, leave .r output), -M=<size>[k] (linker's extra data/stack/parameter memory allocation, in pages or Kbytes), -L=<path> (extra library searched before the standard library), -F=<path> (override output file/module name), -C (emit source as comments alongside generated assembly), -S (suppress stack-checking code generation), -D<name>[=<string>] (equivalent to `#define`).
context:   These flags belong to the `cc`/`cc2` front ends for the 6809 compiler as documented in this 1983 manual; a 68k-targeted compiler's flag set is not confirmed to match this.
source:    OS-9 C Compiler manual, "Compiler Option Flags", p. 1-12 to 1-13
--- END ---

--- CARD ---
id:        compiler-component-files-cc-cc2
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     Compiling a C program requires these component files present in the current execution directory (CMDS): on OS-9 Level I, `ccc` (executive), `c.prep` (macro preprocessor), `c.pass1`/`c.pass2` (two-pass compiler), `c.opt` (assembly optimizer), `c.asm` (relocating assembler), `c.link` (linkage editor); on Level II, `cc2` (executive), `c.prep`, `c.comp` (single-pass compiler proper), `c.opt`, `c.asm`, `c.link`.
context:   This is this specific 1983 6809 toolchain's set of component program names; a 68k toolchain's component names are not confirmed to match.
source:    OS-9 C Compiler manual, "C Compiler Component Files and File Usage", p. 1-10
--- END ---

--- CARD ---
id:        clib-cstart-must-be-in-lib-dir
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler requires two additional files at link time — `clib.r` (the standard library archive: math functions and the system library) and `cstart.r` (the startup code for compiled programs) — both of which must reside in a directory literally named "LIB" on drive /d1, alongside the DEFS directory also required on /d1.
context:   A specific toolchain/filesystem-layout convention of this 1983 6809 SDK; naming and drive letter are not guaranteed to carry over to a 68k SDK layout.
source:    OS-9 C Compiler manual, "C Compiler Component Files and File Usage", p. 1-10
--- END ---

--- CARD ---
id:        stack-reservation-64-byte-overhead-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     On each C function entry, a system-interface routine reserves stack space for that function's use plus a fixed additional 64 bytes, reserved for user-written assembly routines, the system interface, and arithmetic-support routines.
context:   The 64-byte figure is this 6809 runtime's specific constant; not confirmed for a 68k runtime, which would likely need a different (probably larger, given wider registers) reserve.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        stack-overflow-detection-error-message
type:      BEHAVIOR
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     The runtime tracks the lowest stack address granted so far. If a function's stack request would push that watermark down far enough to overlap the data area, the program halts and prints "**** STACK OVERFLOW ****" to standard error rather than proceeding; otherwise the new (lower) limit is accepted and execution continues.
context:   Overflow detection can be disabled entirely with the -S compiler flag (see compiler-cli-flags-6809-toolchain) for time-critical code once the programmer is confident of correct stack usage.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        compile-time-memory-default-4k
type:      FACT
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     Unless told otherwise (via -M=), this compiler's linker automatically allocates 4K bytes more than the total size of a program's variables and strings as its default runtime memory pool — intended to cover the parameter area, stack, and standard-library file buffers. Requests for less than 256 bytes via -M= are ignored by the linker.
context:   This specific 4K default and 256-byte floor are this compiler/linker's own defaults, not confirmed for a 68k linker.
source:    OS-9 C Compiler manual, "Compile Time Memory Allocation", p. 2-6
--- END ---

--- CARD ---
id:        arithmetic-error-signal-codes-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     error-handling
claim:     This compiler's runtime adds three arithmetic error codes to <errno.h>: `#define EFPOVR 40` (floating point overflow or underflow), `#define EDIVERR 41` (division by zero), `#define EINTERR 42` (overflow converting a floating point value to a long integer). When one of these conditions occurs, the running program sends *itself* a signal carrying that error number as its value; if uncaught, the program terminates with an error return to its parent.
context:   These are runtime-generated conditions caught via signal()/intercept(), distinct from the OS-9 kernel's own error-number space; specific numeric values (40/41/42) and the self-signal mechanism are this compiler's own design and not confirmed identical on a 68k compiler.
source:    OS-9 C Compiler manual, "Run-Time Arithmetic Error Handling", p. 1-8
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

--- CARD ---
id:        data-text-data-data-reference-tables
type:      CONCEPT
target:    6809
verify:    from-manual
topic:     modules
claim:     Because no absolute addresses are known at compile time under OS-9, this compiler's runtime resolves pointer-valued initializers (e.g. `char *p = "string";`) at load time using two tables recorded in the module: a "data-text reference" table (offsets needing the base address of the TEXT/code section added) and a "data-data reference" table (offsets needing the base address of the DATA section added). The startup routine walks both tables once, after first copying all initializer data into the DATA section, to patch these pointer values into absolute addresses.
context:   This is this specific compiler's implementation strategy for achieving position-independence; a 68k compiler could plausibly use a different runtime-relocation mechanism (e.g. base-register-relative addressing) to achieve the same PIC goal, so the *mechanism* (not just field values) should not be assumed to carry over.
source:    OS-9 C Compiler manual, "Data References", p. 2-3
--- END ---

--- CARD ---
id:        rma-psect-vsect-csect-6809-mnemonics
type:      CONCEPT
target:    6809
verify:    from-manual
topic:     assembler
claim:     The bundled Relocating Macro Assembler (RMA, invoked as c.asm) organizes a source file into sections: PSECT (code, and where any actual 6809 instruction mnemonic is legal, plus a handful of directives), VSECT (initialized/uninitialized data, optionally the direct page), and CSECT (an offset-only counter for assigning label offsets without EQUs). This replaces the older Microware Interactive Assembler's single MOD/EMOD directive pair; PSECT records the module type/lang/attr/edition/stack-size/entry-point info that used to live in MOD, but for the *linker* (c.link) rather than for OS-9 directly.
context:   PSECT/VSECT/CSECT could be a Microware-linker-family convention worth checking against the 68k assembler, but this manual explicitly restricts PSECT's body to real 6809 opcodes, so the exact directive set and legal-mnemonic list is 6809-specific until independently confirmed for 68k.
source:    OS-9 C Compiler manual, "Relocating Macro Assembler Reference" / "Differences between RMA and MIA", p. D-1 to D-4
--- END ---

--- CARD ---
id:        rma-library-merge-forward-reference-order
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     assembler
claim:     When several separately-compiled modules (ROFs) are merged into one library file for c.link to search, the linker resolves each unresolved external reference on a first-found basis, using whichever ROF's matching symbol it encounters first in search order. Consequently, if procedure A in a library calls procedure B also in that library, B's ROF must be merged into the library *after* A's — i.e. all intra-library references should be forward references only.
context:   This ordering requirement is a consequence of this specific linker's single-pass, first-found symbol resolution; not necessarily true of a different (e.g. two-pass) 68k linker.
source:    OS-9 C Compiler manual, "Using and Linking to User Defined Libraries", p. B-5
--- END ---

--- CARD ---
id:        basic09-interop-register-y-offset
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     A C function meant to be called from BASIC09's RUN statement cannot rely on the normal cstart-provided setup of the Y register (which points at the C data area), because cstart itself is not linked into BASIC09-callable modules. Instead, the function's very first embedded-assembly statement (via #asm) must manually load Y with the address of BASIC09's first stack parameter, using the fixed offset `ldy 6,s`.
context:   Deeply 6809-register-specific interop mechanism between two 6809-era Microware products (the C compiler and BASIC09); has no direct 68k analogue documented here.
source:    OS-9 C Compiler manual, "Example 2 — More Complex Integer Arithmetic Case", p. C-4 to C-5
--- END ---

--- CARD ---
id:        basic09-integer-c-int-both-2-byte
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     BASIC09's INTEGER type and this C compiler's `int` type are documented as identical: both are 2-byte two's-complement integers. BASIC09's BYTE and BOOLEAN types are likewise identical to C `char` (which this compiler sign-extends for comparisons, giving char the range -128 to 127).
context:   This equivalence is explicitly a 6809-only fact — since C `int` is 4 bytes on 68k (independently confirmed for this project), this BASIC09<->C type equivalence cannot hold as stated on a 68k C compiler.
source:    OS-9 C Compiler manual, "Interfacing to BASIC09", p. C-1
--- END ---

--- CARD ---
id:        basic09-string-terminator-0xff
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     BASIC09 strings are terminated by the byte value 0xFF (255), unlike C strings which are terminated by 0x00. If a BASIC09 string happens to be at its declared maximum length, the terminator byte is not present at all. C code that processes a BASIC09 string must therefore check both the string's declared length and for the 0xFF terminator explicitly, rather than relying on either alone.
context:   Specific to BASIC09/C interop on the 6809 toolchain documented here.
source:    OS-9 C Compiler manual, "Interfacing to BASIC09", p. C-1
--- END ---

--- CARD ---
id:        basic09-array-storage-order-transpose
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     data-types
claim:     BASIC09 stores multi-dimensional arrays in column-major order, while this C compiler stores them row-major. Consequently, accessing a BASIC09 array element such as `array(4,2)` from C requires transposing the subscripts, e.g. as `array[2][4]`, to reach the same memory location.
context:   A data-layout mismatch inherent to interoperating between the two languages/runtimes on this platform, independent of which CPU it runs on architecturally, but only documented here in a 6809-era BASIC09 interop context.
source:    OS-9 C Compiler manual, "Interfacing to BASIC09", p. C-1 to C-2
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

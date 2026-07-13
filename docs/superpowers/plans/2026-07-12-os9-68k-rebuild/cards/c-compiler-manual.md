--- CARD ---
id:        module-header-type-language-byte
type:      FACT
target:    68k
verify:    from-manual
topic:     module-format
claim:     The type/language byte of a compiled C program's module header is set to $4, indicating "C Program — 6809 Object Code" (per K&R implementation constraints).
context:   This byte value is specific to the OS-9 module directory entry system; different architectures will use different type codes.
source:    OS-9 C Compiler manual, §"Module Header", p. 2-1

--- END ---
--- CARD ---
id:        module-header-reentrant-attribute
type:      FACT
target:    68k
verify:    from-manual
topic:     module-format
claim:     The attribute/revision byte of compiled C programs is set to $81, encoding Reentrant (bit set) + revision level 1.
context:   The reentrant attribute allows the module to be shared across multiple concurrent processes; revision level is separately overridable via compiler -E= flag.
source:    OS-9 C Compiler manual, §"Module Header", p. 2-1

--- END ---
--- CARD ---
id:        module-components-order
type:      FACT
target:    68k
verify:    from-manual
topic:     module-format
claim:     A compiled C program module contains (in order): header (8 bytes), execution offset (2), storage size (2), module name, information section, executable code, string literals, initializing data, uninitialized data size (2), uninitialized data, data-text reference count (2), data-text offsets, data-data reference count (2), data-data offsets, CRC check value (3 bytes).
context:   Position-independent code relies on reference tables to resolve pointer initialization at load time.
source:    OS-9 C Compiler manual, §"The Object Code Module", p. 2-1 to 2-2

--- END ---
--- CARD ---
id:        storage-size-definition
type:      FACT
target:    68k
verify:    from-manual
topic:     module-format
claim:     Storage Size in the module header is the initial default allocation of memory for data, stack, and parameter area; can be overridden at runtime via the OS-9 Shell "#" command.
context:   The linker automatically allocates 4k bytes more than total size of variables and strings unless specified otherwise via -m= compiler option.
source:    OS-9 C Compiler manual, §"Storage Size", p. 2-2

--- END ---
--- CARD ---
id:        module-name-edition-byte-linkage
type:      FACT
target:    68k
verify:    from-manual
topic:     module-format
claim:     The module name is followed by the edition byte in the module directory; the edition byte is encoded in the startup code (estart) and can be overridden via compiler -E=<number> option.
context:   Edition tracking is an OS-9 convention for version management of modules in the module directory.
source:    OS-9 C Compiler manual, §"Module Name", p. 2-2

--- END ---
--- CARD ---
id:        string-literals-text-section-immutable
type:      FACT
target:    68k
verify:    from-manual
topic:     string-handling
claim:     Quoted strings in C source code are placed in the TEXT (executable) section of the module, not the DATA section, making them read-only by default.
context:   This violates the strict K&R assumption that strings are mutable in the data area, but avoids memory duplication cost across reentrant instances. String initializers for char arrays are an exception — they go in the data area and can be modified.
source:    OS-9 C Compiler manual, §"String Literals", p. 2-2 to 2-3

--- END ---
--- CARD ---
id:        string-array-initializer-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     string-handling
claim:     When a string literal is used to initialize a char array (e.g., `char message[] = "Hello world";`), the string is placed in the data area and can be modified after initialization.
context:   This is the exception that proves the immutability rule for string literals — array initialization creates a writable copy in data memory.
source:    OS-9 C Compiler manual, §"String Literals", p. 2-3

--- END ---
--- CARD ---
id:        initialization-data-pointer-resolution
type:      FACT
target:    68k
verify:    from-manual
topic:     module-loading
claim:     Pointer values in initialized data cannot be known at compile time under OS-9 (no absolute addresses), so the startup routine uses data-text and data-data reference offset tables to adjust pointer initializers to absolute values at runtime.
context:   Example: a pointer initialized to `&string_literal` or `&global_var` is stored as an offset; the startup routine resolves it to an absolute address at load time using reference tables.
source:    OS-9 C Compiler manual, §"Data References", p. 2-3

--- END ---
--- CARD ---
id:        data-area-layout-direct-page
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-layout
claim:     The C program data area layout (low to high memory) is: direct page variables area, initialized data, uninitialized data, free memory (expandable via brk/sbrk), stack area (descending), standard I/O buffers, parameters area.
context:   Direct page variables are a 6809-only architectural feature; 68k code needs careful review of whether this applies.
source:    OS-9 C Compiler manual, §"Typical C Program Memory Map", p. 2-4

--- END ---
--- CARD ---
id:        direct-page-variables-size-limit
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     direct-storage-class
claim:     Direct page storage in 6809 is limited to 255 bytes total (the direct page register addresses one 256-byte page); the linker requires 1 byte, leaving 255 available for programmer-declared direct variables. If exceeded, the linker reports an error.
context:   The 6809's direct-page addressing mode is a hardware feature providing fast 2-byte instructions for direct-page memory; this is 6809-specific and does not exist on 68000.
source:    OS-9 C Compiler manual, §"The 'Direct' Storage Class", p. 1-3 to 1-4

--- END ---
--- CARD ---
id:        direct-storage-class-declaration
type:      FACT
target:    6809
verify:    from-manual
topic:     direct-storage-class
claim:     The OS-9 C compiler extends the storage class specifier list with `direct`, `extern direct`, and `static direct` keywords to place variables in the 6809 direct page for faster access.
context:   These are compiler extensions not in K&R; direct variables are initialized to zero at startup like other globals/statics, and cannot be used as function parameters.
source:    OS-9 C Compiler manual, §"The 'Direct' Storage Class", p. 1-3

--- END ---
--- CARD ---
id:        embedded-assembly-asm-endasms-directives
type:      FACT
target:    68k
verify:    from-manual
topic:     assembly-embedding
claim:     A line beginning with "#asm" switches the compiler into assembly-pass-through mode; all subsequent lines are passed directly to assembler output until a line beginning with "#endasm" is encountered.
context:   Care must be exercised to ensure the correct PSECT (code) or VSECT (variable) section context is maintained; if using VSECT, a #ENDSECT directive must be placed at the end to restore state for following compiler-generated code.
source:    OS-9 C Compiler manual, §"Imbedded Assembly Language", p. 1-4

--- END ---
--- CARD ---
id:        linefeed-escape-sequence-extension
type:      FACT
target:    68k
verify:    from-manual
topic:     escape-sequences
claim:     The escape sequence `\e` (lowercase 'e' with backslash) represents the linefeed character (hex 0A) to distinguish LF from `\n`, which on OS-9 is the carriage return (hex 0D).
context:   This extension is necessary because OS-9 uses CR for end-of-line, not LF like UNIX; K&R programs using `\n` for newline still work correctly due to runtime handling.
source:    OS-9 C Compiler manual, §"Control Character Escape Sequences", p. 1-4

--- END ---
--- CARD ---
id:        octal-decimal-hex-escape-sequences
type:      FACT
target:    68k
verify:    from-manual
topic:     escape-sequences
claim:     Escape sequences for bit patterns are supported: `\NNN` (3 octal digits), `\dNNN` (decimal with 'd' prefix), `\xNNN` (hexadecimal).
context:   Example: `\377`, `\d255`, and `\xff` all represent the byte value 255.
source:    OS-9 C Compiler manual, §"Control Character Escape Sequences", p. 1-4

--- END ---
--- CARD ---
id:        data-type-sizes-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     data-types
claim:     Data type sizes in the 6809 implementation: CHAR=1 byte, INT=2 bytes, UNSIGNED=2 bytes, LONG=4 bytes, FLOAT=4 bytes, DOUBLE=8 bytes, all using two's complement for signed types and unsigned binary for unsigned types.
context:   The INT/LONG sizes documented here are 6809-specific; 68k implementations have 32-bit int/long/pointer types independently confirmed in this project. Do not assume these sizes apply to 68k.
source:    OS-9 C Compiler manual, §"Data Representation and Storage Requirements", p. 1-5

--- END ---
--- CARD ---
id:        short-int-typedef-equivalence
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     Following PDP-11 implementation conventions, "SHORT" and "SHORT INT" are equivalent to INT, and "LONG INT" is equivalent to LONG.
context:   This means the `short` keyword is a no-op in this compiler; use `int` directly for portable code.
source:    OS-9 C Compiler manual, §"Data Representation and Storage Requirements", p. 1-5

--- END ---
--- CARD ---
id:        double-floating-point-format
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     DOUBLE (8 bytes) uses sign-magnitude format for the mantissa (with implied MSB at sign bit position) and bias-128 exponent; 7 bytes for mantissa, 1 byte for exponent. FLOAT is identical except mantissa is 3 bytes; conversion truncates/pads least significant bytes.
context:   This binary floating-point representation is not IEEE 754; programs relying on IEEE semantics may have precision/rounding differences.
source:    OS-9 C Compiler manual, §"Data Representation and Storage Requirements", p. 1-5

--- END ---
--- CARD ---
id:        register-variable-restrictions
type:      FACT
target:    68k
verify:    from-manual
topic:     register-variables
claim:     Only one register variable per function is allowed; only int, unsigned, and pointer types are valid for register declarations. Invalid declarations are silently ignored (storage class becomes auto).
context:   The U1 register is assigned to register variables; declaring more than one or invalid types does not error, it just falls back to auto storage.
source:    OS-9 C Compiler manual, §"Register Variables", p. 1-5

--- END ---
--- CARD ---
id:        register-variable-performance-gotcha
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     register-variables
claim:     Register variables provide significant code-size and speed savings for pointer or loop-counter use, but provide no savings (and can worsen performance) when used in complex arithmetic expressions.
context:   Misuse of register variables for complex expressions can increase register pressure and register-memory moves, negating any benefit.
source:    OS-9 C Compiler manual, §"Register Variables", p. 1-5

--- END ---
--- CARD ---
id:        case-sensitivity-mandatory
type:      FACT
target:    68k
verify:    from-manual
topic:     source-code
claim:     Upper and lower case letters cannot be mixed in identifiers; `Proglc` and `prog.c` are distinct names. On systems using only uppercase by default (e.g., Color Computer), use TMODE -UPC and CLEAR<0> to enable mixed-case input.
context:   This is a source-code portability note for systems with case-sensitive filesystems; the compiler itself respects case.
source:    OS-9 C Compiler manual, §"Implementation Dependent Characteristics", p. 1-5

--- END ---
--- CARD ---
id:        main-argc-argv-command-line-parsing
type:      FACT
target:    68k
verify:    from-manual
topic:     startup
claim:     Standard C arguments `argc` and `argv` are available to main() as documented in K&R. The OS-9 startup routine converts the parent process's parameter string into null-terminated strings; quoted strings (single or double quotes) are treated as single arguments.
context:   If a quote character is part of the actual string, use the other type of quote as the delimiter.
source:    OS-9 C Compiler manual, §"Access to Command Line Parameters", p. 1-6

--- END ---
--- CARD ---
id:        system-call-names-unix-compatibility
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     system-interface
claim:     System call names in the C library are chosen for compatibility with UNIX/portability rather than direct OS-9 names, though OS-9-specific calls are also available when UNIX equivalents don't exist.
context:   Programmers familiar with OS-9 assembler names should note that C system call names may differ; a cross-reference list is provided in the manual.
source:    OS-9 C Compiler manual, §"Operating System Calls", p. 1-7

--- END ---
--- CARD ---
id:        system-call-error-return-errno
type:      FACT
target:    68k
verify:    from-manual
topic:     error-handling
claim:     System calls return -1 on error; the specific error code is stored in the predefined `errno` variable and can be read by the program. Definitions are in "<errno.h>".
context:   `errno` always contains the error from the most recent erroneous system call; it is not cleared on success, so check the return value first.
source:    OS-9 C Compiler manual, §"Introduction to C System Calls", p. 3-1

--- END ---
--- CARD ---
id:        printf-pformat-long-support-requirement
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     standard-library
claim:     If output via printf(), fprintf(), or sprintf() of long integers is required, the program MUST call `pfmitd()` at some point; this informs the linker to include long-output support code.
context:   Programs not using LONG output don't need the overhead; calling `pfmitd()` is a no-op but signals the linker to include the module. Likewise, `pfrintf()` must be called for FLOAT/DOUBLE output support.
source:    OS-9 C Compiler manual, §"The Standard Library", p. 1-7

--- END ---
--- CARD ---
id:        floating-point-output-pfrintf-requirement
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     standard-library
claim:     If output via printf(), fprintf(), or sprintf() of FLOAT or DOUBLE values is required, the program MUST call `pfrintf()` at some point; this informs the linker to include floating-point output support code.
context:   Like long-integer support, this is a linker-signaling mechanism to pull in optional library modules.
source:    OS-9 C Compiler manual, §"The Standard Library", p. 1-7

--- END ---
--- CARD ---
id:        arithmetic-error-signals-efdovr-ediverr-einterr
type:      FACT
target:    68k
verify:    from-manual
topic:     error-handling
claim:     Three arithmetic error codes are defined in <errno.h>: E_FDOVR (40) for floating-point overflow/underflow, E_DIVERR (41) for division by zero, E_INTERR (42) for overflow on conversion of floating-point to long integer.
context:   When these conditions occur, the program receives a signal with the error number as the argument; the signal can be caught via signal() or intercept(), or the program terminates with an error return to the parent.
source:    OS-9 C Compiler manual, §"Run-Time Arithmetic Error Handling", p. 1-8

--- END ---
--- CARD ---
id:        constant-expression-evaluation-limits
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-behavior
claim:     The compiler evaluates constant expressions at compile-time only when all operands are type CHAR, INT, or UNSIGNED; expressions involving LONG, FLOAT, or DOUBLE are evaluated at runtime by the compiled program.
context:   For performance-critical code involving floating-point or long constants, manually compute the value offline rather than relying on runtime evaluation.
source:    OS-9 C Compiler manual, §"Programming Considerations", p. 1-9

--- END ---
--- CARD ---
id:        optimizer-pass-automatic-11-percent-reduction
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-optimization
claim:     The optimizer pass automatically occurs after compilation, reads the assembler source code, removes redundant code, and replaces code sequences with shorter/faster equivalents, achieving ~11% object-code reduction and significant execution speed increase.
context:   The optimizer is recommended for production builds but adds compilation time; can be inhibited with -O flag for debug compilations.
source:    OS-9 C Compiler manual, §"The Optimizer Pass", p. 1-9

--- END ---
--- CARD ---
id:        profiler-function-invocation-statistics
type:      FACT
target:    68k
verify:    from-manual
topic:     profiling
claim:     The profiler (-P compiler option) generates code at function entry to call the "uprormm" profiler module, which counts function invocations during execution and prints statistics after program termination.
context:   Profiling slightly reduces execution speed; useful for identifying hot spots where algorithmic or source-code improvements yield greatest gains.
source:    OS-9 C Compiler manual, §"The Profiler", p. 1-9

--- END ---
--- CARD ---
id:        compiler-phases-multiple-file-linking
type:      FACT
target:    68k
verify:    from-manual
topic:     compilation-model
claim:     The compiler manages compilation through four stages: pre-processor, compilation to assembler code, assembly to relocatable module, and linking to binary executable code (OS-9 memory module format).
context:   Multiple source files can be compiled together or via #include facility in single-source mode; multiple-source mode outputs a single "output" file unless -f= option is used.
source:    OS-9 C Compiler manual, §"Running the Compiler", p. 1-10 to 1-11

--- END ---
--- CARD ---
id:        source-file-suffix-conventions
type:      FACT
target:    68k
verify:    from-manual
topic:     file-handling
claim:     Compiler recognizes three file suffix types: .c (C source), .a (assembler source), .r (relocatable module); no suffix indicates executable binary (OS-9 memory module); output filename is derived by removing the suffix from the input.
context:   Example: `cc prog.c` produces executable `prog`; multiple-file mode outputs `output` unless overridden.
source:    OS-9 C Compiler manual, §"File Name Suffix Conventions", p. 1-11

--- END ---
--- CARD ---
id:        compiler-single-vs-multiple-file-modes
type:      FACT
target:    68k
verify:    from-manual
topic:     compilation-model
claim:     Single-source mode (one .c/.a/.r file on command line) outputs an executable with the source filename (minus suffix); multiple-source mode (multiple files) outputs "output" in the current directory unless -f= flag overrides, and leaves relocatable .r files in their source directories.
context:   In multiple-source mode, intermediate .r files have postfixes changed from .c/.a to .r and reside with their source; e.g., `cc pre1.c fred/pre2.c` creates pre1.r, fred/pre2.r, and output executable.
source:    OS-9 C Compiler manual, §"Running the Compiler", p. 1-11

--- END ---
--- CARD ---
id:        compiler-option-flag-a-suppress-assembly
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -A flag suppresses assembly, leaving the output as assembler code in a file postfixed ".a" (e.g., prog.a).
context:   Useful for inspecting generated assembly or for manual optimization before assembly.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-12

--- END ---
--- CARD ---
id:        compiler-option-flag-e-edition-number
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -E=<number> flag sets the edition number constant byte in the module header to the given number, an OS-9 convention for version tracking of memory modules.
context:   Edition number is encoded in the startup code (estart) and appears in the module directory.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-12

--- END ---
--- CARD ---
id:        compiler-option-flag-o-inhibit-optimizer
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -O flag inhibits the optimizer pass, which normally shortens object code by ~11% and increases execution speed; useful for debug compilations where optimizer adds time.
context:   Production builds should omit -O to benefit from optimization.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-12

--- END ---
--- CARD ---
id:        compiler-option-flag-p-profiler
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -P flag invokes the profiler, generating code to produce function invocation frequency statistics after program execution.
context:   Profiling has a slight performance cost.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-12

--- END ---
--- CARD ---
id:        compiler-option-flag-r-suppress-linking
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -R flag suppresses linking library modules into an executable program, leaving outputs as relocatable .r files.
context:   Useful for building libraries or partial compilation in a large project.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-12

--- END ---
--- CARD ---
id:        compiler-option-flag-m-memory-allocation
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -M=<memory size> flag instructs the linker to allocate <memory size> for data, stack, and parameter area; size may be expressed as pages (integer) or kilobytes (append 'k'). Minimum is 256 bytes; default is total variable/string size + 4k.
context:   Helps tune memory requirements for specific applications; see "Memory Management" section for detailed guidelines.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-12 to 1-13

--- END ---
--- CARD ---
id:        compiler-option-flag-l-library-search
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -L=<filename> flag specifies a library to be searched by the linker before the standard library and system interface.
context:   Allows custom libraries to override standard functions.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-13

--- END ---
--- CARD ---
id:        compiler-option-flag-f-output-filename
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -F=<path> flag overrides default output file naming; the output file will be left with <filename> as its name, and the module will be named the last component in <path>. Does not make sense in multiple-source mode or with -a or -r flags.
context:   Single-source mode only; useful for directing output to a specific location or name.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-13

--- END ---
--- CARD ---
id:        compiler-option-flag-c-source-as-comments
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -C flag outputs the source code as comments interleaved with the generated assembler code.
context:   Useful for debugging or understanding compiler code generation.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-13

--- END ---
--- CARD ---
id:        compiler-option-flag-s-disable-stack-checking
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -S flag stops generation of stack-checking code; should only be used with great care when the application is extremely time-critical and the stack usage by compiler-generated code is fully understood.
context:   Without stack checking, stack overflow will cause silent memory corruption rather than a caught error.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-13

--- END ---
--- CARD ---
id:        compiler-option-flag-d-define-preprocessor-identifier
type:      FACT
target:    68k
verify:    from-manual
topic:     compiler-options
claim:     The -D=<identifier> flag is equivalent to "#define <identifier>" in the source file; useful for conditionally compiling different versions of a program via #ifdef/#ifndef preprocessor directives. Form -D=<identifier>=<string> sets the expansion value to <string>.
context:   Allows compile-time configuration without editing source code.
source:    OS-9 C Compiler manual, §"Compiler Option Flags", p. 1-13

--- END ---
--- CARD ---
id:        startup-routine-initialization-flow
type:      FACT
target:    68k
verify:    from-manual
topic:     startup
claim:     The startup routine (estart) for C programs: (1) copies initialized data from module to data memory, (2) clears uninitialized global/static variables to zero, (3) processes data-text and data-data reference tables to resolve pointer initializers to absolute addresses, (4) converts parameter string to argc/argv, (5) calls main().
context:   This initialization model ensures all variables are zero-initialized unless explicitly initialized, and pointers are correctly relocated for position-independent code.
source:    OS-9 C Compiler manual, §"Data References" and "Characteristics of Compiled Programs", p. 2-3 to 2-4

--- END ---
--- CARD ---
id:        stack-space-reservation-64-byte-overhead
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     When each C function is entered, a system interface routine reserves stack space for function use plus an additional 64 bytes; the 64 bytes are for user-written assembly functions, system interface, and arithmetic routines.
context:   Function arguments and local variables occupy stack space; each function entry adds 4 bytes overhead for return address and register variable storage.
source:    OS-9 C Compiler manual, §"Memory Management", p. 2-5

--- END ---
--- CARD ---
id:        stack-overflow-detection-error-message
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     If a function's stack-space request would overlap the data area, the program stops with the message "HEAP STACK OVERFLOW #3" written to standard error output.
context:   The system tracks the lowest address so far granted for the stack; if a new allocation would go lower than current data top, stack overflow is detected and program terminates.
source:    OS-9 C Compiler manual, §"Memory Management", p. 2-5

--- END ---
--- CARD ---
id:        brk-sbrk-memory-allocation-functions
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     brk() requests additional initialized memory from the free memory area; sbrk() requests additional memory from the operating system and returns its lower bound. If OS-9 refuses to grant memory, sbrk() returns -1.
context:   brk() allocates from pre-allocated free space (set at link time); sbrk() dynamically grows the process memory allocation from the OS.
source:    OS-9 C Compiler manual, §"Memory Management", p. 2-6

--- END ---
--- CARD ---
id:        linker-default-memory-4k-overhead
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     By default, the linker automatically allocates 4k bytes more than the total size of the program's variables and strings; this overhead covers parameter area, stack requirements, and standard library file buffers (256 bytes per opened file).
context:   The default is usually adequate for typical programs; can be overridden with -M= compiler option.
source:    OS-9 C Compiler manual, §"Compile Time Memory Allocation", p. 2-6

--- END ---
--- CARD ---
id:        memory-estimation-guidelines-parameters-stack-locals
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     Memory estimation rules: (1) parameter area must fit any anticipated command-line string, (2) stack should be ≥128 bytes and account for function-call depth and recursion, (3) function arguments and local variables occupy stack space (each function entry adds 4 bytes overhead).
context:   These are rough guidelines; actual requirements depend on the specific program.
source:    OS-9 C Compiler manual, §"Compile Time Memory Allocation", p. 2-6

--- END ---
--- CARD ---
id:        standard-library-file-buffers-256-bytes-per-file
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     The standard library requests 256 bytes per opened file for buffering (via stdio functions like fopen/fread/fwrite); does not apply to lower-level I/O (open/read/write) or unbuffered stderr.
context:   Total memory estimate should account for expected number of concurrently open files.
source:    OS-9 C Compiler manual, §"Compile Time Memory Allocation", p. 2-6

--- END ---
--- CARD ---
id:        position-independent-reentrant-code-generation
type:      FACT
target:    68k
verify:    from-manual
topic:     code-generation
claim:     The compiler generates position-independent, reentrant code; the module format and reference tables ensure that compiled programs can be loaded at any address and shared across multiple concurrent processes.
context:   Reentrancy assumes no self-modifying code and proper use of dynamic allocation; C programmers don't need to worry about relocation details (handled by linker/startup).
source:    OS-9 C Compiler manual, §"The Object Code Module", p. 2-1

--- END ---
--- CARD ---
id:        uninitialized-global-static-zero-initialization
type:      FACT
target:    68k
verify:    from-manual
topic:     initialization
claim:     Uninitialized global and static variables are guaranteed to have the value zero at program startup (the startup routine clears the uninitialized data area).
context:   This follows the C language spec and PDP-11 implementation conventions.
source:    OS-9 C Compiler manual, §"Characteristics of Compiled Programs", p. 2-4

--- END ---
--- CARD ---
id:        direct-page-variables-not-portable
type:      CONCEPT
target:    6809
verify:    from-manual
topic:     direct-storage-class
claim:     The `direct` storage class is unique to this OS-9/6809 C compiler and may not be portable to other environments without modification; programs using direct should be rewritten to use standard storage classes for portability.
context:   Direct variables offer 6809-specific performance gains (fast addressing) but lock programs to that architecture.
source:    OS-9 C Compiler manual, §"The 'Direct' Storage Class", p. 1-4

--- END ---
--- CARD ---
id:        system-interface-names-vs-os9-names
type:      ROSETTA
target:    68k
verify:    from-manual
topic:     system-calls
claim:     Some C system call names follow UNIX convention rather than OS-9 assembler names (e.g., C `open()` vs OS-9 assembler `I$OPEN`); a cross-reference list is provided in the manual for programmers familiar with OS-9 names.
context:   This naming choice aids portability from UNIX systems; OS-9-specific calls without UNIX equivalents use OS-9 names in C as well.
source:    OS-9 C Compiler manual, §"Operating System Calls", p. 1-7

--- END ---
--- CARD ---
id:        char-to-int-sign-extension-conversion
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     CHAR values are converted to INT by sign extension (the sign bit of the CHAR is extended into the upper bits of the INT).
context:   This affects how character values are treated in integer contexts; a CHAR with value 255 becomes -1 when promoted to INT (two's complement).
source:    OS-9 C Compiler manual, §"Data Representation and Storage Requirements", p. 1-5

--- END ---
--- CARD ---
id:        long-float-type-aliases
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     Following PDP-11 convention, "LONG FLOAT" is an alias for DOUBLE; no "LONG INT" exists as a distinct type (it is equivalent to LONG).
context:   Use DOUBLE or LONG directly; LONG FLOAT is recognized but redundant.
source:    OS-9 C Compiler manual, §"Data Representation and Storage Requirements", p. 1-5

--- END ---
--- CARD ---
id:        pdp11-implementation-conventions-charset-format
type:      FACT
target:    68k
verify:    from-manual
topic:     implementation-notes
claim:     The OS-9 C compiler follows PDP-11 implementation conventions for character set, data format, and floating-point representation, ensuring compatibility with programs ported from PDP-11/UNIX systems.
context:   This historical decision aids portability but may diverge from modern systems.
source:    OS-9 C Compiler manual, §"Data Representation and Storage Requirements", p. 1-5

--- END ---
--- CARD ---
id:        preprocessor-directive-limitations-multiline
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     preprocessor
claim:     Preprocessor directives cannot be extended over multiple lines via backslash continuation; macro definitions and strings must fit on a single source line.
context:   This limits complexity of compile-time constructs; workaround is to break long strings into multiple initialized char arrays or use external build tools.
source:    OS-9 C Compiler manual, §"Differences From K&R", p. 1-2

--- END ---
--- CARD ---
id:        preprocessor-conditional-ifdef-not-constant-expression
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     preprocessor
claim:     The preprocessor supports `#ifdef`, `#ifndef`, `#else`, and `#endif` directives, but does NOT support `#if <constant expression>` form.
context:   Use #ifdef/#ifndef with -D= compiler flags for conditional compilation rather than #if expressions.
source:    OS-9 C Compiler manual, §"Differences From K&R", p. 1-2

--- END ---
--- CARD ---
id:        bit-fields-not-supported
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     language-limitations
claim:     Bit field declarations in structures are not supported; attempting to use bit fields will result in compilation error.
context:   Use manual bit manipulation or byte fields as workaround.
source:    OS-9 C Compiler manual, §"Differences From K&R", p. 1-2

--- END ---
--- CARD ---
id:        constant-expression-initializers-operand-restrictions
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     initialization
claim:     Constant expressions used in initializers may only include arithmetic operators if all operands are of type INT or CHAR; mixed-type or LONG/FLOAT/DOUBLE expressions in initializers are not supported.
context:   Workaround: compute constant values offline and hard-code them as literals, or move initialization to program startup code.
source:    OS-9 C Compiler manual, §"Differences From K&R", p. 1-2

--- END ---
--- CARD ---
id:        older-assignment-operators-not-supported
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     language-limitations
claim:     Older forms of assignment operators (e.g., `=+`, `=-`) recognized by some C compilers are not supported; use modern forms (`+=`, `-=`).
context:   This affects only archaic code; modern C style uses the new forms.
source:    OS-9 C Compiler manual, §"Differences From K&R", p. 1-2

--- END ---
--- CARD ---
id:        include-file-angle-brackets-perps-directory
type:      FACT
target:    68k
verify:    from-manual
topic:     preprocessor
claim:     When using angle brackets in #include directives (e.g., `#include <stdio.h>`), the preprocessor searches for files starting at the "perps" (presumably PERPS system directory) rather than the current directory.
context:   Use double quotes for current-directory includes; use angle brackets for system includes.
source:    OS-9 C Compiler manual, §"C Compiler Component Files and File Usage", p. 1-10

--- END ---

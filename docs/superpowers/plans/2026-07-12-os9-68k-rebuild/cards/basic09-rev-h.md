--- CARD ---
id:        basic09-architecture-independent
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-scope
claim:     BASIC09 is an enhanced structured BASIC language created for the 6809 microprocessor, but runs architecture-independent on both 6809 and 68000. It includes elements of PASCAL for modularity and structured programming.
context:   The introduction explicitly states "created for the 6809" but the language constructs themselves (PROCEDURE-based, PASCAL-style control structures) are portable.
source:    BASIC09 Reference Manual (Rev H), Introduction, p. 1-1
--- END ---

--- CARD ---
id:        basic09-interactive-compiler-model
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-architecture
claim:     BASIC09 is an Interactive Compiler that combines fast execution of compiled languages with the ease of use and memory efficiency of interpreters. It includes an integrated text editor, multipass compiler, run-time interpreter, interactive debugger, and system executive.
context:   This architecture is unique and central to how BASIC09 works; users compile and run programs interactively without a separate compile-then-run cycle.
source:    BASIC09 Reference Manual (Rev H), Introduction, p. 1-1 to 1-2
--- END ---

--- CARD ---
id:        basic09-i-code-intermediate-format
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-compilation
claim:     BASIC09 programs are stored in memory as I-code (Intermediate Code), a binary representation between source and machine language. I-code is compact (about 30% smaller than source), executes rapidly, and can be reconstructed back to source by the decompiler.
context:   This I-code format is fundamental to BASIC09's ability to provide instant syntax checking and to allow editing of compiled procedures.
source:    BASIC09 Reference Manual (Rev H), Edit Mode, "How the Editor Works", p. 4-2
--- END ---

--- CARD ---
id:        basic09-procedure-based-structure
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     BASIC09 programs are organized as named procedures, not as monolithic programs. Multiple independent procedures can exist in the workspace simultaneously, each with its own local variables and line numbers (if used).
context:   Procedures are the fundamental building block; they enable modular development and allow reuse via libraries.
source:    BASIC09 Reference Manual (Rev H), Introduction to BASIC09, p. 2-2
--- END ---

--- CARD ---
id:        basic09-variable-locality
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-variables
claim:     Each BASIC09 variable is local to the procedure where it is defined and only known within that procedure. Storage is allocated from the workspace when the procedure is called and returned when the procedure exits.
context:   Variables do not persist across procedure calls unless explicitly passed as parameters. Procedures can call themselves (recursion), creating separate variable storage.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Variables", p. 7-7
--- END ---

--- CARD ---
id:        basic09-variable-initialization-warning
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-variables
claim:     BASIC09 does NOT automatically initialize variables. When a procedure runs, all variables, arrays, and structures have random values. The program must explicitly assign initial values.
context:   This is a significant gotcha for programmers familiar with languages that zero-initialize memory.
source:    BASIC09 Reference Manual (Rev H), "Variables", p. 7-7
--- END ---

--- CARD ---
id:        basic09-parameter-passing-by-reference
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions
claim:     Parameters are passed by reference by default: the storage address of the variable is sent to the called procedure, allowing the called procedure to return values to the caller by modifying the original variable.
context:   This is distinct from pass-by-value semantics.
source:    BASIC09 Reference Manual (Rev H), "Parameter Variables", p. 7-7
--- END ---

--- CARD ---
id:        basic09-parameter-passing-by-value
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions
claim:     Parameters can be passed by value by wrapping the variable in an expression that evaluates but doesn't alter it (e.g., x+0 for numbers, word$+"" for strings). A temporary variable is created and the called procedure receives a copy.
context:   This protects the original variable from being modified by the called procedure.
source:    BASIC09 Reference Manual (Rev H), "Parameter Variables", p. 7-7 to 7-8
--- END ---

--- CARD ---
id:        basic09-run-statement-syntax
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-execution
claim:     The RUN command executes a procedure. If the procedure expects parameters, they can be passed on the same command line (e.g., RUN add(4,7)), but all parameters must be constants or constant expressions, not variables.
context:   This is used both from System Mode and can be used inside procedures via the RUN statement.
source:    BASIC09 Reference Manual (Rev H), Execution Mode, p. 5-1
--- END ---

--- CARD ---
id:        basic09-module-linking
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-modular-system
claim:     BASIC09 can link to external procedures stored as OS-9 memory modules (in ROM or RAM). If a procedure is not found in the workspace, BASIC09 calls OS-9 to link to an external module, falling back to loading from a file if that fails.
context:   This allows reuse of library procedures that may be written in BASIC09 or machine language, potentially shared by multiple users.
source:    BASIC09 Reference Manual (Rev H), System Mode, "RUN command", p. 3-7
--- END ---

--- CARD ---
id:        basic09-workspace-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-memory-management
claim:     The workspace is the area of memory used by BASIC09 to store procedures and their variable data. BASIC09 reserves approximately 1.2K bytes for internal use; the rest is for procedure storage and variable data during execution.
context:   If the workspace fills up, procedures cannot run. The MEM command allows enlarging or shrinking the workspace (to a minimum).
source:    BASIC09 Reference Manual (Rev H), Introduction to BASIC09, "More About the Workspace", p. 2-11
--- END ---

--- CARD ---
id:        basic09-data-type-real-format
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     REAL numbers are stored in 5 consecutive bytes: one 8-bit exponent in two's-complement, and four bytes for the mantissa (31 bits) plus sign bit. Range is 2.938735877 * 10^-39 through 1.701411835 * 10^38. Precision is approximately 9 decimal digits.
context:   This is BASIC09's floating-point format; REAL is the default type for undeclared numeric variables.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Type REAL", p. 7-3
--- END ---

--- CARD ---
id:        basic09-data-type-integer-wraparound
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     INTEGER arithmetic that overflows (e.g., 32767 + 1) wraps around modulo 65536 rather than raising an error. Programmers should be aware that comparisons in the range 32767-65535 actually compare negative numbers.
context:   This silent wraparound can cause subtle bugs; it's not an error condition.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Type INTEGER", p. 7-2
--- END ---

--- CARD ---
id:        basic09-data-type-byte-truncation
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     Storing an integer value outside the BYTE range (0-255) to a BYTE variable stores only the least-significant 8 bits (modulo 256) without error.
context:   This is silent truncation, not an error.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Type BYTE", p. 7-2
--- END ---

--- CARD ---
id:        basic09-data-type-string-storage
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     STRINGs are variable-length with a default maximum of 32 characters (dimensionable). Internally, strings are null-terminated and stored in fixed-size areas; remaining unused bytes after the terminator allow the string to expand/contract during execution.
context:   String concatenation can grow the string up to its maximum length.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Type STRING", p. 7-4
--- END ---

--- CARD ---
id:        basic09-data-type-boolean-operations
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BOOLEAN values (TRUE or FALSE) cannot be used for numeric computation. BOOLEAN operations (AND, OR, XOR, NOT) operate on Boolean values, distinct from the logical bit-by-bit functions (LAND, LOR, LXOR, LNOT) which operate on integers.
context:   Attempting to store a non-BOOLEAN value to a BOOLEAN variable causes a run-time error.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Type BOOLEAN", p. 7-4
--- END ---

--- CARD ---
id:        basic09-complex-data-types
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     User-defined complex data types can be created using the TYPE statement, allowing a single data type to contain mixed-type fields accessed by name. Complex structures are more efficient than arrays because field positions are known at compile-time and need not be calculated at run-time.
context:   This mirrors PASCAL record structures and enables more expressive data modeling than simple arrays.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Complex Data Types", p. 7-8 to 7-9
--- END ---

--- CARD ---
id:        basic09-array-indexing-base-modes
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BASIC09 supports two array indexing modes: BASE0 (indices 0 to n-1) and BASE1 (indices 1 to n). Default is BASE1. The BASEN statement switches modes. This affects how STRING indices work: in BASE1, the first character is position 1; in BASE0, position 0.
context:   -
source:    BASIC09 Reference Manual (Rev H), Data Types section, p. 7-4
--- END ---

--- CARD ---
id:        basic09-atomic-data-type-summary
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BASIC09 has five atomic data types: BYTE (0-255, 1 byte), INTEGER (-32768 to 32767, 2 bytes), REAL (floating-point, 5 bytes), STRING (variable-length), BOOLEAN (true/false, 1 byte). REAL is the default for undeclared numeric variables.
context:   -
source:    BASIC09 Reference Manual (Rev H), Data Types section, p. 7-2
--- END ---

--- CARD ---
id:        basic09-addr-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-io-memory
claim:     The ADDR() function returns the absolute memory address of a variable, array, or structure. This is used to directly access memory locations (e.g., with POKE/PEEK) or to pass addresses to external routines.
context:   This is a low-level facility; misuse can corrupt memory.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-5
--- END ---

--- CARD ---
id:        basic09-peek-poke-statements
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-io-memory
claim:     PEEK(<address>) reads a byte at a memory address. POKE(<address>, <value>) stores a byte at a memory address. These allow direct memory access, bypassing normal variable storage.
context:   POKE can alter any memory address, so care is required to avoid corrupting the system or other programs.
source:    BASIC09 Reference Manual (Rev H), Program Statements section, "POKE Statement", p. 9-3
--- END ---

--- CARD ---
id:        basic09-pack-command-security
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-deployment
claim:     The PACK command compiles a procedure into a packed module (OS-9 memory module format) suitable for ROM or RAM. Packed procedures are smaller, faster, and cannot be edited or debugged, providing security for intellectual property.
context:   Packed modules can be executed by BASIC09 or by the RunB (BASIC09 run-time-only) program. Always SAVE before PACKing, since packed procedures cannot be loaded back into the workspace.
source:    BASIC09 Reference Manual (Rev H), System Mode, "PACK command", p. 3-6
--- END ---

--- CARD ---
id:        basic09-runb-runtime-package
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-deployment
claim:     RunB is a BASIC09 run-time package (approximately half the size of BASIC09) that executes only packed modules. It cannot edit, debug, or create new procedures. When a packed module is invoked at the OS-9 prompt, Shell loads and forks RunB to execute it.
context:   RunB enables distribution of BASIC09 programs without shipping the full development environment.
source:    BASIC09 Reference Manual (Rev H), Appendix D - RunB, p. D-1
--- END ---

--- CARD ---
id:        basic09-shell-command-gateway
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-os9-integration
claim:     The $ command in System Mode calls the OS-9 Shell command interpreter. If followed by text, it executes a single OS-9 command (e.g., $copy filel file2). If given alone, it suspends BASIC09 and allows multiple interactive OS-9 commands, returning on EOF (usually ESCAPE).
context:   This provides a gateway to OS-9 from inside BASIC09 without disturbing the workspace.
source:    BASIC09 Reference Manual (Rev H), System Mode, "$ command", p. 3-3
--- END ---

--- CARD ---
id:        basic09-chd-chx-directory-change
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-os9-integration
claim:     CHD changes the current OS-9 user Data Directory (used for LOAD/SAVE procedures). CHX changes the Execution Directory (used for PACK or auto-load of packed modules). Both take a pathlist argument.
context:   These commands directly interact with OS-9 directory switching.
source:    BASIC09 Reference Manual (Rev H), System Mode, "CHD/CHX commands", p. 3-3
--- END ---

--- CARD ---
id:        basic09-mem-command-workspace-resizing
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-memory-management
claim:     MEM without arguments displays current workspace size in bytes. MEM <size> resizes the workspace to that many bytes (rounded up to next multiple of 256). Hex values prefixed with $ are accepted. Returns "WHAT?" if requested size unavailable or too small for existing procedures.
context:   BASIC09 starts with 4K bytes by default but can request more via the % option at startup (e.g., basic09 %16K).
source:    BASIC09 Reference Manual (Rev H), System Mode, "MEM command", p. 3-5 and Introduction section, p. 2-7
--- END ---

--- CARD ---
id:        basic09-error-handling-on-error-goto
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-error-handling
claim:     The ON ERROR GOTO statement traps run-time errors and transfers control to a specified line number. This allows programs to handle errors gracefully rather than terminating. RunB (the run-time package) can trap CONTROL-C and CONTROL-Q via ON ERROR GOTO, while BASIC09 itself cannot.
context:   -
source:    BASIC09 Reference Manual (Rev H), Appendix D - RunB, p. D-1
--- END ---

--- CARD ---
id:        basic09-err-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-error-handling
claim:     The ERR function returns the error code of the most recent error and automatically resets to zero when referenced. This allows interrogation of what error occurred.
context:   Error codes above 80 are OS-9 or external errors; codes 0-80 are BASIC09-specific.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-5
--- END ---

--- CARD ---
id:        basic09-i-o-formatting-print-using
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-io-formatting
claim:     PRINT USING allows FORTRAN-like format specifications for output. Format strings control how values are printed (e.g., width, decimal places, hexadecimal). This is available in normal PRINT statements and in Debug Mode PRINT commands.
context:   This is more flexible than simple PRINT with separators.
source:    BASIC09 Reference Manual (Rev H), Features list, p. 1-1
--- END ---

--- CARD ---
id:        basic09-i-o-file-operations
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-io-files
claim:     BASIC09 supports file I/O via OPEN, CLOSE, READ, WRITE, SEEK, and DISPLAY statements. Files are accessed via path numbers (file descriptors). Input/output can be redirected to any OS-9 file or device via the standard > and # notation.
context:   This integrates seamlessly with OS-9 file management.
source:    BASIC09 Reference Manual (Rev H), Index and references throughout
--- END ---

--- CARD ---
id:        basic09-eof-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-io-files
claim:     The EOF(#<pathnum>) function returns TRUE if an end-of-file condition exists on the specified path number (file descriptor), allowing programs to detect when all data has been read.
context:   -
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-6
--- END ---

--- CARD ---
id:        basic09-debug-mode-trace
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     Debug Mode trace (TRON/TROFF) reconstructs I-code back to source statements and displays each statement before execution, showing expression results on the following line. Trace is local to a procedure; tracing stops when calling another procedure (unless it also has trace on).
context:   This provides single-step debugging with source-level visibility.
source:    BASIC09 Reference Manual (Rev H), Debug Mode, "TRON/TROFF commands", p. 6-4
--- END ---

--- CARD ---
id:        basic09-debug-mode-breakpoint
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The BREAK command sets a breakpoint on a named procedure. The breakpoint is triggered when that procedure is returned to (via procedure nesting). You can set one breakpoint per active procedure. The breakpoint is removed as soon as it occurs.
context:   This is used in multi-procedure debugging to halt execution at a specific procedure during a call stack.
source:    BASIC09 Reference Manual (Rev H), Debug Mode, "BREAK command", p. 6-2
--- END ---

--- CARD ---
id:        basic09-debug-mode-state-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The STATE command lists the calling (nesting) order of all active procedures. The highest-level (outermost) procedure is shown at the bottom; the lowest-level (currently suspended) procedure is shown at the top.
context:   This reveals the call stack at the point of suspension.
source:    BASIC09 Reference Manual (Rev H), Debug Mode, "STATE command", p. 6-3 to 6-4
--- END ---

--- CARD ---
id:        basic09-decompiler-formatting
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-program-formatting
claim:     When BASIC09 lists programs, they may have slightly different appearance than originally typed (extraneous spaces removed, unnecessary parentheses removed, keywords capitalized). However, they are always functionally identical to the original source.
context:   This is a result of decompiling I-code back to source. Programmers should not rely on exact formatting preservation.
source:    BASIC09 Reference Manual (Rev H), Edit Mode, "How the Editor Works", p. 4-2
--- END ---

--- CARD ---
id:        basic09-line-numbers-optional
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     Line numbers are optional in BASIC09. They should only be used where necessary (e.g., with GOSUB) because they make programs harder to understand, consume memory, and increase compile time.
context:   This is a significant departure from traditional BASIC and enables modern structured programming.
source:    BASIC09 Reference Manual (Rev H), Program Statements section, "Line Numbers", p. 9-1
--- END ---

--- CARD ---
id:        basic09-line-number-scope
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     Line numbers are local to procedures. The same line number can be used in different procedures without conflict.
context:   -
source:    BASIC09 Reference Manual (Rev H), Program Statements section, "Line Numbers", p. 9-1
--- END ---

--- CARD ---
id:        basic09-save-command-format
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-storage
claim:     The SAVE command writes procedures to disk in source format. If no filename is given, the name of the (first) procedure is used. Multiple procedures can be saved to one file with comma syntax (e.g., SAVE proc1,proc2,proc3 >filename). If a file exists, SAVE prompts for overwrite confirmation.
context:   Procedures must be SAVEd before exiting BASIC09 or they are lost.
source:    BASIC09 Reference Manual (Rev H), System Mode, "SAVE command", p. 3-8
--- END ---

--- CARD ---
id:        basic09-load-command-workspace-replacement
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-program-storage
claim:     The LOAD command replaces any procedures in the workspace with the same name as procedures being loaded. To avoid losing work, RENAME or SAVE a conflicting procedure before LOAD.
context:   This can accidentally overwrite procedures not yet saved to disk.
source:    BASIC09 Reference Manual (Rev H), Introduction to BASIC09, "Storing and Recalling Programs", p. 2-8
--- END ---

--- CARD ---
id:        basic09-control-structures-closure
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-control-structures
claim:     BASIC09 control structures (IF/THEN/ENDIF, FOR/NEXT, WHILE/ENDWHILE, REPEAT/UNTIL, LOOP/ENDLOOP, EXITIF/ENDEXIT) all use explicit closure elements (matching END* statements), enabling proper nesting and clear program structure without line-number-based branching.
context:   This structured approach prevents the "spaghetti code" that can result from GOTO/GOSUB overuse.
source:    BASIC09 Reference Manual (Rev H), Features list, p. 1-1
--- END ---

--- CARD ---
id:        basic09-for-next-step-negative
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-control-structures
claim:     FOR/NEXT loops support the STEP clause with negative values, causing the loop to count down. Loop execution terminates when the counter is less than the termination value (for negative STEP).
context:   -
source:    BASIC09 Reference Manual (Rev H), Program Statements section, "FOR/NEXT Statement", p. 9-4
--- END ---

--- CARD ---
id:        basic09-gosub-return-semantics
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-control-structures
claim:     GOSUB transfers execution to a line number (or label) and pushes the return address on the call stack. RETURN pops the return address and resumes execution. GOSUB creates a subroutine call frame separate from procedure calls.
context:   GOSUB/RETURN can be used alongside procedure calls for traditional BASIC-style subroutines.
source:    BASIC09 Reference Manual (Rev H), Index references
--- END ---

--- CARD ---
id:        basic09-renumber-command-procedure-scope
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-editing
claim:     The RENUMBER command uniformly resequences numbered lines in a procedure. Format: r [<start_line>] [,<increment>] or r* for all lines. Default start is 100, default increment is 10.
context:   This is useful for reorganizing line-numbered procedures.
source:    BASIC09 Reference Manual (Rev H), Edit Mode, "LINE-NUMBER ORIENTED EDITING", p. 4-3
--- END ---

--- CARD ---
id:        basic09-expression-stack-evaluation
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-expressions
claim:     Expression evaluation uses an expression stack where values are pushed and operators/functions consume and return results. This allows assignment statements like X=X+1 to work because the variable being assigned can be one of the values in the expression.
context:   -
source:    BASIC09 Reference Manual (Rev H), Expressions section, p. 8-1
--- END ---

--- CARD ---
id:        basic09-type-conversion-automatic
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     Expressions mixing numeric types (BYTE, INTEGER, REAL) are automatically converted to the largest type needed to retain accuracy. Mixing incompatible types (e.g., STRING with numeric) causes compile-time type-mismatch errors.
context:   Type conversion takes time, so expressions of uniform types are preferred.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Automatic Type Conversion", p. 7-6
--- END ---

--- CARD ---
id:        basic09-operator-precedence
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-expressions
claim:     Operator precedence (highest to lowest): NOT/-negate, ^ /**, *, /, +, -, comparison operators (>, <, etc.), AND, OR, XOR. Operators of equal precedence evaluate left-to-right, except exponentiation (right-to-left). Parentheses override precedence.
context:   -
source:    BASIC09 Reference Manual (Rev H), Expressions section, "Operator Precedence", p. 8-3
--- END ---

--- CARD ---
id:        basic09-string-concatenation
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-operators
claim:     The + operator concatenates STRING operands. The concatenation result must fit within the destination string's maximum length; overflow is silent truncation without error.
context:   -
source:    BASIC09 Reference Manual (Rev H), Operators section, p. 8-2
--- END ---

--- CARD ---
id:        basic09-bitwise-logical-functions
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-operators
claim:     Bitwise logical functions (LAND, LOR, LXOR, LNOT) perform bit-by-bit operations on integer/byte data and return integer results. These are distinct from Boolean operators (AND, OR, XOR, NOT) which operate on TRUE/FALSE values.
context:   LNOT operates on one operand; the others take two.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-6
--- END ---

--- CARD ---
id:        basic09-random-number-rnd
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions-math
claim:     RND(<num>) returns: if num=0, random x where 0<=x<1; if num>0, random x where 0<=x<num; if num<0, uses ABS(num) as new random seed.
context:   RND is used to generate pseudo-random sequences and seed the random number generator.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-4
--- END ---

--- CARD ---
id:        basic09-transcendental-functions-accuracy
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions-math
claim:     Transcendental functions (SIN, COS, TAN, ASN, ACS, ATN, LOG, LOG10, EXP, SQR, SQRT) return REAL results with accuracy of 8+ decimal digits. Angles default to radians but can be set to degrees with the DEG command.
context:   -
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-4
--- END ---

--- CARD ---
id:        basic09-print-tab-statement
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-io-formatting
claim:     TAB(<expr>) in a PRINT statement moves the print cursor to the column specified by <expr>. This allows formatted output without needing PRINT USING.
context:   -
source:    BASIC09 Reference Manual (Rev H), I/O Functions section, p. 10-4
--- END ---

--- CARD ---
id:        basic09-pos-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-io-formatting
claim:     The POS function returns the current character position in the PRINT buffer (column position of the next character to be printed).
context:   This is useful for formatting decisions within a program.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-5
--- END ---

--- CARD ---
id:        basic09-pause-statement-debug-entry
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The PAUSE statement suspends program execution and enters Debug Mode with the prompt "D:". This is used to halt execution at specific points for inspection and single-stepping.
context:   Unlike errors or CONTROL-C, PAUSE is intentional and can be embedded in the program logic.
source:    BASIC09 Reference Manual (Rev H), Debug Mode overview, p. 6-1
--- END ---

--- CARD ---
id:        basic09-step-command-source-level
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The STEP command (or just <CR> in Debug Mode) executes one or more source statements at a time. STEP 5 executes the next 5 source statements. A plain carriage return is equivalent to STEP 1.
context:   Note that FOR/NEXT loop structure statements execute once each, so their "top" or "bottom" statement may not execute as many times as expected.
source:    BASIC09 Reference Manual (Rev H), Debug Mode, "STEP command", p. 6-3
--- END ---

--- CARD ---
id:        basic09-let-debug-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     In Debug Mode, the LET command allows setting a variable to a new value: LET <var> := <expr>. Variable names must match the original source program; new variables cannot be created in Debug Mode, and user-defined data structures are not supported.
context:   This enables modification of program state during debugging.
source:    BASIC09 Reference Manual (Rev H), Debug Mode, "LET command", p. 6-3
--- END ---

--- CARD ---
id:        basic09-procedure-name-rules
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     A procedure name may be any combination of alphanumeric characters beginning with a letter. Identifiers are multi-character and case-insensitive.
context:   This is more flexible than many traditional BASIC systems which required single-letter variable names or very restricted naming.
source:    BASIC09 Reference Manual (Rev H), Introduction to BASIC09, p. 2-2
--- END ---

--- CARD ---
id:        basic09-date-string-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions-io
claim:     The DATES() function returns the current date and time as a STRING in format "yy/mm/dd hh:mm:ss", e.g., "26/07/12 14:30:45".
context:   This reads the OS-9 system clock.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-6
--- END ---

--- CARD ---
id:        basic09-dimension-string-length
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     STRING variables can be dimensioned with a custom maximum length using bracket notation: DIM title:STRING[80]. The default maximum is 32 characters. Shortening a string's maximum length (e.g., STRING[4]) saves memory.
context:   -
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Type STRING", p. 7-4
--- END ---

--- CARD ---
id:        basic09-substr-string-search
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions-strings
claim:     SUBSTR(<search_str>, <source_str>) returns the starting position of the first occurrence of <search_str> within <source_str>, or 0 if not found. Position numbering follows the current BASE mode (BASE0 or BASE1).
context:   -
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-5
--- END ---

--- CARD ---
id:        basic09-hex-constant-syntax
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-constants
claim:     INTEGER hexadecimal constants are represented with a leading $ (e.g., $20, $FFFF). Hex values can range from $0000 to $FFFF (0 to 65535 unsigned). This facilitates address calculations.
context:   -
source:    BASIC09 Reference Manual (Rev H), Constants section, p. 7-6
--- END ---

--- CARD ---
id:        basic09-scientific-notation
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-constants
claim:     REAL constants can use scientific notation with E format (e.g., 1.95E+12, 99999.9E-33). A decimal point or E format forces the constant to be stored as REAL even if it could fit in INTEGER/BYTE format.
context:   -
source:    BASIC09 Reference Manual (Rev H), Constants section, p. 7-6
--- END ---

--- CARD ---
id:        basic09-kill-asterisk-workspace-clear
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-management
claim:     The KILL* command erases all procedures from the workspace, effectively resetting BASIC09 to its initial state. This operation may take time if many procedures exist.
context:   This is equivalent to completely restarting BASIC09.
source:    BASIC09 Reference Manual (Rev H), System Mode, "KILL command", p. 3-4
--- END ---

--- CARD ---
id:        basic09-dir-command-current-procedure
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-management
claim:     The DIR command lists procedure names, sizes, and data size requirements. An asterisk marks the current working procedure (last edited, listed, saved, or run). Packed procedures are marked with a dash. A question mark next to data size indicates insufficient workspace memory to run that procedure.
context:   -
source:    BASIC09 Reference Manual (Rev H), System Mode, "DIR command", p. 3-4
--- END ---

--- CARD ---
id:        basic09-list-command-pretty-printing
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-listing
claim:     The LIST command outputs formatted "pretty-printed" source code with I-code addresses in the first column (hexadecimal) and line numbers (if used) in the second column. LIST can be redirected to files/devices for hard-copy printing (e.g., LIST* /p).
context:   The I-code addresses are useful for locating lines when the compiler reports errors with I-code addresses.
source:    BASIC09 Reference Manual (Rev H), System Mode, "LIST command", p. 3-5
--- END ---

--- CARD ---
id:        basic09-pack-before-save-warning
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-deployment
claim:     Always SAVE a procedure before PACKing it. Packed modules cannot be loaded back into the workspace; if you PACK a procedure, then KILL it from memory, the only copy left is the packed (non-editable) version.
context:   This is a critical point to avoid losing editable source.
source:    BASIC09 Reference Manual (Rev H), System Mode, "PACK command", p. 3-6
--- END ---

--- CARD ---
id:        basic09-mode-transition-diagram
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-execution-model
claim:     BASIC09 has four modes (System, Edit, Execution, Debug) with defined transitions: System Mode is the command processor (B: prompt); Edit Mode (E: prompt) for entering/modifying procedures; Execution Mode for running procedures; Debug Mode (D: prompt) for testing. Transitions are controlled by specific commands and events.
context:   Understanding mode transitions is essential for navigating BASIC09.
source:    BASIC09 Reference Manual (Rev H), Introduction to BASIC09, p. 2-10
--- END ---

--- CARD ---
id:        basic09-ctrl-c-interrupt-behavior
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-execution-control
claim:     CONTROL-C (shift-BREAK on some systems) interrupts program execution and enters Debug Mode, allowing inspection and resumption via CONT.
context:   This is different from normal program termination; Debug Mode provides access to program state.
source:    BASIC09 Reference Manual (Rev H), Execution Mode, p. 5-1
--- END ---

--- CARD ---
id:        basic09-ctrl-q-termination
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-execution-control
claim:     CONTROL-Q terminates program execution and returns to System Mode. Unlike CONTROL-C, it does not enter Debug Mode. In RunB, CONTROL-Q can be trapped by ON ERROR GOTO.
context:   -
source:    BASIC09 Reference Manual (Rev H), Execution Mode, p. 5-1
--- END ---

--- CARD ---
id:        basic09-autorun-feature-syntax
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-execution-model
claim:     BASIC09 supports autorun: at the OS-9 prompt, basic09 <filename>[(parameters)] loads the file and runs the procedure with the same name as the file. The file can be either a SAVED procedure file or a PACKED module. Parameters can be passed on the command line.
context:   This allows BASIC09 programs to be invoked directly from the shell like normal OS-9 commands.
source:    BASIC09 Reference Manual (Rev H), Execution Mode, "Auto-run feature", p. 5-2
--- END ---

--- CARD ---
id:        basic09-module-format-os9-standard
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-deployment
claim:     Packed BASIC09 procedures are stored in OS-9 memory module format, suitable for loading into ROM or RAM outside the workspace. The format is compatible with the standard OS-9 LINK mechanism.
context:   This allows BASIC09 code to be distributed as native OS-9 modules alongside machine-language programs.
source:    BASIC09 Reference Manual (Rev H), System Mode, "PACK command", p. 3-6
--- END ---

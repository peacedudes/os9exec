--- CARD ---
id:        microware-basic-68k-architecture
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     basic09-design-history
claim:     Microware BASIC was originally called Basic09 for the 6809 microprocessor (conceived 1978, first release Feb 1980). It was then ported to the 68000 architecture. Version 2.4 (Revision G, Jan 1991) is the documented variant for OS-9 running on 68000.
context:   The 68k version adds performance optimizations exploiting the 68000's instruction set; BASIC09 itself remains architecture-independent at the language level but has processor-specific optimizations.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface, p. 3-4
--- END ---

--- CARD ---
id:        microware-basic-interactive-compiler
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     compilation-model
claim:     Microware BASIC is an interactive compiler that produces I-code (Intermediate Code), a compressed binary format between source and machine language. It combines fast compilation and execution speed with ease of use and memory efficiency of interpreted languages.
context:   Unlike traditional BASIC interpreters that compile at runtime or compile to native code without debugging aids, Microware BASIC offers syntax checking on line entry, decompilation back to source form, and runtime error checking.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface, p. 3
--- END ---

--- CARD ---
id:        data-type-integer-range-68k
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     INTEGER data type range: –2,147,483,648 to 2,147,483,647 (32-bit signed).
context:   This is the 68k architecture range for signed 32-bit integers; INTEGER variables are faster than REAL and use less memory.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Variable Data Types", p. 20
--- END ---

--- CARD ---
id:        data-type-real-precision
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     REAL data type: 14 decimal-digit 64-bit binary floating point, range ±2.210^-308 to ±1.810^308.
context:   The 64-bit IEEE-like floating point offers high precision but slower operations than INTEGER arithmetic.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Variable Data Types", p. 20; preface feature list, p. 4
--- END ---

--- CARD ---
id:        data-type-byte-range
type:      FACT
target:    68k
verify:    from-manual
topic:     data-types
claim:     BYTE data type range: 0 to 255 (unsigned 8-bit).
context:   BYTE operations are faster than INTEGER/REAL; BYTE data types may only be passed to procedures by reference, not by value.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Variable Data Types", p. 20
--- END ---

--- CARD ---
id:        data-type-string-default-length
type:      FACT
target:    all
verify:    from-manual
topic:     data-types
claim:     STRING variables default to 32 characters maximum length when undeclared. Declared STRING length via DIM: `DIM varname: STRING[len]`.
context:   Undeclared string variables must end with $ suffix and get the default 32-char allocation. Strings longer than declared length are truncated.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", p. 21
--- END ---

--- CARD ---
id:        basic09-five-data-types
type:      CONCEPT
target:    all
verify:    from-manual
topic:     data-types
claim:     Microware BASIC defines five fundamental data types: BYTE, INTEGER, REAL, BOOLEAN, and STRING. All expressions and assignments use automatic type conversion between these types.
context:   Type conversion has performance cost; matching operand types in expressions is more efficient than forcing conversions.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface features, p. 4; Chapter 2, "Variable Data Types", p. 20
--- END ---

--- CARD ---
id:        i-code-representation-format
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     compilation-model
claim:     I-code (Intermediate Code) is BASIC's internal compiled representation. Procedures are stored as I-code in the workspace. The editor integrates with the compiler/decompiler to convert source text to I-code on entry and I-code back to source on display.
context:   I-code is approximately 30% more compact than source text, enabling larger programs in given memory. I-code references use actual memory addresses, eliminating runtime table searches.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 6, "How the Editor Works", p. 79-80
--- END ---

--- CARD ---
id:        i-code-address-hexadecimal
type:      FACT
target:    68k
verify:    from-manual
topic:     compilation-model
claim:     I-code addresses are displayed in hexadecimal (base 16) and represent memory offsets within a procedure. Error messages include I-code addresses to pinpoint the failing line.
context:   When LIST or q (quit editor) shows errors, the address format is hexadecimal (e.g., "01FC ERR #000:043" means error 43 at hex address 01FC).
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Writing Your First Procedure", p. 19-20; Chapter 6, "Edit Mode", p. 80
--- END ---

--- CARD ---
id:        procedure-names-allowed-characters
type:      FACT
target:    all
verify:    from-manual
topic:     procedure-syntax
claim:     Procedure names may contain 1-28 characters: uppercase A-Z, lowercase a-z, digits 0-9, underscore (_), dollar sign ($), and period (.). Names must contain at least one letter or digit (may start with any allowed char).
context:   Despite allowing 28 characters, short names are recommended for usability.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Naming Your Procedure", p. 17-18
--- END ---

--- CARD ---
id:        reserved-words-list
type:      FACT
target:    all
verify:    from-manual
topic:     procedure-syntax
claim:     BASIC reserves all keywords and statement names as non-usable variable names. Reserved words include: ABS, ACS, ADDR, AND, ASC, ASN, ATN, BASE, BOOLEAN, BYE, BYTE, CHAIN, CHD, CHR$, CHX, CLOSE, COS, CREATE, DATA, DATE$, DEG, DELETE, DIM, DIGITS, DIR, DO, ELSE, END, ENDEXIT, ENDIF, ENDLOOP, ENDWHILE, EOF, ERR, ERROR, EXEC, EXITIF, EXP, FALSE, FILSIZ, FIX, FLOAT, FOR, GET, GOSUB, GOTO, IF, INKEY, INPUT, INT, INTEGER, KILL, LAND, LEFT$, LEN, LET, LNOT, LOG, LOG10, LOR, LOOP, LXOR, MID$, MOD, NEXT, NOT, ON, OPEN, OR, PARAM, PAUSE, PEEK, PI, POKE, POS, PRINT, PROCEDURE, PUT, RAD, READ, REAL, REM, REPEAT, RESTORE, RETURN, RIGHT$, RND, RUN, SEEK, SGN, SHELL, SIN, SIZE, SQ, SQR, SQRT, STEP, STOP, STR$, STRING, SUBSTR, TAB, TAN, THEN, TO, TRIM$, TROFF, TRON, TRUE, TYPE, UNTIL, UPDATE, USING, VAL, WHILE, WRITE, XOR.
context:   Attempting to use a reserved word as a variable name will cause a syntax error.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Naming Variables: Reserved Words", Table 2.A, p. 23-24
--- END ---

--- CARD ---
id:        dim-statement-syntax-multitype
type:      FACT
target:    all
verify:    from-manual
topic:     procedure-syntax
claim:     DIM statement syntax: `DIM var1,var2: type; var3: type2` — multiple variables of the same type use commas; different types separated by semicolons. Example: `DIM x,y,z: INTEGER; a,b,c: REAL`.
context:   DIM must appear before variable use in the procedure to prevent default type inference. Variables not declared via DIM are assigned default types based on suffix ($) or treated as REAL.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", "The DIM Statement", p. 21
--- END ---

--- CARD ---
id:        variable-default-types
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     procedure-syntax
claim:     Undeclared variables in BASIC receive default types: variables ending in $ become STRING (32 char max); all other undeclared variables become REAL numbers.
context:   This inferred declaration behavior can lead to performance issues and type mismatches. Using explicit DIM statements is preferred for clarity and optimization.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", p. 21
--- END ---

--- CARD ---
id:        assignment-syntax-let-implied
type:      FACT
target:    all
verify:    from-manual
topic:     procedure-syntax
claim:     BASIC supports both explicit LET statement and implied assignment: `LET x:=1` or `x:=1` (both forms valid; both := and = operators accepted).
context:   The implied form (variable := value) is syntactic sugar; both produce identical I-code.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", "Initializing Variables", p. 21-22
--- END ---

--- CARD ---
id:        print-statement-separator-tab-vs-consecutive
type:      FACT
target:    all
verify:    from-manual
topic:     io-operations
claim:     PRINT statement separator rules: comma moves to next 16-column tab stop before printing next value; semicolon prints values consecutively with no space.
context:   Multiple items in PRINT must be separated by either comma or semicolon; no other separators allowed.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", "Writing Your First Procedure", p. 19
--- END ---

--- CARD ---
id:        input-statement-prompt-marker
type:      FACT
target:    all
verify:    from-manual
topic:     io-operations
claim:     INPUT statement waits for keyboard input from standard input (terminal by default), terminates on [Return]. BASIC displays question mark (?) prompt to indicate waiting for input.
context:   Input redirection via OS-9 shell allows reading from files or devices instead of terminal.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", p. 19
--- END ---

--- CARD ---
id:        control-structures-extended-if-then-else-endif
type:      FACT
target:    all
verify:    from-manual
topic:     control-flow
claim:     IF..THEN..ELSE..ENDIF — extended control structure with optional ELSE clause and explicit ENDIF terminator (Pascal-like closure).
context:   Unlike traditional line-numbered BASIC's IF..THEN..GOTO, Microware BASIC uses structured block syntax.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface, p. 4; Chapter 2, Table 2.A reserved words
--- END ---

--- CARD ---
id:        control-structures-for-next-step
type:      FACT
target:    all
verify:    from-manual
topic:     control-flow
claim:     FOR..TO..STEP..NEXT — loop with optional STEP increment (default 1). Syntax: `FOR counter = start TO end [STEP incr] ... NEXT`.
context:   BASIC detects INTEGER loop counters and uses fast integer version of FOR/NEXT. REAL loop counters execute slower.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Program Optimization", "Looping Quickly", p. 62; preface feature list
--- END ---

--- CARD ---
id:        control-structures-while-do-endwhile
type:      FACT
target:    all
verify:    from-manual
topic:     control-flow
claim:     WHILE..DO..ENDWHILE — condition-checked loop that tests condition before entering body.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4
--- END ---

--- CARD ---
id:        control-structures-repeat-until
type:      FACT
target:    all
verify:    from-manual
topic:     control-flow
claim:     REPEAT..UNTIL — post-test loop that executes body at least once before testing condition.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4
--- END ---

--- CARD ---
id:        control-structures-loop-exitif-endexit
type:      FACT
target:    all
verify:    from-manual
topic:     control-flow
claim:     LOOP..ENDLOOP with EXITIF..THEN..ENDEXIT — infinite loop with conditional exit point.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4
--- END ---

--- CARD ---
id:        procedure-name-local-scope
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     procedure-management
claim:     Variables and line numbers are local to the procedure they are declared in; each procedure has its own namespace and data area.
context:   Parameter passing allows data to flow between procedures; procedures are called by name via RUN statement.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4
--- END ---

--- CARD ---
id:        parameter-passing-by-value-vs-reference
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     procedure-management
claim:     Parameter passing rules: constants and expressions passed by value (changes not returned to caller); variables, arrays, and complex data structures passed by reference (changes returned to caller). BYTE types may only be passed by reference.
context:   This mechanism allows procedures to return multiple values through reference parameters.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", "Calling Procedures", p. 53-54
--- END ---

--- CARD ---
id:        run-statement-module-linking
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     procedure-management
claim:     RUN statement procedure resolution order: (1) search workspace, (2) search data directory, (3) search execution directory. If procedure not found, BASIC attempts OS-9 LINK to external module; if LINK fails, attempts to LOAD procedure from file.
context:   This enables dynamic linking to OS-9 modules and late binding of external procedures.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", "RUN" command, p. 76-77
--- END ---

--- CARD ---
id:        pack-command-compression
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     code-optimization
claim:     PACK command compresses a procedure into I-code intermediate form and places it in execution directory. Packed procedures execute 10-30% faster than unpacked (depending on comment/line number density). Packed procedures cannot be reloaded into workspace for editing.
context:   SAVE should always precede PACK to preserve editable source.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", "PACK: Compressing Procedures", p. 16-17
--- END ---

--- CARD ---
id:        workspace-memory-allocation
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     BASIC allocates approximately 4K of workspace memory on startup. Additional memory can be requested via MEM command (always rounds up to next 256-byte multiple) or `-#` option at invocation: `basic #16k`.
context:   Workspace holds procedures in I-code form; required workspace size depends on program size and local variable data areas.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", "MEM: Displaying or Requesting Workspace Memory", p. 15-16
--- END ---

--- CARD ---
id:        dir-command-workspace-listing
type:      FACT
target:    all
verify:    from-manual
topic:     workspace-management
claim:     DIR command lists all procedures in workspace with two columns: (1) proc size = I-code size of procedure, (2) data size = memory required for procedure's local variables. Last line shows free bytes remaining in workspace.
context:   Must have at least as much free workspace as the largest data size needed to run any procedure.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", "DIR: Listing Procedure Names in Your Workspace", p. 14-15
--- END ---

--- CARD ---
id:        editor-i-code-decompiler
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     editing-system
claim:     BASIC editor integrates with a decompiler: when displaying procedures, I-code is reconstructed back to source text. The reconstruction is functionally identical to original but may differ in formatting (extraneous spaces removed, keywords auto-capitalized, unnecessary parentheses removed).
context:   This allows continuous syntax checking on line entry (immediate compiler feedback) and compact I-code storage without losing editability.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 6, "Edit Mode", "How the Editor Works", p. 79-80
--- END ---

--- CARD ---
id:        editor-line-number-range
type:      FACT
target:    all
verify:    from-manual
topic:     editing-system
claim:     Line numbers (when used) must be positive integers in range 1 to 32767. They are optional; well-structured procedures typically omit them. Unnumbered programs are shorter, faster, and easier to read.
context:   Both line-numbered and unnumbered editing modes are supported.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 6, "Edit Mode", "Line-Number Oriented Editing", p. 80-81
--- END ---

--- CARD ---
id:        editor-string-oriented-editing
type:      CONCEPT
target:    all
verify:    from-manual
topic:     editing-system
claim:     Edit mode maintains an edit pointer to track current working location. String-oriented commands (search, change, insert, delete) operate relative to edit pointer. Unnumbered editing is faster and more convenient than line-number oriented editing.
context:   Editor displays asterisk (*) to show edit pointer position; pointer moves via +/- commands or direct line number search.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 6, "Edit Mode", "String-Oriented Editing", p. 81-84
--- END ---

--- CARD ---
id:        editor-syntax-error-timing
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     editing-system
claim:     BASIC compiler performs immediate syntax checking on single-line basis when text is entered. Multi-line syntax checks (GOTO to non-existent line, improperly constructed loops, missing declarations) occur when quit (q) command exits editor.
context:   This two-pass approach: (1) immediate line-by-line checks catch most errors instantly, (2) final pass catches cross-line issues. Errors are reported with hexadecimal I-code address.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 6, "Edit Mode", "How the Editor Works", p. 80
--- END ---

--- CARD ---
id:        execution-modes-four-system-edit-execution-debug
type:      FACT
target:    68k
verify:    from-manual
topic:     program-execution
claim:     BASIC has four operational modes: (1) System Mode — execute system commands, (2) Edit Mode — create/edit procedures, (3) Execution Mode — run procedures, (4) Debug Mode — test procedures for errors.
context:   Some commands only operate in specific modes. Entering wrong mode will cause "What?" error.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", p. 13
--- END ---

--- CARD ---
id:        system-mode-prompt
type:      FACT
target:    all
verify:    from-manual
topic:     program-execution
claim:     System mode prompt is `B:` (when using BASIC interactively).
context:    -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", p. 13
--- END ---

--- CARD ---
id:        edit-mode-prompt
type:      FACT
target:    all
verify:    from-manual
topic:     program-execution
claim:     Edit mode prompt is `E:` (when editing procedures).
context:    -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", p. 13
--- END ---

--- CARD ---
id:        file-io-path-descriptor
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     File I/O uses path descriptor notation: `#path` refers to an open file path (e.g., `OPEN #file, "phonebook": UPDATE`). Path descriptors are INTEGER variables holding OS-9 file handle references.
context:   File operations (OPEN, CREATE, CLOSE, READ, WRITE, GET, PUT, SEEK) all use `#path` syntax to specify target file.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 54-57
--- END ---

--- CARD ---
id:        file-open-modes-update-create
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     File open modes: UPDATE (read/write existing file), CREATE (create new file). Syntax: `OPEN #path, "filename": UPDATE` or `CREATE #path, "filename": UPDATE`.
context:   UPDATE mode allows both reading and writing; files opened in UPDATE mode can also have SEEK operations.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 54-57
--- END ---

--- CARD ---
id:        file-seek-operation
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     SEEK #path, offset — positions file pointer to byte offset within file. Commonly used with FILSIZ() function: `SEEK #file, FILSIZ(#file)` positions to end of file for append operations.
context:   Offset is zero-based; seeking beyond file boundary is undefined behavior.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example line 171, p. 54-57
--- END ---

--- CARD ---
id:        file-filsiz-function
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     FILSIZ(#path) — returns file size in bytes for open path descriptor.
context:   Used to detect EOF and position seek operations; returns 0 if file is empty.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 54-57
--- END ---

--- CARD ---
id:        file-get-put-structured-data
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     GET #path, variable — reads structured data record from file into typed variable. PUT #path, variable — writes structured data record to file. Records are written/read in binary form matching variable structure layout.
context:   GET/PUT work with complex data types (TYPE definitions); enables efficient binary file I/O for databases and structured records.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 55-57
--- END ---

--- CARD ---
id:        type-declaration-complex-types
type:      FACT
target:    all
verify:    from-manual
topic:     data-structures
claim:     TYPE keyword declares user-defined record/structure types: `TYPE typename = field1: datatype; field2: datatype; ...`. Nested types allowed: `TYPE entry = name: STRING[30]; address: rec` where rec is another TYPE.
context:   TYPE definitions enable complex data structures similar to C structs or Pascal records. Fields are accessed via dot notation: `variable.field`.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 54-55
--- END ---

--- CARD ---
id:        type-access-dot-notation
type:      FACT
target:    all
verify:    from-manual
topic:     data-structures
claim:     Complex type field access uses dot notation: `variable.fieldname` or nested `variable.field1.field2`. Example: `Phonebook.address.street`.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 55-57
--- END ---

--- CARD ---
id:        array-declaration-multi-dimensional
type:      FACT
target:    all
verify:    from-manual
topic:     data-structures
claim:     Arrays declared via DIM: `DIM arrayname(size1 [,size2 [,size3]]): datatype`. Supports 1D, 2D, and 3D arrays.
context:   Array indexing is zero-based for type calculations internally but typically referenced 1-based in application code.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4; Chapter 2, "The DIM Statement", p. 21
--- END ---

--- CARD ---
id:        goto-label-addressing-deprecated
type:      GOTCHA
target:    all
verify:    from-manual
topic:     control-flow
claim:     GOTO and GOSUB target line numbers (not labels). Line numbers in range 1-32767. Modern Microware BASIC favors structured control flow (IF/THEN/ELSE, LOOP/WHILE/REPEAT) over GOTO.
context:   Traditional BASIC GOTO/GOSUB are supported for backward compatibility but not recommended for new code.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Line Numbers and the GOTO Statement", p. 24-25
--- END ---

--- CARD ---
id:        on-error-goto-exception-handling
type:      FACT
target:    all
verify:    from-manual
topic:     error-handling
claim:     ON ERROR GOTO linenum — sets error handler that redirects execution to specified line number when runtime error occurs. ON ERROR without GOTO clears error handler (resets to default).
context:   Error handler remains active until explicitly cleared or another ON ERROR statement; allows graceful error recovery in procedures.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 55-57
--- END ---

--- CARD ---
id:        optimization-integer-vs-real-performance
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     code-optimization
claim:     INTEGER operations are significantly faster (often 10x) than REAL operations. BASIC generates dedicated 68000 machine-language instructions for INTEGER arithmetic (+, -, *, /, loops) but must invoke library functions for REAL. Type conversion overhead is substantial; matching operand types is essential for performance.
context:   Inserting a decimal point (e.g., value*2. instead of value*2) forces REAL promotion and slows operation ~10x.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Program Optimization", "Optimum Use of Numeric Data Types", p. 62
--- END ---

--- CARD ---
id:        optimization-for-next-integer-loop-fast
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     code-optimization
claim:     BASIC automatically detects INTEGER-typed FOR loop counters and uses fast 68000-native integer loop instructions. REAL-typed FOR loops execute slower through library interpretation.
context:   Using REAL loop counters incurs type conversion overhead on each iteration.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Program Optimization", "Looping Quickly", p. 62
--- END ---

--- CARD ---
id:        optimization-loop-invariant-motion
type:      GOTCHA
target:    all
verify:    from-manual
topic:     code-optimization
claim:     Any computation that can occur before loop entry (loop invariant) should be moved outside the loop, as loop body statements execute repeatedly. Moving constant expressions outside loops increases speed.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Program Optimization", "Looping Quickly", p. 62
--- END ---

--- CARD ---
id:        icode-machine-instruction-parity
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     performance-characteristics
claim:     Microware BASIC I-code execution has minimal overhead vs. native machine code because: (1) single I-code instruction often translates to multiple 68000 cycles (e.g., floating-point), (2) I-code references use actual memory addresses (no runtime table search), (3) 68000 instruction set is optimized for compiler-produced code patterns.
context:   This allows BASIC to match C compiler performance on 68k despite being interpreted.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Program Optimization", "General Execution Performance of BASIC", p. 61
--- END ---

--- CARD ---
id:        error-checking-runtime-array-bounds
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     error-handling
claim:     BASIC I-code interpreter performs runtime error checking for array bounds, subroutine nesting depth, arithmetic errors, and other conditions. Native machine-code compilers typically do not detect these errors.
context:   This safety comes at small performance cost but prevents crashes and hard-to-debug memory corruption.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Program Optimization", "General Execution Performance of BASIC", p. 61
--- END ---

--- CARD ---
id:        print-using-format-specification
type:      FACT
target:    all
verify:    from-manual
topic:     io-operations
claim:     PRINT [#path] USING format_string, output_list — formatted output with format specs: R (real), E (exponential), I (integer), H (hexadecimal), S (string), B (boolean). Field width required; optional decimal places for real/exponential (e.g., "R10.6"). Justification: < (left), > (right), ^ (center).
context:   Format string cannot contain spaces; each format spec matched with corresponding output item; list repeats if fewer specs than items.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", "Formatted Output: The PRINT .. USING Statement", p. 58-59
--- END ---

--- CARD ---
id:        numeric-constant-hexadecimal-format
type:      FACT
target:    all
verify:    from-manual
topic:     literals
claim:     INTEGER constants accept hexadecimal notation: leading $ followed by hex digits (0-9, A-F). Range 0 to $FFFFFFFF. Example: `$20`, `$FFFE`, `$0`.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", "Numeric Constants", p. 25
--- END ---

--- CARD ---
id:        numeric-constant-exponential-format
type:      FACT
target:    all
verify:    from-manual
topic:     literals
claim:     REAL constants support scientific notation with E format: mantissa followed by E and exponent (can be negative). Examples: 1.95E+12, -99999.9E-33. Treated as REAL regardless of magnitude.
context:   Numbers without decimal point but too large for INTEGER range (e.g., 10000000000) are automatically promoted to REAL.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", "Numeric Constants", p. 25
--- END ---

--- CARD ---
id:        automatic-type-conversion-rules
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     data-types
claim:     BASIC performs automatic type conversions in expressions. Result type depends on operand types: if all operands are BYTE or INTEGER, result is INTEGER; mixed BYTE/INTEGER/REAL produces REAL. BOOLEAN and STRING conversions also supported but less common.
context:   Type conversion has performance cost; matching operand types is preferred.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Variable Data Types", "Automatic Type Conversion", p. 24
--- END ---

--- CARD ---
id:        command-line-parameter-passing
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     procedure-management
claim:     Main procedure can receive command-line parameters from OS-9 shell via PARAM statement (no parentheses required). Parameters determined to be string or numeric by PARAM declaration. String parameters in expression form or ambiguous as numeric must be quoted: `filter 11+4 "this"+"that"`. From within BASIC: parameters require parentheses: `run filter("-z=myfiles", "-p=~", "-l=11")`.
context:   Optional parameters: accessing unpasssed parameter causes error, but no error if extra parameters passed and not accessed. Uninitialized parameter variables should be handled in subroutines.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", "Command Line Parameters", p. 55-56
--- END ---

--- CARD ---
id:        shell-command-execution
type:      FACT
target:    68k
verify:    from-manual
topic:     os9-integration
claim:     SHELL statement executes OS-9 shell commands from within BASIC procedure. Allows launching external programs and returning control to BASIC after command completes.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-20
--- END ---

--- CARD ---
id:        chain-statement-program-transition
type:      FACT
target:    68k
verify:    from-manual
topic:     program-execution
claim:     CHAIN statement loads and runs another BASIC program, passing control to new program. Does not return to original program.
context:   Different from RUN which calls a procedure and returns; CHAIN replaces current program with new program.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-20
--- END ---

--- CARD ---
id:        pause-statement-execution-halt
type:      FACT
target:    all
verify:    from-manual
topic:     program-execution
claim:     PAUSE statement halts execution and returns to debug mode, allowing variable inspection and step-through debugging.
context:   Similar to breakpoint in modern debuggers.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-23
--- END ---

--- CARD ---
id:        chd-chx-directory-change
type:      FACT
target:    68k
verify:    from-manual
topic:     os9-integration
claim:     CHD (change directory) and CHX (change execution directory) statements change current working directory and execution search path within BASIC environment. OS-9 operating system commands.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-4; Chapter 11, "Program Statements", p. 11-24
--- END ---

--- CARD ---
id:        poke-peek-memory-access
type:      FACT
target:    68k
verify:    from-manual
topic:     low-level-access
claim:     POKE and PEEK statements allow direct memory access (POKE to write, PEEK to read). Dangerous for system stability; typically used for OS-9 system globals or hardware access only.
context:    -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 11, "Program Statements", command summary
--- END ---

--- CARD ---
id:        size-function-data-size
type:      FACT
target:    all
verify:    from-manual
topic:     introspection
claim:     SIZE(variable) — returns byte size of variable or data structure. Used in pointer arithmetic (e.g., `SEEK #file, SIZE(record) * (index - 1)`).
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example line 197, p. 54-57
--- END ---

--- CARD ---
id:        string-functions-left-right-mid-substr
type:      FACT
target:    all
verify:    from-manual
topic:     string-operations
claim:     String manipulation functions: LEFT$(string, len) returns leftmost len characters; RIGHT$(string, len) returns rightmost len characters; MID$(string, start, len) returns substring starting at position start for len characters; SUBSTR same as MID$.
context:   String positions are 1-based (first character is position 1).
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists LEFT$, RIGHT$, MID$, SUBSTR
--- END ---

--- CARD ---
id:        string-functions-len-trim
type:      FACT
target:    all
verify:    from-manual
topic:     string-operations
claim:     LEN(string) returns string length in characters. TRIM$(string) removes leading and trailing whitespace.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists LEN, TRIM$
--- END ---

--- CARD ---
id:        string-conversion-str-val-asc-chr
type:      FACT
target:    all
verify:    from-manual
topic:     string-operations
claim:     String conversion functions: STR$(number) converts number to string; VAL(string) converts string to numeric value (stops at first non-numeric character); ASC(char) returns ASCII code of character; CHR$(code) returns character for ASCII code.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists STR$, VAL, ASC, CHR$
--- END ---

--- CARD ---
id:        math-functions-trigonometric
type:      FACT
target:    all
verify:    from-manual
topic:     math-functions
claim:     Trigonometric functions: SIN(x), COS(x), TAN(x), ASN(x) (arcsine), ACS(x) (arccosine), ATN(x) (arctangent). Angles in radians by default; DEG/RAD statements control angle mode.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4; reserved words table
--- END ---

--- CARD ---
id:        math-functions-logarithmic-exponential
type:      FACT
target:    all
verify:    from-manual
topic:     math-functions
claim:     Logarithmic and exponential functions: LOG(x) (natural logarithm), LOG10(x) (base-10 logarithm), EXP(x) (e^x). Full set of transcendental functions documented.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4; reserved words table
--- END ---

--- CARD ---
id:        math-functions-sqrt-abs-sgn
type:      FACT
target:    all
verify:    from-manual
topic:     math-functions
claim:     SQRT(x) (square root), SQR(x) (square), ABS(x) (absolute value), SGN(x) (sign: -1, 0, or 1). INT(x) truncates to integer.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists SQR, SQRT, ABS, SGN, INT
--- END ---

--- CARD ---
id:        random-number-generator
type:      FACT
target:    all
verify:    from-manual
topic:     math-functions
claim:     RND function returns pseudo-random floating-point number in range [0.0, 1.0). RAND-based, not cryptographically secure.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists RND
--- END ---

--- CARD ---
id:        angle-mode-deg-rad-switch
type:      FACT
target:    all
verify:    from-manual
topic:     math-functions
claim:     DEG and RAD statements switch angle mode for trigonometric functions. DEG mode treats angles as degrees; RAD mode as radians (default typically).
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary; Chapter 11, "Program Statements"
--- END ---

--- CARD ---
id:        base-selection-base0-base1
type:      FACT
target:    all
verify:    from-manual
topic:     array-indexing
claim:     BASE0 and BASE1 statements control array indexing convention: BASE0 makes arrays 0-indexed (first element at index 0); BASE1 makes arrays 1-indexed (first element at index 1). Default typically BASE1.
context:   Affects all array indexing within procedure; BASE statement applies to entire procedure.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 11, "Program Statements", command summary, p. 11-25
--- END ---

--- CARD ---
id:        debug-mode-breakpoints-state-inspection
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     debugging
claim:     Debug mode enables breakpoints, single-step execution (STEP command), variable inspection/modification (LET command), trace mode (TRON/TROFF), and calculator mode (PRINT expressions). Execution pauses at breakpoints or PAUSE statements.
context:   Debug mode entered when breakpoint hit, error occurs (with ON ERROR handler), or PAUSE statement executed.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 8, "Debug Mode", p. 85-87
--- END ---

--- CARD ---
id:        trace-mode-tron-troff
type:      FACT
target:    all
verify:    from-manual
topic:     debugging
claim:     TRON (trace on) statement enables execution trace that displays original source statements as they execute. TROFF (trace off) disables trace. Useful for debugging complex procedures.
context:   Trace output is verbose and slows execution; only use for debugging, not production.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 8, "Debug Mode", command summary, p. 86; Chapter 11, "Program Statements", p. 11-25
--- END ---

--- CARD ---
id:        digits-statement-floating-point-precision
type:      FACT
target:    all
verify:    from-manual
topic:     numeric-display
claim:     DIGITS statement controls number of significant digits displayed for floating-point output in PRINT and debug mode. Affects only display precision, not internal calculation precision.
context:   Internal precision is always 14 decimal digits; DIGITS only controls how many are shown.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-5; Chapter 11, "Program Statements", p. 11-22
--- END ---

--- CARD ---
id:        kill-statement-procedure-termination
type:      FACT
target:    68k
verify:    from-manual
topic:     procedure-management
claim:     KILL #pathnum statement terminates a running procedure by its path descriptor. KILL followed by procedure name in system mode deletes procedure from workspace.
context:   Dangerous operation; killing a procedure that is currently executing can cause system instability.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-6; Chapter 11, "Program Statements", p. 11-18
--- END ---

--- CARD ---
id:        operator-precedence-standard-mathematical
type:      FACT
target:    all
verify:    from-manual
topic:     expressions
claim:     BASIC operator precedence follows standard mathematical convention: parentheses first, then exponentiation, then unary -, then *, /, MOD, then +, -, then comparison operators, then logical operators (AND, OR, XOR, NOT, etc.).
context:   Use parentheses liberally for clarity.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 10, "Expressions, Operators, and Functions", "Operator Precedence", p. 10-3
--- END ---

--- CARD ---
id:        boolean-operators-logical-bitwise
type:      FACT
target:    all
verify:    from-manual
topic:     expressions
claim:     Logical/bitwise operators: AND (logical/bitwise AND), OR (logical/bitwise OR), XOR (exclusive OR), NOT (logical/bitwise NOT), LAND (logical AND without bitwise), LOR (logical OR without bitwise), LNOT (logical NOT).
context:   Regular AND/OR/NOT perform bitwise operations on integers; LAND/LOR/LNOT perform pure logical operations (return 0 or 1).
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists AND, OR, XOR, NOT, LAND, LOR, LNOT
--- END ---

--- CARD ---
id:        pos-function-substring-search
type:      FACT
target:    all
verify:    from-manual
topic:     string-operations
claim:     POS(string, substring) returns position (1-based) of first occurrence of substring within string, or 0 if not found.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists POS
--- END ---

--- CARD ---
id:        eof-function-end-of-file-detection
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     EOF(#path) function returns true if file pointer is at or past end-of-file for open path descriptor. Used to detect end of file during sequential read operations.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists EOF
--- END ---

--- CARD ---
id:        err-function-last-error-code
type:      FACT
target:    all
verify:    from-manual
topic:     error-handling
claim:     ERR function returns the error code (numeric) of the last error that occurred. 0 means no error. Used in error handlers to determine error type.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists ERR
--- END ---

--- CARD ---
id:        inkey-function-nonblocking-input
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     INKEY function reads one character from input without blocking (non-blocking read). Returns empty string if no character available.
context:   Useful for interactive programs that need responsiveness.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists INKEY
--- END ---

--- CARD ---
id:        date-function-timestamp
type:      FACT
target:    68k
verify:    from-manual
topic:     system-functions
claim:     DATE$ function returns current date/time as formatted string (format depends on OS-9 system configuration).
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists DATE$
--- END ---

--- CARD ---
id:        fix-float-type-casting
type:      FACT
target:    all
verify:    from-manual
topic:     type-conversion
claim:     FIX(realnum) truncates floating-point to integer; FLOAT(intnum) converts integer to floating-point (adds .0).
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists FIX, FLOAT
--- END ---

--- CARD ---
id:        modulo-operator-remainder
type:      FACT
target:    all
verify:    from-manual
topic:     arithmetic
claim:     MOD operator returns remainder after integer division: `result := dividend MOD divisor`. Example: `17 MOD 5` returns 2.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", reserved words table lists MOD
--- END ---

--- CARD ---
id:        save-command-source-preservation
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     workspace-management
claim:     SAVE [procname] [> filename] writes procedure source to OS-9 file in unformatted text (no I-code addresses). If filename omitted, uses procedure name. If file exists, prompts "rewrite?" (Y/N). SAVE* writes all procedures in one file.
context:   Must SAVE before PACK to preserve editable source; packed procedures cannot be reloaded.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", "SAVE/SAVE*" command, p. 77-78
--- END ---

--- CARD ---
id:        load-command-procedure-import
type:      FACT
target:    all
verify:    from-manual
topic:     workspace-management
claim:     LOAD filename imports procedure(s) from file into workspace. If workspace already has procedure with same name, the loaded version replaces it.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", "LOAD" command, p. 5-7
--- END ---

--- CARD ---
id:        list-command-formatted-procedure-display
type:      FACT
target:    all
verify:    from-manual
topic:     workspace-management
claim:     LIST [procname] displays procedure in formatted text including I-code addresses in hexadecimal. LIST* lists all procedures. Used to view procedures with error location information.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", "LIST/LIST*" command, p. 5-7
--- END ---

--- CARD ---
id:        rename-command-procedure-renaming
type:      FACT
target:    all
verify:    from-manual
topic:     workspace-management
claim:     RENAME oldname, newname changes procedure name in workspace. Allows multiple copies of same procedure under different names.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", "RENAME" command, p. 5-9
--- END ---

--- CARD ---
id:        pi-constant-mathematical
type:      FACT
target:    all
verify:    from-manual
topic:     math-functions
claim:     PI is a built-in constant representing the mathematical constant π. Example: `sin(pi/2)` returns 1.0.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", "RUN" examples, p. 76-77
--- END ---

--- CARD ---
id:        update-statement-file-mode
type:      FACT
target:    68k
verify:    from-manual
topic:     io-operations
claim:     UPDATE is file open mode keyword specifying read/write access to existing file. Syntax: `OPEN #path, "filename": UPDATE`.
context:   Allows both reading and writing to same file; supports SEEK operations for random access.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 54
--- END ---

--- CARD ---
id:        tab-function-column-positioning
type:      FACT
target:    all
verify:    from-manual
topic:     io-operations
claim:     TAB(column) in PRINT statement positions output cursor to specified column (1-based). Example: `PRINT "Name:"; TAB(20); name$` positions name$ at column 20.
context:   Differs from comma separator (16-column tab stops); TAB allows precise column control.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, "Overview", note about TAB command
--- END ---


# BASIC09 language

Sifted/merged from `basic09-language-RAW.md` (65 cards, three sources: BASIC09
Reference Manual Rev H [6809], the third "OS-9 BASIC User Manual Rev G, 1991"
edition [68k], and 5 authored BNF-grammar cards) plus 13 BASIC09-relevant
stray cards pulled in from `misc-unclustered-RAW.md`. Input: 78 cards.
Output: 45 cards. 8 cards pruned outright as generic-BASIC knowledge with no
BASIC09-specific delta (redundant with the BNF cards' own context fields, or
plain textbook trivia). The rest were merged where the two manuals described
the same syntax/behavior — roughly 19 merge groups (2-5 source cards each) —
except for two places where the manuals turned out to genuinely *disagree*,
which are kept as flagged GOTCHA/ROSETTA cards rather than silently resolved.
Two ROSETTA cards consolidate real 6809-vs-68k numeric deltas (INTEGER
range/wraparound/hex-constants, REAL format/precision); 4 cards remain
68k-specific (SHELL statement, CHAIN, command-line PARAM, INTEGER-vs-REAL
performance note); the remaining 39 are `target: all`. The 5 authored BNF
cards are carried forward verbatim and unaltered.

<!-- ============================================================ -->
<!-- SYNTAX REFERENCE (BNF) -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-bnf-declarations-and-types
type:      CONCEPT
target:    all
verify:    authored
topic:     basic09-syntax
claim:     |
  declaration  ::= "DIM" var-list ":" type
  type         ::= "BYTE" | "INTEGER" | "REAL" | "BOOLEAN"
                  | "STRING" [ "[" length "]" ]
                  | type-name          -- a TYPE-defined record

  Example:
    DIM count: INTEGER
    DIM name: STRING[40]
context:   Delta from generic BASIC — a generic BASIC either has no declarations at all (every variable is an untyped global) or infers type purely from a sigil suffix (`A$` = string). BASIC09 requires an explicit `DIM ... : type` declaration with five real atomic types, and an undeclared numeric variable defaults to REAL (5-byte float), not to an untyped/variant slot — mixing an undeclared numeric with a STRING is a compile-time type-mismatch error, not a runtime coercion.
source:    authored
--- END ---

--- CARD ---
id:        basic09-bnf-control-flow
type:      CONCEPT
target:    all
verify:    authored
topic:     basic09-syntax
claim:     |
  if-stmt     ::= "IF" expr "THEN" stmts
                  [ "ELSE" stmts ]
                  "ENDIF"
  for-stmt    ::= "FOR" var "=" expr "TO" expr [ "STEP" expr ]
                  stmts
                  "NEXT" [var]
  while-stmt  ::= "WHILE" expr stmts "ENDWHILE"
  repeat-stmt ::= "REPEAT" stmts "UNTIL" expr
  loop-stmt   ::= "LOOP" stmts [ "EXITIF" expr "THEN" ] stmts "ENDLOOP"

  Example:
    WHILE count < 10
      count = count + 1
    ENDWHILE
context:   Delta from generic BASIC — a generic line-numbered BASIC branches via `GOTO`/`IF...GOTO line#` with no block structure at all. Every BASIC09 control construct instead has an explicit matching closing keyword (ENDIF/NEXT/ENDWHILE/UNTIL/ENDLOOP) — there is no bare `IF...THEN linenum` form. `LOOP`/`ENDLOOP` with an internal `EXITIF` is BASIC09's own construct with no common generic-BASIC equivalent at all (closest analog: a `while(true)` with a conditional `break` in a C-family language).
source:    authored
--- END ---

--- CARD ---
id:        basic09-bnf-procedure-and-parameters
type:      CONCEPT
target:    all
verify:    authored
topic:     basic09-syntax
claim:     |
  procedure   ::= "PROCEDURE" name
                  [ "PARAM" param-list ]
                  stmts
                  "ENDPROC" | "END"
  param-ref   ::= plain-var                 -- by reference (default)
                | value-wrapping-expr        -- by value: e.g. x+0, s$+""

  Example:
    PROCEDURE addone
    PARAM n: INTEGER
    n = n + 1
    ENDPROC
context:   Delta from generic BASIC — a generic BASIC has no named-procedure/parameter concept at all; subroutines are `GOSUB linenum` with no parameters, sharing all state globally. BASIC09 is procedure-based: multiple independent named procedures coexist in the workspace, each with its own local variables, and a call passes parameters by reference by default (the callee can mutate the caller's variable directly) — passing by value requires deliberately wrapping the argument in a no-op expression (`x+0` for a number, `word$+""` for a string) to force a temporary copy, rather than value semantics being the default the way they are in most languages with parameters at all.
source:    authored
--- END ---

--- CARD ---
id:        basic09-bnf-print-formatting
type:      CONCEPT
target:    all
verify:    authored
topic:     basic09-syntax
claim:     |
  print-stmt   ::= "PRINT" [ print-list ]
  print-using  ::= "PRINT" "USING" format-string ":" print-list
  print-item   ::= expr | "TAB" "(" expr ")"

  Example:
    PRINT USING "###.##": price
context:   Delta from generic BASIC — plain `PRINT` with comma/semicolon-separated items behaves close to a generic BASIC's expectation. `PRINT USING` is the real delta: it takes a FORTRAN-style format-specification string (field width, decimal places, hex, etc.) controlling exact output layout, available both in normal program `PRINT` statements and in the interactive debug-mode `PRINT` command — a model expecting only generic BASIC's plain comma-separated PRINT should not assume formatted output requires manual string-padding logic; `PRINT USING` already does it.
source:    authored
--- END ---

--- CARD ---
id:        basic09-bnf-type-records
type:      CONCEPT
target:    all
verify:    authored
topic:     basic09-syntax
claim:     |
  type-def    ::= "TYPE" type-name
                  field-decl+
                  "ENDTYPE"
  field-decl  ::= field-name ":" type

  Example:
    TYPE point
      x: INTEGER
      y: INTEGER
    ENDTYPE
    DIM p: point
context:   Delta from generic BASIC — a generic line-numbered BASIC has no user-defined structured/record type at all; grouping related values means either parallel arrays or string-packing tricks. BASIC09's `TYPE` statement defines a named record type with typed, named fields, and the manual explicitly frames this as *more* efficient than an array of similar values because field offsets are resolved at compile time rather than computed at run time — the opposite of the usual expectation that structuring data adds overhead.
source:    authored
--- END ---

--- CARD ---
id:        basic09-type-records-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-syntax
claim:     TYPE syntax: `TYPE typename = field1: datatype; field2: datatype; ...` (or the block form with ENDTYPE — see basic09-bnf-type-records). Types can nest: a field's type may itself be another TYPE-defined record (e.g. `TYPE entry = name: STRING[30]; address: rec`, where `rec` is a previously-defined TYPE). Fields are accessed via dot notation, including through nested records: `variable.field`, `variable.field1.field2`.
context:   Complex/record types are more efficient than an array of similar values because field offsets are resolved at compile time rather than computed at run time.
source:    BASIC09 Reference Manual (Rev H), Data Types section, "Complex Data Types", p. 7-8 to 7-9; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Program Construction", phonebook example, p. 54-57
--- END ---

<!-- ============================================================ -->
<!-- PROGRAM STRUCTURE & PROCEDURES -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-procedure-naming
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     Procedure names are 1-28 characters: uppercase A-Z, lowercase a-z, digits 0-9, underscore (_), dollar sign ($), and period (.). A name must contain at least one letter or digit (it may start with any allowed character) and is case-insensitive.
context:   Despite allowing 28 characters, short names are recommended for usability. This is more flexible than many traditional BASIC systems, which restricted variable/subroutine names to a single letter or two characters.
source:    BASIC09 Reference Manual (Rev H), Introduction to BASIC09, p. 2-2; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Naming Your Procedure", p. 17-18
--- END ---

--- CARD ---
id:        basic09-procedure-variable-line-scope
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     Each procedure has its own namespace: line numbers and variables are local to the procedure that declares them (the same line number can be reused across different procedures without conflict, and each procedure has its own data area). Data flows between procedures via parameters; procedures are invoked by name via the RUN statement.
context:   -
source:    BASIC09 Reference Manual (Rev H), Program Statements section, "Line Numbers", p. 9-1; "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4
--- END ---

--- CARD ---
id:        basic09-parameter-passing
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     basic09-functions
claim:     Parameters are passed by reference by default for variables, arrays, and complex data structures — the storage address is sent to the called procedure, so the callee can return values by mutating the caller's variable directly. Constants and expressions are always passed by value. To force by-value semantics on a variable, wrap it in an expression that evaluates to the same value without altering it (e.g. `x+0` for a number, `word$+""` for a string) — this creates a temporary that the callee cannot alias back. BYTE-typed parameters may only be passed by reference, never by value.
context:   -
source:    BASIC09 Reference Manual (Rev H), "Parameter Variables", p. 7-7 to 7-8; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Calling Procedures", p. 53-54
--- END ---

--- CARD ---
id:        basic09-dim-syntax-and-default-types
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     DIM declares one or more variables: `DIM var1,var2: type; var3: type2` — commas separate variables sharing a type, semicolons separate groups with different types (e.g. `DIM x,y,z: INTEGER; a,b,c: REAL`). DIM must appear before a variable's first use, or it gets a default type instead: a name ending in `$` defaults to STRING (32-char max); any other undeclared numeric variable defaults to REAL.
context:   Relying on default-type inference instead of explicit DIM can cause performance issues and unexpected type mismatches.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "The DIM Statement", p. 21; "Getting Started", p. 21
--- END ---

--- CARD ---
id:        basic09-assignment-let-implied
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-program-structure
claim:     BASIC09 accepts both an explicit LET statement and implied assignment, and both `:=` and `=` as the assignment operator: `LET x:=1` and `x:=1` (and `x=1`) are all valid and produce identical I-code.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Initializing Variables", p. 21-22
--- END ---

--- CARD ---
id:        basic09-variable-initialization-warning
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-variables
claim:     BASIC09 does NOT automatically initialize variables. When a procedure runs, all variables, arrays, and structures have random (leftover) values. The program must explicitly assign initial values.
context:   This is a significant gotcha for programmers used to languages that zero-initialize memory.
source:    BASIC09 Reference Manual (Rev H), "Variables", p. 7-7
--- END ---

--- CARD ---
id:        basic09-command-line-parameters
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     basic09-os9-integration
claim:     A main procedure can receive command-line parameters from the OS-9 shell via a PARAM statement (no parentheses in the invocation from the shell). Each parameter's string-vs-numeric interpretation is fixed by its PARAM declaration; a string parameter that looks like an expression or is ambiguous as numeric must be quoted: `filter 11+4 "this"+"that"`. Calling the same procedure from within BASIC instead requires parentheses: `run filter("-z=myfiles", "-p=~", "-l=11")`.
context:   Accessing a parameter that wasn't passed is an error, but passing extra parameters that are never accessed is not — subroutines should defensively handle possibly-uninitialized parameter variables.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, "Command Line Parameters", p. 55-56
--- END ---

--- CARD ---
id:        basic09-chain-statement
type:      FACT
target:    68k
verify:    from-manual
topic:     basic09-os9-integration
claim:     CHAIN loads and runs another BASIC program, replacing the current one — it does not return to the original program. This differs from RUN, which calls a procedure and returns.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-20
--- END ---

<!-- ============================================================ -->
<!-- DATA TYPES -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-five-atomic-types
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BASIC09 has five atomic data types: BYTE, INTEGER, REAL, STRING, and BOOLEAN. An undeclared numeric variable defaults to REAL; automatic type conversion is applied across mixed-numeric-type expressions (see basic09-automatic-type-conversion).
context:   -
source:    BASIC09 Reference Manual (Rev H), Data Types section, p. 7-2; "OS-9 BASIC User Manual (Revision G, 1991)" preface, p. 4, Chapter 2 "Variable Data Types", p. 20
--- END ---

--- CARD ---
id:        basic09-integer-rosetta
type:      ROSETTA
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     |
  INTEGER's range and its hex-constant range differ by architecture:
  - 6809 BASIC09: INTEGER is 16-bit signed (-32768 to 32767, 2 bytes). Overflow (e.g. 32767+1) silently wraps modulo 65536 with no error — values in the 32768-65535 range actually compare as negative. Hex constants ($ prefix, e.g. $20, $FFFF) range $0000-$FFFF.
  - 68k BASIC: INTEGER is 32-bit signed (-2,147,483,648 to 2,147,483,647), faster and lower-memory than REAL. Hex constants range $0 to $FFFFFFFF.
context:   Parallels this project's confirmed finding that the C compiler's `int` widens from 2 bytes (6809) to 4 bytes (68k). Whether 68k INTEGER also silently wraps on overflow the way 6809's does (just at the 32-bit boundary) is not stated by either manual excerpt here — don't assume it without separate confirmation.
source:    BASIC09 Reference Manual (Rev H), "Type INTEGER" p. 7-2, Constants p. 7-6; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2 p. 20, "Numeric Constants" p. 25
--- END ---

--- CARD ---
id:        basic09-real-rosetta
type:      ROSETTA
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     |
  REAL's storage format and precision differ by architecture:
  - 6809 BASIC09: REAL is a proprietary 5-byte float — an 8-bit two's-complement exponent plus 4 mantissa bytes (31 bits + sign). Range ~2.938735877e-39 to 1.701411835e38; precision ~9 decimal digits.
  - 68k BASIC: REAL is a 64-bit (8-byte) float; range approximately +/-2.2e-308 to +/-1.8e308; precision ~14 decimal digits. The DIGITS statement controls how many significant digits PRINT/debug-mode output displays — it affects display formatting only, not internal calculation precision (which is fixed).
context:   A 68k-specific widening parallel to INTEGER's 16->32-bit change. Whether 68k REAL is true IEEE-754 double or another Microware proprietary 8-byte encoding is not confirmed by either excerpt — don't assume IEEE-754 without separate confirmation.
source:    BASIC09 Reference Manual (Rev H), "Type REAL" p. 7-3; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2 "Variable Data Types" p. 20, Chapter 5 p. 5-5, Chapter 11 p. 11-22
--- END ---

--- CARD ---
id:        basic09-byte-type
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BYTE is 0-255, 1 byte, unsigned — same range on both 6809 and 68k. Storing a value outside 0-255 to a BYTE variable silently stores only the least-significant 8 bits (modulo 256), with no error. A BYTE parameter may only be passed by reference, never by value (see basic09-parameter-passing).
context:   Silent truncation, not an error condition.
source:    BASIC09 Reference Manual (Rev H), "Type BYTE" p. 7-2; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2 "Variable Data Types" p. 20
--- END ---

--- CARD ---
id:        basic09-string-type
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     STRING is variable-length with a default maximum of 32 characters when undeclared (an undeclared string variable must end in `$`); declared explicitly via `DIM varname: STRING[len]`. Internally, strings are null-terminated within a fixed-size buffer — the unused bytes after the terminator let the string grow/shrink during execution, up to its declared maximum. A string that grows past its max (e.g. via concatenation) is truncated.
context:   -
source:    BASIC09 Reference Manual (Rev H), "Type STRING" p. 7-4; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Getting Started", p. 21
--- END ---

--- CARD ---
id:        basic09-boolean-type
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BOOLEAN values (TRUE/FALSE) cannot be used in numeric computation. Assigning a non-BOOLEAN value to a BOOLEAN variable is a run-time error.
context:   See basic09-boolean-bitwise-operator-conflict for which operator family (AND/OR/XOR/NOT vs. LAND/LOR/LXOR/LNOT) operates on BOOLEAN vs. raw integer bits — the two source manuals disagree.
source:    BASIC09 Reference Manual (Rev H), "Type BOOLEAN", p. 7-4
--- END ---

--- CARD ---
id:        basic09-array-base-modes
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     BASE0 and BASE1 statements select the array (and STRING character-position) indexing convention for the current procedure: BASE0 = indices 0 to n-1, BASE1 (default) = indices 1 to n. In BASE1, a string's first character is position 1; in BASE0, position 0. (Rev H additionally documents a BASEN statement for switching modes.) Scope is per-procedure.
context:   -
source:    BASIC09 Reference Manual (Rev H), Data Types section, p. 7-4; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 11, command summary, p. 11-25
--- END ---

--- CARD ---
id:        basic09-array-declaration
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     Arrays are declared via DIM: `DIM arrayname(size1 [,size2 [,size3]]): datatype`, supporting 1D, 2D, and 3D arrays.
context:   Whether index 1 or 0 refers to the first element is controlled separately by the BASE0/BASE1 statement (see basic09-array-base-modes), not by the DIM syntax itself.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4; Chapter 2, "The DIM Statement", p. 21
--- END ---

--- CARD ---
id:        basic09-automatic-type-conversion
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-data-types
claim:     Expressions mixing numeric types (BYTE, INTEGER, REAL) are automatically converted to the largest type needed: if every operand is BYTE/INTEGER, the result is INTEGER; if any operand is REAL, the result is REAL. Mixing an incompatible type — e.g. STRING with a numeric type — is a compile-time type-mismatch error, not a runtime coercion.
context:   Type conversion has a runtime cost, so expressions with uniform operand types are more efficient than ones that force conversions.
source:    BASIC09 Reference Manual (Rev H), "Automatic Type Conversion", p. 7-6; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Automatic Type Conversion", p. 24
--- END ---

--- CARD ---
id:        basic09-scientific-notation
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-constants
claim:     REAL constants support scientific notation: a mantissa followed by `E` and a (possibly signed) exponent, e.g. `1.95E+12`, `-99999.9E-33`. Any constant containing a decimal point or E-notation is stored as REAL even if its value would otherwise fit in INTEGER or BYTE; a large integer literal without a decimal point that exceeds INTEGER range is likewise auto-promoted to REAL.
context:   -
source:    BASIC09 Reference Manual (Rev H), Constants section, p. 7-6; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Numeric Constants", p. 25
--- END ---

<!-- ============================================================ -->
<!-- CONTROL STRUCTURES -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-for-next-step
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-control-structures
claim:     `FOR var = start TO end [STEP incr] ... NEXT`. STEP defaults to 1 and may be negative to count down (the loop terminates once the counter passes `end` in the STEP direction). BASIC09 detects an INTEGER-typed loop counter and compiles a fast native-integer loop; a REAL-typed counter executes slower, via library interpretation.
context:   -
source:    BASIC09 Reference Manual (Rev H), "FOR/NEXT Statement", p. 9-4; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Looping Quickly", p. 62
--- END ---

--- CARD ---
id:        basic09-loop-exitif-endexit-nuance
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-control-structures
claim:     One manual documents LOOP's EXITIF clause with its own closing keyword nested inside the outer loop: `LOOP ... EXITIF expr THEN stmts ENDEXIT ... ENDLOOP` — as opposed to the simpler `LOOP ... EXITIF expr THEN stmts ... ENDLOOP` form (no ENDEXIT) shown elsewhere.
context:   Whether ENDEXIT is a required closing keyword, optional, or specific to one manual/version is not consistent across sources here — confirm against a live BASIC09 session before relying on it.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" preface feature list, p. 4
--- END ---

--- CARD ---
id:        basic09-gosub-goto-legacy
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-control-structures
claim:     GOTO and GOSUB target line numbers, not labels, in the range 1-32767. GOSUB pushes a return address onto the call stack (a frame distinct from a procedure-call frame); RETURN pops it and resumes after the GOSUB. Traditional line-number GOTO/GOSUB are supported for backward compatibility, but structured control flow (IF/ENDIF, LOOP/WHILE/REPEAT) is the recommended style for new code.
context:   -
source:    BASIC09 Reference Manual (Rev H), Index references; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, "Line Numbers and the GOTO Statement", p. 24-25
--- END ---

<!-- ============================================================ -->
<!-- EXPRESSIONS & OPERATORS -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-operator-precedence
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-expressions
claim:     Operator precedence, highest to lowest: NOT / unary negate; exponentiation (`^` or `**`); `*`, `/`, `MOD`; `+`, `-`; comparison operators (`>`, `<`, `=`, etc.); `AND`; `OR`; `XOR`. Operators of equal precedence evaluate left-to-right, except exponentiation (right-to-left). Parentheses override precedence.
context:   -
source:    BASIC09 Reference Manual (Rev H), "Operator Precedence", p. 8-3; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 10, "Operator Precedence", p. 10-3
--- END ---

--- CARD ---
id:        basic09-boolean-bitwise-operator-conflict
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-operators
claim:     The two source manuals disagree on which operator family is Boolean-only vs. bit-by-bit. BASIC09 Reference Manual (Rev H) states AND/OR/XOR/NOT operate on Boolean (TRUE/FALSE) values, while LAND/LOR/LXOR/LNOT are the bit-by-bit integer functions. The OS-9 BASIC User Manual (Rev G, 1991) states the reverse: AND/OR/XOR/NOT perform bitwise operations on integers, while LAND/LOR/LNOT are the pure-logical functions returning 0 or 1. Both agree the *NOT member of each family (LNOT / NOT) is the odd one out taking a single operand, versus two operands for the rest.
context:   Verify empirically against a live BASIC09 session before writing code that depends on either family for bit manipulation vs. Boolean logic — do not trust either manual alone here.
source:    BASIC09 Reference Manual (Rev H), Functions section p. 8-6, "Type BOOLEAN" p. 7-4; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, reserved-words table
--- END ---

--- CARD ---
id:        basic09-fix-float-casting
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions
claim:     FIX(realnum) truncates a floating-point value to an integer; FLOAT(intnum) converts an integer to floating-point (adds `.0`).
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 2, reserved words table
--- END ---

--- CARD ---
id:        basic09-substr-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions-strings
claim:     `SUBSTR(<search_str>, <source_str>)` returns the starting position of the first occurrence of `<search_str>` within `<source_str>`, or 0 if not found. Position numbering follows the current BASE mode (see basic09-array-base-modes).
context:   -
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-5
--- END ---

--- CARD ---
id:        basic09-size-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-functions
claim:     `SIZE(variable)` returns the byte size of a variable or data structure; commonly used in pointer/record arithmetic, e.g. `SEEK #file, SIZE(record) * (index - 1)`.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 3, phonebook example line 197, p. 54-57
--- END ---

<!-- ============================================================ -->
<!-- I/O & OS-9 INTEGRATION -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-shell-dollar-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-os9-integration
claim:     The `$` command in System Mode calls the OS-9 Shell command interpreter. Followed by text, it executes a single OS-9 command (e.g. `$copy file1 file2`); given alone, it suspends BASIC09 and allows multiple interactive OS-9 commands, returning to BASIC09 on EOF (usually ESCAPE).
context:   A gateway to OS-9 from inside BASIC09's interactive System Mode, without disturbing the workspace.
source:    BASIC09 Reference Manual (Rev H), System Mode, "$ command", p. 3-3
--- END ---

--- CARD ---
id:        basic09-shell-statement
type:      FACT
target:    68k
verify:    from-manual
topic:     basic09-os9-integration
claim:     The SHELL statement executes an OS-9 shell command from within a running BASIC procedure (not just interactively from System Mode) — it launches an external program and returns control to BASIC after the command completes.
context:   -
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 5, "System Mode", command summary, p. 5-20
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

<!-- ============================================================ -->
<!-- DEBUGGING & EDITING COMMANDS -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-debug-trace
type:      CONCEPT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     Debug Mode trace (TRON/TROFF) reconstructs I-code back to source statements and displays each statement before execution, with expression results shown on the following line. Trace is local to a procedure — it stops when calling into another procedure unless that procedure also has trace on.
context:   Trace output is verbose and slows execution; use only for debugging, not production.
source:    BASIC09 Reference Manual (Rev H), "TRON/TROFF commands", p. 6-4; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 8, command summary, p. 86
--- END ---

--- CARD ---
id:        basic09-debug-breakpoint
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The BREAK command sets a breakpoint on a named procedure, triggered when that procedure is returned to (via procedure nesting). One breakpoint may be set per active procedure, and it is removed as soon as it triggers.
context:   Used in multi-procedure debugging to halt execution at a specific point in the call stack.
source:    BASIC09 Reference Manual (Rev H), "BREAK command", p. 6-2
--- END ---

--- CARD ---
id:        basic09-debug-state-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The STATE command lists the calling (nesting) order of all active procedures: the outermost procedure is shown at the bottom, the currently suspended (innermost) procedure at the top.
context:   Reveals the call stack at the point of suspension.
source:    BASIC09 Reference Manual (Rev H), "STATE command", p. 6-3 to 6-4
--- END ---

--- CARD ---
id:        basic09-debug-step-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     STEP (or a bare carriage return) executes one or more source statements at a time in Debug Mode; `STEP 5` executes the next 5. A plain CR is equivalent to STEP 1.
context:   FOR/NEXT structure statements execute once each per STEP, so the loop's "top" or "bottom" line may not appear to execute as many times as expected.
source:    BASIC09 Reference Manual (Rev H), "STEP command", p. 6-3
--- END ---

--- CARD ---
id:        basic09-debug-let-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     In Debug Mode, `LET <var> := <expr>` sets a variable to a new value. The variable name must match the original source program; new variables cannot be created in Debug Mode, and user-defined data structures are not supported by LET.
context:   -
source:    BASIC09 Reference Manual (Rev H), "LET command", p. 6-3
--- END ---

--- CARD ---
id:        basic09-debug-pause-and-prompts
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-debugging
claim:     The PAUSE statement suspends program execution and enters Debug Mode, prompting `D:` — used to halt at a specific point in the code for inspection and single-stepping. (Compare: Edit Mode uses the `E:` prompt.)
context:   Unlike errors or CONTROL-C, PAUSE is intentional and can be embedded directly in program logic — functionally similar to a breakpoint in a modern debugger.
source:    BASIC09 Reference Manual (Rev H), Debug Mode overview, p. 6-1; "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 1, p. 13, Chapter 5, p. 5-23
--- END ---

--- CARD ---
id:        basic09-decompiler-formatting
type:      GOTCHA
target:    all
verify:    from-manual
topic:     basic09-editing
claim:     When BASIC09 lists a program, it may look slightly different from what was originally typed (extraneous spaces removed, unnecessary parentheses removed, keywords capitalized) — but it is always functionally identical to the original source.
context:   A byproduct of decompiling I-code back to source; don't rely on exact formatting being preserved.
source:    BASIC09 Reference Manual (Rev H), "How the Editor Works", p. 4-2
--- END ---

--- CARD ---
id:        basic09-renumber-command
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-editing
claim:     The RENUMBER command resequences numbered lines in a procedure: `r [<start_line>] [,<increment>]` or `r*` for all lines. Default start is 100, default increment is 10.
context:   -
source:    BASIC09 Reference Manual (Rev H), "LINE-NUMBER ORIENTED EDITING", p. 4-3
--- END ---

<!-- ============================================================ -->
<!-- ERROR HANDLING & RUNTIME BEHAVIOR -->
<!-- ============================================================ -->

--- CARD ---
id:        basic09-runtime-error-checking
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     basic09-error-handling
claim:     BASIC09's I-code interpreter performs runtime error checking for array bounds, subroutine/procedure nesting depth, arithmetic errors, and other conditions that a native machine-code compiler typically would not detect.
context:   This safety costs a small amount of performance but prevents crashes and hard-to-debug memory corruption — a core characteristic of BASIC09's I-code-interpreted design, not just a 68k-only feature.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "General Execution Performance of BASIC", p. 61
--- END ---

--- CARD ---
id:        basic09-integer-vs-real-performance
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     basic09-error-handling
claim:     INTEGER operations are significantly faster than REAL (often ~10x) — BASIC generates dedicated 68000 machine instructions for INTEGER arithmetic and loops but must invoke library functions for REAL. Inserting a decimal point where one isn't needed (e.g. `value*2.` instead of `value*2`) forces REAL promotion and incurs the same ~10x slowdown.
context:   Matching operand types (avoiding unnecessary INTEGER<->REAL conversion) is important for performance-sensitive code.
source:    "OS-9 BASIC User Manual (Revision G, 1991)" Chapter 4, "Optimum Use of Numeric Data Types", p. 62
--- END ---

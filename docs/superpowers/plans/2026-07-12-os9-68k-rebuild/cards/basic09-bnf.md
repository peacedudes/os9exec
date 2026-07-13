# BASIC09 grammar-shortcut cards

Informal grammar shorthand, not strict formal BNF — obvious leaf
productions (numeric literals, identifiers) are skipped. Each card pairs a
grammar sketch with one tiny valid example and states the delta from what
a model would expect of a generic line-numbered BASIC. Grounded in facts
already extracted in `basic09-rev-h.md` and `rbasic-manual.md`
(architecture-independent — BASIC09 runs identically on 6809 and 68k).

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

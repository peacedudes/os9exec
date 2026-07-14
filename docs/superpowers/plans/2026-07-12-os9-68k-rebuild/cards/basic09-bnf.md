# BASIC09 grammar-shortcut cards

Informal grammar shorthand, not strict formal BNF — obvious leaf
productions (numeric literals, identifiers) are skipped. Each card pairs a
grammar sketch with one tiny valid example and states the delta from what
a model would expect of a generic line-numbered BASIC. Grounded in facts
already extracted in `basic09-rev-h.md` and `rbasic-manual.md`
(architecture-independent — BASIC09 runs identically on 6809 and 68k).

**CORRECTION (2026-07-14):** all five cards below are marked `verify:
authored` (self-invented, not derived from a manual passage) — a live
dogfood pass plus a direct grep of both primary manuals found that 4 of
these 5 cards contained real errors, corrected in place below and now
also fixed in the shipped skill (`~/.claude/skills/os9-dev/references/
basic09/basic09-language.md`): `ENDTYPE` and `ENDPROC` never existed in
either manual (zero occurrences of either string, confirmed by grep) —
both are fabrications, not real-but-broken features. `WHILE...ENDWHILE`
was missing the mandatory `DO` (manual: `WHILE bool expr DO` / `ENDWHILE`,
p.42, and the manual's own table-of-contents entry is literally
"WHILE..DO Statement"). `PRINT USING` used a colon separator where the
manual uses a comma (`PRINT USING str expr, output list`, confirmed in
both the BASIC09 Reference Manual and the OS-9 BASIC User Manual) — the
colon form is a live syntax error. Only `basic09-bnf-declarations-and-
types` checked out clean. Lesson: `verify: authored` cards are exactly
the category most likely to carry this kind of error, since nothing
forced a check against the actual manual grammar at authoring time —
treat any `authored` card as needing exactly this kind of direct-grep
verification before trusting it, not just internal-consistency review.

**SECOND CORRECTION (2026-07-14, later same day):** the `print-formatting`
card's fix above was itself incomplete — it only fixed the comma/colon
separator and left the card's `"###.##"` pound-sign format-string example
in place. That entire format-string style was ALSO a fabrication — never
real BASIC09 syntax at all (BASIC09 uses directive LETTERS: `R8.2`, `I4`,
`S8`, `H4`, `B8`, `E12.3`, not `#` placeholders). This was live-tested and
fully rewritten below. Lesson on top of the lesson: catching one error in
a card doesn't mean the card is now clean — the colon-vs-comma fix
"looked complete" and passed casual review for an entire session before
a deeper live-test (triggered by trying to actually run the corrected
example) revealed the format-string grammar itself was never checked
against the manual at all, just the separator punctuation around it.

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
  while-stmt  ::= "WHILE" expr "DO" stmts "ENDWHILE"
  repeat-stmt ::= "REPEAT" stmts "UNTIL" expr
  loop-stmt   ::= "LOOP" stmts [ "EXITIF" expr "THEN" ] stmts "ENDLOOP"

  Example:
    WHILE count < 10 DO
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
                  [ "END" ]           -- optional; not a mandatory closer
  param-ref   ::= plain-var                 -- by reference (default)
                | value-wrapping-expr        -- by value: e.g. x+0, s$+""

  Example:
    PROCEDURE addone
    PARAM n: INTEGER
    n = n + 1
    END
context:   Delta from generic BASIC — a generic BASIC has no named-procedure/parameter concept at all; subroutines are `GOSUB linenum` with no parameters, sharing all state globally. BASIC09 is procedure-based: multiple independent named procedures coexist in the workspace, each with its own local variables, and a call passes parameters by reference by default (the callee can mutate the caller's variable directly) — passing by value requires deliberately wrapping the argument in a no-op expression (`x+0` for a number, `word$+""` for a string) to force a temporary copy, rather than value semantics being the default the way they are in most languages with parameters at all.
source:    authored
--- END ---

--- CARD ---
id:        basic09-bnf-print-formatting
type:      CONCEPT
target:    all
verify:    live-tested (2026-07-14 — see below; the original "authored"
           card here was a fabrication, corrected)
topic:     basic09-syntax
claim:     |
  print-stmt   ::= "PRINT" [ print-list ]
  print-using  ::= "PRINT" [ "#" path-expr ] "USING" format-string "," print-list
  print-item   ::= expr | "TAB" "(" expr ")"
  format-spec  ::= directive-letter width [ "." fraction ] [ justify ]
                  | "T" n | "X" n | "'" literal-text "'"
                  | count "(" format-spec { "," format-spec } ")"
  directive-letter ::= "R" | "E" | "I" | "H" | "S" | "B"
                  ! real / exponential / integer / hex-dump / string / boolean
  justify      ::= "<" | ">" | "^"     ! left / right / center

  Example:
    PRINT USING "'Average: ',R6.2", avg
context:   Delta from generic BASIC — plain `PRINT` with comma/semicolon-separated items behaves close to a generic BASIC's expectation. `PRINT USING` is the real delta, and NOT in the way this card originally claimed: BASIC09's format string does NOT use `#`-placeholder syntax the way many other BASICs do (`"###.##"` was a fabrication — never real BASIC09 syntax, and caused a live compile/runtime error). The real format string uses directive LETTERS (`R8.2` for a real number 8 chars wide with 2 fraction digits, `I4` for a 4-wide integer, `S8` for an 8-wide string, `H4` for a 4-wide hex dump, `B8` for boolean, `E12.3` for exponential), each optionally followed by a justify character. A model expecting `#`-placeholder formatting (from Microsoft BASIC, COBOL, etc.) will guess wrong here — this is the single most dangerous "looks like something I already know" trap in the whole language. Also note the path-number placement: `PRINT #path USING fmt, list` puts `#path` right after `PRINT`, before `USING` — `PRINT USING #path, ...` is a syntax error.
source:    live-tested — see basic09-language.md's PRINT USING section
           and tools/benchmarks/basic09-printusing-*.bas in os9exec-git_code
           for the full live-verification trail, including a confirmed
           manual-vs-implementation divergence (BOOLEAN format prints
           "True" not "TRUE") and a resolved OCR ambiguity in the
           justify symbols ("^" for center, not the manual's garbled
           degree-sign character).
--- END ---

--- CARD ---
id:        basic09-bnf-type-records
type:      CONCEPT
target:    all
verify:    authored
topic:     basic09-syntax
claim:     |
  type-def    ::= "TYPE" type-name "=" field-decl { ";" field-decl }
  field-decl  ::= field-name [ "(" size ")" ] ":" type

  Example:
    TYPE point = x:INTEGER; y:INTEGER
    DIM p: point
context:   Delta from generic BASIC — a generic line-numbered BASIC has no user-defined structured/record type at all; grouping related values means either parallel arrays or string-packing tricks. BASIC09's `TYPE` statement defines a named record type with typed, named fields, and the manual explicitly frames this as *more* efficient than an array of similar values because field offsets are resolved at compile time rather than computed at run time — the opposite of the usual expectation that structuring data adds overhead. It is a single-line statement (BASIC09 Reference Manual, Rev H, p.54-55: `TYPE type decl {; type decl}`) — there is no multi-line block form.
source:    authored
--- END ---

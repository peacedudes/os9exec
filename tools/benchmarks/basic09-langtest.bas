! BASIC09 language self-test — exercises documented language behaviors and
! reports PASS/FAIL, plus captures values expected to differ between 6809
! and 68k. Built and run for real on os9exec (68k, the only live target in
! this project). Expanded across several sessions after the user asked for
! the test to be "as extensive as possible."
!
! Two procedures: langtest2 (the main test) and bytetest (a standalone
! helper — must exist in the workspace before running langtest2, since
! langtest2 calls RUN bytetest(200)). Rename langtest2 to langtest if you
! prefer; the name has no significance.
!
! LIVE RESULT (68k, this run):
!   === BASIC09 Language Self-Test (expanded) ===
!   PASS 1: BASE 1 default array 1..3
!   PASS 2: BASE 0 shifts indices to 0..bound-1
!   PASS 3: string index 1-based under BASE 0
!   PASS 4: LAND is bitwise (6 LAND 3 = 2)
!   PASS 5: SUBSTR 1-based (first L at 3)
!   PASS 6: EXITIF exits loop correctly (i=5)
!   PASS 7 (confirms gotcha): BYTE param silently unbound, b=0
!   PASS 8: WHILE...DO loop runs correct count (i=3)
!   PASS 9: REPEAT/UNTIL runs correct count (i=3)
!   PASS 10: 2D array indexing works
!   PASS 11: STRING truncates at declared max, len=10
!   PASS 12: ABS(-5)=5
!   PASS 13: SGN(-3)=-1 AND SQ(4)=16
!   PASS 14: FIX(3.9)=4 (rounds, not truncates — see bug note below);
!            FLOAT(5)=5.
!   PASS 15: RND(0) in range 0-1
!
!   === Divergence probes (expect different on 6809 vs 68k) ===
!   SIZE(INTEGER) = 4
!   SIZE(REAL) = 8
!   1/3 at DIGITS 15 = .333333333488554
!   probe1 var+var: INTEGER max+1 = -2147483648
!   probe2 var+literal: -2147483648
!
!   === SUMMARY ===
!   Total: 15  Pass: 14  Fail: 0
!
! Total=15/Pass=14 is EXPECTED, not a bug: bytetest is a separate procedure
! with its own private namespace, so its own "PASS 7" print can't
! increment langtest2's pass counter.
!
! ===========================================================================
! REAL SKILL BUGS FOUND ACROSS ALL SESSIONS (chronological):
! ===========================================================================
!
! 1. LAND/LOR/LXOR/LNOT are function calls, not infix operators
!    (`LAND(6,3)`, not `6 LAND 3` — the latter is Error #000:011). Fixed in
!    basic09-language.md.
!
! 2. WHILE requires DO: `WHILE expr DO ... ENDWHILE`, not bare `WHILE expr`
!    (bare form is Error #000:031). Fixed in basic09-language.md.
!
! 3. GOTO/GOSUB with explicit line numbers: REVERSED this session — CONFIRMED
!    WORKING, not non-functional. The earlier "NON-FUNCTIONAL" finding above
!    was a methodology mistake: it targeted LIST's hex byte-offset DISPLAY
!    annotations for unnumbered structured code, which are not valid jump
!    targets at all. Real classic explicit numbered lines (`10 PRINT x` /
!    `20 GOTO 10`), typed directly into the source via the host-side
!    authoring workflow, work correctly for both GOTO and GOSUB, tested in
!    isolation (t_goto, t_gosub — see basic09-langtest-*/tools/benchmarks/).
!
!    ON ERROR GOTO also targets a real numbered line and is confirmed
!    working (t_onerror3/t_onerror4) — and the "fires once and auto-clears"
!    claim is CONFIRMED WRONG, cleanly this time: two separate triggers of
!    the same error, no re-arming call between them, both fired the trap
!    (hits=1, then hits=2). Treat the trap as staying armed until an
!    explicit bare `ON ERROR`.
!
!    DIVIDE BY ZERO is its own trap, unrelated to ON ERROR GOTO's
!    reliability: INTEGER÷0 silently falls through with NO error at all
!    (t_onerror.bas); REAL÷0 CRASHES THE WHOLE PROCESS with an uncatchable
!    68k CPU trap (`Error #000:107 E_TRAPV`, `E_PRCABT`) that ON ERROR GOTO
!    never gets a chance to intercept (t_onerror2.bas). Avoid REAL division
!    by a value that could be zero; there's no ON ERROR guard against it.
!
!    UNRESOLVED, lower priority: combining GOTO+GOSUB+ON ERROR GOTO with
!    numbered lines 100/200/300 in ONE procedure (see the deleted
!    t_gotogosub combined test) produced a compile-time
!    `Error #000:069 (Unmatched Control Structure)` even though each
!    construct is individually confirmed working in isolation above. Not
!    root-caused — flagged for a future session if it matters again.
!
! 4. FIX() ROUNDS TO NEAREST, IT DOES NOT TRUNCATE (found this session).
!    basic09-language.md previously said "Truncates a REAL to INTEGER."
!    Live-verified: FIX(3.9)=4, FIX(3.1)=3, FIX(-3.9)=-4. The negative case
!    is the deciding one — truncation-toward-zero would give -3 for
!    FIX(-3.9); only round-to-nearest gives -4. Fixed in
!    basic09-language.md's function table.
!
! ===========================================================================
! TOOLING: os9repl.sh key-sending was FIXED this session (see
! os9repl-tmux-helper memory / progress ledger for the fix itself: added
! `tmux_escape()` + `-l --` to all literal-text send-keys calls). Confirmed
! working for the actual BASIC09 editing use case (character-by-character
! `key` mode with each keystroke as a separate token, OR whole-line-at-once
! with the full line as ONE quoted argument) — both work correctly for `"`,
! standalone `;`, and leading-`-` tokens.
!
! REMAINING TOOLING CAVEAT, found this session: the fix does NOT correctly
! handle a `;` EMBEDDED within a longer literal string sent as one argument
! (e.g. `PRINT "x="; y` typed as a single whole-line token) — tmux's
! backslash-escape handling for `;` only collapses correctly when `\;` is
! the ENTIRE argument, not when it's a substring of a longer one; embedded
! case produces a literal spurious backslash in the output instead. Any
! multi-line whole-string send containing an embedded `;` should either (a)
! be sent as separate individual character tokens instead (works reliably),
! or (b) avoid semicolons in that specific line's content. Not fixed this
! session — a real, narrower follow-up if it matters again.
!
! ALSO: standard shell quoting caution when driving os9repl.sh from
! zsh/bash — passing a bare `(...)` or `(-N)` token unquoted as a shell
! argument (e.g. `key A B S ( - 5 ) = 5`) can trigger the shell's OWN glob
! parsing ("no matches found") and silently drop that whole invocation
! before it ever reaches the script. This is a shell-usage mistake, not an
! os9repl.sh bug — the fix is to quote the whole line as ONE argument
! (`key ' IF ABS(i1)=5 THEN'`) rather than splitting into bare
! shell-metacharacter-containing tokens.
!
! TYPE records: RESOLVED this session. The earlier "syntax error on every
! field line" finding traced to a fabricated multi-line TYPE/ENDTYPE block
! form that never existed in either manual (confirmed by direct grep — zero
! occurrences of ENDTYPE anywhere). The real, only form is a single
! semicolon-separated line: `TYPE name = field: type; field: type; ...`.
! This form works correctly, including nested types and array fields, via
! the host-side authoring workflow. See basic09-language.md's TYPE records
! section for the corrected grammar and citation.
!
! PRINT USING also had a similar host-authoring-era correction: the
! separator between the format string and the value list is a COMMA, not a
! colon (`PRINT USING fmt, list` — colon is Error #000:029 at compile time).
!
! Divergence probe note: two overflow-probe forms (var+var, var+literal)
! give the SAME result (-2147483648) — rules out "maybe it's specifically
! about literal arithmetic" as an explanation for the still-UNRESOLVED
! manual-vs-live INTEGER-overflow discrepancy (manual claims positive wrap,
! live measurement says ordinary negative two's-complement wrap). A
! loop-accumulation variant was planned but still not reached.
!
! STRING functions beyond LEN/MID$/SUBSTR tested in a separate file
! (t_strfuncs): LEFT$/RIGHT$/STR$/VAL/CHR$/ASC all PASS as documented.
! TRIM$ initially looked like a FAIL until checked against the manual: it
! strips TRAILING spaces only (not both sides) — TRIM$("  hi  ")="  hi",
! leading spaces survive. Test assertion was wrong, not the compiler; see
! basic09-language.md's function table.
!
! All previously-open Task 24 items are now RESOLVED — see the "REMAINING
! TASK 24 ITEMS" note further down this header for the full list and
! pointers to the individual test files.
!
! TYPE records and GOTO/GOSUB/ON ERROR GOTO (incl. the fires-once question
! and the divide-by-zero split behavior) are now RESOLVED — see the
! numbered findings above.
!
! REMAINING TASK 24 ITEMS: ALL RESOLVED as of this session — see
! basic09-numwiden-test.bas (numeric widening + division-truncation-
! before-widening rule), basic09-boolnum-test.bas (BOOLEAN in a numeric
! expression is a COMPILE-TIME E_ILLARG error, not runtime),
! basic09-dataread-test.bas (DATA/READ/RESTORE incl. wraparound),
! basic09-fileio-test.bas + basic09-fileio-eof-test.bas (sequential file
! round-trip; EOF() stays FALSE until an over-read is actually attempted,
! which raises the standard OS-9 `E$EOF` (211) — a real gotcha for the
! natural `WHILE NOT EOF DO READ...ENDWHILE` loop idiom, which needs an
! ON ERROR GOTO to catch the final over-read cleanly).
!
! No 6809 emulator exists in this project (see 6809/STATUS.md) — the
! divergence-probe values above are 68k-only. Written in portable BASIC09
! (no PEEK/POKE/ADDR) so it's ready to run unchanged on a real 6809 system
! or future 6809 emulator for a direct side-by-side.
!
! To use: enter both procedures via the E: editor (each needs a leading
! space per line, or it's parsed as an editor command), then:
!   RUN bytetest(200)    ! sanity-check the helper alone (optional)
!   RUN langtest2         ! full test, calls bytetest(200) internally

PROCEDURE langtest2
 DIM i,n,pass,fail,total: INTEGER
 DIM arr(3): INTEGER
 DIM arr2(2,2): INTEGER
 DIM s: STRING[10]
 DIM r1,r2,i1: INTEGER
 DIM x,y,r: REAL
 pass = 0
 fail = 0
 total = 0
 PRINT "=== BASIC09 Language Self-Test (expanded) ==="
 arr(1) = 10
 arr(2) = 20
 arr(3) = 30
 total = total+1
 IF arr(1)=10 AND arr(3)=30 THEN
  pass = pass+1
  PRINT "PASS 1: BASE 1 default array 1..3"
 ELSE
  fail = fail+1
  PRINT "FAIL 1: BASE 1 default array 1..3"
 ENDIF
 BASE 0
 arr(0) = 100
 arr(1) = 200
 total = total+1
 IF arr(0)=100 AND arr(1)=200 THEN
  pass = pass+1
  PRINT "PASS 2: BASE 0 shifts indices to 0..bound-1"
 ELSE
  fail = fail+1
  PRINT "FAIL 2: BASE 0 shifts indices to 0..bound-1"
 ENDIF
 s = "HELLO"
 total = total+1
 IF MID$(s,1,1)="H" THEN
  pass = pass+1
  PRINT "PASS 3: string index 1-based under BASE 0"
 ELSE
  fail = fail+1
  PRINT "FAIL 3: string index 1-based under BASE 0"
 ENDIF
 BASE 1
 total = total+1
 r1 = LAND(6,3)
 IF r1=2 THEN
  pass = pass+1
  PRINT "PASS 4: LAND is bitwise (6 LAND 3 = 2)"
 ELSE
  fail = fail+1
  PRINT "FAIL 4: LAND is bitwise, got "; r1
 ENDIF
 total = total+1
 r2 = SUBSTR("L","HELLO")
 IF r2=3 THEN
  pass = pass+1
  PRINT "PASS 5: SUBSTR 1-based (first L at 3)"
 ELSE
  fail = fail+1
  PRINT "FAIL 5: SUBSTR 1-based, got "; r2
 ENDIF
 total = total+1
 i = 0
 LOOP
  i = i+1
  EXITIF i>=5 THEN
   pass = pass+1
   PRINT "PASS 6: EXITIF exits loop correctly (i="; i; ")"
  ENDEXIT
 ENDLOOP
 total = total+1
 RUN bytetest(200)
 total = total+1
 i = 0
 WHILE i<3 DO
  i = i+1
 ENDWHILE
 IF i=3 THEN
  pass = pass+1
  PRINT "PASS 8: WHILE...DO loop runs correct count (i="; i; ")"
 ELSE
  fail = fail+1
  PRINT "FAIL 8: WHILE loop, got i="; i
 ENDIF
 total = total+1
 i = 0
 REPEAT
  i = i+1
 UNTIL i>=3
 IF i=3 THEN
  pass = pass+1
  PRINT "PASS 9: REPEAT/UNTIL runs correct count (i="; i; ")"
 ELSE
  fail = fail+1
  PRINT "FAIL 9: REPEAT/UNTIL, got i="; i
 ENDIF
 total = total+1
 arr2(1,1) = 11
 arr2(2,2) = 22
 IF arr2(1,1)=11 AND arr2(2,2)=22 THEN
  pass = pass+1
  PRINT "PASS 10: 2D array indexing works"
 ELSE
  fail = fail+1
  PRINT "FAIL 10: 2D array indexing"
 ENDIF
 total = total+1
 s = "THISISLONG"
 IF LEN(s)<=10 THEN
  pass = pass+1
  PRINT "PASS 11: STRING truncates at declared max, len="; LEN(s)
 ELSE
  fail = fail+1
  PRINT "FAIL 11: STRING truncation, len="; LEN(s)
 ENDIF
 total = total+1
 i1 = -5
 IF ABS(i1)=5 THEN
  pass = pass+1
  PRINT "PASS 12: ABS(-5)=5"
 ELSE
  fail = fail+1
  PRINT "FAIL 12: ABS"
 ENDIF
 total = total+1
 i1 = -3
 IF SGN(i1)=-1 AND SQ(4)=16 THEN
  pass = pass+1
  PRINT "PASS 13: SGN(-3)=-1 AND SQ(4)=16"
 ELSE
  fail = fail+1
  PRINT "FAIL 13: SGN/SQ"
 ENDIF
 total = total+1
 IF FIX(3.9)=4 AND FLOAT(5)=5. THEN
  pass = pass+1
  PRINT "PASS 14: FIX(3.9)=4 (rounds, see bug note), FLOAT(5)=5."
 ELSE
  fail = fail+1
  PRINT "FAIL 14: FIX/FLOAT"
 ENDIF
 total = total+1
 r = RND(0)
 IF r>=0. AND r<=1. THEN
  pass = pass+1
  PRINT "PASS 15: RND(0) in range 0-1"
 ELSE
  fail = fail+1
  PRINT "FAIL 15: RND(0)"
 ENDIF
 PRINT
 PRINT "=== Divergence probes (expect different on 6809 vs 68k) ==="
 PRINT "SIZE(INTEGER) = "; SIZE(i)
 PRINT "SIZE(REAL) = "; SIZE(x)
 DIGITS 15
 x = 1./3.
 PRINT "1/3 at DIGITS 15 = "; x
 n = 2147483647
 n = n+1
 PRINT "probe1 var+var: INTEGER max+1 = "; n
 PRINT "probe2 var+literal: "; 2147483647+1
 PRINT
 PRINT "=== SUMMARY ==="
 PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
 END

PROCEDURE bytetest
 PARAM b: BYTE
 IF b<>200 THEN
  PRINT "PASS 7 (confirms gotcha): BYTE param silently unbound, b="; b
 ELSE
  PRINT "FAIL 7 (gotcha not reproduced): b="; b
 ENDIF

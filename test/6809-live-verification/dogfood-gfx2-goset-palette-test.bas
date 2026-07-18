! Dogfood 2026-07-18: BASIC09 test procedure(s) used to probe GFX2's
! GOSET and PALETTE, backing the Live-tagged findings added to
! 6809/gfx-windowing.md's "Open ends" section. See
! dogfood-report-gfx2-2026-07-18.md for the full narrative (install of
! the community GFX/GFX2 package, the asm/MIA porting fixes needed, etc.)
!
! This is a LOG of six variants of one procedure (all named "goxtest"),
! not a single loadable file -- BASIC09 has no direct "type a statement,
! it runs" immediate mode (confirmed live: a bare statement at the B:
! Ready prompt gives "Error #051 -- Line with Compiler Error" or "What?",
! never a result), so each variant below was entered via BASIC09's own
! line editor (`e goxtest`, `D n` to clear the old body, leading-space
! insert for each new line, `q` to save) and executed with `run`, one
! variant at a time, via tools/nitros9repl.sh's `key` interface (`send`
! doesn't recognize BASIC09's own E: sub-prompt).
!
! To replay any one variant: copy just its PROCEDURE...END block alone
! into goxtest and `run` it.

! ---- Variant 1: does GOSET exist at all? ----
PROCEDURE goxtest
RUN GFX2("GOSET",1)
END
! Result (live): "Error #048 -- Unimplemented Routine", then BASIC09
! BREAKs into its own debugger (D:). This is GFX2's function-name
! dispatch table returning "not found" (E$NoRout) -- confirmed directly
! against gfx2.asm's own FuncTbl fcc-string list, which has no "GOSET"
! entry anywhere among its ~60 registered function names. GOSET is
! genuinely absent from this real, community-maintained GFX2 build, not
! merely untested.

! ---- Variant 2: PALETTE, in-range guess, no path (current window) ----
PROCEDURE goxtest
RUN GFX2("PALETTE",5,10)
END
! Result (live): ran clean, no error, straight back to Ready. Confirms
! the calling form RUN GFX2([path,]"PALETTE",register,color) is real,
! and that register 5 / color 10 are both accepted.

! ---- Variant 3: PALETTE, register 20, no path ----
PROCEDURE goxtest
RUN GFX2("PALETTE",20,10)
END
! Result (live): ran clean, no error.

! ---- Variant 4: PALETTE, register 99, no path ----
PROCEDURE goxtest
RUN GFX2("PALETTE",99,10)
END
! Result (live): ran clean, no error either -- no visible rejection for
! a register number far outside the presumed 0-15 (16-hardware-register)
! range.

! ---- Variant 5: PALETTE against an explicit, real window path ----
! (rules out "the no-path default target isn't a real window, so of
! course nothing complained" as an alternative explanation for 2-4)
PROCEDURE goxtest
DIM p: INTEGER
OPEN #p,"/W1":WRITE
RUN GFX2(p,"PALETTE",99,10)
CLOSE #p
END
! Result (live): ran clean, no error. /W1 (a real CoCo3 hardware text
! window device) opened fine and raised no objection to register 99
! either.

! ---- Variant 6: PALETTE, in-range register, out-of-range color ----
PROCEDURE goxtest
DIM p: INTEGER
OPEN #p,"/W1":WRITE
RUN GFX2(p,"PALETTE",5,200)
CLOSE #p
END
! 200 is far outside the documented 0-63 (64-color) range.
! Result (live): ran clean, no error either.

! Conclusion: this GFX2 build's PALETTE implementation (gfx2.asm, label
! L0585: "lda #$31 / bra L0526 [append 3 parameters or exit with
! parameter error]") only checks the BASIC09 *parameter count* (must
! resolve to exactly 3 args including the function name) before
! packaging register+color into an escape-code byte stream and writing
! it out via I$Write. There is no source-level bounds check on either
! value, and nothing downstream (BASIC09, GFX2, or whatever received the
! write on /W1) rejected out-of-range values either. The presumed 0-15
! register / 0-63 color hardware limits are therefore NOT
! software-enforced in this implementation -- out-of-range calls are
! silently accepted, not validated and rejected. This directly answers
! (in the negative) the "bracket the real valid range by seeing what's
! rejected" question as originally framed: nothing was rejected up to
! the values tried (register 99, color 200), so no upper bound could be
! located this way. What the *hardware* actually does with an
! out-of-range register/color value (wrap, alias, corrupt adjacent
! state) was not established -- that needs actual pixel/palette
! observation, which a text-only REPL channel can't provide (matches the
! existing "text-REPL testing can't observe pixel output" caveat already
! in gfx-windowing.md).

! BASIC09 math speed test — REAL arithmetic + transcendental functions.
!
! Built 2026-07-14 to compare interpreted (RUN from workspace) vs
! PACK-compiled (RUN after PACK) execution speed on os9exec (68k).
!
! Result: PACK does NOT speed up in-workspace execution — both ran in
! ~2.05s for N=3000, printing the identical value 12558.8286. BASIC09
! always executes I-code either way; PACK only strips listability/
! editability, not the execution path. See
! ~/.claude/skills/os9-dev/references/common/module-format.md
! ("BASIC09 Module Packing & Linking") for the full writeup, including
! why the theoretically-faster standalone path (RunB) is untestable on
! this project's disk image (the RunB binary doesn't exist under
! /h0/CMDS).
!
! Domain is deliberately clamped to (0,1] — an earlier, larger-N version
! let x range wide enough that TAN() swept past its asymptotes and blew
! up numerically. Increase N for a longer/more precise timing run, but
! keep x's range bounded the same way if you do.
!
! To use: load into BASIC09 (e.g. `build` a copy on the OS-9 side, or
! type into the E: editor — remember each line needs a leading space in
! the editor or it's parsed as an editor command instead of program
! text). Then:
!   RUN mt4          ! interpreted timing
!   PACK mt4         ! compiles in place (non-listable afterward)
!   RUN mt4          ! packed timing — confirm it matches unpacked
!
! Timing itself has no in-language primitive (F$Time isn't attested in
! this skill's syscall-reference.md) — time it from the host shell
! (date before/after) or by polling the tmux pane for the prompt.

PROCEDURE mt4
 DIM i,n: INTEGER
 DIM total,x,y: REAL
 n = 3000
 total = 0.
 FOR i = 1 TO n
 x = FLOAT(i)/n
 y = SIN(x)+COS(x)+SQR(x+1.)+LOG(x+1.)+EXP(x)-ATN(x)
 total = total+y
 NEXT i
 PRINT total

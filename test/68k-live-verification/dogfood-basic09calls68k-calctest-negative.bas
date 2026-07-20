PROCEDURE calctest2
! Dogfood task 2026-07-18: sign-handling cross-check for
! dogfood-basic09calls68k-addmulmod.a, paired with
! dogfood-basic09calls68k-calctest.bas. Same routine, a negative operand.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "dog", `basic #32k` -> LOAD calctest2.bas -> RUN calctest2):
!   7
!   -30
! i.e. sum=-3+10=7, prod=-3*10=-30 -- confirms the assembly's muls.w
! (16x16->32 signed multiply) produces a correctly signed result, not
! just a correct magnitude, for a negative input.
DIM a,sum,prod:INTEGER
a=-3
RUN addmulmod(a,10,sum,prod)
PRINT sum
PRINT prod
END

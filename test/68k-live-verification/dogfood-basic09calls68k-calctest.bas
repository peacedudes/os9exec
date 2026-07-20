PROCEDURE calctest
! Dogfood task 2026-07-18: calls the hand-written 68k assembly Sbrtn
! module dogfood-basic09calls68k-addmulmod.a (assembled/linked to
! /dd/CMDS/DOG/addmulmod on the live system) via RUN, passing `a` by
! reference and the literal 7 by value, and reading back two by-reference
! outputs.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "dog", `basic #32k` -> LOAD calctest.bas -> RUN calctest):
!   13
!   42
! i.e. sum=6+7=13, prod=6*7=42 -- proving the mechanism BOTH generalizes
! to more than one input/output parameter AND handles a mixed
! by-reference/by-value call correctly.
DIM a,sum,prod:INTEGER
a=6
RUN addmulmod(a,7,sum,prod)
PRINT sum
PRINT prod
END

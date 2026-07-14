! Emulated-interpreted vs. compiled-native speed comparison (trig-heavy).
! Run on the real Microware BASIC09 binary via os9exec (68k, interpreted
! I-code), timed from the host shell bracketing the RUN command.
!
! LIVE RESULT (68k, interpreted): N=50000, 32.7s wall-clock, result
! 209294.837 -- roughly 654 microseconds/iteration.
!
! The equivalent program compiled via basic09c (LLVM, native, -O2) at
! N=3,000,000 (basic09c's INTEGER is 16-bit -- see basic09-language.md
! -- so large N needs a nested loop to stay under 32767 per dimension)
! ran in 0.05s -- roughly 16.7 nanoseconds/iteration. That's
! approximately a 39,000x speedup for compiled native code over the
! emulated interpreter on trig-heavy work. See
! basic09-vs-basic09c-differences.md for the full writeup including the
! integer-only comparison (which showed an even larger ratio, since
! interpreter dispatch overhead dominates more when there's less real
! math work per iteration to amortize it against).
PROCEDURE trigbench
DIM i,n: INTEGER
DIM total,x,y: REAL
n = 50000
total = 0.
FOR i = 1 TO n
x = FLOAT(i)/FLOAT(n)
y = SIN(x)+COS(x)+SQR(x+1.)+LOG(x+1.)+EXP(x)-ATN(x)
total = total+y
NEXT i
PRINT total
END

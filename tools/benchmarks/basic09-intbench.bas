! Emulated-interpreted vs. compiled-native speed comparison (integer-only,
! no transcendental calls -- contrast case for basic09-trigbench.bas).
! Run on the real Microware BASIC09 binary via os9exec (68k, interpreted
! I-code), timed from the host shell.
!
! LIVE RESULT (68k, interpreted): N=500000, ~8.6s wall-clock (upper
! bound -- finished within the first 1s poll interval after an ~8s
! command round-trip, so true time is somewhere under 8.6s, not
! precisely pinned down) -- roughly <17 microseconds/iteration.
!
! The equivalent program compiled via basic09c (LLVM, native, -O2) at
! N=100,000,000 (nested loop, same 16-bit INTEGER-width reason as
! trigbench) ran in 0.02s -- roughly 0.2 nanoseconds/iteration. That's
! at least an ~86,000x speedup for compiled native code, likely more
! given the interpreted side's time is an upper bound. Note: the
! accumulated `total` values are NOT expected to match between the two
! runs -- basic09c's INTEGER is 16-bit vs. the real 68k binary's 32-bit,
! so the accumulator itself wraps differently on each side; this
! benchmark compares iteration throughput, not the arithmetic checksum.
! See basic09-vs-basic09c-differences.md for the full writeup.
PROCEDURE intbench
DIM i,n: INTEGER
DIM total: INTEGER
n = 500000
total = 0
FOR i = 1 TO n
total = total + LAND(i,255) - MOD(i,7)
NEXT i
PRINT total
END

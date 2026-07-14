! Recursion -- previously entirely untested. Confirmed on 68k with both
! a simple single-recursive case (factorial, RUN calling itself with a
! decremented argument and an output PARAM) and a doubly-recursive case
! (Fibonacci, two recursive calls per stack frame) -- the second is a
! meaningfully harder test since it exercises multiple live activation
! records of the same procedure simultaneously, not just a simple
! unwind-as-you-go chain.
!
! LIVE RESULT (68k): fact(5)=120, fib(10)=55 -- both PASS
PROCEDURE t_recursion
DIM result: INTEGER
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
result = 0
RUN fact(5, result)
total = total+1
IF result=120 THEN
 pass = pass+1
 PRINT "PASS: recursive fact(5)=120, got "; result
ELSE
 fail = fail+1
 PRINT "FAIL: fact(5) got "; result
ENDIF
result = 0
RUN fib(10, result)
total = total+1
IF result=55 THEN
 pass = pass+1
 PRINT "PASS: recursive fib(10)=55, got "; result
ELSE
 fail = fail+1
 PRINT "FAIL: fib(10) got "; result
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END

PROCEDURE fact
PARAM n: INTEGER
PARAM result: INTEGER
DIM sub: INTEGER
IF n<=1 THEN
 result = 1
ELSE
 sub = 0
 RUN fact(n-1, sub)
 result = n * sub
ENDIF
END

PROCEDURE fib
PARAM n: INTEGER
PARAM result: INTEGER
DIM a,b: INTEGER
IF n<=1 THEN
 result = n
ELSE
 a = 0
 b = 0
 RUN fib(n-1, a)
 RUN fib(n-2, b)
 result = a + b
ENDIF
END

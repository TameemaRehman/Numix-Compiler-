; MathSeq Compiler Output
; Source: test/examples/fibonacci.mathseq
; =======================

fibonacci:
n = param_n
a = 0
b = 1
t0 = []
t0 = 0 STORE 0
t0 = 1 STORE 1
result = t0
goto L1
L0:
t1 = 1
next = t1
t2 = []
t2 = next STORE 0
t3 = result + t2
result = t3
a = 1
b = next
L1:
param result
t4 = call length, result
t5 = t4 < n
if t5 goto L0
L2:
return result
square:
x = param_x
t6 = x * x
return t6
main:
count = 10
param 10
t7 = call fibonacci, 10
fib_sequence = t7
param fib_sequence
param square
t8 = call map, fib_sequence, square
squared_sequence = t8
param Fibonacci sequence: 
param fib_sequence
t9 = call print, Fibonacci sequence: , fib_sequence
param Squared sequence: 
param squared_sequence
t10 = call print, Squared sequence: , squared_sequence
param fib_sequence
t11 = call length, fib_sequence
t12 = t11 > 5
param fib_sequence
param 0
t13 = call get, fib_sequence, 0
t14 = t13 == 0
t15 = t12 && t14
ifFalse t15 goto L3
param Pattern matched: Sequence starts with 0 and has more than 5 elements
t16 = call print, Pattern matched: Sequence starts with 0 and has more than 5 elements
goto L4
L3:
L4:
return 0

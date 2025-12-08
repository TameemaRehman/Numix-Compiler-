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
main:
param How many Fibonacci numbers?
t6 = call input, How many Fibonacci numbers?
count = t6
param count
t7 = call fibonacci, count
fib_sequence = t7
param Fibonacci sequence: 
param fib_sequence
t8 = call print, Fibonacci sequence: , fib_sequence
return 0

; Program Output
; --------------
; Fibonacci sequence:  [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]
; Exit Code: 0

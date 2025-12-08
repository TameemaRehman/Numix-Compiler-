; MathSeq Compiler Output
; Source: test\examples\simple.mathseq
; =======================

main:
param Enter an integer:
t0 = call input, Enter an integer:
value = t0
param You entered: 
param value
t1 = call print, You entered: , value
return 0

; Program Output
; --------------
; You entered:  43
; Exit Code: 0

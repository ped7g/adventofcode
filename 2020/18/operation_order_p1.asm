; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; WARNING: this is unfinished solution, using only 32b math evaluator
; It works for test-input data, but overflows on real input
; As I felt the 64b solution is pointless chore, I switched to C++ for this puzzle
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

; https://adventofcode.com/2020/day/18 (part 1)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input_t.txt"    ; test input expected results: 71 + 51 + 26 + 437 + 12240 + 13632 = 26457
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  128,0 : ALIGN 256,0 ; zeroed padding after + align address

; parse_expression:
; stack.push(0, '+')
; while not[')', "\n"] {
;   parse_value
;   value = stack.pop() value
;   if ['+', '*'] stack.push(value, <<in)
; }

; parse_value:
; if '(' -> parse expression+1, check ')', +1 -> value
; if 0..9 -> parse number -> value

stackBeg:
stack = stackBeg
inPtr = inputFile
sum = 0
line = 1
value = 0
stackValue = 0

    MACRO SKIP_BLANKS
        WHILE ' ' == {b inPtr}
inPtr = inPtr + 1
        ENDW
    ENDM

    MACRO PARSE_VALUE
        SKIP_BLANKS
        IF '(' == {b inPtr}
inPtr = inPtr + 1
            PARSE_EXPRESSION
            ASSERT ')' == {b inPtr}
inPtr = inPtr + 1
        ELSE
value = 0
            WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
value = (value * 10) + {b inPtr} - '0'
inPtr = inPtr + 1
            ENDW
        ENDIF
    ENDM

    MACRO PARSE_EXPRESSION
        ; stack.push(0, '+')
        ORG stack : DD 0 : DB '+'
stack = stack + 5
        ; while not[')', "\n"] {
        WHILE ')' != {b inPtr} && "\n" != {b inPtr}
            PARSE_VALUE
            ; value = stack.pop() +/* value
stack = stack - 5
stackValue = {stack} | ({stack+2} << 16)
            IF '+' == {b stack+4}
value = stackValue + value
            ELSE
value = stackValue * value
            ENDIF
            ; if ['+', '*'] stack.push(value, <<in)
            SKIP_BLANKS
            IF '+' == {b inPtr} || '*' == {b inPtr}
                ORG stack : DD value : DB {b inPtr}
stack = stack + 5
inPtr = inPtr + 1
            ENDIF
        ENDW
    ENDM
    
    WHILE inPtr < inputEnd && {b inPtr}
        PARSE_EXPRESSION
        ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1
sum = sum + value
        DISPLAY "Line ",/D,line,", value ",/D,value,", sum ",/D,sum
line = line + 1
    ENDW

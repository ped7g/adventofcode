; https://adventofcode.com/2020/day/13 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
    INCBIN "input.txt"
inputEnd:

value = 0
    MACRO PARSE_VALUE
value = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
value = value * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

    ; init variables
inPtr = inputFile + (3 - __PASS__) * inputEnd   ; make it point beyond input in early passes
earliest = 0
wait = 0
minWait = (1<<31)-1
minID = -1

    ; parse earliest timestamp to depart
    IF 3 == __PASS__
        PARSE_VALUE : ASSERT "\n" == {b inPtr}
earliest = value
inPtr = inPtr + 1
    ENDIF

    DISPLAY "Earliest timestamp: ",/D,earliest

    WHILE inPtr < inputEnd && {b inPtr}
        ; skip non-number characters
        WHILE (',' == {b inPtr}) || ("\n" == {b inPtr}) || ('x' == {b inPtr})
inPtr = inPtr + 1
        ENDW
        ; parse bus ID number
        IF '0' <= {b inPtr} && {b inPtr} <= '9'
            PARSE_VALUE     ; value = bus ID
wait = -(earliest%value)
            IF wait < 0
wait = wait + value
            ENDIF
            DISPLAY "Bus id: ",/D,value,"\twait for it: ",/D,wait
            IF wait < minWait
minWait = wait
minID = value
            ENDIF
        ELSE
inPtr = inPtr + 1   ; some unexpected char in input (or "\0")
        ENDIF
    ENDW
    DISPLAY "Minimum wait ",/D,minWait," for bus ",/D,minID," - dot product: ",/D,minWait*minID

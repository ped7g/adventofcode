; https://adventofcode.com/2020/day/10 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
;     INCBIN "input_t2.txt"
    INCBIN "input.txt"
inputEnd:
    ds      5,0   ; some zeroed mem in case the algorithm reads way beyond input
    ALIGN 256,0

MAX_ADAPTER EQU 255

adapters:
    ds      MAX_ADAPTER+1,0

parseNumberVal = 0
    MACRO PARSE_VALUE
parseNumberVal = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
parseNumberVal = parseNumberVal * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

    ; init variables
inPtr = inputFile

    ; parse all adapters into memory
    WHILE inPtr < inputEnd && {b inPtr}
        PARSE_VALUE : ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1   ; skip newline char
        ORG adapters + parseNumberVal : DB 1    ; mark adapter in table
    ENDW

diff1 = 0
diff3 = 1   ; max adapter to device produces one 3-jolt difference already
prevA = adapters
nextA = adapters
    DUP MAX_ADAPTER+1
        IF {b nextA}
            IF 3 == nextA - prevA
diff3 = diff3 + 1
            ELSEIF 1 == nextA - prevA
diff1 = diff1 + 1
            ELSE
                DISPLAY "Found jolt difference ",/D,nextA - prevA,"?!"
            ENDIF
prevA = nextA
        ENDIF
nextA = nextA + 1
    EDUP
    DISPLAY "1-jolt diffs: ",/D,diff1,", 3-jolt diffs: ",/D,diff3,", dot = ",/D,diff1*diff3

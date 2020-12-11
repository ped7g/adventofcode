; https://adventofcode.com/2020/day/6 (part 2)
; (part 1 lost.. sry)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000

answers:  ds 256, 0

inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
    DB  "\n\n"              ; force trigger last group aftermath
inputEnd:

    ; init variables
inPtr = inputFile
inByte = 0
totalYes = 0
groups = 0
persons = 0

    ; read+process whole input file
    WHILE inPtr < inputEnd
        IF "\n" == {b inPtr} && "\n" == {b inPtr+1}
persons = persons + 1
            IF 0 < persons
                ORG 'a'
                DUP ('z'-'a'+1)
                    IF persons == {b $ }
totalYes = totalYes + 1
                    ENDIF
                    DB 0
                EDUP
            ENDIF
            DISPLAY "Group ",/D,groups," persons: ",/D,persons, " total yes: ",/D,totalYes
groups = groups + 1
persons = 0
        ELSEIF "\n" == {b inPtr}
persons = persons + 1
        ENDIF
        ; skip any whitespace
        WHILE inPtr < inputEnd && ("\n" == {b inPtr} || " " == {b inPtr} || "\t" == {b inPtr})
inPtr = inPtr + 1
        ENDW
inByte = {b inPtr}
inPtr = inPtr + 1
        IF 'a' <= inByte && inByte <= 'z'
            ORG inByte : DB {b inByte} + 1
        ENDIF
    ENDW
    DISPLAY "Groups: ",/D,groups
    DISPLAY "Sum of yes: ",/D,totalYes

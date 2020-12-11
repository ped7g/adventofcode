; https://adventofcode.com/2020/day/4 (part 2)
;(I forgot to preserve part 1 ... well, let's leave it as exercise to the reader)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000

inputFile:  ; load the input.txt into virtual device memory for processing
    DB "\n\n"       ; fake <EOL><EOL> to hit "new passport" state at beginning
    INCBIN "input.txt"
    DB "\n\n"       ; fake <EOL><EOL> to hit "new passport" state after last passsword too
inputEnd:

checkNumberPtr = 0
checkNumberVal = -1
    MACRO CHECK_DIGIT
        IF '0' <= {b checkNumberPtr} && {b checkNumberPtr} <= '9'
checkNumberVal = checkNumberVal * 10 + {b checkNumberPtr} - '0'
checkNumberPtr = checkNumberPtr + 1
        ELSE
checkNumberVal = -1
        ENDIF
    ENDM

    MACRO CHECK_HEXDIGIT
        IF '0' <= {b checkNumberPtr} && {b checkNumberPtr} <= '9'
checkNumberPtr = checkNumberPtr + 1
        ELSEIF 'a' <= {b checkNumberPtr} && {b checkNumberPtr} <= 'f'
checkNumberPtr = checkNumberPtr + 1
        ELSE
checkNumberVal = -1
        ENDIF
    ENDM

    MACRO CHECK_NUMBER digits?, idx?, ptr?, minY?, maxY?
checkNumberPtr = ptr?
checkNumberVal = 0
        .digits? CHECK_DIGIT
        IF minY? <= checkNumberVal && checkNumberVal <= maxY?
            CHECK_DIGIT
            IF checkNumberVal < 0
detectedFields = detectedFields | (1<<(idx?))
            ENDIF
        ENDIF
    ENDM

    MACRO CHECK_HEXNUMBER digits?, idx?, ptr?
checkNumberPtr = ptr?
checkNumberVal = 0
        .digits? CHECK_HEXDIGIT
        IF 0 <= checkNumberVal
            CHECK_HEXDIGIT
            IF checkNumberVal < 0
detectedFields = detectedFields | (1<<(idx?))
            ENDIF
        ENDIF
    ENDM

    MACRO CHECK_BYR idx?, ptr?
        CHECK_NUMBER 4, idx?, ptr?, 1920, 2002
    ENDM

    MACRO CHECK_IYR idx?, ptr?
        CHECK_NUMBER 4, idx?, ptr?, 2010, 2020
    ENDM

    MACRO CHECK_EYR idx?, ptr?
        CHECK_NUMBER 4, idx?, ptr?, 2020, 2030
    ENDM

    MACRO CHECK_HGT idx?, ptr?
        IF 'mc' == { ptr?+3 }       ; there is "cm" after three chars
            CHECK_NUMBER 3, idx?, ptr?, 150, 193
        ELSEIF 'ni' == { ptr?+2 }   ; there is "in" after two chars
            CHECK_NUMBER 2, idx?, ptr?, 59, 76
        ENDIF
    ENDM

    MACRO CHECK_HCL idx?, ptr?
        IF '#' == {b ptr? }
            CHECK_HEXNUMBER 6, idx?, (ptr?+1)
        ENDIF
    ENDM

eyeColor = 0
    MACRO CHECK_ECL idx?, ptr?
        IF ' ' == {b ptr?+3} || "\t" == {b ptr?+3} || "\n" == {b ptr?+3}
eyeColor = ({b ptr?+0}<<16) | ({b ptr?+1}<<8) | {b ptr?+2}  ; swap bytes to get "string" order
            IF 'amb' == eyeColor || 'blu' == eyeColor || 'brn' == eyeColor || 'gry' == eyeColor || 'grn' == eyeColor || 'hzl' == eyeColor || 'oth' == eyeColor
detectedFields = detectedFields | (1<<(idx?))
            ENDIF
        ENDIF
    ENDM

    MACRO CHECK_PID idx?, ptr?
        CHECK_NUMBER 9, idx?, ptr?, 0, 999999999
    ENDM

    MACRO CHECK_CID idx?, ptr?
detectedFields = detectedFields | (1<<(idx?))
    ENDM
                                    ; expected fields
    DEFARRAY _BYR_ 'b', 'y', 'r'    ; byr (Birth Year)
    DEFARRAY _IYR_ 'i', 'y', 'r'    ; iyr (Issue Year)
    DEFARRAY _EYR_ 'e', 'y', 'r'    ; eyr (Expiration Year)
    DEFARRAY _HGT_ 'h', 'g', 't'    ; hgt (Height)
    DEFARRAY _HCL_ 'h', 'c', 'l'    ; hcl (Hair Color)
    DEFARRAY _ECL_ 'e', 'c', 'l'    ; ecl (Eye Color)
    DEFARRAY _PID_ 'p', 'i', 'd'    ; pid (Passport ID)
    DEFARRAY _CID_ 'c', 'i', 'd'    ; cid (Country ID)

    DEFARRAY ALL_FIELDS _BYR_, _IYR_, _EYR_, _HGT_, _HCL_, _ECL_, _PID_, _CID_
    DEFARRAY ALL_CHECKS CHECK_BYR, CHECK_IYR, CHECK_EYR, CHECK_HGT, CHECK_HCL, CHECK_ECL, CHECK_PID, CHECK_CID

DETECTED_NONE       EQU     0
DETECTED_ALL        EQU     (1<<ALL_FIELDS[#])-1
DETECTED_ALL_NO_CID EQU     DETECTED_ALL ^ (1<<7)

    ; init variables
valid = 0
detectedFields = DETECTED_NONE
inPtr = inputFile

    ; read+process whole input file
    WHILE inPtr < inputEnd
        IF "\n" == {b inPtr} && "\n" == {b inPtr+1 }
            IF (detectedFields & DETECTED_ALL_NO_CID) == DETECTED_ALL_NO_CID
valid = valid + 1
                DISPLAY "passport is valid, total valid: ",/D,valid
            ENDIF
detectedFields = DETECTED_NONE
inPtr = inPtr + 2
            WHILE inPtr < inputEnd && ("\n" == {b inPtr} || " " == {b inPtr} || "\t" == {b inPtr})
inPtr = inPtr + 1
            ENDW
            DISPLAY "new passport starts at: ",/A,inPtr
        ENDIF
        IF "\n" != {b inPtr} && " " != {b inPtr} && "\t" != {b inPtr}
fieldIndex = 0
            WHILE fieldIndex < ALL_FIELDS[#]
charIndex = 0
                WHILE charIndex < ALL_FIELDS[fieldIndex][#]
                    IF {b inPtr + charIndex } != ALL_FIELDS[fieldIndex][charIndex]
charIndex = 1 + ALL_FIELDS[fieldIndex][#]
                    ENDIF
charIndex = charIndex + 1
                ENDW
                IF charIndex == ALL_FIELDS[fieldIndex][#]
                    IF {b inPtr + charIndex } == ':'
                        ;DISPLAY "Found keyword ",/D,fieldIndex," ",ALL_FIELDS[fieldIndex][0],ALL_FIELDS[fieldIndex][1],ALL_FIELDS[fieldIndex][2],".."
                        ALL_CHECKS[fieldIndex] fieldIndex, (inPtr+charIndex+1)
fieldIndex = ALL_FIELDS[#]
                    ENDIF
                ENDIF
fieldIndex = fieldIndex + 1
            ENDW
            ; skip all chars until colon, and then skip colon too
            WHILE inPtr < inputEnd && (':' != {b inPtr})
inPtr = inPtr + 1
            ENDW
            IF ':' == {b inPtr }
inPtr = inPtr + 1
                ; skip value
                WHILE inPtr < inputEnd && ("\n" != {b inPtr} && " " != {b inPtr} && "\t" != {b inPtr})
inPtr = inPtr + 1   ; skip non-white space chars
                ENDW
                WHILE inPtr < inputEnd && (" " == {b inPtr} || "\t" == {b inPtr} || ("\n" == {b inPtr} && "\n" != {b inPtr+1 }))
inPtr = inPtr + 1   ; skip space and single newline
                ENDW
            ENDIF
        ENDIF
    ENDW
    DISPLAY "Valid passports: ",/D,valid

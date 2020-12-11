; https://adventofcode.com/2020/day/2 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
    DB      10  ; fake <EOL> to make sure there's one
inputEnd:

PHASE_NUM1      EQU 0   ; reading policy number1
PHASE_NUM2      EQU 1   ; reading policy number2
PHASE_LETTER    EQU 2   ; reading policy letter
PHASE_CHECKING  EQU 3   ; checking the password against the policy

    ; init variables
inPtr = inputFile
valid = 0
phase = PHASE_NUM1
number = -1
num1 = -1
num2 = -1
count = -1
letter = -1

    ; parse the input
    DUP (inputEnd - inputFile)  ; read+process whole input file
inByte = {b inPtr}
inPtr = inPtr + 1
        IF phase == PHASE_NUM1 || phase == PHASE_NUM2
            IF number < 0
                ; was outside of number, keep skipping bytes until new digit is found
                IF '0' <= inByte && inByte <= '9'
number = inByte - '0'    ; start of new number
                ENDIF
            ELSE
                ; was inside number, keep adding digits or process it
                IF '0' <= inByte && inByte <= '9'   ; another digit
number = 10*number + inByte - '0'
                ELSE    ; not a digit, set num1 or num2, and switch to next phase
                    IF phase == PHASE_NUM1
num1 = number
                    ELSE
num2 = number
                    ENDIF
phase = phase + 1
number = -1         ; reinit for next number parsing
                ENDIF
            ENDIF
        ELSEIF phase == PHASE_LETTER
            IF ' ' != inByte
letter = inByte
phase = phase + 1
count = 0       ; DISPLAY "Checking policy letter ",letter," ",/B,letter," ",/D,num1,"-",/D,num2
            ENDIF
        ELSE
            ASSERT phase == PHASE_CHECKING
            IF inByte < ' '     ; EOL (or other garbage character)
                IF num1 <= count && count <= num2
valid = valid + 1
                ENDIF
phase = PHASE_NUM1
            ELSE
                IF letter == inByte
count = count + 1
                ENDIF
            ENDIF
        ENDIF
    EDUP
    DISPLAY "Valid passwords: ",/D,valid

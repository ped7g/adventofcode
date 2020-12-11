; https://adventofcode.com/2020/day/1 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
valuesHit:  ; byte table keeping notes about values hit in input
    block   2020+1, 0
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
    DB      10  ; fake <EOL> to make sure there's no digit as last char
inputEnd:
    
    ; init variables
inPtr = inputFile
number = -1

    ; parse the input (to mark each number in the table)
    DUP (inputEnd - inputFile)  ; read+process whole input file
inByte = {b inPtr}
inPtr = inPtr + 1
        IF number < 0
            ; was outside of number, keep skipping bytes until new digit is found
            IF '0' <= inByte && inByte <= '9'
number = inByte - '0'    ; start of new number
            ENDIF
        ELSE
            ; was inside number, keep adding digits or process it
            IF '0' <= inByte && inByte <= '9'   ; another digit
number = 10*number + inByte - '0'
            ELSE    ; not a digit, mark the number
                IF 0 <= number && number <= 2020    ; only these make sense
                    IF {b number} : DISPLAY "Number ",/D,number," found twice!" : ENDIF
                    ORG number : DB 1   ; mark it table, set more bits for repeated numbers
                ENDIF
number = -1         ; start skipping of non-digit characters
            ENDIF
        ENDIF
    EDUP

    ; do the two nested loops looking for triplets summing as 2020
num1 = 0
num2 = 0
num3 = 0
    DUP 2020/3 + 1  ; num1 0..673 (looking for "num1 <= num2 <= num3" type of result)
        IF {b num1}
num2 = num1
            DUP ((2020-num1)/2) - num2 + 1  ; loop num2 = num1 .. (2020-num1)/2 ; to enforce num2 <= num3
                IF {b num2}
num3 = 2020 - num1 - num2
                    ASSERT 0 <= num1 && num1 <= num2 && num2 <= num3 && num3 <= 2020
                    IF {b num3}
                        DISPLAY "Sum 2020: ",/D,num1," + ",/D,num2," + ",/D,num3," dot: ",/D,num1*num2*num3
                    ENDIF
                ENDIF
num2 = num2 + 1
            EDUP
        ENDIF
num1 = num1 + 1
    EDUP

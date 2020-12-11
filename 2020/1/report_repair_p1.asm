; https://adventofcode.com/2020/day/1 (part 1)
    DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
valuesHit:  ; byte table keeping notes about values hit in input
    block   2020+1, 0
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
    DB      10  ; fake <EOL> to make sure there's no digit as last char
inputEnd:

    ; init variables
inPtr = inputFile
number = -1

    ; process the input and display results
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
            ELSE    ; not a digit, process the number
                IF 0 <= number && number <= 2020    ; only these make sense
                    ORG number : DB 1               ; mark it in table
                    IF {b 2020-number}  ; check if sum 2020 is possible
                        DISPLAY "Sum 2020: ", /D, number, " + ", /D, 2020-number, " dot: ", /D, (2020-number)*number
                    ENDIF
                ENDIF
number = -1         ; start skipping of non-digit characters
            ENDIF
        ENDIF
    EDUP

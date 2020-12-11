; https://adventofcode.com/2020/day/5 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000

inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
inputEnd:

    ; init variables
inPtr = inputFile
seatID = 0
maxSeatID = 0

    ; read+process whole input file
    WHILE inPtr < inputEnd
        WHILE inPtr < inputEnd && ("\n" == {b inPtr} || " " == {b inPtr} || "\t" == {b inPtr})
inPtr = inPtr + 1
        ENDW
        IF 'F' == {b inPtr} || 'B' == {b inPtr}     ; seat "name" starts here
seatID = 0
            DUP 10
seatID = (seatID<<1) | (1 & ('B' == {b inPtr} || 'R' == {b inPtr}))
inPtr = inPtr + 1
            EDUP
            DISPLAY "Seat ID: ",/A,seatID
maxSeatID = maxSeatID >? seatID
        ELSE
inPtr = inPtr + 1       ; unexpected char, maybe end of input, try to reach inputEnd
        ENDIF
    ENDW
    DISPLAY "Max Seat ID: ",/A,maxSeatID

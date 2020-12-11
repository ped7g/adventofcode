; https://adventofcode.com/2020/day/5 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000

inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
inputEnd:

seats:  ds (1<<10), 0

    ; init variables
inPtr = inputFile
seatID = 0
minSeatID = (1<<10)
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
            ORG seats + seatID : DB 1   ; mark the seat as seated (not free)
maxSeatID = maxSeatID >? seatID
minSeatID = minSeatID <? seatID
        ELSE
inPtr = inPtr + 1       ; unexpected char, maybe end of input, try to reach inputEnd
        ENDIF
    ENDW
    DISPLAY "Min Seat ID: ",/A,minSeatID
    DISPLAY "Max Seat ID: ",/A,maxSeatID
    ; search for the free seats between min/max seat IDs, and display it (them)
checkPtr = seats + minSeatID
    WHILE checkPtr <= seats + maxSeatID
        IF !{b checkPtr}
            DISPLAY "Free Seat ID: ",/A,checkPtr-seats
        ENDIF
checkPtr = checkPtr + 1
    ENDW

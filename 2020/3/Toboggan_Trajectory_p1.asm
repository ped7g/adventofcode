; https://adventofcode.com/2020/day/3 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
    IF {b $-1 } != 10 : DB 10 : ENDIF    ; fake <EOL> if there's none
inputEnd = $    ; DEFL label type, so it can be +-1 in last pass (depends on fake EOL)

    ; init variables
trees = 0
posx = 0
width = -1
height = 1

    ; check the input data to figure out map width x height and count trees
inPtr = inputFile
    ASSERT '.' == {b inPtr }    ; top-left corner must be empty
    DUP (inputEnd - inputFile)  ; read+process whole input file
inByte = {b inPtr}
        IF 10 == inByte
            IF -1 == width      ; end of first line, set up "width" variable
width = inPtr - inputFile       ; the real width in bytes is +1 for EOL byte
            ELSE
                ASSERT inPtr == height * (width+1) + width  ; check width of other lines
                ; check if there's tree at toboggan position
                IF '#' == {b height * (width+1) + posx }
trees = trees + 1
                ENDIF
height = height + 1
            ENDIF
posx = (posx + 3) % width       ; move toboggan +3 to right (wrap around for "infinite" forest)
        ENDIF
inPtr = inPtr + 1
    EDUP
    DISPLAY "Map size ",/D,width,"x",/D,height," trees hit: ",/D,trees

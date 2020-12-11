; https://adventofcode.com/2020/day/3 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
    IF {b $-1 } != 10 : DB 10 : ENDIF    ; fake <EOL> if there's none
inputEnd = $    ; DEFL label type, so it can be +-1 in last pass (depends on fake EOL)

    ; init variables
trees11 = 0
trees31 = 0
trees51 = 0
trees71 = 0
trees12 = 0
posx11 = 0
posx31 = 0
posx51 = 0
posx71 = 0
posx12 = 0
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
                IF '#' == {b height * (width+1) + posx11 }
trees11 = trees11 + 1
                ENDIF
                IF '#' == {b height * (width+1) + posx31 }
trees31 = trees31 + 1
                ENDIF
                IF '#' == {b height * (width+1) + posx51 }
trees51 = trees51 + 1
                ENDIF
                IF '#' == {b height * (width+1) + posx71 }
trees71 = trees71 + 1
                ENDIF
                IF 0==(height&1) && '#' == {b height * (width+1) + posx12 }
trees12 = trees12 + 1
                ENDIF
height = height + 1
            ENDIF
            ; move toboggans by their offset to right (wrap around for "infinite" forest)
posx11 = (posx11 + 1) % width
posx31 = (posx31 + 3) % width
posx51 = (posx51 + 5) % width
posx71 = (posx71 + 7) % width
            IF height&1
posx12 = (posx12 + 1) % width
            ENDIF
        ENDIF
inPtr = inPtr + 1
    EDUP
    DISPLAY "Map size ",/D,width,"x",/D,height
    DISPLAY "Trees hit: ",/D,trees11," ",/D,trees31," ",/D,trees51," ",/D,trees71," ",/D,trees12
    DISPLAY /D,trees11*trees31*trees51*trees71*trees12

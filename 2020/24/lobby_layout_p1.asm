; https://adventofcode.com/2020/day/24 (part 1)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 10
    INCBIN "input.txt"      ; expected result: 277
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  10,0 : ALIGN 256,0  ; zeroed padding after + align address

; both input are very low on count of lines, so instead of having full map, I will
; keep list of tiles with coordinates, which were affected
; the coordinates system:
; __/__  origin [0,0]
;  /     ne=[0,-1], sw=[0,+1], e=[+1,0], w=[-1,0], nw=[-1,-1], se=[+1,+1]

flips=0                     ; total count of flips of any tile (for curiosity/check)
black=0                     ; current count of black (flipped odd times) tiles (= result)
tiles:  ; memory array to create list of currently black tiles so far [int8_t,int8_t]
tx = 0
ty = 0
tilesPtr = 0
tilesEnd = tiles

    STRUCT S_TILE           ; [x,y] coordinates [int8_t, int8_t] (-128..+127)
x       BYTE    0           ; actually the code will mostly work with them as 0..255
y       BYTE    0
    ENDS

inPtr = inputFile
    WHILE inPtr < inputEnd && {b inPtr}
        ASSERT 's' == {b inPtr} || 'n' == {b inPtr} || 'e' == {b inPtr} || 'w' == {b inPtr}
        ; start new tile from [0,0] origin
tx = 0
ty = 0
        ; read all directions, adjust coordinates
        WHILE "\n" != {b inPtr}
            IF     'en' == {inPtr}  ; little-endian encoding of char pair => flipped order
ty = ty - 1
inPtr = inPtr + 2
            ELSEIF 'ws' == {inPtr}
ty = ty + 1
inPtr = inPtr + 2
            ELSEIF 'e' == {b inPtr}
tx = tx + 1
inPtr = inPtr + 1
            ELSEIF 'w' == {b inPtr}
tx = tx - 1
inPtr = inPtr + 1
            ELSEIF 'wn' == {inPtr}
tx = tx - 1
ty = ty - 1
inPtr = inPtr + 2
            ELSEIF 'es' == {inPtr}
tx = tx + 1
ty = ty + 1
inPtr = inPtr + 2
            ELSE
                ASSERT 0 && "invalid char"
inPtr = inPtr + 1
            ENDIF
        ENDW
        ; flip the tile at resulting coordinates
        ASSERT -128 <= tx && tx <= 127 && -128 <= ty && ty <= 127
tx = tx & $FF       ; make them uint8_t (0..255)
ty = ty & $FF
        DISPLAY "Flipping tile [",/D,tx,",",/D,ty,"]"
flips = flips + 1           ; ++flips
tilesPtr = tiles
        WHILE tilesPtr < tilesEnd && ({b tilesPtr + S_TILE.x} != tx || {b tilesPtr + S_TILE.y} != ty)
tilesPtr = tilesPtr + S_TILE
        ENDW
        IF tilesPtr == tilesEnd
            ; add new tile to the list
            ORG tilesEnd : DB tx, ty
black = black + 1           ; ++black
tilesEnd = tilesEnd + S_TILE
        ELSE
            ; remove tile from the list (
black = black - 1           ; --black
tilesEnd = tilesEnd - S_TILE
            ORG tilesPtr : DW {tilesEnd}    ; move last tile to the empty slot
        ENDIF
inPtr = inPtr + 1           ; skip EOL
    ENDW
    DISPLAY "Current black tiles: ",/D,black,", total flips: ",/D,flips

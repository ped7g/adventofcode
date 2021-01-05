; https://adventofcode.com/2020/day/24 (part 2)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 2208
    INCBIN "input.txt"      ; expected result: 3531
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  10,0 : ALIGN 256,0  ; zeroed padding after + align address

; the coordinates system:
; __/__  origin [128,128]
;  /     ne=[0,-1], sw=[0,+1], e=[+1,0], w=[-1,0], nw=[-1,-1], se=[+1,+1]
; the maximum initial offset from origin in both inputs is +-13
; the black can spread at +-1 per axis per turn at most
; so max. coordinate after 100 turns are 128+-113 on both axis (still fit uint8_t)

DAYS_TO_DO  EQU     100
ORIGIN_X    EQU     115
ORIGIN_Y    EQU     115
INPUT_MAX   EQU     13      ; hard-coded optimization, both inputs are +-13 at most
MAX_SIZE_X  EQU     ORIGIN_X * 2 + 1
MAX_SIZE_XB EQU     (MAX_SIZE_X + 7) >>> 3  ; max size X in bytes when bit-mapped
MAX_SIZE_Y  EQU     ORIGIN_Y * 2 + 1

day = 0                     ; how many days passed (iterations of tile floor)
black = 0                   ; current count of black (flipped odd times) tiles (= result)
adjanced = 0

floor1:     DS      MAX_SIZE_XB * MAX_SIZE_Y, 0
floor2:     DS      MAX_SIZE_XB * MAX_SIZE_Y, 0
wFloor = floor1
rFloor = floor1

    ; set tile in "wFloor"
    MACRO SET_TILE x?, y?
        ORG wFloor + (y?) * MAX_SIZE_XB + ((x?) >> 3)
        DB {b $} | (1 << ((x?) & 7))
    ENDM

    ; clear tile in "wFloor"
    MACRO CLEAR_TILE x?, y?
        ORG wFloor + (y?) * MAX_SIZE_XB + ((x?) >> 3)
        DB {b $} & ~(1 << ((x?) & 7))
    ENDM

    ; flip tile in "wFloor"
    MACRO FLIP_TILE x?, y?
        ORG wFloor + (y?) * MAX_SIZE_XB + ((x?) >> 3)
        DB {b $} ^ (1 << ((x?) & 7))
    ENDM

    ; get tile from "rFloor" (0 / 1)
    MACRO GET_TILE x?, y?, r?
r? = 1 + (0 == ({b rFloor + (y?) * MAX_SIZE_XB + ((x?) >> 3)} & (1 << ((x?) & 7))))
    ENDM

    ; get tile from "rFloor" and add it to result
    MACRO GET_SUM_TILE x?, y?, r?
r? = r? - (0 != ({b rFloor + (y?) * MAX_SIZE_XB + ((x?) >> 3)} & (1 << ((x?) & 7))))
    ENDM

tx = ORIGIN_X
ty = ORIGIN_Y

inPtr = inputFile
    WHILE inPtr < inputEnd && {b inPtr}
        ASSERT 's' == {b inPtr} || 'n' == {b inPtr} || 'e' == {b inPtr} || 'w' == {b inPtr}
        ; start new tile from [0,0] origin
tx = ORIGIN_X
ty = ORIGIN_Y
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
        ASSERT 0 <= tx && tx <= 255 && 0 <= ty && ty <= 255
        DISPLAY "Flipping tile [",/D,tx,",",/D,ty,"]"
        FLIP_TILE tx, ty
inPtr = inPtr + 1           ; skip EOL
    ENDW

    IF 3 == __PASS__        ; do everything else only in third pass

    DUP DAYS_TO_DO
        ; advance day and pick read/write floor buffer
day = day + 1
        IF day & 1
rFloor = floor1
wFloor = floor2
        ELSE
rFloor = floor2
wFloor = floor1
        ENDIF
        ; do only inner rectangle leaving 1 tile border which is only read for adjanced status (but not written)
black = 0
ty = ORIGIN_Y - INPUT_MAX - day
        DUP (INPUT_MAX + day) * 2 + 1
tx = ORIGIN_X - INPUT_MAX - day
            DUP (INPUT_MAX + day) * 2 + 1
                GET_TILE tx+1,ty,adjanced
                GET_SUM_TILE tx-1,ty,adjanced
                GET_SUM_TILE tx+1,ty+1,adjanced
                GET_SUM_TILE tx-1,ty-1,adjanced
                GET_SUM_TILE tx,ty+1,adjanced
                GET_SUM_TILE tx,ty-1,adjanced
                GET_TILE tx,ty,inPtr ; tile itself
                IF 2 == adjanced || 2 == inPtr + adjanced
                    SET_TILE tx,ty
black = black + 1
                ELSE
                    CLEAR_TILE tx,ty
                ENDIF
tx = tx + 1
            EDUP
ty = ty + 1
        EDUP
        DISPLAY "Day ",/D,day,", current black tiles: ",/D,black
    EDUP

    ENDIF   ; end of IF 3 == __PASS__

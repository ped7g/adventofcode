; https://adventofcode.com/2020/day/12 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
    INCBIN "input.txt"
inputEnd:
    ds      5,0   ; some zeroed mem in case the algorithm reads way beyond input
    ALIGN 256,0

value = 0
    MACRO PARSE_VALUE
value = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
value = value * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

DIR_EAST    EQU 0   ; [+1, 0]
DIR_SOUTH   EQU 1   ; [0, +1]
DIR_WEST    EQU 2   ; [-1, 0]
DIR_NORTH   EQU 3   ; [0, -1]
    DEFARRAY ofsX +1, 0, -1, 0
    DEFARRAY ofsY 0, +1, 0, -1

    ; init variables
inPtr = inputFile + (3 - __PASS__) * inputEnd   ; make it point beyond input in early passes
wx = 10         ; initial waypoint position is E:10,N:1 = [+10,-1]
wy = -1
sx = 0          ; initial ship position is [0,0]
sy = 0
command = 0
dispVal = 0
temp = 0

    ; parse commands and adjust positions
    WHILE inPtr < inputEnd && {b inPtr}
command = {b inPtr}
inPtr = inPtr + 1   ; skip command char
        PARSE_VALUE : ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1   ; skip newline char
dispVal = value     ; preserve it for debug display
        IF 'E' == command
wx = wx + value * ofsX[DIR_EAST]
wy = wy + value * ofsY[DIR_EAST]
        ELSEIF 'S' == command
wx = wx + value * ofsX[DIR_SOUTH]
wy = wy + value * ofsY[DIR_SOUTH]
        ELSEIF 'W' == command
wx = wx + value * ofsX[DIR_WEST]
wy = wy + value * ofsY[DIR_WEST]
        ELSEIF 'N' == command
wx = wx + value * ofsX[DIR_NORTH]
wy = wy + value * ofsY[DIR_NORTH]
        ELSEIF 'F' == command
sx = sx + value * wx
sy = sy + value * wy
        ELSEIF 'L' == command || 'R' == command
            ASSERT 0 == value%90
value = (value / 90) & 3
            IF 'L' == command
value = (4 - value) & 3     ; flip the rotation
            ENDIF
            WHILE value     ; rotate waypoint coordinates by 90 degree clock-wise
temp = wx
wx = -wy
wy = temp
value = value - 1
            ENDW
        ELSE : ASSERT 0 && "invalid command" && command : ENDIF
        IF wx <= 0 : DEFINE+ WX_NAME "w " : ELSE : DEFINE+ WX_NAME "e " : ENDIF
        IF wy <= 0 : DEFINE+ WY_NAME ",\tn " : ELSE : DEFINE+ WY_NAME ",\ts " : ENDIF
        IF sx <= 0 : DEFINE+ SX_NAME "w " : ELSE : DEFINE+ SX_NAME "e " : ENDIF
        IF sy <= 0 : DEFINE+ SY_NAME ",\tn " : ELSE : DEFINE+ SY_NAME ",\ts " : ENDIF
        DISPLAY /C,command," ",/D,dispVal,",\twaypt: ",WX_NAME,/D,abs(wx),WY_NAME,/D,abs(wy),",\tship: ",SX_NAME,/D,abs(sx),SY_NAME,/D,abs(sy)
    ENDW
    DISPLAY "Ship's Manhattan distance: ",/D,abs(sx)+abs(sy)

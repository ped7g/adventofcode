; https://adventofcode.com/2020/day/12 (part 1)
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
x = 0
y = 0
sd = DIR_EAST   ; ship direction
cd = DIR_EAST   ; command direction
command = 0
dispVal = 0

    ; parse commands and adjust position
    WHILE inPtr < inputEnd && {b inPtr}
command = {b inPtr}
inPtr = inPtr + 1   ; skip command char
        PARSE_VALUE : ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1   ; skip newline char
dispVal = value     ; preserve it for debug display
        IF 'E' == command
cd = DIR_EAST
        ELSEIF 'S' == command
cd = DIR_SOUTH
        ELSEIF 'W' == command
cd = DIR_WEST
        ELSEIF 'N' == command
cd = DIR_NORTH
        ELSEIF 'F' == command
cd = sd
        ELSEIF 'L' == command
sd = (sd - value/90) & 3    : ASSERT (0 == value%90) && (0 <= sd) && (sd <= 3)
value = 0
        ELSEIF 'R' == command
sd = (sd + value/90) & 3    : ASSERT (0 == value%90) && (0 <= sd) && (sd <= 3)
value = 0
        ELSE
            ASSERT 0 && "invalid command" && command
value = 0
        ENDIF
x = x + value * ofsX[cd]
y = y + value * ofsY[cd]
        IF x <= 0 : DEFINE+ X_NAME ",\tw " : ELSE : DEFINE+ X_NAME ",\te " : ENDIF
        IF y <= 0 : DEFINE+ Y_NAME ",\tn " : ELSE : DEFINE+ Y_NAME ",\ts " : ENDIF
        DISPLAY "Command ",/C,command," value: ",/D,dispVal,X_NAME,/D,abs(x),Y_NAME,/D,abs(y),",\tfacing ",/D,sd
    ENDW
    DISPLAY "Manhattan distance: ",/D,abs(x)+abs(y)

; https://adventofcode.com/2020/day/15 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
    DS  2*32000,0           ; age table (holds last turn the number was spoken), WORDS

input:
;     DW  0,3,6               ; test input
;     DW  3,1,2               ; test input 2
    DW  2,20,0,4,1,17       ; part 1 input
inputEnd:

    ; init sequence
turn = 0
    DUP (inputEnd - input)>>1
num = { input + (turn << 1) }
turn = turn + 1
        DISPLAY "Turn ",/D,turn," the number is: ",/D,num
        ORG num << 1 : DW turn
    EDUP
num = 0                     ; next number to speak (this is where the seed ends)
    ; speak following numbers until 2020th number
    DUP 2020 - turn
turn = turn + 1
        DISPLAY "Turn ",/D,turn," the number is: ",/D,num
prevTurn = { num << 1 }
        ORG num << 1 : DW turn
        IF prevTurn
num = turn - prevTurn
        ELSE
num = 0
        ENDIF
    EDUP

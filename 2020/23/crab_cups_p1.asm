; https://adventofcode.com/2020/day/23 (part 1)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $8000

inputTxt:
;     DB  "389125467"         ; test input, expected result: 67384529
    DB  "318946572"         ; puzzle input, expected result: 52864379
inputEnd:
TOTAL_MOVES_TO_DO   EQU 100

    ORG 0
cups:
    DS  (inputEnd - inputTxt), $FF  ; link to next cup for [number]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parse input into linked list

    ; read last cup as "previous cup" (which will be linked to newly read cup)
previousCup = {b inputEnd - 1} - '0' :  ASSERT 1 <= previousCup && previousCup <= 9
inPtr = inputTxt
    WHILE inPtr < inputEnd
cup = {b inPtr} - '0' :     ASSERT 1 <= cup && cup <= 9
        DISPLAY /D,previousCup," -> ",/D,cup
        IF 3 == __PASS__ : ORG previousCup : DB cup : ENDIF
previousCup = cup
inPtr = inPtr + 1
    ENDW

    IF 3 == __PASS__    ; memory reads zeroes only in first two passes, avoid invalid values
firstCup = {b inputTxt} - '0' :         ASSERT 1 <= firstCup && firstCup <= 9
    ELSE
firstCup = 1
nextCup = 1
    ENDIF
    DISPLAY "First cup: ",/D,firstCup

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; simulate N moves

moves = 0
    DUP TOTAL_MOVES_TO_DO
moves = moves + 1   ; this move starts at "firstCup"
        DISPLAY "Move ",/D,moves," starts at cup: ",/D,firstCup," -> ",/D,{b firstCup}
        ; calculate possible nextCups bitmask (first bits set by "firstCup - 1")
nextCupsAvailable = %1'0000'00001 << (firstCup-1)   ; now add three following ones as fallback
nextCupsAvailable = nextCupsAvailable | (nextCupsAvailable>>1) | (nextCupsAvailable>>2) | (nextCupsAvailable>>3)
        ; fetch the three cups which will be moved away, and the following new "firstCup" for next move
moveCup1 = {b firstCup}
moveCup2 = {b moveCup1}
moveCup3 = {b moveCup2}
newFirstCup = {b moveCup3}
        ; mask-out the fetched cups from nextCupsAvailable (and remove "cup 0" bit if set)
nextCupsAvailable = nextCupsAvailable & ~(1<<moveCup1) & ~(1<<moveCup2) & ~(1<<moveCup3) & ~1
        ; identify the cup to reconnect
nextCup = %1'0000'00001 << (firstCup-1) ; the "-1" cup bit (bits, two of them, to wrap from 1 -> 9)
        WHILE 3 == __PASS__ && 0 == (nextCup & nextCupsAvailable & %111'111'111'0)
nextCup = nextCup >> 1                  ; do "-1" until it is found in nextCupsAvailable (in the low 1..9 range)
        ENDW
nextCupsAvailable = nextCup & %111'111'111'0    ; keep only bit masks of cups 1..9 (in the lower range)
nextCup = -1
        WHILE 3 == __PASS__ && nextCupsAvailable
nextCup = nextCup + 1
nextCupsAvailable = nextCupsAvailable >> 1      ; count the bits from right (to figure out cup number)
        ENDW
        DISPLAY "Next move from cup: ",/D,newFirstCup," | Triplet [",/D,moveCup1,",",/D,moveCup2,",",/D,moveCup3,"] connects to cup: ",/D,nextCup
        IF 3 == __PASS__
            ; relink the +3 cup onto the starting cup
            ORG firstCup : DB newFirstCup
            ; relink the three cups onto nextCup
            ORG moveCup3 : DB {b nextCup}           ; end of triplets connects to where nextCup was leading to
            ORG nextCup : DB moveCup1               ; nextCup leads to the triplet now
        ENDIF
firstCup = newFirstCup
    EDUP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; display final result (starting at cup 1)

    ; display final chain from cup 1
    DISPLAY "Final from cup 1:"
firstCup = 1
    DUP 8
        DISPLAY /D,{b firstCup}
firstCup = {b firstCup}
    EDUP

; https://adventofcode.com/2020/day/23 (part 2)

; !!! WARNING !!! This takes about ~25 minutes on my machine

    OPT listoff : DEVICE ZXSPECTRUM4096 ; needs 4MiB of memory for 1 million of DWORDs

;     DEFINE INIT_TEST_INPUT  ; uncomment to run over test input data
        ; test input: 389125467, expected cups: 934001, 159792, dot = 149245887792
        ; main input: 318946572, expected cups: 54006, 214632, dot = 11591415792

LAST_CUP            EQU 1'000'000
; TOTAL_MOVES_TO_DO   EQU 100
TOTAL_MOVES_TO_DO   EQU 10'000'000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; init/reserve variable symbols (have to be defined in first pass)
nextCup = 0
nextCupFinal = 0
firstCup = 0
moves = 0
moveCup1 = 0
moveCup2 = 0
moveCup3 = 0
newFirstCup = 0

    MACRO PEEK_CUP cup?, result?
        MMU 0, (cup?)>>12, ((cup?)<<2)&$3FFC
result? = {$} | ({$+2}<<16)
    ENDM

    MACRO LINK_CUPS from?, to?
        MMU 0, (from?)>>12, ((from?)<<2)&$3FFC
        DD to?
    ENDM

    IF 3 == __PASS__    ; do everything else in the last third pass

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; prepare inputs already as finished linked list structure (no parsing code from string)
    IFDEF INIT_TEST_INPUT
        ; test input: 389125467
firstCup = 3
        MMU 0 n, 0, $0000
        DD 0    ; 0 -> 0 (not used)
        DD 2    ; 1 -> 2
        DD 5    ; 2 -> 5
        DD 8    ; 3 -> 8
        DD 6    ; 4 -> 6
        DD 4    ; 5 -> 4
        DD 7    ; 6 -> 7
        DD 10   ; 7 -> 10
        DD 9    ; 8 -> 9
        DD 1    ; 9 -> 1
        ; here onward from 10 the cups link the next cup (10 -> 11, 11 -> 12, ...)
nextCup = 11
        DUP LAST_CUP - 10
            DD nextCup
nextCup = nextCup + 1
        EDUP
        ASSERT LAST_CUP + 1 == nextCup
        DD firstCup ; LAST_CUP -> 3 (beginning of test input)
    ELSE
        ; main input: 318946572
firstCup = 3
        MMU 0 n, 0, $0000
        DD 0    ; 0 -> 0 (not used)
        DD 8    ; 1 -> 8
        DD 10   ; 2 -> 10
        DD 1    ; 3 -> 1
        DD 6    ; 4 -> 6
        DD 7    ; 5 -> 7
        DD 5    ; 6 -> 5
        DD 2    ; 7 -> 2
        DD 9    ; 8 -> 9
        DD 4    ; 9 -> 4
        ; here onward from 10 the cups link the next cup (10 -> 11, 11 -> 12, ...)
nextCup = 11
        DUP LAST_CUP - 10
            DD nextCup
nextCup = nextCup + 1
        EDUP
        ASSERT LAST_CUP + 1 == nextCup
        DD firstCup ; LAST_CUP -> 3 (beginning of real input)
    ENDIF
    DISPLAY "First cup: ",/D,firstCup

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; simulate N moves

    DUP TOTAL_MOVES_TO_DO
moves = moves + 1   ; this move starts at "firstCup"
        ; fetch the three cups which will be moved away, and the following new "firstCup" for next move
        PEEK_CUP firstCup, moveCup1
        PEEK_CUP moveCup1, moveCup2
        PEEK_CUP moveCup2, moveCup3
        PEEK_CUP moveCup3, newFirstCup
        ; find possible nextCupFinal, where the triplet will be connected
nextCupFinal = 0
nextCup = firstCup - 1
        WHILE !nextCupFinal
            IF 0 == nextCup
nextCup = LAST_CUP
            ENDIF
            IF nextCup != moveCup1 && nextCup != moveCup2 && nextCup != moveCup3
nextCupFinal = nextCup
            ELSE
nextCup = nextCup - 1
            ENDIF
        ENDW
        ; execute the move
;         DISPLAY "Move ",/D,moves,", from cup: ",/D,firstCup,", next from: ",/D,newFirstCup," | Triplet [",/D,moveCup1,",",/D,moveCup2,",",/D,moveCup3,"] connects to cup: ",/D,nextCupFinal
        ; relink the +3 cup onto the starting cup
        LINK_CUPS firstCup, newFirstCup
        ; end of triplets connects to where nextCup was leading to
        PEEK_CUP nextCupFinal, nextCup
        LINK_CUPS moveCup3, nextCup
        ; nextCupFinal links onto the triplet
        LINK_CUPS nextCupFinal, moveCup1
firstCup = newFirstCup
    EDUP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; display two cups from cup 1

    PEEK_CUP 1, moveCup1
    PEEK_CUP moveCup1, moveCup2
    DISPLAY "Final from cup 1: ",/D,moveCup1,",",/D,moveCup2," | =",/D,moveCup1,"*",/D,moveCup2

    ENDIF   ; end of `IF 3 == __PASS__`

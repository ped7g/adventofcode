; https://adventofcode.com/2020/day/25 (part 1)

; WARNING - this would run searching for secret key probably about 25 days
; on my notebook, just because of 64bit math, so I gave up, and wrote it in C++
; (also I avoided 64b math by very lame approach, if I would instead write 64b mul+mod,
; it would be running within few minutes probably, but the C++ solution needs 0.2s)
; (and there was enough of 64b math done in previous tasks - nothing new to explore)

    OPT listoff

    DEFINE TEST_INPUT       ;; comment to work with real input
;     DEFINE SEARCH_SECRETS   ;; comment to search for the encryption key

; expected secret key - test input: 14897079
; expected secret key - real input: ?

    IFDEF TEST_INPUT
; test input
CARD_PK     EQU     5764801
DOOR_PK     EQU     17807724
    ELSE
; real input
CARD_PK     EQU     2069194
DOOR_PK     EQU     16426071
    ENDIF

KEY_MOD     EQU     20201227

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SEARCH_SECRETS mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    IFDEF SEARCH_SECRETS

card_sec    = -1
door_sec    = -1
loop        = 0
value       = 1

    IF 3 == __PASS__

        ; loop long enough to find both secrets:
        ; (can't use while as that has guardian limit of 100k loops only)
        DUP KEY_MOD
value = (value * 7) % KEY_MOD
loop = loop + 1
            IF CARD_PK == value
card_sec = loop :   DISPLAY "card secret loop: ",/D,card_sec
            ENDIF
            IF DOOR_PK == value
door_sec = loop :   DISPLAY "door secret loop: ",/D,door_sec
            ENDIF
        EDUP

; the output for real input is:
; > door secret loop: 8229037
; > card secret loop: 13207740
; and the total runtime on my machine was: 318 seconds (doing KEY_MOD many loops)

    ENDIF   ;; end of "IF 3 == __PASS__"

    ENDIF   ;; end of "IFDEF SEARCH_SECRETS"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; encryption key search mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    IFNDEF SEARCH_SECRETS

        IFDEF TEST_INPUT
; test input secret loop numbers
CARD_SK     EQU     8
DOOR_SK     EQU     11
        ELSE
; real input secret loop numbers (obtained by running this asm for ~6 minutes in SEARCH_SECRETS mode)
CARD_SK     EQU     13207740
DOOR_SK     EQU     8229037
        ENDIF

value = 1
oldval100 = 0
oldvalRem = 0

    IF 3 == __PASS__

        ; now the straightfoward value = (value * CARD_PK) % KEY_MOD would overflow
        ; so instead of writing yet another 64bit custom math, I will do it in step per 100
        ; (100 * value) does fit into 32bit (100 * (KEY_MOD-1) at worst)

        DUP DOOR_SK
oldval100 = 100 * value
oldvalRem = (CARD_PK % 100) * value
value = 0   ; restart value from zero to sum (value * CARD_PK) in the end
            DUP CARD_PK / 100
value = (value + oldval100) % KEY_MOD
            EDUP
value = (value + oldvalRem) % KEY_MOD
        EDUP
    
        DISPLAY "Secret key: ",/D,value

    ENDIF   ;; end of "IF 3 == __PASS__"

    ENDIF   ;; end of "IFNDEF SEARCH_SECRETS"


; https://adventofcode.com/2020/day/13 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
    INCBIN "input.txt"  ; all busIDs provided are prime numbers!
inputEnd:
    DS  5,0
    ALIGN 256,0         ; some padding after input (just to be sure + nice addresses)

value = 0
    MACRO PARSE_VALUE
value = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
value = value * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

; Algorithm description (probably too sparse, I took calc spreadsheet and tinkered with numbers for a while)
; * all busID in input files are prime numbers
;   -> this means the Least Common Multiple is equal to dot product of all IDs (LCM calculation is thus trivial)
; * before rading next busID, I have current minimal "t" (timestamp) and current LCM
;   (ie. by what to advance "t" to preserve it's current features for previous lines) 
; * for a start of algorithm, the first bus is parsed and its busID becomes initial LCM, t=0, busIndex = 0
; * for following buses:
;      ++busIndex
;      expectedMod = adjusted(busID - busIndex) ; adjusted to 0..busID-1 (ie. 55 for busID 59 at t+4 minute)
;      if ('x' != busID) {
;          while (expectedMod != (t mod busID)) t += LCM
;          LCM *= busID
;      }
; because the "t" is incremented by previous busIDs LCM, all the previous "expectedMod == (t mod busID)" stay true
; so the "t" reached by the while is the minimum new "t" incorporating also new busID

; because sjasmplus internal expression evaluator is strictly 32bit,
; the script must implement "big math" to calculate with at least 64 bit values
; (using virtual-device-memory 16bit words and loops)

BC_WORDS    EQU     4       ; how many words has single value (4 -> 4*16 = 64 bit math)
temp = 0
aPtr = 0
bPtr = 0
rPtr = 0

    MACRO bcadd a?, b?, r?  ; r? = a? + b? (pointers to BC_WORDS words of memory)
temp = 0
aPtr = a?
bPtr = b?
        ORG r?
        DUP BC_WORDS
temp = temp + { aPtr } + { bPtr }
            DW temp & $FFFF
temp = temp >>> 16
aPtr = aPtr + 2
bPtr = bPtr + 2
        EDUP
        ASSERT 0 == temp    ; if temp is still non-zero, the addition did overflow!
    ENDM

    MACRO bcmul a?, b?, r?  ; r? = a? * b? (pointers to BC_WORDS words of memory)
        ORG r? : DUP BC_WORDS: DW 0 : EDUP    ; zero the result first
aPtr = a?
rPtr = r?
        DUP BC_WORDS
bPtr = b?
temp = 0
            ORG rPtr
            DUP BC_WORDS - (rPtr - (r?))/2
temp = temp + { $ } + ({ aPtr } * { bPtr })
                DW temp & $FFFF
temp = temp >>> 16
bPtr = bPtr + 2
            EDUP
            ASSERT 0 == temp    ; if temp is still non-zero, the multiplication did overflow!
aPtr = aPtr + 2
rPtr = (r?) + (aPtr - (a?))
        EDUP
    ENDM

    MACRO bcmod a?, uint16      ; temp = mod(a?, uint16) (a? is pointer to BC_WORDS words of memory, uint16 is immediate)
aPtr = (a?) + (BC_WORDS * 2)
temp = 0
        DUP BC_WORDS
aPtr = aPtr - 2
temp = ((temp<<16) | { aPtr }) % uint16
        EDUP
    ENDM

    MACRO bcassign r?, a?       ; r? = a? (pointers to BC_WORDS words of memory)
aPtr = a?   : ORG r?
        DUP BC_WORDS
            DW { aPtr }
aPtr = aPtr + 2
        EDUP
    ENDM

    ; init variables
inPtr = inputFile + (3 - __PASS__) * inputEnd   ; make it point beyond input in early passes
t:      DD  0, 0
lcm:    DD  0, 0
b64:    DD  0, 0    ; temporary for "b" value
r64:    DD  0, 0    ; temporary for results

    ; parse earliest timestamp to depart -> ignored for Part 2
    IF 3 == __PASS__
        PARSE_VALUE : ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1
    ENDIF

    ; first busId parse -> t = 0, lcm = busId, busIndex = 0
busIndex = 0
expectedMod = 0
    IF inPtr < inputEnd && {b inPtr}
        PARSE_VALUE     ; value = busID
inPtr = inPtr + 1       ; skip "," or "\n" after first busID
        ORG lcm : DD value  ; LCM = busID (no need to use bigmath yet, busID are 16bit only)
        DISPLAY "Bus id: ",/D,value," (expectedMod = ",/D,expectedMod,")"
        DISPLAY "- earliest timestamp: ",{t + 6},":",{t + 4},":",{t + 2},":",{t + 0}
        DISPLAY "- lcm: ",{lcm + 6},":",{lcm + 4},":",{lcm + 2},":",{lcm + 0}
    ENDIF

    ; * for following buses:
    ;      ++busIndex
    ;      expectedMod = adjusted(busID - busIndex) ; adjusted to 0..busID-1 (ie. 55 for busID 59 at t+4 minute)
    ;      if ('x' != busID) {
    ;          while (expectedMod != (t mod busID)) t += LCM
    ;          LCM *= busID
    ;      }
    WHILE inPtr < inputEnd && {b inPtr}
        ; skip separator characters
        WHILE (',' == {b inPtr}) || ("\n" == {b inPtr})
inPtr = inPtr + 1
        ENDW
busIndex = busIndex + 1
        IF '0' <= {b inPtr} && {b inPtr} <= '9'
            PARSE_VALUE     ; value = bus ID
expectedMod = value - busIndex
            WHILE expectedMod < 0
expectedMod = expectedMod + value
            ENDW
            DISPLAY "Bus id: ",/D,value," (expectedMod = ",/D,expectedMod,")"
            bcmod t, value  ; first temp=(t mod busID)
            WHILE expectedMod != temp
                bcadd t, lcm, t     ; bcadd can overwrite the same memory for a? vs r?
                bcmod t, value      ; refresh temp=(t mod busID) for WHILE condition
            ENDW
            ORG b64 : DD value, 0   ; store busID into memory for bcmul
            bcmul lcm, b64, r64     ; r64 = lcm * busID
            bcassign lcm, r64       ; lcm = r64
            DISPLAY "- earliest timestamp: ",{t + 6},":",{t + 4},":",{t + 2},":",{t + 0}
            DISPLAY "- lcm: ",{lcm + 6},":",{lcm + 4},":",{lcm + 2},":",{lcm + 0}
        ELSE
inPtr = inPtr + 1   ; "x" or some unexpected char in input (or "\0")
        ENDIF
    ENDW
    DISPLAY "Earliest timestamp: ",{t + 6},":",{t + 4},":",{t + 2},":",{t + 0}

; for my input.txt, the final answer is:
; > Earliest timestamp: 0x0001:0xF8A5:0xADE5:0x7D2B
; which means in decimal: 554865447501099

; https://adventofcode.com/2020/day/14 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
    INCBIN "input.txt"
        ; by some luck? all values fit into int32_t => simple parsing possible
        ; by another miracle all addresses are uint16_t => total memory fits into ZX NEXT device
inputEnd:
    DS  5,0
    ALIGN 256,0         ; some padding after input (just to be sure + nice addresses)

; Big Calculations support
BC_WORDS    EQU     4   ; how many words has single value (4 -> 4*16 = 64 bit math)
temp = 0
aPtr = 0
bPtr = 0
rPtr = 0

    MACRO bcadd a?, b?, r?  ; r? = a? + b? (pointers to BC_WORDS words of memory)
temp = 0                    ; r? may be alias of a? or b? (result is still valid)
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

    MACRO bcsub a?, b?, r?  ; r? = a? - b? (pointers to BC_WORDS words of memory)
temp = 0                    ; r? may be alias of a? or b? (result is still valid)
aPtr = a?
bPtr = b?
        ORG r?
        DUP BC_WORDS
temp = temp + { aPtr } - { bPtr }
            DW temp & $FFFF
temp = temp >> 16
aPtr = aPtr + 2
bPtr = bPtr + 2
        EDUP
        ASSERT 0 == temp    ; if temp is still non-zero, the subtraction did overflow!
    ENDM

sum:    DS      BC_WORDS*2, 0       ; 64bit total sum of values in memory

maskO:  HEX     AA AA AA AA AA      ; 40 bits to make it round 5 bytes
maskA:  HEX     AA AA AA AA AA

    ASSERT $ < $4000    ; make sure the input + sum/mask variables fit into bottom 16ki

; Parsed values "mem[adr] = val" will be stored in device memory at "adr" like this:
; 8 bytes reserved per value to keep mapping simple = 512kiB of total memory, from page 2
mappedAdr = 0
    MACRO MAP_MEM_ADR adr?
        MMU 2, 2 + ((adr?) >> 10)   ; pageNum = 2 + (adr? / 1024)
mappedAdr = $4000 + (((adr?) << 3) & $1FFF) ; valueAdr = $4000 + ((adr? * 8) & 8191)
    ENDM

value = 0
    MACRO PARSE_VALUE
value = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
value = value * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

    ; init variables
inPtr = inputFile + (3 - __PASS__) * inputEnd   ; make it point beyond input in early passes
outIdx = 0
memA = 0
memV = 0

    WHILE inPtr < inputEnd
        IF 'am' == {inPtr} && 'ks' == {inPtr+2} && '= ' == {inPtr+4} && ' ' == {b inPtr+6}
inPtr = inPtr + 7
outIdx = 4              ; point to the MSB of masks
memV = $10              ; bit marker to know when full byte was read + OR mask (fake 4 bits)
memA = $0F              ; AND mask (fake 4 bits)
            DUP 36
memV = (memV << 1) + (1 & ('1' == {b inPtr}))
memA = (memA << 1) + (1 & ('0' != {b inPtr}))
inPtr = inPtr + 1
                IF 256 <= memV  ; enough bits collected, write masks byte into memory
                    ORG maskO+outIdx : DB memV&$FF : ORG maskA+outIdx : DB memA
outIdx = outIdx - 1
memV = $01
memA = $00
                ENDIF
            EDUP
            DISPLAY "mask OR:  ",{b maskO+4},":",{maskO+2},":",{maskO+0}
            DISPLAY "mask AND: ",{b maskA+4},":",{maskA+2},":",{maskA+0}
        ELSEIF 'em' == {inPtr} && '[m' == {inPtr+2}
inPtr = inPtr + 4 : PARSE_VALUE : ASSERT 0 <= value && value <= 65535
memA = value
inPtr = inPtr + 4 : PARSE_VALUE : ASSERT 0 <= value && value <= $7FFFFFFF
memV = value
            DISPLAY "Mem[",memA,"] = ",/A,memV
            MAP_MEM_ADR memA
            IF {mappedAdr} || {mappedAdr+2} || {mappedAdr+4}    ; subtract old value
                DISPLAY "overwrite with new value of old: ",{mappedAdr+6},":",{mappedAdr+4},":",{mappedAdr+2},":",{mappedAdr+0}
                bcsub sum, mappedAdr, sum       ; sum - mappedAdr = sum
            ENDIF
            ORG mappedAdr   ; write memV into memory, including changed induced by current mask
            DW (memV&$FFFF) & {maskA+0} | {maskO+0}
            DW (memV>>>16) & {maskA+2} | {maskO+2}
            DB 0 & {b maskA+4} | {b maskO+4}    ; bits 39:32 are just from mask (no value is 32b+ in input)
            DISPLAY "Stored as: ",{mappedAdr+6},":",{mappedAdr+4},":",{mappedAdr+2},":",{mappedAdr+0}
            bcadd sum, mappedAdr, sum           ; sum + mappedAdr = sum
        ELSE
            ASSERT "\n" == {b inPtr} || inputEnd <= inPtr   ; empty line or beyond data
        ENDIF
        ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1
    ENDW

    DISPLAY "Total sum: ",{sum+6},":",{sum+4},":",{sum+2},":",{sum+0}
; for my input.txt: Total sum: 0x00000E026D6D13FA = 15403588588538

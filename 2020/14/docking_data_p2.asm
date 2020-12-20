; https://adventofcode.com/2020/day/14 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t3.txt"
    INCBIN "input.txt"
        ; by some luck? all addresses and values fit into int32_t => simple parsing possible
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

memcpyTempOrg = 0
memcpyTempSrc = 0
    MACRO memcpy src?, dst?, size?
memcpyTempOrg = $
memcpyTempSrc = src? + ((size?) & 1)
        ORG dst? : IF 1&(size?) : DB {b src? } : ENDIF
        DUP (size?)/2
            DW { memcpyTempSrc }
memcpyTempSrc = memcpyTempSrc + 2
        EDUP
        ORG memcpyTempOrg
    ENDM

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

    MACRO bcmul a?, uint16? ; a? = a? * uint16? (a? is pointer to BC_WORDS words of memory, uint16? is immediate)
bPtr=$: ORG a?  : ASSERT 0 <= (uint16?) && (uint16?) <= 65535
temp=0: DUP BC_WORDS
temp = temp + ({$} * (uint16?)) : DW temp & $FFFF
temp = temp >>> 16
        EDUP
        ORG bPtr : ASSERT 0 == temp ; if temp is still non-zero, the multiplication did overflow!
    ENDM

sum:    DS      BC_WORDS*2, 0       ; 64bit total sum of values in memory

maskO:  HEX     AA AA AA AA AA      ; 40 bits to make it round 5 bytes
maskA:  HEX     AA AA AA AA AA
maskF:  HEX     AA AA AA AA AA

wrkR:   DS      S_WRITE * 1024      ; working buffer to split one write rule until no-collision
wrkEnd:

    ASSERT $ < $8000    ; make sure the input + sum/mask variables fit into bottom 16ki

    STRUCT S_WRITE
floatA  DS  5           ; 40 bit float mask
adr     DS  5           ; 40 bit address (fixed bits) (under float "0")
value   DD              ; 32bit value written
        DD  0           ; extend value to 64bit by zeroed top 32bit DWORD (for bcadd)
    ENDS

    ALIGN 256
writes:                 ; empty list of S_WRITE instances

    ; init variables
inPtr = inputFile + (3 - __PASS__) * inputEnd   ; make it point beyond input in early passes
outIdx = 0
memA = 0
mmO = 0
mmA = 0
mmF = 0

    WHILE inPtr < inputEnd
        IF 'am' == {inPtr} && 'ks' == {inPtr+2} && '= ' == {inPtr+4} && ' ' == {b inPtr+6}
writesEnd = $
inPtr = inPtr + 7
outIdx = 4          ; point to the MSB of masks
mmO = $10           ; bit marker to know when full byte was read + OR mask (fake 4 bits)
mmA = $0F           ; AND mask (fake 4 bits)
mmF = $00           ; FLOAT mask (fake 4 bits)
            DUP 36
mmO = (mmO << 1) + (1 & ('1' == {b inPtr}))
mmA = (mmA << 1) + (1 & ('0' == {b inPtr}))
mmF = (mmF << 1) + (1 & ('X' == {b inPtr}))
inPtr = inPtr + 1
                IF 256 <= mmO  ; enough bits collected, write masks byte into memory
                    ORG maskO+outIdx : DB mmO&$FF
                    ORG maskA+outIdx : DB mmA
                    ORG maskF+outIdx : DB mmF
outIdx = outIdx - 1
mmO = $01
mmA = $00
mmF = $00
                ENDIF
            EDUP
;             DISPLAY "mask OR:  ",{b maskO+4},":",{maskO+2},":",{maskO+0}
;             DISPLAY "mask AND: ",{b maskA+4},":",{maskA+2},":",{maskA+0}
;             DISPLAY "mask FLT: ",{b maskF+4},":",{maskF+2},":",{maskF+0}
            ORG writesEnd   ; restore write pointer for rules recording
        ELSEIF 'em' == {inPtr} && '[m' == {inPtr+2}
mmA = $     ; temporary helper where the rule was written
inPtr = inPtr + 4 : PARSE_VALUE : ASSERT 0 <= value && value <= 65535
memA = value
inPtr = inPtr + 4 : PARSE_VALUE : ASSERT 0 <= value && value <= $7FFFFFFF
            DW {maskF+0}, {maskF+2} : DB {b maskF+4}
            DW memA & {maskA+0} | {maskO+0}, {maskO+2} : DB {b maskO+4} ; no address is more than 16b, so just OR mask in upper words
            DD value, 0
;             DISPLAY "Mem[",memA,"] = ",/D,value,"\t\t| Stored as: F[",{b mmA+4},":",{mmA+2},":",{mmA+0},"] A[",{b mmA+9},":",{mmA+7},":",{mmA+5},"] V: ",/A,{mmA+10} | ({mmA+12}<<16)
        ELSE
            ASSERT "\n" == {b inPtr} || inputEnd <= inPtr   ; empty line or beyond data
        ENDIF
        ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1
    ENDW

writesEnd = $
    ASSERT $ < $FF00    ; make sure everything did fit into 64kiB memory window

    DISPLAY "---------------------------------------------------------------"
    DISPLAY "Total writes recorded: ",/A,(writesEnd - writes) / S_WRITE
    DISPLAY "Processing write records (calculating total sum):"
    DISPLAY "---------------------------------------------------------------"
wPtr1 = 0
wPtr2 = 0
checkPtr = 0
checkBit = 0
checkBitIdx = 0
checkBitMsk = 0
totalCombs = 0

    ; process all write records backward i = N..1, and add to total-sum all which have
    ; no-collision against records from i+1..N list (already processed writes)
inPtr = writesEnd - S_WRITE     ; last recorded write
    WHILE writes <= inPtr
        ; copy original write record to working buffer
        ; (where it will be dismantled into non-colliding sub-writes)
        memcpy inPtr, wrkR, S_WRITE

        ; prune writes in working buffer until there's no rule left or no-collision
wPtr1 = wrkR
wPtr2 = wPtr1 + S_WRITE
        WHILE wPtr1 < wPtr2
checkPtr = writesEnd - S_WRITE
;             DISPLAY "Write: F[",{b wPtr1+4},":",{wPtr1+2},":",{wPtr1+0},"] A[",{b wPtr1+9},":",{wPtr1+7},":",{wPtr1+5},"] V: ",/A,{wPtr1+10} | ({wPtr1+12}<<16)
            WHILE inPtr < checkPtr
;                 DISPLAY "Checking againts: F[",{b checkPtr+4},":",{checkPtr+2},":",{checkPtr+0},"] A[",{b checkPtr+9},":",{checkPtr+7},":",{checkPtr+5},"] V: ",/A,{checkPtr+10} | ({checkPtr+12}<<16)
                ; wPtr1:    XXX 000 111
                ; checkPtr: X01 X01 X01
                ; resolve:  -10 --+ -+- ; "+" -> whole write is no-collision, "-" collision w/o variant
                ; "0" or "1" - non-collision variant to create (one for each bit)
value = ((~{b wPtr1+4}) & (~{b checkPtr+4}) & ({b wPtr1+S_WRITE.adr+4} ^ {b checkPtr+S_WRITE.adr+4}))
value = value | ((~{wPtr1+2}) & (~{checkPtr+2}) & ({wPtr1+S_WRITE.adr+2} ^ {checkPtr+S_WRITE.adr+2}))
value = value | ((~{wPtr1+0}) & (~{checkPtr+0}) & ({wPtr1+S_WRITE.adr+0} ^ {checkPtr+S_WRITE.adr+0}))
                IF !value
;                     DISPLAY "no fixed bits differ -> rule must be reduced!"
checkBit = 0
                    DUP 36
checkBitIdx = checkBit >> 3
checkBitMsk = 1<<(checkBit&7)
                        IF ({b wPtr1+checkBitIdx}&checkBitMsk) && ((~{b checkPtr+checkBitIdx})&checkBitMsk)
                            ; write rule has X, check rule has 0 or 1 fixed -> create reduced variant
value = {b checkPtr+S_WRITE.adr+checkBitIdx} & checkBitMsk  ; check rule fixed bit
;                             DISPLAY "Reducing bit ",/D,checkBit," to ",/D,1 - (value >> (checkBit&7))
                            ORG wPtr1+checkBitIdx : DB {b wPtr1+checkBitIdx}^checkBitMsk    ; clear float bit (in both variants: old+new)
                            memcpy wPtr1, wPtr2, S_WRITE
                            ; set fixed bit to check-rule-bit (collision) value in current "old" rule
                            ORG wPtr1+S_WRITE.adr+checkBitIdx : DB {b wPtr1+S_WRITE.adr+checkBitIdx}^value
                            ; set fixed bit to inverted value in "new" rule (no-collision)
                            ORG wPtr2+S_WRITE.adr+checkBitIdx : DB {b wPtr1+S_WRITE.adr+checkBitIdx}^checkBitMsk
;                             DISPLAY "Resulting write: F[",{b wPtr2+4},":",{wPtr2+2},":",{wPtr2+0},"] A[",{b wPtr2+9},":",{wPtr2+7},":",{wPtr2+5},"] V: ",/A,{wPtr2+10} | ({wPtr2+12}<<16)
wPtr2 = wPtr2 + S_WRITE
                        ENDIF
checkBit = checkBit + 1
                    EDUP
                    ; remove the source write from working buffer (by copying last write over it)
                    IF wPtr1 < wPtr2-S_WRITE
                        memcpy wPtr2-S_WRITE, wPtr1, S_WRITE
;                         DISPLAY "Write: F[",{b wPtr1+4},":",{wPtr1+2},":",{wPtr1+0},"] A[",{b wPtr1+9},":",{wPtr1+7},":",{wPtr1+5},"] V: ",/A,{wPtr1+10} | ({wPtr1+12}<<16)
checkPtr = writesEnd - S_WRITE  ; rerun check again for new wPtr1
                    ELSE
checkPtr = inPtr    ; that was last write in working buffer, end the checking
                    ENDIF
wPtr2 = wPtr2 - S_WRITE : ASSERT wPtr1 <= wPtr2
                ELSE
checkPtr = checkPtr - S_WRITE
                ENDIF
            ENDW
wPtr1 = wPtr1 + S_WRITE
        ENDW

        ; calculate amount of writes left in working buffer, and add it to total sum
value = 0
        IF wrkR < wPtr2
wPtr1 = wrkR
            DUP (wPtr2 - wPtr1) / S_WRITE
;                 DISPLAY "Calculating combinations for: F[",{b wPtr1+4},":",{wPtr1+2},":",{wPtr1+0},"] A[",{b wPtr1+9},":",{wPtr1+7},":",{wPtr1+5},"] V: ",/A,{wPtr1+10} | ({wPtr1+12}<<16)
temp = 1        ; 1 for fixed address, now multiply by two for every remaining FLOAT bit
checkBit = 0
                    DUP 36
checkBitIdx = checkBit >> 3
checkBitMsk = 1<<(checkBit&7)
                        IF ({b wPtr1+checkBitIdx}&checkBitMsk)
temp = temp << 1            ; temp *= 2
                        ENDIF
checkBit = checkBit + 1
                    EDUP
value = value + temp
wPtr1 = wPtr1 + S_WRITE
            EDUP
        ENDIF
totalCombs = totalCombs + value
        bcmul inPtr + S_WRITE.value, value
        bcadd sum, inPtr + S_WRITE.value, sum   ; sum + write_sum = sum
;         DISPLAY "combinations ",/D,value," | Total sum: ",{sum+6},":",{sum+4},":",{sum+2},":",{sum+0}
inPtr = inPtr - S_WRITE         ; previous recorded write
    ENDW
    DISPLAY "Total sum: ",{sum+6},":",{sum+4},":",{sum+2},":",{sum+0}," | Total combinations: ",/D,totalCombs

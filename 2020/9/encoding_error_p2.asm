; https://adventofcode.com/2020/day/9 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
; NN          EQU 5       ; preamble size
;     INCBIN "input_t.txt"
NN          EQU 25      ; preamble size
    INCBIN "input.txt"
    ; this will need big-number math, the max number is 108550179178800 -> 47 bit number
inputEnd:
    ds      5,0   ; some zeroed mem in case the algorithm reads way beyond input
    ALIGN 256,0

v:      ; start of parsed-value array, each value is 4B long (64 bit)

vl = 0
vh = 0
tl8 = 0
th8 = 0
tl2 = 0
th2 = 0
    MACRO PARSE_VALUE
        ASSERT '0' <= {b inPtr} && {b inPtr} <= '9'
vl = 0
vh = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
            ; vh:vl = vh:vl * 10 + digit
tl8 = vl<<3
th8 = (vh<<3) | (vl>>>29)
tl2 = vl<<1
th2 = (vh<<1) | (vl>>>31)
vl = tl8 + tl2 + {b inPtr} - '0'    ; add also digit, that will never affect overflow of x10
            IF ((tl8&tl2) < 0) || ((tl8^tl2) < 0 && 0 <= vl)    ; check overflow (only x10 part is enough)
vh = th8 + th2 + 1
            ELSE
vh = th8 + th2
            ENDIF
inPtr = inPtr + 1
        ENDW
        ;DISPLAY "Parse value: ",vh,":",vl
        DD vl, vh
    ENDM

    ; init variables
line = 0
inPtr = inputFile
v1Ptr = 0
v2Ptr = 0
vPtr = 0
noSum = 0
temp = 0

    ; parse big number into memory (little-endian form) - preamble
    WHILE inPtr < inputEnd && {b inPtr} && line < NN
        PARSE_VALUE : ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1   ; skip newline char
line = line + 1
    ENDW
    ; parse big number into memory (little-endian form) - after preamble
    WHILE inPtr < inputEnd && {b inPtr}
noSum = 1
vPtr = $
        PARSE_VALUE : ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1   ; skip newline char
line = line + 1
v1Ptr = vPtr - NN * 8
        WHILE noSum && v1Ptr < vPtr-8
v2Ptr = v1Ptr + 8
            WHILE noSum && v2Ptr < vPtr
                ; check *v1Ptr + *v2Ptr == vh:vl
                ;DISPLAY "v1: ",v1Ptr," v2: ",v2Ptr," (in): ",vPtr
temp = {v1Ptr} + {v2Ptr}
                IF (temp&$FFFF) == (vl&$FFFF)
temp = (temp>>>16) + {v1Ptr+2} + {v2Ptr+2}
                    IF (temp&$FFFF) == (vl>>>16)
temp = (temp>>>16) + {v1Ptr+4} + {v2Ptr+4}
                        IF (temp&$FFFF) == (vh&$FFFF)
temp = (temp>>>16) + {v1Ptr+6} + {v2Ptr+6}
                            IF (temp&$FFFF) == (vh>>>16)
noSum = 0
                            ENDIF
                        ENDIF
                    ENDIF
                ENDIF
v2Ptr = v2Ptr + 8
            ENDW
v1Ptr = v1Ptr + 8
        ENDW
        IF noSum
            DISPLAY "no sum for vh:vl: ",vh,":",vl," line: ",/D,line
inPtr = inputEnd
        ENDIF
    ENDW
    ; vh:vl (*vPtr) contains the value which has no pair-sum, stop parsing file, find sequence-sum
inPtr = v
v1Ptr = v
v2Ptr = v
seqL = 0
seqH = 0
oldSeqL = 0
    WHILE v1Ptr < vPtr && v2Ptr < vPtr
        ; check current sequence sum against target value - cheating a bit, I know vh==0 (!)
        IF 0 == seqH
temp = (seqL>>>16) - (vl>>>16)
            IF seqL == vl
                DISPLAY "=="
                ; sum found
inPtr = v1Ptr   ; remember start of sequence
v1Ptr = vPtr    ; exit loop early
            ELSEIF temp < 0 || (0 == temp && (seqL&$FFFF) < (vl&$FFFF))
                ; sequence sum is too small, add next number
                ASSERT 0 == {v2Ptr+6}   ; will be ignored
temp = {v2Ptr} | ({v2Ptr+2}<<16)
oldSeqL = seqL
seqL = seqL + temp
                IF ((temp&oldSeqL) < 0) || ((temp^oldSeqL) < 0 && 0 <= seqL)  ; check overflow
seqH = {v2Ptr+4} + 1
                ELSE
seqH = {v2Ptr+4}
                ENDIF
v2Ptr = v2Ptr + 8
                DISPLAY "+ -> ",seqH,":",seqL
            ELSE
                ; sequence sum is too large, subtract first number
                ASSERT 0 == {v1Ptr+6}   ; will be ignored
seqH = -{v1Ptr+4}
oldSeqL = seqL
seqL = {v1Ptr} | ({v1Ptr+2}<<16)
temp = (oldSeqL>>>1) - (seqL>>>1)
                IF (temp < 0) || (0 == temp && -1 == ((oldSeqL&1)-(seqL&1)))  ; check overflow
seqH = seqH - 1
                ENDIF
seqL = oldSeqL - seqL
v1Ptr = v1Ptr + 8
                DISPLAY "- -> ",seqH,":",seqL
            ENDIF
        ELSE
            ; sequence sum is too large (0 < seqH), subtract first number
            ASSERT 0 == {v1Ptr+6}   ; will be ignored
seqH = seqH - {v1Ptr+4}
oldSeqL = seqL
seqL = {v1Ptr} | ({v1Ptr+2}<<16)
temp = (oldSeqL>>>1) - (seqL>>>1)
                IF (temp < 0) || (0 == temp && -1 == ((oldSeqL&1)-(seqL&1)))  ; check overflow
seqH = seqH - 1
                ENDIF
seqL = oldSeqL - seqL
v1Ptr = v1Ptr + 8
            DISPLAY "- -> ",seqH,":",seqL
        ENDIF
    ENDW
    DISPLAY "Sequence found from ",/D,1+(inPtr-v)/8," to ",/D,(v2Ptr-v)/8
    ; search for min/max member of the sequence
    ; (cheating on the big math, I know ahead that the int32_t fits the results)
minV = (1<<31)-1
maxV = 0
    WHILE inPtr < v2Ptr
temp = {inPtr} | ({inPtr+2}<<16)
minV = minV <? temp
maxV = maxV >? temp
inPtr = inPtr + 8
    ENDW
    DISPLAY "MinV: ",/D,minV," MaxV: ",/D,maxV," sum = ",/D,minV+maxV

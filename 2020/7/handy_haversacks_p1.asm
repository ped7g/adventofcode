; https://adventofcode.com/2020/day/7 (part 1)
; regex covering all input rules (to have some plan for writing the script):
; "\w+ \w+ bags contain (no other bags\.|(\d \w+ \w+ bags?, )*\d \w+ \w+ bags?\.)"
    DEFINE SWITCH_TO_INPUT_MEM MMU 0 7, 0, $0000
    DEFINE SWITCH_TO_PACKIN_MEM MMU 0 7, 8, $0000

    OPT listoff : DEVICE ZXSPECTRUMNEXT
    SWITCH_TO_INPUT_MEM
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input.txt"
inputEnd = $        ; DEFL type of symbol, so it can be modified later
    ds      256,0   ; some zeroed mem in case the algorithm reads way beyond input

bagToCheckTxt:
    db      "shiny gold "
bagToCheckTxtEnd:
    ds      10, 0

    ; hash tabs to create 0..N-1 indexing for first/second word of the bag name
hashTabW1:  ds  256, -1   ; on the provided input.txt there are 18 different first words
hashTabW2:  ds  256, -1   ; on the provided input.txt there are 33 different second words
    ASSERT $ < $10000

    ; use another set of 64kiB for packed-in table
    SWITCH_TO_PACKIN_MEM
BUCKET_SIZE EQU 18
packInTab:  ds  2*(1<<10)*BUCKET_SIZE     ; 2* for WORD sizes, (1<<10) buckets
hasBagTab:  ds  (1<<10), 0
toResearch: ds  4096*2
    ASSERT $ < $10000
    SWITCH_TO_INPUT_MEM

    ; init variables
inPtr = inputFile
hash = 0
nextW1 = 0
nextW2 = 0
W1idx = 0
W2idx = 0
BagIdx = 0
ContainingBagIdx = 0

    MACRO HashWord
        WHILE inPtr < inputEnd && {b inPtr} && ' ' != {b inPtr}  ; until space is reached
hash = (((hash<<1)|(hash>>>7))&$FF) ^ {b inPtr}
inPtr = inPtr + 1
        ENDW
inPtr = inPtr + 1   ; skip the following space
    ENDM

    MACRO GetBagIndex
        ; 18 options for first word, 33 for second word -> 1..594 bag index -> 10 bits for index
hash = 0
        HashWord
        IF $FF == {b hashTabW1 + hash}
            ORG hashTabW1 + hash : DB nextW1
nextW1 = nextW1 + 1
        ENDIF
W1idx = {b hashTabW1 + hash}
hash = 0
        HashWord
        IF $FF == {b hashTabW2 + hash}
            ORG hashTabW2 + hash : DB nextW2
nextW2 = nextW2 + 1
        ENDIF
W2idx = {b hashTabW2 + hash}
BagIdx = (W1idx*33) + W2idx + 1     ; turn W1/W2 indices into 1..594 total index
;         DISPLAY BagIdx,"     = bag: [",/D,W1idx,",",/D,W2idx,"]"
    ENDM

    ; parse whole input file -> produce "contained by" data
    WHILE inPtr < inputEnd && {b inPtr}
        ASSERT 'a' <= {b inPtr} && {b inPtr} <= 'z'
        ; parse line, full regex: "\w+ \w+ bags contain (no other bags\.|(\d \w+ \w+ bags?, )*\d \w+ \w+ bags?\.)"
        GetBagIndex     ; parses "\w+ \w+ " part
ContainingBagIdx = BagIdx   ; remember bag index which contains others
        ASSERT 'ab' == {inPtr+0} && 'sg' == {inPtr+2} && ' ' == {b inPtr+4}
inPtr = inPtr + 5       ; parses "bags " part
        ASSERT 'oc' == {inPtr+0} && 'tn' == {inPtr+2} && 'ia' == {inPtr+4} && ' n' == {inPtr+6}
inPtr = inPtr + 8       ; parses "contain " part
        IF 'on' == {inPtr+0} && 'o ' == {inPtr+2} && 'ht' == {inPtr+4} && 're' == {inPtr+6}
inPtr = inPtr + 14      ; parses "no other bags\." part, ensure it's all
            ASSERT "\n" == {b inPtr}
        ELSE
            WHILE inPtr < inputEnd && '1' <= {b inPtr} && {b inPtr} <= '9'
                ; parse "(\d \w+ \w+ bags?, )*\d \w+ \w+ bags?\." groups
inPtr = inPtr + 2       ; "parses" "\d " part
                GetBagIndex     ; parses "\w+ \w+ " part
                DISPLAY "Bag ",/D,BagIdx," can be contained inside ",/D,ContainingBagIdx
                SWITCH_TO_PACKIN_MEM    ; find free slot
                ORG packInTab + BagIdx*BUCKET_SIZE*2 : WHILE { $ } : ORG $ + 2 : ENDW
                DW ContainingBagIdx     ; write containing bag index into bucket
                ASSERT ($ - (packInTab + BagIdx*BUCKET_SIZE*2)) <= BUCKET_SIZE*2
                SWITCH_TO_INPUT_MEM
                ASSERT 'ab' == {inPtr+0} && 'g' == {b inPtr+2}
inPtr = inPtr + 3       ; parses "bag"
                IF 's' == {b inPtr}
inPtr = inPtr + 1       ; +1 for "s" in "bags"
                ENDIF
                IF ' ,' == {inPtr}
inPtr = inPtr + 2       ; +2 for ", " one bag, if another follows, "\d" is under inPtr
                    ASSERT '1' <= {b inPtr} && {b inPtr} <= '9'
                ELSEIF '.' ==  {b inPtr}
inPtr = inPtr + 1       ; +1 for last "\.", no more follows ("\n" is under inPtr)
                    ASSERT "\n" == {b inPtr}
                ENDIF
            ENDW
        ENDIF
        ASSERT "\n" == {b inPtr}
        ; skip any whitespace after line
        WHILE inPtr < inputEnd && ("\n" == {b inPtr} || " " == {b inPtr} || "\t" == {b inPtr})
inPtr = inPtr + 1
        ENDW
    ENDW
    ; get ID of the bag which should be researched
inPtr = bagToCheckTxt
inputEnd = bagToCheckTxtEnd
    GetBagIndex
    DISPLAY "shiny gold bag is index: ",/D,BagIdx
    SWITCH_TO_PACKIN_MEM
; packInTab:  ds  2*(1<<10)*BUCKET_SIZE     ; 2* for WORD sizes, (1<<10) buckets
; hasBagTab:  ds  (1<<10), 0

researchPtr = 0
toResearchPtr = toResearch
nextToResearchPtr = toResearch

    MACRO RESEARCH_BAG idx?
        IF !{b hasBagTab + idx? }
            ORG hasBagTab + idx? : DB 1     ; mark as already researched
researchPtr = packInTab + idx?*BUCKET_SIZE*2
            DUP BUCKET_SIZE
                IF { researchPtr }
                    DISPLAY "bag ",/D,idx?," can be (directly) in: ",/D,{ researchPtr }
                    ORG nextToResearchPtr : DW { researchPtr }
nextToResearchPtr = nextToResearchPtr + 2
                ENDIF
researchPtr = researchPtr + 2
            EDUP
        ENDIF
    ENDM

    ; BagIdx has bag index of "shiny gold bag", put it into queue
    ORG nextToResearchPtr : DW BagIdx
nextToResearchPtr = nextToResearchPtr + 2

    ; process the queue
    WHILE toResearchPtr < nextToResearchPtr
        RESEARCH_BAG { toResearchPtr }
toResearchPtr = toResearchPtr + 2
    ENDW

    ; count all bags which got tainted by processing
inPtr = hasBagTab
taintedBags = 0
    DUP (1<<10)
        IF {b inPtr}
taintedBags = taintedBags + 1
        ENDIF
inPtr = inPtr + 1
    EDUP
    DISPLAY "Bags able to contain shiny gold bag: ",/D,taintedBags-1

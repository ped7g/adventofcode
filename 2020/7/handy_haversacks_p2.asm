; https://adventofcode.com/2020/day/7 (part 2)
; regex covering all input rules (to have some plan for writing the script):
; "\w+ \w+ bags contain (no other bags\.|(\d \w+ \w+ bags?, )*\d \w+ \w+ bags?\.)"
    DEFINE SWITCH_TO_INPUT_MEM MMU 0 7, 0, $0000
    DEFINE SWITCH_TO_RULES_MEM MMU 0 7, 8, $0000

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

    ; use another set of 64kiB for parsed rules tables
    SWITCH_TO_RULES_MEM

BUCKET_SIZE EQU 4
    STRUCT S_RULE
complete    BYTE    0
bags        DWORD   1
contains    BLOCK   BUCKET_SIZE * 3, 0  ; byte: count, word: bagIndex
    ENDS

rulesTab:
    .(1<<10)    S_RULE
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
numOfBags = 0
numOfBags2 = 0

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

    ; parse whole input file -> produce "rulesTab" data
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
            SWITCH_TO_RULES_MEM    ; mark bag as "complete" with .bags == 1 (total count)
            ORG rulesTab + ContainingBagIdx * S_RULE + S_RULE.complete : DB 1
            SWITCH_TO_INPUT_MEM
            ASSERT "\n" == {b inPtr}
        ELSE
            WHILE inPtr < inputEnd && '1' <= {b inPtr} && {b inPtr} <= '9'
                ; parse "(\d \w+ \w+ bags?, )*\d \w+ \w+ bags?\." groups
numOfBags = {b inPtr} - '0'
inPtr = inPtr + 2       ; "parses" "\d " part
                GetBagIndex     ; parses "\w+ \w+ " part
                DISPLAY "Bag ",/D,ContainingBagIdx," contains ",/D,numOfBags," bags ",/D,BagIdx
                SWITCH_TO_RULES_MEM        ; find free slot
                ORG rulesTab + ContainingBagIdx * S_RULE + S_RULE.contains : WHILE {b $ } : ORG $ + 3 : ENDW
                DB numOfBags : DW BagIdx    ; write Nx bagIndex to the rule table
                ASSERT $ <= rulesTab + (ContainingBagIdx+1) * S_RULE
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
ShinyGoldId = BagIdx
ShinyGoldBags = rulesTab + ShinyGoldId * S_RULE + S_RULE.bags   ; must be DEFL to work in third pass
    ; otherwise with regular asm source it would be possible to map full struct to desired address
hasIncompleteOne = 0

    DISPLAY "shiny gold bag is index: ",/D,ShinyGoldId
    SWITCH_TO_RULES_MEM

    MACRO SUM_BAG idx?
        IF 0 == {b rulesTab + idx? * S_RULE + S_RULE.complete }
            DISPLAY "Processing bag: ",/D,idx?
inPtr = rulesTab + idx? * S_RULE + S_RULE.contains
hasIncompleteOne = 0
            DUP BUCKET_SIZE
                IF {b inPtr}    ; if there is non-zero amount of some sub-bag, sum it or enqueue it
BagIdx = {inPtr + 1}
                    IF {b rulesTab + BagIdx * S_RULE + S_RULE.complete }
numOfBags = { rulesTab + BagIdx * S_RULE + S_RULE.bags } | ({ rulesTab + BagIdx * S_RULE + S_RULE.bags + 2 }<<16)
numOfBags2 = { rulesTab + idx? * S_RULE + S_RULE.bags } | ({ rulesTab + idx? * S_RULE + S_RULE.bags + 2 }<<16)
                        DISPLAY "Adding: ",/D,{b inPtr},"x ",/D,numOfBags," by bag ",/D,BagIdx," to current ",/D,numOfBags2
                        ORG rulesTab + idx? * S_RULE + S_RULE.bags : DD numOfBags2 + {b inPtr} * numOfBags   ; sum it
                        ORG inPtr : DB 0 : DW 0     ; kill the detailed rule data, already summed
numOfBags2 = { rulesTab + idx? * S_RULE + S_RULE.bags } | ({ rulesTab + idx? * S_RULE + S_RULE.bags + 2 }<<16)
                        DISPLAY "Result = ",/D,numOfBags2
                    ELSE
hasIncompleteOne = 1
                        ORG nextToResearchPtr : DW BagIdx   ; enqueue it for processing
nextToResearchPtr = nextToResearchPtr + 2
                    ENDIF
                ENDIF
inPtr = inPtr + 3
            EDUP
            IF hasIncompleteOne
                ORG nextToResearchPtr : DW idx?     ; enqueue self again after the sub-bags
nextToResearchPtr = nextToResearchPtr + 2
            ELSE
                ORG rulesTab + idx? * S_RULE + S_RULE.complete : DB 1   ; mark self as complete
            ENDIF
        ENDIF
    ENDM

researchPtr = 0
toResearchPtr = toResearch
nextToResearchPtr = toResearch

    ; put ShinyGoldId into queue
    ORG nextToResearchPtr : DW ShinyGoldId
nextToResearchPtr = nextToResearchPtr + 2

    ; process the queue
    WHILE 3 == __PASS__ && toResearchPtr < nextToResearchPtr
        SUM_BAG { toResearchPtr }
toResearchPtr = toResearchPtr + 2
    ENDW

numOfBags = { ShinyGoldBags } | ({ ShinyGoldBags + 2 }<<16)
    DISPLAY "Shiny gold back forms a heap of ",/D,numOfBags," bags (so answer is ",/D,numOfBags-1,")"
    ORG rulesTab + ShinyGoldId * S_RULE + S_RULE.contains
    DUP BUCKET_SIZE
        IF {b $}
            DISPLAY /D,{b $},"x id ",/D,{$+1}
        ENDIF
        ORG $ + 3
    EDUP

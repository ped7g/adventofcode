; https://adventofcode.com/2020/day/19 (part 2)
;
; Technically this is the same file as "part 1 fix" (with minor patches/improvements),
; because there's no difference in the algorithm, the loops just work.
; Which leaves me wondering what kind of shortcut there may be in part 1 possible,
; if I would not allow looping rules...

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     DEFINE MAX_PER_RULE 3 : INCBIN "input_t3.txt"   ; expected result 12
    DEFINE MAX_PER_RULE 3 : INCBIN "input_p2.txt"   ; regular input with patched rules 8 and 11
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  128,0 : ALIGN 256,0 ; zeroed padding after + align address

RULE_A      EQU 250
RULE_B      EQU 251
RULE_EOL    EQU 252
RULE_N      EQU 255     ; no rule
RULE_EX     EQU 254     ; rule was already expanded

    ; memory structures for parsing rules
rulesA:     DS  150*MAX_PER_RULE,RULE_N
rulesB:     DS  150*MAX_PER_RULE,RULE_N

    DS  10,0 : ALIGN 256,0 ; zeroed padding after + align address

CHOICES_ARRAY_LEN   EQU 256

    MACRO CLEAR_CHOICES
        ORG choices : DS CHOICES_ARRAY_LEN,0
    ENDM

choices:    CLEAR_CHOICES   ; zeroed choices (use variant-A always)

    ; memory structures for validation tree
    STRUCT S_NODE
NA          WORD    0       ; next node variant A
NB          WORD    0       ; next node variant B
RULE        BYTE    RULE_N  ; rule itself (RULE_A/B/EOL/N/EX or just number 0..249)
    ENDS

    DS 2*S_NODE,0 : ALIGN 256,0 ; some padding + align address
nodes:

    MACRO PARSE_VALUE symbol?
symbol? = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
symbol? = symbol? * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

    MACRO PARSE_RULE tab?, rule_num?
        ORG (tab?) + (rule_num?) * MAX_PER_RULE
        WHILE ('0' <= {b inPtr} && {b inPtr} <= '9') || ('"' == {b inPtr})
            IF '"' == {b inPtr}
                DB {b inPtr + 1} - 'a' + RULE_A
                ASSERT '"a' == {inPtr+1} || '"b' == {inPtr+1}
inPtr = inPtr + 3
            ELSE
                PARSE_VALUE r1 : DB r1
                IF ' ' == {b inPtr}
inPtr = inPtr + 1
                ENDIF
            ENDIF
        ENDW
    ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parse rules first
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

inPtr = inputFile
rules = 0
rule_num = 0
max_rule_num = -1
r1 = 0
    WHILE inPtr < inputEnd && {b inPtr} && "\n" != {b inPtr}
        PARSE_VALUE rule_num : ASSERT ' :' == {inPtr}
inPtr = inPtr + 2
max_rule_num = max_rule_num >? rule_num
        PARSE_RULE rulesA, rule_num
        IF ' |' == {inPtr}
inPtr = inPtr + 2
            PARSE_RULE rulesB, rule_num
        ENDIF
        ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1
rules = rules + 1
    ENDW
    DISPLAY "Parsed ",/D,rules," rules, max rule num: ",/D,max_rule_num
    ; skip empty line between rules and input lines
    ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; substitute trivial rules
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

r1Ptr = 0
r2Ptr = 0
wasChange = 1
    WHILE 3 == __PASS__ && wasChange
r1Ptr = rulesA
wasChange = 0
        DUP max_rule_num+1
            IF $FF != {b r1Ptr} && $FF == {b r1Ptr+1} && $FF == {b r1Ptr-rulesA+rulesB}
rule_num = (r1Ptr-rulesA)/MAX_PER_RULE
r1 = {b r1Ptr}
                DISPLAY "Trivial rule: ",/D,rule_num," -> ",/D,r1
wasChange = 1
r2Ptr = rulesA
                DUP (max_rule_num+1) * MAX_PER_RULE ; substitute trivial rules
                    IF rule_num == {b r2Ptr} : ORG r2Ptr : DB r1 : ENDIF
r2Ptr = r2Ptr + 1
                EDUP
r2Ptr = rulesB
                DUP (max_rule_num+1) * MAX_PER_RULE ; substitute trivial rules
                    IF rule_num == {b r2Ptr} : ORG r2Ptr : DB r1 : ENDIF
r2Ptr = r2Ptr + 1
                EDUP
                ORG r1Ptr : DB $FF
            ENDIF
r1Ptr = r1Ptr + MAX_PER_RULE
        EDUP
    ENDW

r1Ptr = rulesA
    DUP max_rule_num+1
        IF $FF != {b r1Ptr+0}       ; display only non-empty rules
r2Ptr = r1Ptr - rulesA + rulesB
            IF 3 == MAX_PER_RULE
                DISPLAY "Rule ",/D,(r1Ptr-rulesA)/MAX_PER_RULE,": ",/D,{b r1Ptr}," ",/D,{b r1Ptr+1}," ",/D,{b r1Ptr+2}," | ",/D,{b r2Ptr}," ",/D,{b r2Ptr+1}," ",/D,{b r2Ptr+2}
            ELSE
                DISPLAY "Rule ",/D,(r1Ptr-rulesA)/MAX_PER_RULE,": ",/D,{b r1Ptr}," ",/D,{b r1Ptr+1}," | ",/D,{b r2Ptr}," ",/D,{b r2Ptr+1}
            ENDIF
        ENDIF
r1Ptr = r1Ptr + MAX_PER_RULE
    EDUP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; check the messages for legitimacy
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

nodesNext = 0
    MACRO ADD_NODE nextA?, rule?
;         DISPLAY "Adding node ",nodesNext,": nextA ",nextA?," rule ",/D,rule?
        ORG nodesNext
        DW nextA?, 0    ; nextB is always nullptr in ADD_NODE (EXPAND is using it as needed)
        DB rule?
nodesNext = $   : ASSERT $ < $FFF0
    ENDM

    ; rPtr? - pointer into RulesA/RulesB, nextPtr? -> chain result here, endNode? -> reconnect here
    MACRO INSERT_RULE rPtr?, endNode?, nextPtr?
        ; chain to the node(s) which will be added
        ORG nextPtr? : IF $FF == {b (rPtr?) + 0} : DW 0 : ELSE : DW nodesNext : ENDIF
        ; calculate length of rule (0,1,..,MAX_PER_RULE)
        IF 3 == MAX_PER_RULE && $FF != {b (rPtr?) + 2}
            ADD_NODE nodesNext + S_NODE, {b (rPtr?) + 0}
            ADD_NODE nodesNext + S_NODE, {b (rPtr?) + 1}
            ADD_NODE (endNode?),         {b (rPtr?) + 2}
        ELSEIF $FF != {b (rPtr?) + 1}
            ADD_NODE nodesNext + S_NODE, {b (rPtr?) + 0}
            ADD_NODE (endNode?),         {b (rPtr?) + 1}
        ELSEIF $FF != {b (rPtr?) + 0}
            ADD_NODE (endNode?),         {b (rPtr?) + 0}
        ENDIF
    ENDM

expandRuleReconnectTo = 0
    MACRO EXPAND_RULE       ; node point to current node (to expand)
;         DISPLAY "Expanding node ",node," (",/D,{b node + S_NODE.RULE},")"
expandRuleReconnectTo = { node + S_NODE.NA }
        ASSERT 0 == { node + S_NODE.NB }        ; B-variant on unexpanded should be zero
        ; expand both variants (will set zero to NA/NB if they don't exist)
        INSERT_RULE rulesA + {b node + S_NODE.RULE} * MAX_PER_RULE, expandRuleReconnectTo, node + S_NODE.NA
        INSERT_RULE rulesB + {b node + S_NODE.RULE} * MAX_PER_RULE, expandRuleReconnectTo, node + S_NODE.NB
        ORG node + S_NODE.RULE : DB RULE_EX     ; mark as already expanded
    ENDM

    ASSERT 2 == S_NODE.NB - S_NODE.NA   ; will be exploited in code
match = 0
checkPtr = 0
checkLetter = 0
node = 0
choice = choices
this_choice = 0
words = 0

    WHILE inPtr < inputEnd && ('a' == {b inPtr} || 'b' == {b inPtr})
words = words + 1
        DISPLAY "Starting new word check (",/D,words,"), matches so far: ",/D,match
        ; check against rule 0 (test and real inputs have only single variant for 0)
nodesNext = nodes
        ADD_NODE nodesNext + S_NODE, 0  ; add node with rule 0 (leading to EOL node)
        ADD_NODE 0, RULE_EOL            ; end it with EOL node
        CLEAR_CHOICES                   ; clear all variant-choices to zero
choice = choices                        ; just to pass initial WHILE test
        WHILE choices <= choice         ; some choice was modified for next check, do it
;             DISPLAY "Starting new check traversal"
checkPtr = inPtr                        ; check the word from first character
choice = choices                        ; start following choices from beginning
node = nodes                            ; start at first rule-node
            WHILE node
                IF {b node + S_NODE.RULE} < RULE_A || {b node + S_NODE.RULE} == RULE_EX
                    ; if rule is unexpanded - expand it, before traversing it's choices
                    IF {b node + S_NODE.RULE} < RULE_A : EXPAND_RULE : ENDIF
                    ; traverse expanded rule (choosing variant A/B based on "choices" array)
                    ASSERT RULE_EX == {b node + S_NODE.RULE}
;                     DISPLAY "Traversing through expanded node ",node," to [",{node + S_NODE.NA},",",{node + S_NODE.NB},"] choice[",/D,choice-choices,"]: ",/D,{b choice}
                    IF {node + S_NODE.NB}       ; has both NA and NB options, use choices[i] array
node = {node + S_NODE.NA + {b choice}}
choice = choice + 1 :       ASSERT node
                    ELSE
node = {node + S_NODE.NA} : ASSERT node
                    ENDIF
                ELSEIF {b node + S_NODE.RULE} <= RULE_EOL
;                     DISPLAY "Letter ",/C,{b node + S_NODE.RULE}-RULE_A+'a'," check, node ",node," vs ",/C,{b checkPtr}," @",checkPtr
                    ; convert input character to RULE_A/B/EOL value
                    IF "\n" == {b checkPtr}
checkLetter = RULE_EOL
                    ELSE
checkLetter = {b checkPtr} - 'a' + RULE_A
                    ENDIF
                    IF {b node + S_NODE.RULE} == checkLetter
                        ; letter match
                        IF RULE_EOL == checkLetter
match = match + 1
choice = choices - 1        ; node will become 0 too, so this will end outer WHILE
                        ELSE
checkPtr = checkPtr + 1 :   ASSERT {node + S_NODE.NA} && !{node + S_NODE.NB}    ; letter nodes've only NextA variant
                        ENDIF
node = {node + S_NODE.NA}
                    ELSE
                        ; letter mismatch - change last zero choice (leading here) in array and zero all following
choice = choice - 1
                        WHILE choices <= choice && {b choice}
choice = choice - 1
                        ENDW
                        IF choices <= choice
;                             DISPLAY "Letter mismatch, setting choice [",/D,choice-choices,"] (",choice,") to B, clearing following"
                            ORG choice : DB 2 : DS (choices + CHOICES_ARRAY_LEN - $),0
                        ELSE
;                             DISPLAY "No dual choice with variant A lead up to this point, terminating validation"
                        ENDIF
node = 0                ; restart check from first node
                    ENDIF
                ELSE : ASSERT 0 && "??2" : ENDIF
            ENDW
        ENDW
        ; check is done, skip remaining characters of message (if any are left)
        WHILE "\n" != {b inPtr} && inPtr < inputEnd
inPtr = inPtr + 1
        ENDW
inPtr = inPtr + 1   ; eat newline too
    ENDW
    DISPLAY "Total match: ",/D,match

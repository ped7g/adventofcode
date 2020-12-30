; https://adventofcode.com/2020/day/21 (part 1)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 5
    INCBIN "input.txt"      ; expected result: 2211
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  10,0 : ALIGN 256,0  ; zeroed padding after + align address

MAX_WORDS       EQU 250     ; max unique words in input file (including allergens)
MAX_ALLERGENS   EQU 10      ; max unique allergens in input file + 1 working buffer
MAX_INGREDIENTS EQU 100     ; max ingredients to resolve per one allergen
ALLERGEN_SIZE   EQU 1 + 1 + MAX_INGREDIENTS

allergens   DS  (MAX_ALLERGENS + 1) * ALLERGEN_SIZE, 0
    ; single allergen record:
        ; BYTE allergen wordId
        ; BYTE number of ingredients
        ; Nx BYTE ingredients (Nx wordId)

assignedIngredients     DS MAX_WORDS,$FF    ; wordId -> allergenId

dictionary:
    DS      MAX_WORDS*4, 0
        ; WORD address into input file with the word letters (end by non-letter char)
        ; WORD counter how many times the word is in input of ingredients

strcmpResult = 0
strcmpPtr1 = 0
strcmpPtr2 = 0
    MACRO STRCMP str1?, str2?
strcmpPtr1 = str1?
strcmpPtr2 = str2?
strcmpResult = 0
        WHILE !strcmpResult && 'a' <= {b strcmpPtr1} && {b strcmpPtr1} <= 'z' && 'a' <= {b strcmpPtr2} && {b strcmpPtr2} <= 'z'
strcmpResult = {b strcmpPtr1} - {b strcmpPtr2}
strcmpPtr1 = strcmpPtr1 + 1
strcmpPtr2 = strcmpPtr2 + 1
        ENDW
        IF !strcmpResult    ; possible match, check if both words did end
strcmpResult = ('a' <= {b strcmpPtr2} && {b strcmpPtr2} <= 'z') - ('a' <= {b strcmpPtr1} && {b strcmpPtr1} <= 'z')
        ENDIF
    ENDM

    MACRO ADVANCE_AFTER_WORD symbol?
        WHILE 'a' <= {b symbol?} && {b symbol?} <= 'z'
symbol? = symbol? + 1
        ENDW
        WHILE ' ' == {b symbol?} || ',' == {b symbol?}
symbol? = symbol? + 1
        ENDW
    ENDM

MODE_INGREDIENTS    EQU     0
MODE_ALLERGENS      EQU     1

wordsN = 0
allergensN = 0
allergenIngredients = 0
wordId = 0
allergenId = 0
inPtr = inputFile
mode = MODE_INGREDIENTS
allerIng2 = 0
checkIngredient = 0
checkIngredientEnd = 0
checkNewIngredient = 0
checkNewIngredientEnd = 0

    WHILE inPtr < inputEnd && {b inPtr}
        ; search for current input word in dictionary, get its wordId (or add it if new word)
wordId = -1
strcmpResult = 1
        WHILE wordId + 1 < wordsN && strcmpResult
wordId = wordId + 1
            STRCMP inPtr, {dictionary + (wordId<<2)}
        ENDW
        IF strcmpResult
wordId = wordId + 1
            ORG dictionary + (wordsN<<2) : DW inPtr, 1 & (mode == MODE_INGREDIENTS)  ; init counter to 1 for ingredient, 0 for allergen word
wordsN = wordsN + 1 :   ASSERT wordsN <= MAX_WORDS
        ELSEIF mode == MODE_INGREDIENTS
            ORG dictionary + (wordId<<2) + 2 : DW {$} + 1   ; increment counter
        ENDIF
        IF mode == MODE_INGREDIENTS
            ; store the ingredient word into allergen structure (the working-buffer at end)
allergenIngredients = {b allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 1}
            ORG allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 1 : DB allergenIngredients + 1  ; ++ingredients
            ORG allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 2 + allergenIngredients : DB wordId
        ELSE
            ; check allergens: if new, add full list ingredients, otherwise prune existing
allergenId = 0
            WHILE allergenId < allergensN && wordId != {b allergens + allergenId * ALLERGEN_SIZE}
allergenId = allergenId + 1
            ENDW
            IF allergensN <= allergenId     ; new allergen found, copy full list of ingredients
                ORG allergens + allergensN * ALLERGEN_SIZE : DB wordId  ; store allergen word
allergenId = allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 1  ; reuse as pointer for memcpy
                DUP 1 + {b allergenId}
                    DB {b allergenId}
allergenId = allergenId + 1
                EDUP
allergensN = allergensN + 1
            ELSE                            ; known allergen found, prune the ingredients list
allergenIngredients = {b allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 1} :   ASSERT 0 < allergenIngredients
allerIng2 = {b allergens + allergenId * ALLERGEN_SIZE + 1} :                ASSERT 0 < allerIng2
;                 DISPLAY "Allergen ",/D,allergenId," again, new ingredients: ",/D,allergenIngredients,", old ingredients: ",/D,allerIng2
checkIngredient = allergens + allergenId * ALLERGEN_SIZE + 2
checkIngredientEnd = allergens + allergenId * ALLERGEN_SIZE + 2 + allerIng2
                WHILE checkIngredient < checkIngredientEnd
checkNewIngredient = allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 2
checkNewIngredientEnd = allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 2 + allergenIngredients
                    WHILE checkNewIngredient < checkNewIngredientEnd && {b checkNewIngredient} != {b checkIngredient}
checkNewIngredient = checkNewIngredient + 1
                    ENDW
                    IF checkNewIngredient == checkNewIngredientEnd
;                         DISPLAY "Ingredient missing: ",/D,{b checkIngredient}," -> removing"
checkIngredientEnd = checkIngredientEnd - 1                     ; refresh end pointer
                        IF checkIngredient < checkIngredientEnd ; copy last item over removed one
                            ORG checkIngredient : DB {b checkIngredientEnd}
                        ENDIF
                        ORG allergens + allergenId * ALLERGEN_SIZE + 1 : DB {b $ } - 1   ; --ingredients
                    ELSE
checkIngredient = checkIngredient + 1   ; try next ingredient if this one stays in list
                    ENDIF
                ENDW
            ENDIF
        ENDIF
        ; advance inPtr beyond the word and space/comma, but don't eat parentheses or newlines
        ADVANCE_AFTER_WORD inPtr

        ; check if the allergen list starts here
        IF '(' == {b inPtr}
mode = MODE_ALLERGENS
inPtr = inPtr + 1
            ADVANCE_AFTER_WORD inPtr    ; skip "contains"
        ELSEIF ')' == {b inPtr}
            ASSERT "\n" == {b inPtr+1}
mode = MODE_INGREDIENTS
inPtr = inPtr + 2
            ORG allergens + MAX_ALLERGENS * ALLERGEN_SIZE + 1 : DB 0    ; reset ingredients counter
        ENDIF
    ENDW
    DISPLAY "Total words: ",/D,wordsN,", \tTotal allergens: ",/D,allergensN

    ; assign allergen to particular ingredient
allergenModified = 1
w1 = 0
w2 = 0
    WHILE allergenModified && allergensN
allergenModified = 0
allergenId = 0
        DUP allergensN
checkNewIngredient = allergens + allergenId * ALLERGEN_SIZE
            ; check if this allergen was not assigned yet
            IF $FF != {b checkNewIngredient + 1}
wordId = -2
checkIngredient = checkNewIngredient + 2
checkIngredientEnd = checkIngredient + {b checkNewIngredient + 1}
                WHILE checkIngredient < checkIngredientEnd
                    IF $FF == {b assignedIngredients + {b checkIngredient}}
                        IF -2 == wordId
wordId = {b checkIngredient}    ; first unassigned ingredient -> remember it
                        ELSE
wordId = -1                     ; more than one unassigned, abort assignment
                        ENDIF
                    ENDIF
checkIngredient = checkIngredient + 1
                ENDW
                IF 0 <= wordId  ; some single unassigned ingredient found -> assign
                    ORG assignedIngredients + wordId : DB allergenId
                    ORG checkNewIngredient + 1 : DB $FF, wordId   ; list size = $FF (marker), first item = assigned wordId
allergenModified = 1
w1 = {dictionary + ({b checkNewIngredient}<<2)}
w2 = {dictionary + (wordId<<2)}
                    DISPLAY "Assigning allergen ",/D,allergenId," (",/D,{b checkNewIngredient}," ",/C,{w1+0},/C,{w1+1},/C,{w1+2},/C,{w1+3},"..) to wordId ",/D,wordId," ",/C,{w2+0},/C,{w2+1},/C,{w2+2},/C,{w2+3},".."
                ENDIF
            ENDIF
allergenId = allergenId + 1
        EDUP
    ENDW

    ; sum unassigned words and display result
sum = 0
wordId = 0
    WHILE wordId < wordsN
        IF $FF == {b assignedIngredients + wordId}
sum = sum + {dictionary + (wordId<<2) + 2}
;             DISPLAY "sum: adding ",/D,{dictionary + (wordId<<2) + 2}," for wordId ",/D,wordId," ",/C,{b {dictionary + (wordId<<2)}+0},/C,{b {dictionary + (wordId<<2)}+1},/C,{b {dictionary + (wordId<<2)}+2},/C,{b {dictionary + (wordId<<2)}+3}
        ENDIF
wordId = wordId + 1
    ENDW
    DISPLAY "Ingredients that cannot contain allergens appear ",/D,sum," times"

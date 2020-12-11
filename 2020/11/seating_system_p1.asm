; https://adventofcode.com/2020/day/11 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
source:     ; source area will overlap with input file
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
    INCBIN "input.txt"
inputEnd:
    ds  1024,0  ; some padding to incorporate larger map with OOB areas
sourceEnd:
    ALIGN 256,0
target:
    ds      (sourceEnd-source),'.'  ; same space second time, prefill with "floor"

    ; init variables
inPtr = inputFile

    ; figure out width of map
    WHILE 3 == __PASS__ && inPtr < inputEnd && "\n" != {b inPtr}
inPtr = inPtr + 1
    ENDW
width = inPtr - inputFile
height = (inputEnd-inputFile)/(width+1) ; assume rectangular map and single newline char
widthB = width + 2      ; with OOB bumper (real map width)
heightB = height + 2    ; with OOB bumper (real map height)
    DISPLAY "Map size: ",/D,width,"x",/D,height," extending to ",/D,widthB,"x",/D,heightB
    ASSERT (widthB*heightB) <= (sourceEnd-source)
    ; parse map to target area (removing newlines and adding OOB bumper areas)
    IF 3 == __PASS__
inPtr = inputFile
        ORG target+widthB+1 ; target [1,1] inward
        DUP height
            DUP width
                ASSERT '.' == {b inPtr} || 'L' == {b inPtr}
                DB {b inPtr}
inPtr = inPtr + 1
            EDUP
            ORG $ + 2   ; skip rigth bumper + left bumper on new line
inPtr = inPtr + 1   ; skip newline
        EDUP
        ASSERT ($ + widthB - 1) == target + (widthB * heightB)
    ENDIF

    ; run the simulation of people teaking/leaving seats
didSeatChange = 1
rounds = -1
occupied = 0
    WHILE 3 == __PASS__ && didSeatChange
        ; first copy target area back to source, and check if some seat did change
        ; (this will also make first copy of target to source raise change because of missing newlines)
inPtr = target : ORG source
didSeatChange = 0
        DUP widthB*heightB
didSeatChange = didSeatChange | ({b $} != {b inPtr})
            DB {b inPtr}
inPtr = inPtr + 1
        EDUP

        ; run one round of simulation, writing the result into target area
inPtr = source : ORG target
        DUP heightB
            DUP widthB
                IF '.' == {b inPtr}
                    DB '.'
                ELSE
occupied = (1&('#' == {b inPtr-widthB-1})) + (1&('#' == {b inPtr-widthB})) + (1&('#' == {b inPtr-widthB+1}))
occupied = occupied + (1&('#' == {b inPtr-1})) + (1&('#' == {b inPtr+1}))
occupied = occupied + (1&('#' == {b inPtr+widthB-1})) + (1&('#' == {b inPtr+widthB})) + (1&('#' == {b inPtr+widthB+1}))
                    IF 0 == occupied
                        DB '#'      ; take a seat
                    ELSEIF 4 <= occupied
                        DB 'L'      ; leave a seat
                    ELSE
                        DB {b inPtr}    ; keep state
                    ENDIF
                ENDIF
inPtr = inPtr + 1
            EDUP
        EDUP
rounds = rounds + 1
    ENDW

    DISPLAY "Rounds to reach stability: ",/D,rounds
    ; count occupied seats in stable position + output debug map of final state
inPtr = source : ORG target
occupied = 0
    DUP heightB
        DUP widthB
            DB {b inPtr}
            IF '#' == {b inPtr}
occupied = occupied + 1
            ENDIF
inPtr = inPtr + 1
        EDUP
        DB "\n"
    EDUP
    DISPLAY "Final state has ",/D,occupied," seats occupied."
    SAVEBIN "final.txt",target,(widthB+1)*heightB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; !!! WARNING !!!
;
; this would probably take ages to finish even with test data, as the algorithm favours
; memory usage, trading the CPU. After like 1 hour I got to "Cycle 5 | active cubes: 1056",
; which means there's about 1 billion of iterations in the nested loop to be done, and
; probably another billion+ in pruning the tags - I don't have patience to let this run.
; ^^ and that's the TEST input, the smaller one. The task input would be worse.
; (also there's risk of running out of memory, as there's only about 40kiB work space, 
; without adding banking code)

;     END     ; the following code theoretically works, but it is impractically slow

; https://adventofcode.com/2020/day/17 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
    INCBIN "input_t.txt"
;     INCBIN "input.txt"
inputEnd:
    DS  128,0 : ALIGN 256,0 ; zeroed padding after + align address
cubes:      ; cubes data will be stored from this memory address

    ; parse input file, create active cubes only (with coordinates starting at [50,50,50])
origin      EQU     50
c1x = origin
c1y = origin
c1z = origin
c1w = origin
inPtr = inputFile
    WHILE inPtr < inputEnd && {b inPtr}
        IF "\n" == {b inPtr}
c1x = origin - 1  ; -1 compensates for following +1
c1y = c1y + 1
        ELSEIF "#" == {b inPtr}
            DB c1x,c1y,c1z,c1w
            DISPLAY "Active cube at [",/D,c1x,",",/D,c1y,",",/D,c1z,",",/D,c1w,"]"
        ENDIF
c1x = c1x + 1
inPtr = inPtr +1
    ENDW

cubesEnd = $
maxMem = $

; simulation algorithm, having list of active cubes at start of cycle:
; tagList.clear();    ; list of tags (counters) for particular cube-coordinates
; for (c1 : list) for (c2 : list(c1+1,..) for (c3 : list (c2+1,..) {
;     mergeX/Y/Z/W1 = max(c1x/y/z/w,c2x/y/z/w,c3x/y/z/w)-1, mergeX/Y/Z/W2 = min(c1x/y/z/w,c2x/y/z/w,c3x/y/z/w)+1
;     for ( mergeCC : merge1..2 coordinates ) tagList.addOrUpdate(mergeCC)
; }
; for (c1 : list) {
;   tags = tagList.find(c1)
;   if (1 != tags && 4 != tags) list.delete(c1)
;   tagList.delete(c1)  ; remove tags for active cubes (not needed anymore)
; }
; for (ck => tags : tagList) if (1 == tags) list.add(ck)
;
; Where "tagList.addOrUpdate(cube)" means to count "tags" (clamping at 5 max)

    DISPLAY "----------- simulating --------------"
    ; simulate 6 cycles (well, 7 cycles, but displays state at beginning)
cycle = 0
c1Ptr = 0
c2Ptr = 0
c3Ptr = 0
mx1 = 0
mx2 = 0
my1 = 0
my2 = 0
mz1 = 0
mz2 = 0
mw1 = 0
mw2 = 0
mx = 0
my = 0
mz = 0
mw = 0
tagBeg = 0
tagEnd = 0
tagPtr = 0

    MACRO FINDTAG x?,y?,z?,w?   ; find tag-cube at x,y,z,w
tagPtr = tagBeg
        WHILE tagPtr < tagEnd && (x? != {b tagPtr+0} || y? != {b tagPtr+1} || z? != {b tagPtr+2} || w? != {b tagPtr+3})
tagPtr = tagPtr + 5
        ENDW
    ENDM

    MACRO BUMPTAG x?,y?,z?,w?   ; init/increment tag-cube at x,y,z,w
        FINDTAG x?,y?,z?,w?
        IF tagEnd == tagPtr ; not found, insert new tag-cube (with tags count initialized to 1)
            ORG tagEnd : DB x?, y?, z?, w?, 1
tagEnd = $
        ELSE                ; found, increment tag by one if not 5 yet
            IF {b tagPtr+4} < 5 : ORG tagPtr+4 : DB 1 + {b $} : ENDIF
        ENDIF
    ENDM

    WHILE cycle < 6
cuNum = (cubesEnd - cubes)/4
        DISPLAY "Cycle ",/D,cycle," | active cubes: ",/D,cuNum," \t| memory usage: ",/A,(maxMem - cubes)
        ; clear tags list (can start right after active cubes, active cubes list will grow into used tags only
tagBeg = cubesEnd
tagEnd = cubesEnd
        ; do the "triplets" nested loops to create tags
c1Ptr = cubes
        IF 3 <= cuNum : DUP cuNum-2
c2Ptr = c1Ptr + 4
            DUP (cubesEnd - c2Ptr)/4 - 1
c3Ptr = c2Ptr + 4
                DUP (cubesEnd - c3Ptr)/4
                    ; mergeX/Y/Z/W1 = max(c1x/y/z/w,c2x/y/z/w,c3x/y/z/w)-1, mergeX/Y/Z/W2 = min(c1x/y/z/w,c2x/y/z/w,c3x/y/z/w)+1
mx1 = ({b c1Ptr+0} >? {b c2Ptr+0} >? {b c3Ptr+0}) - 1
mx2 = ({b c1Ptr+0} <? {b c2Ptr+0} <? {b c3Ptr+0}) + 1
my1 = ({b c1Ptr+1} >? {b c2Ptr+1} >? {b c3Ptr+1}) - 1
my2 = ({b c1Ptr+1} <? {b c2Ptr+1} <? {b c3Ptr+1}) + 1
mz1 = ({b c1Ptr+2} >? {b c2Ptr+2} >? {b c3Ptr+2}) - 1
mz2 = ({b c1Ptr+2} <? {b c2Ptr+2} <? {b c3Ptr+2}) + 1
mw1 = ({b c1Ptr+3} >? {b c2Ptr+3} >? {b c3Ptr+3}) - 1
mw2 = ({b c1Ptr+3} <? {b c2Ptr+3} <? {b c3Ptr+3}) + 1
                    ; for ( mergeCC : merge1..2 coordinates ) tagList.addOrUpdate(mergeCC)
mw = mw1        :   WHILE mw <= mw2
mz = mz1        :       WHILE mz <= mz2
my = my1        :           WHILE my <= my2
mx = mx1        :               WHILE mx <= mx2
;                                     DISPLAY "tagging [",/D,mx,",",/D,my,",",/D,mz,",",/D,mw,"]"
                                    BUMPTAG mx,my,mz,mw ; insert/increment tagcube at tagPtr
mx = mx + 1
                                ENDW
my = my + 1
                            ENDW
mz = mz + 1
                        ENDW
mw = mw + 1
                    ENDW
c3Ptr = c3Ptr + 4
                EDUP
c2Ptr = c2Ptr + 4
            EDUP
c1Ptr = c1Ptr + 4
        EDUP : ENDIF ; (3 <= cuNum)
        IF maxMem < tagEnd
maxMem = tagEnd
        ENDIF
        IF 0    ; DEBUG - switch off by 0
        DISPLAY "Tags list size: ",/D,(tagEnd - tagBeg)/5
tagPtr = tagBeg
        WHILE tagPtr < tagEnd
            DISPLAY "Tag @[",/D,{b tagPtr+0},",",/D,{b tagPtr+1},",",/D,{b tagPtr+2},",",/D,{b tagPtr+3},"] = ",/D,{b tagPtr+4}
tagPtr = tagPtr + 5
        ENDW
        ENDIF
        ; check all current active cubes, and de-activate each which has 0 or 5 tags (go backwards)
c1Ptr = cubesEnd - 4
        WHILE cubes <= c1Ptr
            FINDTAG {b c1Ptr+0},{b c1Ptr+1},{b c1Ptr+2},{b c1Ptr+3}
            IF tagEnd == tagPtr || 0 == {b tagPtr+4} || 5 == {b tagPtr+4}   ; de-activate cube
cubesEnd = cubesEnd - 4             ; shorten list, and copy last cube to current
                IF c1Ptr < cubesEnd : ORG c1Ptr : DW {cubesEnd}, {cubesEnd+2} : ENDIF
            ENDIF
            ; delete the tag, if used by active cube (not needed anymore)
            IF tagPtr < tagEnd      ; the tag for cube was found, delete it
tagEnd = tagEnd - 5                 ; shorten list, and copy last tag to current
                IF tagPtr < tagEnd : ORG tagPtr : DW {tagEnd}, {tagEnd+2} : DB {b tagEnd+4} : ENDIF
            ENDIF
c1Ptr = c1Ptr - 4
        ENDW
        ; go through remaining tags, and activate all cubes with tags == 1
        DISPLAY "Reduced active cubes: ",/D,(cubesEnd - cubes)/4
        DISPLAY "Reduced tags list size: ",/D,(tagEnd - tagBeg)/5
tagPtr = tagBeg
        WHILE tagPtr < tagEnd
;             DISPLAY "Tag @[",/D,{b tagPtr+0},",",/D,{b tagPtr+1},",",/D,{b tagPtr+2},",",/D,{b tagPtr+3},"] = ",/D,{b tagPtr+4}
            IF 1 == {b tagPtr+4}    ; activate cube
                ORG cubesEnd : DW {tagPtr+0}, {tagPtr+2}
cubesEnd = cubesEnd + 4
            ENDIF
tagPtr = tagPtr + 5
        ENDW
cycle = cycle + 1
    ENDW

cuNum = (cubesEnd - cubes)/4
    DISPLAY "Cycle ",/D,cycle," | active cubes: ",/D,cuNum," \t| memory usage: ",/A,(maxMem - cubes)

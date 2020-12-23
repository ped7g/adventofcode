; https://adventofcode.com/2020/day/17 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"
    INCBIN "input.txt"
inputEnd:
    DS  128,0 : ALIGN 256,0 ; zeroed padding after + align address

world1:  DS  16*16*22*24/8,0    ; 24x22x16x16 bitmap
world2:  DS  16*16*22*24/8,0    ; 24x22x16x16 bitmap
    ASSERT $ < $10000

wofs_bitmask = 0
wofs_offset = 0
    MACRO WORLD_OFFSET x?, y?, z?, w?
wofs_bitmask = $80 >> ((x?) & $07)
wofs_offset = (w?) * 1056 + (z?) * 66 + (y?) * 3 + ((x?) >>> 3)
        ASSERT 0 <= (x?) && (x?) <= 23 && 0 <= (y?) && (y?) <= 21 && 0 <= (z?) && (z?) <= 15 && 0 <= (w?) && (w?) <= 15
    ENDM

    MACRO ACTIVATE_CUBE world?, x?, y?, z?, w?
        WORLD_OFFSET x?, y?, z?, w?
        ORG (world?) + wofs_offset : DB {b $} | wofs_bitmask
cuNum = cuNum + 1
    ENDM

active_count = 0
    MACRO COUNT_ACTIVE1 world?, x?, y?, z?, w?
        WORLD_OFFSET x?, y?, z?, w?
active_count = active_count + (1 & (!!({b (world?) + wofs_offset } & wofs_bitmask)))
    ENDM

    MACRO COUNT_ACTIVE world?, x?, y?, z?, w?
active_count = 0
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)  , (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)-1, (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)+1, (w?)
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)  , (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)-1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)+1, (w?)-1
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)  , (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)-1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)  , (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)  , (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)  , (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)-1, (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)-1, (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)-1, (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)  , (y?)+1, (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)-1, (y?)+1, (z?)+1, (w?)+1
        COUNT_ACTIVE1 world?, (x?)+1, (y?)+1, (z?)+1, (w?)+1
        ; if there are exactly 2 neighbours active, count also self (making it 3 for active cube)
        IF 2 == active_count : COUNT_ACTIVE1 world?, x?, y?, z?, w? : ENDIF
    ENDM

    ; parse input file with coordinates starting at [7,7,7,7], set into world1 bitmap
cuNum = 0
origin      EQU     7       ; there are six cycles, 7+ cubes leeway at each side is enough
cx = origin
cy = origin
cz = origin
cw = origin
inPtr = inputFile
    WHILE inPtr < inputEnd && {b inPtr}
        IF "\n" == {b inPtr}
cx = origin - 1  ; -1 compensates for following +1
cy = cy + 1
        ELSEIF "#" == {b inPtr}
            ACTIVATE_CUBE world1,cx,cy,cz,cw
            DISPLAY "Active cube at [",/D,cx,",",/D,cy,",",/D,cz,",",/D,cw,"]"
        ENDIF
cx = cx + 1
inPtr = inPtr +1
    ENDW

    ; simulation goes only through coordinates 1..20 for y/z/w axis, and 1..22 for x axis
    ; to avoid extra checks at boundaries, making neighbours cubes always exist
    DISPLAY "----------- simulating --------------"
    ; simulate 6 cycles
cycle = 0
worldSrc = 0
worldDst = 0

    WHILE cycle < 6 && 3 == __PASS__
        DISPLAY "Cycle ",/D,cycle," | active cubes: ",/D,cuNum
cuNum = 0
        ; select source/destination bitmap for this cycle
        IF cycle & 1
worldSrc = world2
worldDst = world1
        ELSE
worldSrc = world1
worldDst = world2
        ENDIF
        ; clear the destination world
        ORG worldDst : DS 16*16*22*24/8,0   ; 24x22x16x16 bitmap
        ; scan the source world and active all cubes which fit the rules
cw = 6-cycle
        DUP 3 + (cycle<<1) : DISPLAY "cw = ",/D,cw
cz = 6-cycle
            DUP 3 + (cycle<<1)
cy = 6-cycle
                DUP 10 + (cycle<<1)
cx = 6-cycle
                    DUP 11 + (cycle<<1)
                        COUNT_ACTIVE worldSrc, cx, cy, cz, cw
                        IF 3 == active_count : ACTIVATE_CUBE worldDst, cx, cy, cz, cw : ENDIF
cx = cx + 1
                    EDUP
cy = cy + 1
                EDUP
cz = cz + 1
            EDUP
cw = cw + 1
        EDUP
cycle = cycle + 1
    ENDW

    DISPLAY "Cycle ",/D,cycle," | active cubes: ",/D,cuNum

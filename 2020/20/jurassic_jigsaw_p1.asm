; https://adventofcode.com/2020/day/20 (part 1)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 1951 * 3079 * 2971 * 1171 = 20899048083289
    INCBIN "input.txt"
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  128,0 : ALIGN 256,0 ; zeroed padding after + align address

; about input files:
; Tiles have 4-digit ID of [1000,9999] range, tiles are 10x10 chars in both inputs
; The test input has 9 tiles 10x10, forming 3x3 image
;   1951    2311    3079    internal Ids: 1 0 8
;   2729    1427    2473                  7 3 5
;   2971    1489    1171                  6 4 2
; The puzzle input has 144 tiles, supposedly forming 12x12 image

    MACRO PARSE_VALUE symbol?
symbol? = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
symbol? = symbol? * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

parseEdgeCharPtr = 0
    MACRO PARSE_EDGE charPtr?, symbol?
symbol? = 0
parseEdgeCharPtr = charPtr?
        DUP 10
            ASSERT '.' == {b parseEdgeCharPtr} || '#' == {b parseEdgeCharPtr}
symbol? = (symbol? << 1) | (1 & ('#' == {b parseEdgeCharPtr}))
parseEdgeCharPtr = parseEdgeCharPtr + 1
        EDUP
    ENDM

mirror2_10b_from
    MACRO MIRROR2_10B from?, to?
mirror2_10b_from = from?
to? = 0
        DUP 10
to? = (to? << 1) | (1 & mirror2_10b_from)
mirror2_10b_from = mirror2_10b_from >>> 1
        EDUP
    ENDM

mirror10b_result = 0
    MACRO MIRROR_10B symbol?
        MIRROR2_10B symbol?, mirror10b_result
    ENDM

addTileToEdgeNewCount = 0
    MACRO ADD_TILE_TO_EDGE_IMPL edge?, tileId?
        ORG edgeHasTile + (edge?) * EDGE_HAS_TILE_SZ
addTileToEdgeNewCount = 1 + {b $} : ASSERT addTileToEdgeNewCount < EDGE_HAS_TILE_SZ
        DB addTileToEdgeNewCount
;         DISPLAY "Adding tile ",/D,tileId?," to edge ",edge?,", current count ",/D,addTileToEdgeNewCount
        ORG $ - 1 + addTileToEdgeNewCount : DB tileId?
    ENDM

addTileToEdgeMirrored = 0
    MACRO ADD_TILE_TO_EDGE edge?, tileId?
        ADD_TILE_TO_EDGE_IMPL edge?, tileId?
        MIRROR2_10B edge?, addTileToEdgeMirrored
        ADD_TILE_TO_EDGE_IMPL addTileToEdgeMirrored, tileId?
    ENDM

tileIds:    DS  512,0
tileEdges:  DS  256*4*2,$FF
EDGE_HAS_TILE_SZ EQU 8          ; 1B counter + 7 bytes for internal tile ID
edgeHasTile:    DS  (1<<10)*EDGE_HAS_TILE_SZ,0
tiles = 0
tileId = 0
tId = 0         ; internal ID (0..143)
inEdgeT = 0
inEdgeR = 0
inEdgeB = 0
inEdgeL = 0

inEdgeWorkBuffer:   DS  32*2

    ASSERT $ < $FFFF

    ; parse tiles from input
inPtr = inputFile
    WHILE inPtr < inputEnd && 'T' == {b inPtr}
        ASSERT 'iT' == {inPtr} && 'el' == {inPtr+2} && ' ' == {b inPtr+4}
inPtr = inPtr + 5
        ; parse + store 4-digit ID of next tile
tId = tiles
        PARSE_VALUE tileId : ORG tileIds + (tId<<1) : DW tileId
        DISPLAY "Reading tile ",/D,tileId," (internal: ",/D,tId,")"
tiles = tiles + 1
        ASSERT "\n:" == {inPtr}
inPtr = inPtr + 2
        ; copy side-edge chars into work buffer from top line
        ORG inEdgeWorkBuffer : DB {b inPtr} : ORG $+31 : DB {b inPtr+9} : ASSERT "\n" == {b inPtr+10}
        PARSE_EDGE inPtr, inEdgeT       ; turn top edge into 10 bit integer value
inPtr = inPtr + 11
        DUP 8
            ; copy side-edge chars into work buffer from middle lines
            ORG $-32 : DB {b inPtr} : ORG $+31 : DB {b inPtr+9} : ASSERT "\n" == {b inPtr+10}
inPtr = inPtr + 11
        EDUP
        ; copy side-edge chars into work buffer from bottom line
        ORG $-32 : DB {b inPtr} : ORG $+31 : DB {b inPtr+9} : ASSERT "\n\n" == {inPtr+10}
        ; turn bottom/left/right edge chars into 10 bit integer values
        PARSE_EDGE inPtr, inEdgeB
        PARSE_EDGE inEdgeWorkBuffer, inEdgeL
        PARSE_EDGE (inEdgeWorkBuffer + 32), inEdgeR
inPtr = inPtr + 12
        ; top and bottom are left-to-right, left and right are top-to-bottom (msb-to-lsb)
        ; from the "clockwise" point of view the bottom and right edge are mirrored
        DISPLAY "Edges [top, right, bottom, left]: ",inEdgeT,",",inEdgeR,",",inEdgeB,",",inEdgeL
        ORG tileEdges + (tId << 3) : DW inEdgeT,inEdgeR,inEdgeB,inEdgeL
        ADD_TILE_TO_EDGE inEdgeT, tId
        ADD_TILE_TO_EDGE inEdgeR, tId
        ADD_TILE_TO_EDGE inEdgeB, tId
        ADD_TILE_TO_EDGE inEdgeL, tId
    ENDW

    ; display tiles-count-per-edge (including mirrored variants, so total = tiles * 4 * 2
    DISPLAY "Parsed tiles ",/D,tiles," edges counts:"
inEdgeT = 0
inEdgeB = 0
    DUP ((1<<10) & (3 == __PASS__)) >? 1
        ORG edgeHasTile + inEdgeT * EDGE_HAS_TILE_SZ
inEdgeB = inEdgeB + {b $}
        IF 0 < {b $}
            DISPLAY "Edge ",inEdgeT," has ",/D,{b $}," tiles: ",/D,{b $+1},", ",/D,{b $+2},", ",/D,{b $+3},", ",/D,{b $+4},", ",/D,{b $+5},", ",/D,{b $+6},", ",/D,{b $+7}
        ENDIF
inEdgeT = inEdgeT + 1
    EDUP
    DISPLAY "Total tiles assigned to some edge: ",/D,inEdgeB : ASSERT tiles * 4 * 2 == inEdgeB

    ; search for tiles which have two edges with count = 1 (guaranteed to be corners)
    DISPLAY "Tiles with two or more edges which are completely unique:"
tId = 0
    DUP tiles
inPtr = tileEdges + tId * 4 * 2
inEdgeT = {b edgeHasTile + {inPtr+0} * EDGE_HAS_TILE_SZ}
inEdgeR = {b edgeHasTile + {inPtr+2} * EDGE_HAS_TILE_SZ}
inEdgeB = {b edgeHasTile + {inPtr+4} * EDGE_HAS_TILE_SZ}
inEdgeL = {b edgeHasTile + {inPtr+6} * EDGE_HAS_TILE_SZ}
        IF ((1 == inEdgeT) + (1 == inEdgeR) + (1 == inEdgeB) + (1 == inEdgeL)) <= -2    ; "true" in sjasmplus has value of -1
            DISPLAY "Tile (",/D,tId,") ",/D,{tileIds + (tId<<1)}," tiles matching edges: ",/D,inEdgeT,", ",/D,inEdgeR,", ",/D,inEdgeB,", ",/D,inEdgeL
        ENDIF
tId = tId + 1
    EDUP

; output for regular input:
; > Tiles with two or more edges which are completely unique:
; > Tile (73) 2347 tiles matching edges: 1, 2, 2, 1
; > Tile (94) 1747 tiles matching edges: 1, 2, 2, 1
; > Tile (107) 2281 tiles matching edges: 1, 2, 2, 1
; > Tile (124) 1811 tiles matching edges: 2, 1, 1, 2
; answer is then: =2347*1747*2281*1811 = 16937516456219

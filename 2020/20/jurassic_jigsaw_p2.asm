; https://adventofcode.com/2020/day/20 (part 2)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 273
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; what follows is basically Part 1 copy -> it will find the corner tiles
; (maybe not the most optimal approach, but there's little incentive to rewrite it)

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

mirror2_10b_from = 0
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

EDGE_HAS_TILE_SZ EQU 8          ; 1B counter + 7 bytes for internal tile ID

; transformations bits (could be combined into final transformation value)
T_ROT_90    EQU $01             ; rotate 90 degree clockwise
T_MIR_X     EQU $02             ; mirror x-axis (right side to left)
T_MIR_Y     EQU $04             ; mirror y-axis (bottom to top)

tileIds:    DS  512,0
tileEdges:  DS  256*4*2,$FF
finalEdges: DS  256*4*2,$FF     ; same as tileEdges, but after transformations are applied
edgeHasTile:    DS  (1<<10)*EDGE_HAS_TILE_SZ,0
tileMap     DS  12*12*2         ; max tile size is 12x12 (2 bytes per tile for transformation info)
tilePlaced  DS  256,0
tiles = 0
tileId = 0
tId = 0         ; internal ID (0..143)
inEdgeT = 0
inEdgeR = 0
inEdgeB = 0
inEdgeL = 0
tEdgeT = 0
tEdgeR = 0
tEdgeB = 0
tEdgeL = 0
tEdge = 0
transformation = 0

inEdgeWorkBuffer:   DS  32*2

inTileGfxWorkBuffer:    DS 8*8

    ALIGN 256
part2Image  DS  100*100,' '
    ORG part2Image+99 : DUP 100 : DB "\n" : ORG $+99 : EDUP ; put EOLs at +99 offset

    ASSERT $ < $FFFF

    ; parse tiles from input
inPtr = inputFile
    WHILE inPtr < inputEnd && 'T' == {b inPtr}
        ASSERT 'iT' == {inPtr} && 'el' == {inPtr+2} && ' ' == {b inPtr+4}
inPtr = inPtr + 5
        ; parse + store 4-digit ID of next tile
tId = tiles
        PARSE_VALUE tileId : ORG tileIds + (tId<<1) : DW tileId
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
        DISPLAY "Reading tile ",/D,tileId," (internal: ",/D,tId,"),\tedges [top, right, bottom, left]: ",inEdgeT,",",inEdgeR,",",inEdgeB,",",inEdgeL
        ORG tileEdges + (tId << 3) : DW inEdgeT,inEdgeR,inEdgeB,inEdgeL
        ADD_TILE_TO_EDGE inEdgeT, tId
        ADD_TILE_TO_EDGE inEdgeR, tId
        ADD_TILE_TO_EDGE inEdgeB, tId
        ADD_TILE_TO_EDGE inEdgeL, tId
    ENDW

    DISPLAY "Parsed tiles ",/D,tiles

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; part 2 changes...

    ; calculate possible size of big image in tiles (3x3 for test data, 12x12 for real)
Tsize = 1
    WHILE Tsize * Tsize < tiles
Tsize = Tsize + 1
    ENDW
    ASSERT Tsize * Tsize == tiles && Tsize <= 12 : DISPLAY "Map size in tiles: ",/D,Tsize,"x",/D,Tsize

    ; search for one corner tile to start tiling process
cornerTileId = -1
tId = 0
    WHILE -1 == cornerTileId && tId < tiles
inPtr = tileEdges + tId * 4 * 2                             ; four edges of tile tId
inEdgeT = {b edgeHasTile + {inPtr+0} * EDGE_HAS_TILE_SZ}    ; count of tiles with same edge (top)
inEdgeR = {b edgeHasTile + {inPtr+2} * EDGE_HAS_TILE_SZ}    ; right
inEdgeB = {b edgeHasTile + {inPtr+4} * EDGE_HAS_TILE_SZ}    ; bottom
inEdgeL = {b edgeHasTile + {inPtr+6} * EDGE_HAS_TILE_SZ}    ; left
        IF ((1 == inEdgeT) + (1 == inEdgeR) + (1 == inEdgeB) + (1 == inEdgeL)) <= -2    ; "true" in sjasmplus has value of -1
cornerTileId = tId      ; the tile has two or more unique edges, must be a corner (border tiles have single unique edge, inner tiles zero)
        ELSE
tId = tId + 1
        ENDIF
    ENDW

    ; tiling macro
placeTileTranform = 0
    MACRO PLACE_TILE ; tId, tx, ty, inEdgeR, inEdgeB
        DISPLAY "Place tile ",/D,tId," at [",/D,tx,",",/D,ty,"] with edges [L: ",inEdgeR&$FFFF,", T: ",inEdgeB&$FFFF,"]"
        ASSERT 0 == {b tilePlaced + tId}
placeTileTranform = 0
inPtr = tileEdges + tId * 4 * 2                             ; original four edges of tile tId
        WHILE 3 == __PASS__ && 0 == {b tilePlaced + tId}
            ASSERT placeTileTranform <= (T_ROT_90|T_MIR_X|T_MIR_Y)
            ; transform the original edges by placeTileTranform (rot = 1, mir_x = 2, mir_y = 4)
            IF placeTileTranform&T_ROT_90
                MIRROR2_10B {inPtr+6}, tEdgeT               ; top = mirror(left)
tEdgeR = {inPtr+0}                                          ; right = top
                MIRROR2_10B {inPtr+2}, tEdgeB               ; bottom = mirror(right)
tEdgeL = {inPtr+4}                                          ; left = bottom
            ELSE
tEdgeT = {inPtr+0}
tEdgeR = {inPtr+2}
tEdgeB = {inPtr+4}
tEdgeL = {inPtr+6}
            ENDIF
            IF placeTileTranform&T_MIR_X
tEdge = tEdgeR      ; swap right<->left
tEdgeR = tEdgeL     ; and mirror top and bottom
tEdgeL = tEdge
                MIRROR2_10B tEdgeT, tEdgeT
                MIRROR2_10B tEdgeB, tEdgeB
            ENDIF
            IF placeTileTranform&T_MIR_Y
tEdge = tEdgeT      ; swap top<->bottom
tEdgeT = tEdgeB     ; and mirror left and right
tEdgeB = tEdge
                MIRROR2_10B tEdgeL, tEdgeL
                MIRROR2_10B tEdgeR, tEdgeR
            ENDIF
            ; store transformed edges
            ORG finalEdges + tId * 4 * 2 : DW tEdgeT, tEdgeR, tEdgeB, tEdgeL
            ; reuse the bottom/right to find number of shared tiles for top/left
tEdgeR = {b edgeHasTile + tEdgeL * EDGE_HAS_TILE_SZ}
tEdgeB = {b edgeHasTile + tEdgeT * EDGE_HAS_TILE_SZ}
            ; if the current transformation fits the requirements, then place it
            IF ((-1 == inEdgeR) || (tEdgeL == inEdgeR) || (1<<10 == inEdgeR && 1 == tEdgeR)) && ((-1 == inEdgeB) || (tEdgeT == inEdgeB) || (1<<10 == inEdgeB && 1 == tEdgeB))
                ORG tilePlaced + tId : DB 1
                ORG tileMap + ty * Tsize * 2 + tx * 2 : DB tId, placeTileTranform
            ENDIF
placeTileTranform = placeTileTranform + 1
        ENDW
    ENDM

    ; start tiling (finding correct tiles and transformations, one by one
    DISPLAY "Tile (",/D,cornerTileId,") ",/D,{tileIds + (cornerTileId<<1)}," choosen as corner tile (t ",/D,inEdgeT,", r ",/D,inEdgeR,", b ",/D,inEdgeB,", l ",/D,inEdgeL,")"
tmpId = 0
ty = 0
    DUP Tsize
tx = 0
        DUP Tsize
            IF 0 == tx
                IF 0 == ty
tId = cornerTileId      ; top-left [0,0] corner is seeded from cornerTileId variable
inEdgeR = 1<<10                                     ; any edge, but must be unique
inEdgeB = 1<<10                                     ; any edge, but must be unique
                ELSE
tmpId = {b tileMap + (ty - 1) * Tsize * 2}          ; id of tile[0,ty-1] (above of [0,ty])
inEdgeR = -1                                        ; don't check
inEdgeB = {finalEdges + tmpId * 4 * 2 + 4}          ; bottom-edge of tile above
                    ; find the other tile id for the shared edge and assign to tId
                    ASSERT 2 <= {b edgeHasTile + inEdgeB * EDGE_HAS_TILE_SZ}    ; must be shared
tId = {b edgeHasTile + inEdgeB * EDGE_HAS_TILE_SZ + 1}
                    IF tmpId == tId
tId = {b edgeHasTile + inEdgeB * EDGE_HAS_TILE_SZ + 2}
                    ENDIF
                ENDIF
            ELSE
tmpId = {b tileMap + ty * Tsize * 2 + (tx - 1) * 2} ; id of tile[tx-1,ty] (left of [tx,ty])
inEdgeR = {finalEdges + tmpId * 4 * 2 + 2}          ; right-edge of tile left
inEdgeB = -1                                        ; don't check
                ; find the other tile id for the shared edge and assign to tId
                ASSERT 2 <= {b edgeHasTile + inEdgeR * EDGE_HAS_TILE_SZ}    ; must be shared
tId = {b edgeHasTile + inEdgeR * EDGE_HAS_TILE_SZ + 1}
                IF tmpId == tId
tId = {b edgeHasTile + inEdgeR * EDGE_HAS_TILE_SZ + 2}
                ENDIF
            ENDIF
            PLACE_TILE ; tId, tx, ty, inEdgeR, inEdgeB
tx = tx + 1
        EDUP
ty = ty + 1
    EDUP

    ; debug output of final tile map before re-reading input to get image
    DISPLAY "Map ",/D,Tsize,"x",/D,Tsize,": (ignore extra values on line if map is smaller than 12x12)"
ty = 0
    DUP Tsize
        ORG tileMap+ty*Tsize*2
        DISPLAY /D,{tileIds+({b $+0}<<1)}," t",/D,{b $+1}," ",/D,{tileIds+({b $+2}<<1)}," t",/D,{b $+3}," ",/D,{tileIds+({b $+4}<<1)}," t",/D,{b $+5}," ",/D,{tileIds+({b $+6}<<1)}," t",/D,{b $+7}," ",/D,{tileIds+({b $+8}<<1)}," t",/D,{b $+9}," ",/D,{tileIds+({b $+10}<<1)}," t",/D,{b $+11}," ",/D,{tileIds+({b $+12}<<1)}," t",/D,{b $+13}," ",/D,{tileIds+({b $+14}<<1)}," t",/D,{b $+15}," ",/D,{tileIds+({b $+16}<<1)}," t",/D,{b $+17}," ",/D,{tileIds+({b $+18}<<1)}," t",/D,{b $+19}," ",/D,{tileIds+({b $+20}<<1)}," t",/D,{b $+21}," ",/D,{tileIds+({b $+22}<<1)}," t",/D,{b $+23}
ty = ty + 1
    EDUP

    ; "draw" into memory the full image without the tile-edges, so the monsters can be searched in it
gfxSrc = 0
gfxSrcDX = 1
gfxSrcDY = 0
gfxSrcLDX = 0
gfxSrcLDY = 1
roughWater = 0

    IF 3 == __PASS__
inPtr = inputFile
tId = 0
        DISPLAY "Reading tiles gfx and creating big image..."
        DUP tiles
            ; read the tile "tId" gfx into inTileGfxWorkBuffer
            ASSERT inPtr < inputEnd && 'iT' == {inPtr} && 'el' == {inPtr+2} && ' ' == {b inPtr+4} && "\n:" == {inPtr+9} && "\n" == {b inPtr+21}
inPtr = inPtr + 11 + 11 + 1             ; skip to second line to second character
            ORG inTileGfxWorkBuffer     ; read 8 lines of inner pixels into work buffer
            DUP 8
                DUP 8
                    IF '#' == {b inPtr}
roughWater = roughWater + 1
                    ENDIF
                    DB {b inPtr}
inPtr = inPtr + 1
                EDUP
                ASSERT "\n" == {b inPtr+1}
inPtr = inPtr + 3                       ; skip last character, EOL and first character of next line
            EDUP
inPtr = inPtr + 9 : ASSERT "\n\n" == {inPtr}
inPtr = inPtr + 2                       ; point onto the next tile in input file
            ; find the "tId" in tileMap and read the transformation to do upon the gfx data
tx = 0
ty = 0
            WHILE tId != {b tileMap + ty * Tsize * 2 + tx * 2}
tx = tx + 1
                IF Tsize <= tx
tx = 0
ty = ty + 1
                ENDIF
            ENDW
transformation = {b tileMap + ty * Tsize * 2 + tx * 2 + 1}
;             DISPLAY "Tile ",/D,tId," placed at [",/D,tx,",",/D,ty,"] with tranformation: ",/D,transformation
            ; "draw" the gfx data into big image with the correct transformation
gfxSrc = inTileGfxWorkBuffer
gfxSrcDX = 1
gfxSrcDY = 0
gfxSrcLDX = 0
gfxSrcLDY = 1
            IF transformation&T_ROT_90
gfxSrc = inTileGfxWorkBuffer + 7*8
gfxSrcDX = 0
gfxSrcDY = -1
gfxSrcLDX = 1
gfxSrcLDY = 0
            ENDIF
            IF transformation&T_MIR_X
gfxSrc = gfxSrc + 7 * (gfxSrcDX + 8 * gfxSrcDY)
gfxSrcDX = -gfxSrcDX
gfxSrcDY = -gfxSrcDY
            ENDIF
            IF transformation&T_MIR_Y
gfxSrc = gfxSrc + 7 * (gfxSrcLDX + 8 * gfxSrcLDY)
gfxSrcLDX = -gfxSrcLDX
gfxSrcLDY = -gfxSrcLDY
            ENDIF
            ORG part2Image + ty*8*100 + tx*8
            DUP 8
                DUP 8
                    DB {b gfxSrc}
gfxSrc = gfxSrc + gfxSrcDX + 8 * gfxSrcDY
                EDUP
                ORG $ - 8 + 100
gfxSrc = gfxSrc - 8 * gfxSrcDX - 8 * 8 * gfxSrcDY
gfxSrc = gfxSrc + gfxSrcLDX + 8 * gfxSrcLDY
            EDUP
tId = tId + 1
        EDUP
;         SAVEBIN "dbg_image.txt",part2Image,100*100
    ENDIF
    DISPLAY "Total roughness of water before monster search: ",/D,roughWater

    ; search for the sea monsters (in all possible orientations)
transformation = 0                      ; transformation of sea monster
    DUP (T_ROT_90|T_MIR_X|T_MIR_Y)+1
        DISPLAY "Searching for sea monsters in transformation ",/D,transformation
smOfs = 0
smDX = 1
smDY = 0
smLDX = 0
smLDY = 1
smWidth = 20
smHeight = 3
        IF transformation&T_ROT_90
smOfs = 20*100
smDX = 0
smDY = -1
smLDX = 1
smLDY = 0
smWidth = 3
smHeight = 20
        ENDIF
        IF transformation&T_MIR_X
smOfs = smOfs + 19 * (smDX + 100 * smDY)
smDX = -smDX
smDY = -smDY
        ENDIF
        IF transformation&T_MIR_Y
smOfs = smOfs + 2 * (smLDX + 100 * smLDY)
smLDX = -smLDX
smLDY = -smLDY
        ENDIF
monsters = 0
        IF 3 == __PASS__
ty = 0
            DUP Tsize * 8 - smHeight
inPtr = part2Image + 100 * ty
                DUP Tsize * 8 - smWidth
                    ; head (three [+18,+0],[+18,+1],[+19,+1])
                    IF '#' == {b inPtr + smOfs + 18*(smDX + 100*smDY) + 0*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 18*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 19*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)}
                    ; first "S" (four [+0,+1],[+1,+2],[+4,+2],[+5,+1])
                    IF '#' == {b inPtr + smOfs + 0*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 1*(smDX + 100*smDY) + 2*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 5*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 4*(smDX + 100*smDY) + 2*(smLDX + 100*smLDY)}
                    ; second "S" (like first, but x+6)
                    IF '#' == {b inPtr + smOfs + 6*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 7*(smDX + 100*smDY) + 2*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 11*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 10*(smDX + 100*smDY) + 2*(smLDX + 100*smLDY)}
                    ; third "S" (like first, but x+12)
                    IF '#' == {b inPtr + smOfs + 12*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 13*(smDX + 100*smDY) + 2*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 17*(smDX + 100*smDY) + 1*(smLDX + 100*smLDY)} && '#' == {b inPtr + smOfs + 16*(smDX + 100*smDY) + 2*(smLDX + 100*smLDY)}
monsters = monsters + 1
                    ENDIF : ENDIF : ENDIF : ENDIF
inPtr = inPtr + 1
                EDUP
ty = ty + 1
            EDUP
            DISPLAY "Found ",/D,monsters,", roughness of water: ",/D,roughWater - 15*monsters
        ENDIF
transformation = transformation + 1
    EDUP

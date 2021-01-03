; https://adventofcode.com/2020/day/22 (part 2)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 291
;     INCBIN "input_t2.txt"   ; expected result: 105 (player 1 wins because recursive)

    INCBIN "input.txt"      ; expected result: Winner player 1, score: 35055, in 371 rounds
    ;; !!!!!!!!!!!!!!!
    ;; !!! WARNING !!! this takes about 1h:10min on my AMD Ryzen notebook
    ;; !!!!!!!!!!!!!!!
    ;; There are some ways how to improve the runtime in implementation details, and
    ;; I think there may be some way to add cache for decks for sub-games (no idea how
    ;; much would that improve runtime), but overall this one doesn't work well as
    ;; sjasmplus script (I bet the C++ solution would finish within couple of seconds).

inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  10,0 : ALIGN 256,0  ; zeroed padding after + align address

DECK_SIZE   EQU 64
DECK_SZ_AND EQU DECK_SIZE-1

    STRUCT S_DECK
top     BYTE
bot     BYTE
score   WORD
deck    BLOCK   DECK_SIZE
    ENDS

    STRUCT S_GAME
pGame   WORD    0           ; previous game pointer
gameN   WORD    0           ; number of current game
winner  BYTE    0           ; who did win this game (if finished)
round   WORD    0           ; number of current round
p1deck  S_DECK
p2deck  S_DECK
history                     ; first history item starts from here (dynamic size)
    ENDS

    STRUCT S_TURN           ; to keep game history, to prevent infinite loop
p1card  BYTE
p2card  BYTE
p1score WORD
p2score WORD
    ENDS

    MACRO PARSE_VALUE symbol?
symbol? = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
symbol? = symbol? * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

    MACRO DECK_SIZE deck?, result?
result? = {b deck? + S_DECK.bot} - {b deck? + S_DECK.top}
        IF result? < 0
result? = result? + DECK_SIZE
        ENDIF
    ENDM

    MACRO DECK_ADD deck?, card?
        ORG deck? + S_DECK.deck + {b deck? + S_DECK.bot} : DB card? ; write card
        ORG deck? + S_DECK.bot : DB ({b $} + 1) & DECK_SZ_AND       ; ++bot
    ENDM

    MACRO DECK_PEEK deck?, card?
card? = {b deck? + S_DECK.deck + {b deck? + S_DECK.top}}            ; read top card
    ENDM

    MACRO DECK_DEAL deck?, card?
        DECK_PEEK deck?, card?
        ORG deck? + S_DECK.top : DB ({b $} + 1) & DECK_SZ_AND       ; ++top
    ENDM

    MACRO DECK_PEEK_X deck?, pos?, card?
card? = {b deck? + S_DECK.deck + (((pos?) + {b deck? + S_DECK.top}) & DECK_SZ_AND)}
    ENDM

deckScore_cardValue = 0
deckScore_cardPos = 0
    MACRO DECK_SCORE deck?, result?
        DECK_SIZE deck?, deckScore_cardValue    ; get deck size (== card value)
result? = 0
deckScore_cardPos = {b deck? + S_DECK.top}
        WHILE 0 < deckScore_cardValue
result? = result? + deckScore_cardValue * {b deck? + S_DECK.deck + deckScore_cardPos}
deckScore_cardValue = deckScore_cardValue - 1
deckScore_cardPos = (deckScore_cardPos + 1) & DECK_SZ_AND
        ENDW
        ASSERT result? < $FFFF
        ORG deck? + S_DECK.score : DW result?   ; store score also into deck struct
    ENDM

    MACRO ALLOCATE_GAME
gGameN = gGameN + 1
        ; init new S_GAME structure at heap pointer
        ORG heap
        DW  game
        DW  gGameN
        DB  0
        DW  0       ; round = 0
        ORG heap + S_GAME.p1deck : DB 0, 0      ; p1deck: top = bot = 0
        ORG heap + S_GAME.p2deck : DB 0, 0      ; p2deck: top = bot = 0
game = heap
history = game + S_GAME.history
heap = heap + S_GAME
        DISPLAY "Game @",game," (",/D,{game + S_GAME.gameN},"), previous game: ",{game + S_GAME.pGame},", history @",history,", heap @",heap
    ENDM

    MACRO DELETE_GAME
heap = game
game = {game + S_GAME.pGame}
history = game + S_GAME.history
        DISPLAY "Back to game @",game," (",/D,{game + S_GAME.gameN},"), round ",/D,{game + S_GAME.round},", previous game: ",{game + S_GAME.pGame},", history @",history,", heap @",heap
    ENDM

    ; using globals: game, history, heap
playGameHistoryCheck = 0
playGameP1deck = 0
playGameP2deck = 0
playGameCardPos = 0
    MACRO PLAY_GAME result?, score?
result? = 0
        WHILE !result?
            ; score(p1deck, p2deck) and peek p1/p2 top card
            DECK_PEEK game + S_GAME.p1deck, p1card
            DECK_PEEK game + S_GAME.p2deck, p2card
            DECK_SCORE game + S_GAME.p1deck, p1score
            DECK_SCORE game + S_GAME.p2deck, p2score
            IF 0 == ({game + S_GAME.round} & 63)
                DISPLAY "status: game ",/D,{game + S_GAME.gameN},", round ",/D,1+{game + S_GAME.round},", P1 ",/D,p1card,":",/D,p1score," | P2 ",/D,p2card,":",/D,p2score
            ENDIF
            ; check (p1card,p2card,p1score1,p2score) against turn history
playGameHistoryCheck = history
            WHILE playGameHistoryCheck < heap && !result?
                IF p1card == {b playGameHistoryCheck + S_TURN.p1card} && p2card == {b playGameHistoryCheck + S_TURN.p2card} && p1score == {playGameHistoryCheck + S_TURN.p1score} && p2score == {playGameHistoryCheck + S_TURN.p2score}
result? = 1         ; same game state detected as one from historic turns, player 1 wins
score? = p1score    ; the check does rely on deck score (as sort of weak hash), which may
                    ; eventually collide for different decks, but let's hope it works
                    ; for provided inputs
                    DISPLAY "! history repeats itself, player 1 wins the infinite loop"
                ENDIF
playGameHistoryCheck = playGameHistoryCheck + S_TURN
            ENDW
            ; if history didn't match, add current game state to history list
            IF !result?
                ORG heap : DB p1card, p2card : DW p1score, p2score
heap = $
                ; check if one player won by having whole deck or if sub-game is possible and will decide winner
                DECK_SIZE game + S_GAME.p1deck, p1d_size
                DECK_SIZE game + S_GAME.p2deck, p2d_size
                IF 0 == p1d_size * p2d_size
                    ; one deck is empty, winner is the player with cards in deck
result? = (3 & (0 != p1d_size)) ^ 2     ; 1 or 2
score? = p1score >? p2score
                ELSE
                    ORG game + S_GAME.round : DW {$} + 1    ; ++game.round
                    ; decide winner of current rount (higher card or sub-game result)
                    IF p1card < p1d_size && p2card < p2d_size
;                         DISPLAY "Playing a sub-game to determine winner..."
playGameP1deck = game + S_GAME.p1deck   ; remember deck pointers across ALLOCATE_GAME
playGameP2deck = game + S_GAME.p2deck
                        ALLOCATE_GAME
                        ; clone sub-decks of players
playGameCardPos = 1
                        DUP p1card
                            DECK_PEEK_X playGameP1deck, playGameCardPos, p1card
                            DECK_ADD game + S_GAME.p1deck, p1card
playGameCardPos = playGameCardPos + 1
                        EDUP
playGameCardPos = 1
                        DUP p2card
                            DECK_PEEK_X playGameP2deck, playGameCardPos, p2card
                            DECK_ADD game + S_GAME.p2deck, p2card
playGameCardPos = playGameCardPos + 1
                        EDUP
                        PLAY_GAME result?, score?
                        DELETE_GAME
                    ELSE
result? = (3 & (p2card < p1card)) ^ 2   ; 1 or 2
                    ENDIF
;                     DISPLAY "Player ",/D,result?," wins, adjusting decks..."
                    ; "deal" top card from each deck (and refresh p1card/p2card symbols)
                    DECK_DEAL game + S_GAME.p1deck, p1card
                    DECK_DEAL game + S_GAME.p2deck, p2card
                    IF 1 == result?         ; player 1 wins round
                        DECK_ADD game + S_GAME.p1deck, p1card
                        DECK_ADD game + S_GAME.p1deck, p2card
                    ELSE
                        DECK_ADD game + S_GAME.p2deck, p2card
                        DECK_ADD game + S_GAME.p2deck, p1card
                    ENDIF
result? = 0         ; do next round, until one player completely wins or history loops
                ENDIF
            ENDIF
        ENDW
    ENDM

gGameN      = 0     ; number of previously allocated game
game        = 0     ; pointer to current S_GAME
history     = 0     ; start of history for current game
heap        = $     ; pointer to free memory

    ALLOCATE_GAME

    ; parse input file and create two decks of cards
playerNumber = -1
card = -1
deck = 0
inPtr = inputFile
    WHILE inPtr < inputEnd && {b inPtr}
        ASSERT 'lP' == {inPtr} && "\n:" == {inPtr+8}
playerNumber = {b inPtr+7} - '0'
inPtr = inPtr + 10
        DISPLAY "Reading deck of player ",/D,playerNumber," ..."
        IF 1 == playerNumber
deck = game + S_GAME.p1deck
        ELSE
deck = game + S_GAME.p2deck
        ENDIF
        WHILE inPtr < inputEnd && "\n" != {b inPtr}
            ASSERT '0' <= {b inPtr} && {b inPtr} <= '9'
            PARSE_VALUE card : ASSERT 1 <= card && card < DECK_SIZE
            DECK_ADD deck, card
            ASSERT "\n" == {b inPtr}
inPtr = inPtr + 1
        ENDW
        ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1
    ENDW
    DECK_SIZE game + S_GAME.p1deck, p1d_size
    DECK_SIZE game + S_GAME.p2deck, p2d_size
cards = p1d_size + p2d_size
    DISPLAY "Cards ",/D,cards,", Player 1 deck size: ",/D,p1d_size,", Player 2 deck size: ",/D,p2d_size

p1card = 0
p2card = 0
p1score = 0
p2score = 0
winner = 0
score = 0

    ; simulate a game until one player has won
    PLAY_GAME winner, score
    DISPLAY "Winner player ",/D,winner,", score: ",/D,score,", in ",/D,{game + S_GAME.round}," rounds"

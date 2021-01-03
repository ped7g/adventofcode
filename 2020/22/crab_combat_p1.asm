; https://adventofcode.com/2020/day/22 (part 1)

    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input.txt into virtual device memory for processing
;     INCBIN "input_t.txt"    ; expected result: 306
    INCBIN "input.txt"      ; expected result: 33098
inputEnd:
    ASSERT "\n" == {b $-1}  ; verify there is ending EOL
    DS  10,0 : ALIGN 256,0  ; zeroed padding after + align address

DECK_SIZE   EQU 64
DECK_SZ_AND EQU DECK_SIZE-1

    STRUCT S_DECK
top     BYTE
bot     BYTE
score   DWORD
deck    BLOCK   DECK_SIZE
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
        ORG deck? + S_DECK.score : DD result?   ; store score also into deck struct
    ENDM

p1deck  S_DECK
p2deck  S_DECK

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
deck = p1deck
        ELSE
deck = p2deck
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
    DECK_SIZE p1deck, p1d_size
    DECK_SIZE p2deck, p2d_size
cards = p1d_size + p2d_size
    DISPLAY "Cards ",/D,cards,", Player 1 deck size: ",/D,p1d_size,", Player 2 deck size: ",/D,p2d_size

    ; simulate a game until one player has won
p1card = 0
p2card = 0
rounds = 0
    WHILE p1d_size * p2d_size
rounds = rounds + 1
        DECK_DEAL p1deck, p1card
        DECK_DEAL p2deck, p2card
        IF p2card < p1card      ; player 1 wins round
            DISPLAY "P1 wins: ",/D,p1card," vs ",/D,p2card
            DECK_ADD p1deck, p1card
            DECK_ADD p1deck, p2card
        ELSE
            ASSERT p1card < p2card
            DISPLAY "P2 wins: ",/D,p2card," vs ",/D,p1card
            DECK_ADD p2deck, p2card
            DECK_ADD p2deck, p1card
        ENDIF
        DECK_SIZE p1deck, p1d_size
        DECK_SIZE p2deck, p2d_size
    ENDW
    DECK_SCORE p1deck, p1score
    DECK_SCORE p2deck, p2score
    DISPLAY "Rounds ",/D,rounds," P1 deck: ",/D,p1d_size," score: ",/D,p1score,", P2 deck: ",/D,p2d_size," score: ",/D,p2score

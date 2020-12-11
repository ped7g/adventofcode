; https://adventofcode.com/2020/day/10 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
;     INCBIN "input_t.txt"
;     INCBIN "input_t2.txt"
    INCBIN "input.txt"
inputEnd:
    ds      5,0     ; some zeroed mem in case the algorithm reads way beyond input
    ALIGN 256,0

MAX_ADAPTER EQU 255

    dd      0, 0, 0, 0, 0, 0    ; there's nothing below outlet at 0 jolts (0 ways)
adapters:
    dd      1, 0    ; outlet itself at 0 jolts is available in 1 way
    ds      MAX_ADAPTER*8,0

parseNumberVal = 0
    MACRO PARSE_VALUE
parseNumberVal = 0
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
parseNumberVal = parseNumberVal * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
    ENDM

    ; init variables
inPtr = inputFile

    ; parse all adapters into memory
    WHILE inPtr < inputEnd && {b inPtr}
        PARSE_VALUE : ASSERT "\n" == {b inPtr} || inputEnd <= inPtr
inPtr = inPtr + 1   ; skip newline char
        ORG adapters + parseNumberVal*8 : DD 1  ; mark adapter in table
    ENDW

inPtr = adapters+8      ; start from adapter 1-jolt (if available)
waysA1L = 0
waysA1H = 0
waysA2L = 0
waysA2H = 0
waysA3L = 0
waysA3H = 0
waysL = 0
waysH = 0
overflow = 0
    DUP MAX_ADAPTER
        IF {inPtr}      ; some adapter found, check how many options it has
waysA1L = {inPtr-24}|({inPtr-22}<<16)
waysA1H = {inPtr-20}|({inPtr-18}<<16)
waysA2L = {inPtr-16}|({inPtr-14}<<16)
waysA2H = {inPtr-12}|({inPtr-10}<<16)
waysA3L = {inPtr-8}|({inPtr-6}<<16)
waysA3H = {inPtr-4}|({inPtr-2}<<16)
waysL = waysA1L + waysA2L + waysA3L
overflow = ((waysA1L&$FFFF) + (waysA2L&$FFFF) + (waysA3L&$FFFF))>>>16
overflow = (overflow + (waysA1L>>>16) + (waysA2L>>>16) + (waysA3L>>>16))>>>16
waysH = waysA1H + waysA2H + waysA3H + overflow
            ORG inPtr : DD waysL, waysH
            DISPLAY "adapter ",/D,(inPtr-adapters)/8," has ",waysH,":",waysL," ways"
        ENDIF
inPtr = inPtr + 8
    EDUP
    DISPLAY "Possible ways: ",waysH,":",waysL

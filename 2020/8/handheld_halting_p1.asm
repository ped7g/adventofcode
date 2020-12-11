; https://adventofcode.com/2020/day/8 (part 1)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
    INCBIN "input.txt"
inputEnd:
    ds      256,0   ; some zeroed mem in case the algorithm reads way beyond input

OP_JMP      EQU $8000
OP_ACC      EQU $0000
OP_FN_MASK  EQU $8000
OP_EXE_MASK EQU $4000
OP_VAL_BITS EQU 14  ; 14 bits for values (int14_t => 1 sign bit, 13 value bits, -8192..+8191 range)
OP_VAL_MASK EQU (1<<OP_VAL_BITS)-1
OP_VAL_MIN  EQU -(1<<(OP_VAL_BITS-1))
OP_VAL_MAX  EQU (1<<(OP_VAL_BITS-1))-1

parseNumberVal = 0
parseNumberSgn = 0
    MACRO PARSE_VALUE
parseNumberVal = 0
parseNumberSgn = ('-' == {b inPtr})
inPtr = inPtr + 1
        WHILE '0' <= {b inPtr} && {b inPtr} <= '9'
parseNumberVal = parseNumberVal * 10 + {b inPtr} - '0'
inPtr = inPtr + 1
        ENDW
        IF parseNumberSgn
parseNumberVal = -parseNumberVal
        ENDIF
        IF parseNumberVal < OP_VAL_MIN || OP_VAL_MAX < parseNumberVal
            DISPLAY "Value ",/A,parseNumberVal," is out of supported range!"
        ENDIF
    ENDM

code:
    ; here the code from input will be assembled

    ; init variables
inPtr = inputFile

    ; assemble the input into 16-bit opcodes
    WHILE inPtr < inputEnd && {b inPtr}
        IF 'on' == {inPtr+0} && ' p' == {inPtr+2}       ; "nop" instruction (can have any value!)
            DW OP_ACC
inPtr = inPtr + 6   ; at least one digit is there, skip all of that
        ELSEIF 'ca' == {inPtr+0} && ' c' == {inPtr+2}   ; "acc" instruction
inPtr = inPtr + 4   ; to start of value
            PARSE_VALUE
            DW OP_ACC | (parseNumberVal & OP_VAL_MASK)
        ELSEIF 'mj' == {inPtr+0} && ' p' == {inPtr+2}   ; "jmp" instruction
inPtr = inPtr + 4   ; to start of value
            PARSE_VALUE
            DW OP_JMP | (parseNumberVal & OP_VAL_MASK)
        ELSEIF
            DISPLAY "Invalid instruction: ",/C,{b inPtr+0},/C,{b inPtr+1},/C,{b inPtr+2},/C,{b inPtr+3}
            ASSERT 0 && "Invalid instruction"
        ENDIF
        ; skip any remaining chars until end of line
        WHILE inPtr < inputEnd && "\n" != {b inPtr}
inPtr = inPtr + 1
        ENDW
inPtr = inPtr + 1   ; skip newline char
    ENDW
codeEnd = $
    DISPLAY "Assembled ",/D,($ - code)/2," instructions."

    ; execute the opcodes until infinite loop is reached
RegPC = code
RegA = 0
AccUponLoop = OP_VAL_MASK
instruction = OP_ACC        ; nop
    WHILE code <= RegPC && RegPC < codeEnd && OP_VAL_MASK == AccUponLoop
instruction = { RegPC }
        IF OP_EXE_MASK & instruction    ; this instruction was already executed before
AccUponLoop = RegA                      ; remember the Acc value and exit the loop
        ELSE        ; new instruction, execute it
            ORG RegPC : DW instruction | OP_EXE_MASK    ; mark the instruction as executed
            ; extract the value from the opcode (extend it to 32 bit integer)
parseNumberVal = instruction & OP_VAL_MASK
            IF OP_VAL_MAX < parseNumberVal              ; negative value encoded, fix it
parseNumberVal = parseNumberVal - (1<<OP_VAL_BITS)
            ENDIF
            IF (instruction & OP_FN_MASK) == OP_JMP
                ; JMP instruction
RegPC = RegPC + 2 * parseNumberVal
            ELSE
                ; ACC instruction
RegA = RegA + parseNumberVal
RegPC = RegPC + 2
            ENDIF
        ENDIF
    ENDW
    IF OP_VAL_MASK == AccUponLoop
        DISPLAY "PC went out of bounds before infinite loop was reached, PC = ",RegPC," ROM:",code,"..",codeEnd
    ELSE
        DISPLAY "Acc value upon hitting instruction at ",RegPC," second time: ",/A,AccUponLoop
    ENDIF

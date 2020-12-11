; https://adventofcode.com/2020/day/8 (part 2)
    OPT listoff : DEVICE ZXSPECTRUMNEXT : MMU 0 7, 0, $0000
inputFile:  ; load the input into virtual device memory for processing
    INCBIN "input.txt"
inputEnd:
    ds      256,0   ; some zeroed mem in case the algorithm reads way beyond input

OP_ACC      EQU $0000
OP_NOP      EQU $4000
OP_JMP      EQU $8000
OP_FN_MASK  EQU $C000
OP_EXE_MASK EQU $2000
OP_VAL_BITS EQU 13  ; 13 bits for values (int13_t => 1 sign bit, 12 value bits, -4096..+4095 range)
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
inPtr = inPtr + 4   ; to start of value
            PARSE_VALUE
            DW OP_NOP | (parseNumberVal & OP_VAL_MASK)
        ELSEIF 'ca' == {inPtr+0} && ' c' == {inPtr+2}   ; "acc" instruction
inPtr = inPtr + 4   ; to start of value
            PARSE_VALUE
            DW OP_ACC | (parseNumberVal & OP_VAL_MASK)
        ELSEIF 'mj' == {inPtr+0} && ' p' == {inPtr+2}   ; "jmp" instruction
inPtr = inPtr + 4   ; to start of value
            PARSE_VALUE
            DW OP_JMP | (parseNumberVal & OP_VAL_MASK)
        ELSE
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

    ; modify every nop<->jmp and run the emulation, until the non-loop result is reached
patchPC = code
AccUponExit = OP_VAL_MASK
AccUponLoop = OP_VAL_MASK
RegPC = code
RegA = 0
instruction = OP_NOP
    WHILE OP_VAL_MASK == AccUponExit && patchPC < codeEnd
        ; skip all "acc" instructions
        WHILE patchPC < codeEnd && OP_ACC == ({patchPC} & OP_FN_MASK)
patchPC = patchPC + 2
        ENDW
        ; patch the nop<->jmp instruction
        ORG patchPC : DW {patchPC} ^ OP_NOP ^ OP_JMP
        ; clear "executed" flag in all instructions
        ORG code : .((codeEnd - code)/2) DW {$} & (~OP_EXE_MASK)
        ; run the emulation (until infinite loop is reached or code does exit)
RegPC = code
RegA = 0
AccUponLoop = OP_VAL_MASK
        WHILE code <= RegPC && RegPC < codeEnd && OP_VAL_MASK == AccUponExit && OP_VAL_MASK == AccUponLoop
instruction = { RegPC }
            IF OP_EXE_MASK & instruction    ; this instruction was already executed before
AccUponLoop = RegA                          ; remember the Acc value and exit the loop
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
                    ; ACC or NOP instruction
                    IF (instruction & OP_FN_MASK) == OP_ACC
RegA = RegA + parseNumberVal
                    ENDIF
RegPC = RegPC + 2
                ENDIF
            ENDIF
        ENDW
        IF OP_VAL_MASK == AccUponLoop       ; PC went out of bounds = boot code did finish
AccUponExit = RegA
        ENDIF
        ; patch the nop<->jmp instruction back
        ORG patchPC : DW {patchPC} ^ OP_NOP ^ OP_JMP
patchPC = patchPC + 2
    ENDW
    IF OP_VAL_MASK == AccUponExit
        DISPLAY "no instruction patch found to fix infinite loop"
    ELSE
        DISPLAY "Patched instruction at ",patchPC," made the code exit with A = ",/A,AccUponExit
    ENDIF

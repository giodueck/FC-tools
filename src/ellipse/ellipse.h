#ifndef ELLIPSE_H
#define ELLIPSE_H

enum ellipse_register {
    // Hardwired zero
    EL_ZERO,
    // Program Counter
    EL_PC,
    // Return Address
    EL_RA,
    // Stack Pointer
    EL_SP,
    // IO Receive
    EL_RX,
    // IO Receive Size
    EL_RC,
    // IO Transmit
    EL_TX,
    // IO Transmit Size
    EL_TC,
    // GP argument/return registers
    EL_A0,
    EL_A1,
    EL_A2,
    EL_A3,
    EL_A4,
    EL_A5,
    EL_A6,
    EL_A7,
    // GP saved registers
    EL_S0,
    EL_S1,
    EL_S2,
    EL_S3,
    EL_S4,
    EL_S5,
    EL_S6,
    EL_S7,
    // GP temporary registers
    EL_T0,
    EL_T1,
    EL_T2,
    EL_T3,
    EL_T4,
    EL_T5,
    // PRNG seed and next value
    EL_PRNG,
    // GP Ticker
    EL_TICK
};

enum ellipse_opcode {
    EL_HALT     = 0,
    EL_MOV      = 8,
    EL_MOVI     = 9,
    EL_MOVL     = 10,
    EL_MOVH     = 11,
    EL_ADD      = 16,
    EL_SUB      = 17,
    EL_MUL      = 18,
    EL_DIV      = 19,
    EL_MOD      = 20,
    EL_POW      = 21,
    EL_LSH      = 22,
    EL_RSH      = 23,
    EL_AND      = 24,
    EL_OR       = 25,
    EL_XOR      = 26,
    EL_SUBN     = 27,
    EL_ADDI     = 32,
    EL_SUBI     = 33,
    EL_MULI     = 34,
    EL_DIVI     = 35,
    EL_MODI     = 36,
    EL_POWI     = 37,
    EL_LSHI     = 38,
    EL_RSHI     = 39,
    EL_ANDI     = 40,
    EL_ORI      = 41,
    EL_XORI     = 42,
    EL_SUBNI    = 43,
    EL_JEQ      = 48,
    EL_JNE      = 49,
    EL_JLT      = 50,
    EL_JGT      = 51,
    EL_JLE      = 52,
    EL_JGE      = 53,
    EL_JNG      = 54,
    EL_JPZ      = 55,
    EL_JVS      = 56,
    EL_JVC      = 57,
    EL_JMP      = 58,
    EL_JEQR     = 64,
    EL_JNER     = 65,
    EL_JLTR     = 66,
    EL_JGTR     = 67,
    EL_JLER     = 68,
    EL_JGER     = 69,
    EL_JNGR     = 70,
    EL_JPZR     = 71,
    EL_JVSR     = 72,
    EL_JVCR     = 73,
    EL_JMPR     = 74,
    EL_STORER   = 80,
    EL_STOREI   = 81,
    EL_STORERI  = 82,
    EL_STORERD  = 83,
    EL_LOADR    = 84,
    EL_LOADI    = 85,
    EL_LOADRI   = 86,
    EL_LOADRD   = 87,
    EL_PUSH     = 88,
    EL_PUSHI    = 89,
    EL_POP      = 90,
    EL_RAMCP    = 96,
    EL_POPRX    = 97,
    EL_POPTX    = 98,
    EL_FLUSHTX  = 99,
};

#endif // ELLIPSE_H

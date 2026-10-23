#ifndef INSNS_H
#define INSNS_H


enum op {
    NOP,    // 0
    LD,     // 1
    INC,    // 2
    DEC,    // 3
    ADD,    // 4
    ADC,    // 5
    SUB,    // 6
    SBC,    // 7
    AND,    // 8
    OR,     // 9
    XOR,    // 10
    CP,     // 11
    RLCA,   // 12
    RRCA,   // 13
    RLA,    // 14
    RRA,    // 15
    DAA,    // 16
    CPL,    // 17
    SCF,    // 18
    CCF,    // 19
    JR,     // 20
    JP,     // 21
    CALL,   // 22
    RET,    // 23
    POP,    // 24
    PUSH,   // 25
    DI,     // 26
    EI,     // 27
    RETI,   // 28
    RST,    // 29
    STOP,   // 30
    HALT,   // 31
    PREFIX, // 32
};

enum param {
    NONE,   // 0
    A,      // 1
    B,      // 2
    C,      // 3
    D,      // 4
    E,      // 5
    H,      // 6
    L,      // 7
    BC,     // 8
    DE,     // 9
    HL,     // 10
    AF,     // 11
    SP,     // 12
    AH_C,   // 13
    A_BC,   // 14
    A_DE,   // 15
    A_HL,   // 16
    A_HLI,  // 17
    A_HLD,  // 18
    SP_SIMM8,// 19
    IMM16,  // 20
    IMM8,   // 21
    SIMM8,  // 22
    A_IMM16,// 23
    A_IMM8, // 24
    AH_IMM8,// 25
    F_NZ,   // 26
    F_NC,   // 27
    F_Z,    // 28
    F_C,    // 29
    SET_P2, // 30
};

struct insn {
    enum op op;
    enum param p1;
    enum param p2;
    unsigned int cycles;
};

extern const struct insn INSNS[256];

#endif
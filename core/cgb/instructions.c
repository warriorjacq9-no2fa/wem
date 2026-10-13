#include "instructions.h"

const struct insn INSNS[256] = {
    (struct insn) {
        .op = NOP,
    },
    (struct insn) {
        .op = LD,
        .p1 = BC,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_BC,
        .p2 = A,
    },
    (struct insn) {
        .op = INC,
        .p1 = BC,
    },
    (struct insn) {
        .op = INC,
        .p1 = B,
    },
    (struct insn) {
        .op = DEC,
        .p1 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RLCA,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_IMM16,
        .p2 = SP,
    },
    (struct insn) {
        .op = ADD,
        .p1 = HL,
        .p2 = BC,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A_BC,
    },
    (struct insn) {
        .op = DEC,
        .p1 = BC,
    },
    (struct insn) {
        .op = INC,
        .p1 = C,
    },
    (struct insn) {
        .op = DEC,
        .p1 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RRCA,
    },
    (struct insn) {
        .op = STOP,
        .p1 = IMM8,
    },
    (struct insn) {
        .op = LD,
        .p1 = DE,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_DE,
        .p2 = A,
    },
    (struct insn) {
        .op = INC,
        .p1 = DE,
    },
    (struct insn) {
        .op = INC,
        .p1 = D,
    },
    (struct insn) {
        .op = DEC,
        .p1 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RLA,
    },
    (struct insn) {
        .op = JR,
        .p1 = SIMM8,
    },
    (struct insn) {
        .op = ADD,
        .p1 = HL,
        .p2 = DE,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A_DE,
    },
    (struct insn) {
        .op = DEC,
        .p1 = DE,
    },
    (struct insn) {
        .op = INC,
        .p1 = E,
    },
    (struct insn) {
        .op = DEC,
        .p1 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RRA
    },
    (struct insn) {
        .op = JR,
        .p1 = F_NZ,
        .p2 = SIMM8,
    },
    (struct insn) {
        .op = LD,
        .p1 = HL,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HLI,
        .p2 = A,
    },
    (struct insn) {
        .op = INC,
        .p1 = HL,
    },
    (struct insn) {
        .op = INC,
        .p1 = H,
    },
    (struct insn) {
        .op = DEC,
        .p1 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = DAA,
    },
    (struct insn) {
        .op = JR,
        .p1 = F_Z,
        .p2 = SIMM8,
    },
    (struct insn) {
        .op = ADD,
        .p1 = HL,
        .p2 = HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A_HLI,
    },
    (struct insn) {
        .op = DEC,
        .p1 = HL,
    },
    (struct insn) {
        .op = INC,
        .p1 = L,
    },
    (struct insn) {
        .op = DEC,
        .p1 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = CPL,
    },
    (struct insn) {
        .op = JR,
        .p1 = F_NC,
        .p2 = SIMM8,
    },
    (struct insn) {
        .op = LD,
        .p1 = SP,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HLD,
        .p2 = A,
    },
    (struct insn) {
        .op = INC,
        .p1 = SP,
    },
    (struct insn) {
        .op = INC,
        .p1 = A_HL,
    },
    (struct insn) {
        .op = DEC,
        .p1 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = SCF,
    },
    (struct insn) {
        .op = JR,
        .p1 = F_C,
        .p2 = SIMM8,
    },
    (struct insn) {
        .op = ADD,
        .p1 = HL,
        .p2 = SP,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A_HLD,
    },
    (struct insn) {
        .op = DEC,
        .p1 = SP,
    },
    (struct insn) {
        .op = INC,
        .p1 = A,
    },
    (struct insn) {
        .op = DEC,
        .p1 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = CCF,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = B,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = C,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = D,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = E,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = H,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = L,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = L,
    },
    (struct insn) {
        .op = HALT,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_HL,
        .p2 = A,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = B,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = C,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = D,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = E,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = H,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = L,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = A_HL,
    },
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = A,
    },
    (struct insn) {
        .op = RET,
        .p1 = F_NZ,
    },
    (struct insn) {
        .op = POP,
        .p1 = BC,
    },
    (struct insn) {
        .op = JP,
        .p1 = F_NZ,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = JP,
        .p1 = IMM16,
    },
    (struct insn) {
        .op = CALL,
        .p1 = F_NZ,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = PUSH,
        .p1 = BC,
    },
    (struct insn) {
        .op = ADD,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x00,
    },
    (struct insn) {
        .op = RET,
        .p1 = F_Z,
    },
    (struct insn) {
        .op = RET,
    },
    (struct insn) {
        .op = JP,
        .p1 = F_Z,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = PREFIX,
    },
    (struct insn) {
        .op = CALL,
        .p1 = F_Z,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = CALL,
        .p1 = IMM16,
    },
    (struct insn) {
        .op = ADC,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x08,
    },
    (struct insn) {
        .op = RET,
        .p1 = F_NC,
    },
    (struct insn) {
        .op = POP,
        .p1 = DE,
    },
    (struct insn) {
        .op = JP,
        .p1 = F_NC,
        .p2 = IMM16,
    },
    (struct insn) {0},
    (struct insn) {
        .op = CALL,
        .p1 = F_NC,
        .p2 = IMM16,
    },
    (struct insn) {
        .op = PUSH,
        .p1 = DE,
    },
    (struct insn) {
        .op = SUB,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x10,
    },
    (struct insn) {
        .op = RET,
        .p1 = F_C,
    },
    (struct insn) {
        .op = RETI
    },
    (struct insn) {
        .op = JP,
        .p1 = F_C,
        .p2 = IMM16,
    },
    (struct insn) {0},
    (struct insn) {
        .op = CALL,
        .p1 = F_C,
        .p2 = IMM16,
    },
    (struct insn) {0},
    (struct insn) {
        .op = SBC,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x18,
    },
    (struct insn) {
        .op = LD,
        .p1 = AH_IMM8,
        .p2 = A,
    },
    (struct insn) {
        .op = POP,
        .p1 = HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = AH_C,
        .p2 = A,
    },
    (struct insn) {0},
    (struct insn) {0},
    (struct insn) {
        .op = PUSH,
        .p1 = HL,
    },
    (struct insn) {
        .op = AND,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x20,
    },
    (struct insn) {
        .op = ADD,
        .p1 = SP,
        .p2 = SIMM8,
    },
    (struct insn) {
        .op = JP,
        .p1 = HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = A_IMM16,
        .p2 = A,
    },
    (struct insn) {0},
    (struct insn) {0},
    (struct insn) {0},
    (struct insn) {
        .op = XOR,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x28,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = AH_IMM8,
    },
    (struct insn) {
        .op = POP,
        .p1 = AF,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = AH_C,
    },
    (struct insn) {
        .op = DI,
    },
    (struct insn) {0},
    (struct insn) {
        .op = PUSH,
        .p1 = AF,
    },
    (struct insn) {
        .op = OR,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x30,
    },
    (struct insn) {
        .op = LD,
        .p1 = HL,
        .p2 = SP_SIMM8
    },
    (struct insn) {
        .op = LD,
        .p1 = SP,
        .p2 = HL,
    },
    (struct insn) {
        .op = LD,
        .p1 = A,
        .p2 = A_IMM16,
    },
    (struct insn) {
        .op = EI
    },
    (struct insn) {0},
    (struct insn) {0},
    (struct insn) {
        .op = CP,
        .p1 = A,
        .p2 = IMM8,
    },
    (struct insn) {
        .op = RST,
        .p1 = SET_P2,
        .p2 = 0x38,
    }
};
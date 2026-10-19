#include "cpu.h"
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "instructions.h"
#include "memmap.h"

struct typed_ptr {
    union {
        uint8_t* u8;
        uint16_t* u16;
        int8_t* s8;
        int16_t* s16;
        void* raw;
    };
    enum ptr_type{
        NUL,
        U8,
        U16,
        S8,
        S16,
    } type;
    bool addr;
    uint16_t a;
};

void typed_ptr_dbg(char* s, struct typed_ptr p) {
    switch(p.type) {
        case U8:
            snprintf(s, 8,"(%02x)", *p.u8);
            break;
        case S8:
            snprintf(s, 8,"(%s%x)", (*p.s8 < 0) ? "-" : "", (*p.s8 < 0) ? -*p.s8 : *p.s8);
            break;
        case U16:
            snprintf(s, 8,"(%04x)", *p.u16);
            break;
        case S16:
            snprintf(s, 8,"(%s%x)", (*p.s16 < 0) ? "-" : "", (*p.s16 < 0) ? -*p.s16 : *p.s16);
            break;
        case NUL:
            break;
    }
}

#if DEBUG
static const char* const insn_names[] = {
    "NOP",
    "LD",
    "INC",
    "DEC",
    "ADD",
    "ADC",
    "SUB",
    "SBC",
    "AND",
    "OR",
    "XOR",
    "CP",
    "RLCA",
    "RRCA",
    "RLA",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF",
    "JR",
    "JP",
    "CALL",
    "RET",
    "POP",
    "PUSH",
    "DI",
    "EI",
    "RETI",
    "RST",
    "STOP",
    "HALT",
    "PREFIX"
};
static const char* const pfx_names[] = {
    "RLC",
    "RRC",
    "RL",
    "RR",
    "SLA",
    "SRA",
    "SWAP",
    "SRL",
    "BIT",
    "BIT",
    "BIT",
    "BIT",
    "BIT",
    "BIT",
    "BIT",
    "BIT",
    "RES",
    "RES",
    "RES",
    "RES",
    "RES",
    "RES",
    "RES",
    "RES",
    "SET",
    "SET",
    "SET",
    "SET",
    "SET",
    "SET",
    "SET",
};
static const char* const param_names[] = {
    "",
    "A",
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "BC",
    "DE",
    "HL",
    "AF",
    "SP",
    "[C + 0xFF00]",
    "[BC]",
    "[DE]",
    "[HL]",
    "[HL+]",
    "[HL-]",
    "SP + s8",
    "n16",
    "n8",
    "s8",
    "[a16]",
    "[a8]",
    "[a8 + 0xFF00]",
    "NZ",
    "NC",
    "Z",
    "C",
    ""
};
#endif

uint16_t tmp;
struct typed_ptr enum_param(enum param p, struct registers* regs) {
    struct typed_ptr res = (struct typed_ptr) {0};
    switch(p) {
        case A:
            res.u8 = &regs->a;
            res.type = U8;
            break;
        case B:
            res.u8 = &regs->b;
            res.type = U8;
            break;
        case C:
            res.u8 = &regs->c;
            res.type = U8;
            break;
        case D:
            res.u8 = &regs->d;
            res.type = U8;
            break;
        case E:
            res.u8 = &regs->e;
            res.type = U8;
            break;
        case H:
            res.u8 = &regs->h;
            res.type = U8;
            break;
        case L:
            res.u8 = &regs->l;
            res.type = U8;
            break;
        case BC:
            res.u16 = &regs->bc;
            res.type = U16;
            break;
        case DE:
            res.u16 = &regs->de;
            res.type = U16;
            break;
        case HL:
            res.u16 = &regs->hl;
            res.type = U16;
            break;
        case AF:
            res.u16 = &regs->af;
            res.type = U16;
            break;
        case SP:
            res.u16 = &regs->sp;
            res.type = U16;
            break;
        case AH_C:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->c + 0xFF00);
            res.type = U8;
            res.addr = true;
            res.a = regs->c + 0xFF00;
            break;
        case A_BC:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->bc);
            res.type = U8;
            res.addr = true;
            res.a = regs->bc;
            break;
        case A_DE:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->de);
            res.type = U8;
            res.addr = true;
            res.a = regs->de;
            break;
        case A_HL:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->hl);
            res.type = U8;
            res.addr = true;
            res.a = regs->hl;
            break;
        case A_HLI:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->hl);
            res.type = U8;
            res.addr = true;
            res.a = regs->hl;
            regs->hl++;
            break;
        case A_HLD:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->hl);
            res.type = U8;
            res.addr = true;
            res.a = regs->hl;
            regs->hl--;
            break;
        case SP_SIMM8:
            res.u16 = &tmp;
            *res.u16 = regs->sp + (int8_t)memread(regs->pc++);
            res.type = U16;
            break;
        case IMM16:
            res.u16 = &tmp;
            *res.u16 = memread(regs->pc) | (memread(regs->pc + 1) << 8);
            res.type = U16;
            regs->pc += 2;
            break;
        case IMM8:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->pc++);
            res.type = U8;
            break;
        case SIMM8:
            res.s8 = (int8_t*)&tmp;
            *res.s8 = memread(regs->pc++);
            res.type = S8;
            break;
        case A_IMM16:
            res.u8 = (uint8_t*)&tmp;
            res.addr = true;
            res.a = memread(regs->pc) | (memread(regs->pc + 1) << 8);
            *res.u8 = memread(res.a);
            res.type = U8;
            regs->pc += 2;
            break;
        case A_IMM8:
            res.u8 = (uint8_t*)&tmp;
            res.addr = true;
            res.a = memread(regs->pc++);
            *res.u8 = memread(res.a);
            res.type = U8;
            break;
        case AH_IMM8:
            res.u8 = (uint8_t*)&tmp;
            res.addr = true;
            res.a = memread(regs->pc++) + 0xFF00;
            *res.u8 = memread(res.a);
            res.type = U8;
            break;
        default:
            res = (struct typed_ptr) {0};
            break;
    }
    return res;
}

bool pfx;

void run_insn_pfx(struct registers* regs, uint8_t opcode) {
    pfx = false;
    uint8_t* param = NULL;
    int scratch;
    switch(opcode & 0x07) {
        case 0x00: param = &regs->b; break;
        case 0x01: param = &regs->c; break;
        case 0x02: param = &regs->d; break;
        case 0x03: param = &regs->e; break;
        case 0x04: param = &regs->h; break;
        case 0x05: param = &regs->l; break;
        case 0x07: param = &regs->a; break;
        default: break;
    }
#if DEBUG
    char* pname = "";
    switch(opcode & 0x07) {
        case 0x00: pname = "B"; break;
        case 0x01: pname = "C"; break;
        case 0x02: pname = "D"; break;
        case 0x03: pname = "E"; break;
        case 0x04: pname = "H"; break;
        case 0x05: pname = "L"; break;
        case 0x06: pname = "[HL]"; break;
        case 0x07: pname = "A"; break;
        default: break;
    }
    fprintf(stderr,
        "%s(%02x) %s(%02x) ; %04x\n",
        pfx_names[(opcode & 0xF8) >> 3], opcode, 
        pname, (param != NULL ? *param : 0),
        regs->pc - 1
    );
#endif
    if((opcode & 0x07) == 0x06) {
        uint8_t val = memread(regs->hl);
        switch(opcode & 0xF8) {
            case 0x00: // RLC
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val << 1;
                if(val & 0x80) {
                    regs->f |= FLAG_C;
                    scratch |= 0x01;
                } else {
                    regs->f &= ~FLAG_C;
                    scratch &= ~0x01;
                }
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x08: // RRC
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val >> 1;
                if(val & 0x01) {
                    regs->f |= FLAG_C;
                    scratch |= 0x80;
                } else {
                    regs->f &= ~FLAG_C;
                    scratch &= ~0x80;
                }
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x10: // RL
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val << 1;
                if(regs->f & FLAG_C)
                    scratch |= 0x01;
                else
                    scratch &= ~0x01;
                
                if(val & 0x80)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x18: // RR
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val >> 1;
                if(regs->f & FLAG_C)
                    scratch |= 0x80;
                else
                    scratch &= ~0x80;
                
                if(val & 0x01)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x20: // SLA
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val << 1;
                
                if(val & 0x80)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x28: // SRA
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val >> 1 | (val & 0x80);
                
                if(val & 0x01)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x30: // SWAP
                regs->f &= ~(FLAG_N | FLAG_H | FLAG_C);
                scratch = val >> 4;
                scratch |= ( val << 4) & 0xF0;

                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x38: // SRL
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = val >> 1;
                
                if(val & 0x01)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                memwrite(regs->hl, scratch);

                if(scratch == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            // BIT
            case 0x40: scratch = 0x01; goto _do_bit_hl;
            case 0x48: scratch = 0x02; goto _do_bit_hl;
            case 0x50: scratch = 0x04; goto _do_bit_hl;
            case 0x58: scratch = 0x08; goto _do_bit_hl;
            case 0x60: scratch = 0x10; goto _do_bit_hl;
            case 0x68: scratch = 0x20; goto _do_bit_hl;
            case 0x70: scratch = 0x40; goto _do_bit_hl;
            case 0x78: scratch = 0x80; goto _do_bit_hl;
_do_bit_hl:
                if((val & (scratch & 0xFF)) == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;

                regs->f &= ~FLAG_N;
                regs->f |= FLAG_H;
                break;
            // RES
            case 0x80: scratch = 0x01; goto _do_res_hl;
            case 0x88: scratch = 0x02; goto _do_res_hl;
            case 0x90: scratch = 0x04; goto _do_res_hl;
            case 0x98: scratch = 0x08; goto _do_res_hl;
            case 0xA0: scratch = 0x10; goto _do_res_hl;
            case 0xA8: scratch = 0x20; goto _do_res_hl;
            case 0xB0: scratch = 0x40; goto _do_res_hl;
            case 0xB8: scratch = 0x80; goto _do_res_hl;
 _do_res_hl:
                memwrite(regs->hl, val & ~(scratch & 0xFF));
                break;
            // SET
            case 0xC0: scratch = 0x01; goto _do_set_hl;
            case 0xC8: scratch = 0x02; goto _do_set_hl;
            case 0xD0: scratch = 0x04; goto _do_set_hl;
            case 0xD8: scratch = 0x08; goto _do_set_hl;
            case 0xE0: scratch = 0x10; goto _do_set_hl;
            case 0xE8: scratch = 0x20; goto _do_set_hl;
            case 0xF0: scratch = 0x40; goto _do_set_hl;
            case 0xF8: scratch = 0x80; goto _do_set_hl;
_do_set_hl:
                memwrite(regs->hl, val | (scratch & 0xFF));
                break;
        }
    } else {
        switch(opcode & 0xF8) {
            case 0x00: // RLC
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param << 1;
                if(*param & 0x80) {
                    regs->f |= FLAG_C;
                    scratch |= 0x01;
                } else {
                    regs->f &= ~FLAG_C;
                    scratch &= ~0x01;
                }
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x08: // RRC
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param >> 1;
                if(*param & 0x01) {
                    regs->f |= FLAG_C;
                    scratch |= 0x80;
                } else {
                    regs->f &= ~FLAG_C;
                    scratch &= ~0x80;
                }
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x10: // RL
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param << 1;
                if(regs->f & FLAG_C)
                    scratch |= 0x01;
                else
                    scratch &= ~0x01;
                
                if(*param & 0x80)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x18: // RR
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param >> 1;
                if(regs->f & FLAG_C)
                    scratch |= 0x80;
                else
                    scratch &= ~0x80;
                
                if(*param & 0x01)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x20: // SLA
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param << 1;
                
                if(*param & 0x80)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x28: // SRA
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param >> 1 | (*param & 0x80);
                
                if(*param & 0x01)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x30: // SWAP
                regs->f &= ~(FLAG_N | FLAG_H | FLAG_C);
                scratch = *param >> 4;
                *param <<= 4;
                *param |= scratch & 0x0F;

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            case 0x38: // SRL
                regs->f &= ~(FLAG_N | FLAG_H);
                scratch = *param >> 1;
                
                if(*param & 0x01)
                    regs->f |= FLAG_C;
                else
                    regs->f &= ~FLAG_C;
                
                *param = (uint8_t)(scratch & 0xFF);

                if(*param == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;
                break;
            // BIT
            case 0x40: scratch = 0x01; goto _do_bit;
            case 0x48: scratch = 0x02; goto _do_bit;
            case 0x50: scratch = 0x04; goto _do_bit;
            case 0x58: scratch = 0x08; goto _do_bit;
            case 0x60: scratch = 0x10; goto _do_bit;
            case 0x68: scratch = 0x20; goto _do_bit;
            case 0x70: scratch = 0x40; goto _do_bit;
            case 0x78: scratch = 0x80; goto _do_bit;
_do_bit:
                if((*param & (scratch & 0xFF)) == 0)
                    regs->f |= FLAG_Z;
                else
                    regs->f &= ~FLAG_Z;

                regs->f &= ~FLAG_N;
                regs->f |= FLAG_H;
                break;
            // RES
            case 0x80: scratch = 0x01; goto _do_res;
            case 0x88: scratch = 0x02; goto _do_res;
            case 0x90: scratch = 0x04; goto _do_res;
            case 0x98: scratch = 0x08; goto _do_res;
            case 0xA0: scratch = 0x10; goto _do_res;
            case 0xA8: scratch = 0x20; goto _do_res;
            case 0xB0: scratch = 0x40; goto _do_res;
            case 0xB8: scratch = 0x80; goto _do_res;
 _do_res:
                *param &= ~(scratch & 0xFF);
                break;
            // SET
            case 0xC0: scratch = 0x01; goto _do_set;
            case 0xC8: scratch = 0x02; goto _do_set;
            case 0xD0: scratch = 0x04; goto _do_set;
            case 0xD8: scratch = 0x08; goto _do_set;
            case 0xE0: scratch = 0x10; goto _do_set;
            case 0xE8: scratch = 0x20; goto _do_set;
            case 0xF0: scratch = 0x40; goto _do_set;
            case 0xF8: scratch = 0x80; goto _do_set;
_do_set:
                *param |= scratch & 0xFF;
                break;
        }
    }
}

void run_insn(struct registers* regs) {
#if DEBUG
    uint16_t pc = regs->pc;
#endif
    uint8_t opcode = memread(regs->pc++);
    if(pfx) {
        run_insn_pfx(regs, opcode);
        return;
    }
    struct insn i = INSNS[opcode];
    struct typed_ptr p1 = enum_param(i.p1, regs);
    struct typed_ptr p2;
    if(i.p1 == SET_P2) {
        p2 = (struct typed_ptr) {
            .u8 = (uint8_t*)&i.p2,
            p2.type = U8,
        };
    } else {
        p2 = enum_param(i.p2, regs);
    }
    int scratch;
#if DEBUG
    char p1_str[8] = {0}, p2_str[8] = {0};
    typed_ptr_dbg(p1_str, p1);
    typed_ptr_dbg(p2_str, p2);
    fprintf(stderr,
        "%s(%02x) %s%s, %s%s ; %04x\n",
        insn_names[(i.op < sizeof(insn_names) / sizeof(insn_names[0])) ? i.op : 0], opcode, 
        param_names[(i.p1 < sizeof(insn_names) / sizeof(insn_names[0])) ? i.p1 : 0], p1_str,
        param_names[(i.p2 < sizeof(insn_names) / sizeof(insn_names[0])) ? i.p2 : 0], p2_str,
        pc
    );
#endif
    switch(i.op) {
        case NOP:
            break;
        case LD:
            if(p1.addr) {
                switch(p2.type) {
                    case U8:
                    case S8:
                        memwrite(p1.a, *p2.u8);
                        break;
                    case U16:
                    case S16:
                        memwrite(p1.a, *p2.u16 & 0xFF);
                        memwrite(p1.a + 1, *p2.u16 >> 8);
                        break;
                    default:
                        break;
                }
            } else {
                switch(p1.type) {
                    case U8:
                    case S8:
                        *p1.u8 = *p2.u8;
                        break;
                    case U16:
                    case S16:
                        *p1.u16 = *p2.u16;
                        break;
                    default:
                        break;
                }
            }
            break;
        case INC:
            switch(p1.type){
                case U8:
                case S8:
                    regs->f &= ~FLAG_N;
                    if((*p1.u8 & 0xF) == 0xF) regs->f |= FLAG_H;
                    else regs->f &= ~FLAG_H;
                    scratch = *p1.u8;

                    if(scratch == 0xFF) regs->f |= FLAG_Z;
                    else regs->f &= ~FLAG_Z;

                    if(p1.addr)
                        memwrite(p1.a, scratch + 1);
                    else
                        (*p1.u8) = scratch + 1;
                    break;
                case U16:
                case S16:
                    (*p1.u16)++;
                    break;
                default:
                    break;
            }
            break;
        case DEC:
            switch(p1.type){
                case U8:
                case S8:
                    regs->f |= FLAG_N;
                    if((*p1.u8 & 0xF) == 0) regs->f |= FLAG_H;
                    else regs->f &= ~FLAG_H;
                    scratch = *p1.u8;

                    if(scratch == 1) regs->f |= FLAG_Z;
                    else regs->f &= ~FLAG_Z;

                    if(p1.addr)
                        memwrite(p1.a, scratch - 1);
                    else
                        (*p1.u8) = scratch - 1;
                    break;
                case U16:
                case S16:
                    (*p1.u16)--;
                    break;
                default:
                    break;
            }
            break;
        case ADC:
        case ADD:
            regs->f &= ~FLAG_N;

            if(i.op == ADC) {
                if((*p1.u8 & 0xF) + (*p2.u8 & 0xF) +
                    (regs->f & FLAG_C ? 1 : 0) > 0xF)
                    regs->f |= FLAG_H;
                else
                    regs->f &= ~FLAG_H;
            } else {
                if(p1.type == U16) {
                    if((*p1.u16 & 0x0F00) + (*p2.u8 & 0x0F00) > 0x0F00)
                        regs->f |= FLAG_H;
                    else
                        regs->f &= ~FLAG_H;
                } else {
                    if((*p1.u8 & 0xF) + (*p2.u8 & 0xF) > 0xF)
                        regs->f |= FLAG_H;
                    else
                        regs->f &= ~FLAG_H;
                }
            }
            switch(p1.type) {
                case U8:
                    if(i.op == ADC)
                        scratch = *p1.u8 + *p2.u8 + 
                            (regs->f & FLAG_C ? 1 : 0);
                    else
                        scratch = *p1.u8 + *p2.u8;

                    if(scratch > 0xFF)
                        regs->f |= FLAG_C;
                    else
                        regs->f &= ~FLAG_C;

                    if((scratch & 0xFF) == 0)
                        regs->f |= FLAG_Z;
                    else
                        regs->f &= ~FLAG_Z;

                    if(p1.addr)
                        memwrite(p1.a, scratch);
                    else
                        *p1.u8 = (uint8_t)scratch;
                    break;
                case U16:
                    if(p2.type == S8) {
                        scratch = *p1.u16 + *p2.s8;
                        regs->f &= ~FLAG_Z;
                    } else {
                        scratch = *p1.u16 + *p2.u16;
                    }

                    if(scratch > 0xFFFF)
                        regs->f |= FLAG_C;
                    else
                        regs->f &= ~FLAG_C;

                    *p1.u16 = (uint16_t)scratch;
                    break;
                default:
                    break;
            }
            break;
        case SBC:
        case SUB:
            regs->f |= FLAG_N;

            if(i.op == SBC) {
                if((*p1.u8 & 0xF) < (*p2.u8 & 0xF) +
                    (regs->f & FLAG_C ? 1 : 0))
                    regs->f |= FLAG_H;
                else
                    regs->f &= ~FLAG_H;
            } else {
                if((*p1.u8 & 0xF) < (*p2.u8 & 0xF))
                    regs->f |= FLAG_H;
                else
                    regs->f &= ~FLAG_H;
            }
            if(i.op == SBC)
                scratch = *p1.u8 - (*p2.u8 + 
                    (regs->f & FLAG_C ? 1 : 0));
            else
                scratch = *p1.u8 - *p2.u8;

            if(scratch < 0)
                regs->f |= FLAG_C;
            else
                regs->f &= ~FLAG_C;

            if((scratch & 0xFF) == 0)
                regs->f |= FLAG_Z;
            else
                regs->f &= ~FLAG_Z;

            if(p1.addr)
                memwrite(p1.a, scratch);
            else
                *p1.u8 = (uint8_t)scratch;
            break;
        case AND:
            regs->f |= FLAG_H;
            regs->f &= ~(FLAG_N | FLAG_C);

            *p1.u8 &= *p2.u8;

            if(*p1.u8 == 0)
                regs->f |= FLAG_Z;
            else
                regs->f &= ~FLAG_Z;
            break;
        case OR:
            regs->f &= ~(FLAG_N | FLAG_H | FLAG_C);

            *p1.u8 |= *p2.u8;

            if(*p1.u8 == 0)
                regs->f |= FLAG_Z;
            else
                regs->f &= ~FLAG_Z;
            break;
        case XOR:
            regs->f &= ~(FLAG_N | FLAG_H | FLAG_C);

            *p1.u8 ^= *p2.u8;

            if(*p1.u8 == 0)
                regs->f |= FLAG_Z;
            else
                regs->f &= ~FLAG_Z;
            break;
        case CP:
            regs->f |= FLAG_N;

            if((*p1.u8 & 0xF) < (*p2.u8 & 0xF))
                regs->f |= FLAG_H;
            else
                regs->f &= ~FLAG_H;

            scratch = *p1.u8 - *p2.u8;

            if(scratch < 0)
                regs->f |= FLAG_C;
            else
                regs->f &= ~FLAG_C;

            if(scratch == 0)
                regs->f |= FLAG_Z;
            else
                regs->f &= ~FLAG_Z;

            break;
        case RLCA:
            regs->f &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->a << 1;
            if(regs->a & 0x80) {
                regs->f |= FLAG_C;
                scratch |= 0x01;
            } else {
                regs->f &= ~FLAG_C;
                scratch &= ~0x01;
            }

            regs->a = scratch;
            break;
        case RRCA:
            regs->f &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->a >> 1;
            if(regs->a & 0x01) {
                regs->f |= FLAG_C;
                scratch |= 0x80;
            } else {
                regs->f &= ~FLAG_C;
                scratch &= ~0x80;
            }

            regs->a = scratch;
            break;
        case RLA:
            regs->f &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->a << 1;

            if(regs->f & FLAG_C)
                scratch |= 0x01;
            else
                scratch &= ~0x01;

            if(regs->a & 0x80)
                regs->f |= FLAG_C;
            else
                regs->f &= ~FLAG_C;

            regs->a = scratch;
            break;
        case RRA:
            regs->f &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->a >> 1;

            if(regs->f & FLAG_C)
                scratch |= 0x80;
            else
                scratch &= ~0x80;

            if(regs->a & 0x01)
                regs->f |= FLAG_C;
            else
                regs->f &= ~FLAG_C;

            regs->a = scratch;
            break;
        case DAA:
            scratch = regs->a;
            if(regs->f & FLAG_N) {
                if(regs->f & FLAG_H)
                    scratch -= 0x06;

                if(regs->f & FLAG_C)
                    scratch -= 0x60;
            } else {
                if((scratch & 0x0F) > 0x09 || regs->f & FLAG_H)
                    scratch += 0x06;

                if(scratch > 0x99 || regs->f & FLAG_C) {
                    scratch += 0x60;
                    regs->f |= FLAG_C;
                }

            }

            if(scratch == 0)
                regs->f |= FLAG_Z;
            else
                regs->f &= ~FLAG_Z;
            
            regs->f &= ~FLAG_H;

            regs->a = scratch;
            break;
        case CPL:
            regs->a = ~regs->a;

            regs->f |= FLAG_N | FLAG_H;
            break;
        case SCF:
            regs->f |= FLAG_C;
            regs->f &= ~(FLAG_N | FLAG_H);
            break;
        case CCF:
            regs->f ^= FLAG_C;
            regs->f &= ~(FLAG_N | FLAG_H);
            break;
        case JR:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->f & FLAG_Z))
                        regs->pc += *p2.s8;
                    break;
                case F_NC:
                    if(!(regs->f & FLAG_C))
                        regs->pc += *p2.s8;
                    break;
                case F_Z:
                    if(regs->f & FLAG_Z)
                        regs->pc += *p2.s8;
                    break;
                case F_C:
                    if(regs->f & FLAG_C)
                        regs->pc += *p2.s8;
                    break;
                default:
                    regs->pc += *p1.s8;
                    break;
            }
            break;
        case JP:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->f & FLAG_Z))
                        regs->pc = *p2.u16;
                    break;
                case F_NC:
                    if(!(regs->f & FLAG_C))
                        regs->pc = *p2.u16;
                    break;
                case F_Z:
                    if(regs->f & FLAG_Z)
                        regs->pc = *p2.u16;
                    break;
                case F_C:
                    if(regs->f & FLAG_C)
                        regs->pc = *p2.u16;
                    break;
                default:
                    regs->pc = *p1.u16;
                    break;
            }
            break;
        case CALL:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->f & FLAG_Z))
                        goto _do_call;
                    break;
                case F_NC:
                    if(!(regs->f & FLAG_C))
                        goto _do_call;
                    break;
                case F_Z:
                    if(regs->f & FLAG_Z)
                        goto _do_call;
                    break;
                case F_C:
                    if(regs->f & FLAG_C)
                        goto _do_call;
                    break;
                default:
                    regs->sp -= 2;
                    memwrite(regs->sp, regs->pc & 0xFF);
                    memwrite(regs->sp + 1, regs->pc >> 8);
                    regs->pc = *p1.u16;
                    break;
            }
            break;
_do_call:
            regs->sp -= 2;
            memwrite(regs->sp, regs->pc & 0xFF);
            memwrite(regs->sp + 1, regs->pc >> 8);
            regs->pc = *p2.u16;
            break;
        case RET:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->f & FLAG_Z))
                        goto _do_ret;
                    break;
                case F_NC:
                    if(!(regs->f & FLAG_C))
                        goto _do_ret;
                    break;
                case F_Z:
                    if(regs->f & FLAG_Z)
                        goto _do_ret;
                    break;
                case F_C:
                    if(regs->f & FLAG_C)
                        goto _do_ret;
                    break;
                default:
                    goto _do_ret;
                    break;
            }
            break;
_do_ret:
            regs->pc = memread(regs->sp) | (memread(regs->sp + 1) << 8);
            regs->sp += 2;
            break;
        case POP:
            *p1.u16 = memread(regs->sp) | (memread(regs->sp + 1) << 8);
            regs->sp += 2;
            break;
        case PUSH:
            regs->sp -= 2;
            memwrite(regs->sp, *p1.u16 & 0xFF);
            memwrite(regs->sp + 1, *p1.u16 >> 8);
            break;
        case DI:
            regs->ime = false;
            break;
        case EI:
            regs->ime = true;
            break;
        case RETI:
            regs->pc = memread(regs->sp) | (memread(regs->sp + 1) << 8);
            regs->sp += 2;
            regs->ime = true;
            break;
        case RST:
            regs->sp -= 2;
            memwrite(regs->sp, regs->pc & 0xFF);
            memwrite(regs->sp + 1, regs->pc >> 8);
            regs->pc = *p2.u16;
            break;
        case STOP:
            regs->state = S_STOP;
            break;
        case HALT:
            regs->state = S_HALT;
            break;
        case PREFIX:
            pfx = true;
            break;
        default:
            break;
    }
}

enum int_status {
    INT_NONE,
    INT_PENDING,
    INT_TRIGGERED
};

enum int_status run_int(struct registers* regs) {
    uint8_t in = memread(REG_IE) & memread(REG_IF);
    if(in) {
        if(regs->ime) {
            uint8_t bit = in & -in;
            uint8_t handler = 0x40 + ((bit - 1) << 3);

            regs->ime = false;
            memwrite(REG_IF, memread(REG_IF) & ~bit);

            regs->sp -= 2;
            memwrite(regs->sp, regs->pc & 0xFF);
            memwrite(regs->sp + 1, regs->pc >> 8);
            regs->pc = handler;

            return INT_TRIGGERED;
        } else {
            return INT_PENDING;
        }
    }
    return INT_NONE;
}

void run_cpu(struct registers* regs) {
    enum int_status in = run_int(regs);
    switch(regs->state) {
        case S_RUN:
            run_insn(regs);
            break;
        case S_STOP:
            // TODO: wait for joypad button
            break;
        case S_HALT:
            if(in == INT_TRIGGERED) {
                regs->state = S_RUN;
            } else if(in == INT_PENDING) {
                regs->state = S_RUN; // TODO: HALT bug
            }
            break;
    }
}
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
        uint16_t addr;
    };
    enum ptr_type{
        NUL,
        U8,
        U16,
        S8,
        S16,
    } type;
};

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
            break;
        case A_BC:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->bc);
            res.type = U8;
            break;
        case A_DE:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->de);
            res.type = U8;
            break;
        case A_HL:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->hl);
            res.type = U8;
            break;
        case A_HLI:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->hl);
            regs->hl++;
            res.type = U8;
            break;
        case A_HLD:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(regs->hl);
            regs->hl--;
            res.type = U8;
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
            *res.u8 = memread(
                memread(regs->pc) | (memread(regs->pc + 1) << 8)
            );
            res.type = U8;
            regs->pc += 2;
            break;
        case A_IMM8:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(memread(regs->pc++));
            res.type = U8;
            break;
        case AH_IMM8:
            res.u8 = (uint8_t*)&tmp;
            *res.u8 = memread(memread(regs->pc++) + 0xFF00);
            res.type = U8;
            break;
        default:
            res = (struct typed_ptr) {0};
            break;
    }
    return res;
}

void run_insn(struct registers* regs) {
    uint8_t opcode = memread(regs->pc++);
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
    switch(i.op) {
        case NOP:
            break;
        case LD:
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
            break;
        case INC:
            switch(p1.type){
                case U8:
                case S8:
                    regs->af &= ~FLAG_N;
                    if(*p1.u8 == 0xF) regs->af |= FLAG_H;
                    else regs->af &= ~FLAG_H;

                    (*p1.u8)++;

                    if(*p1.u8 == 0) regs->af |= FLAG_Z;
                    else regs->af &= ~FLAG_Z;

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
                    regs->af |= FLAG_N;
                    if(*p1.u8 == 0) regs->af |= FLAG_H;
                    else regs->af &= ~FLAG_H;

                    (*p1.u8)--;

                    if(*p1.u8 == 0) regs->af |= FLAG_Z;
                    else regs->af &= ~FLAG_Z;

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
            regs->af &= ~FLAG_N;

            if(i.op == ADC) {
                if((*p1.u8 & 0xF) + (*p2.u8 & 0xF) +
                    (regs->af & FLAG_C) ? 1 : 0 > 0xF)
                    regs->af |= FLAG_H;
                else
                    regs->af &= ~FLAG_H;
            } else {
                if((*p1.u8 & 0xF) + (*p2.u8 & 0xF) > 0xF)
                    regs->af |= FLAG_H;
                else
                    regs->af &= ~FLAG_H;
            }
            switch(p1.type) {
                case U8:
                    if(i.op == ADC)
                        scratch = *p1.u8 + *p2.u8 + 
                            (regs->af & FLAG_C) ? 1 : 0;
                    else
                        scratch = *p1.u8 + *p2.u8;

                    if(scratch > 0xFF)
                        regs->af |= FLAG_C;
                    else
                        regs->af &= ~FLAG_C;

                    if(scratch == 0)
                        regs->af |= FLAG_Z;
                    else
                        regs->af &= ~FLAG_Z;

                    *p1.u8 = (uint8_t)scratch;
                    break;
                case U16:
                    if(p2.type == S8) {
                        scratch = *p1.u16 + *p2.s8;
                        regs->af &= ~FLAG_Z;
                    } else {
                        scratch = *p1.u16 + *p2.u16;
                    }

                    if(scratch > 0xFFFF)
                        regs->af |= FLAG_C;
                    else
                        regs->af &= ~FLAG_C;

                    *p1.u16 = (uint16_t)scratch;
                    break;
                default:
                    break;
            }
            break;
        case SBC:
        case SUB:
            regs->af |= FLAG_N;

            if(i.op == SBC) {
                if((*p1.u8 & 0xF) < (*p2.u8 & 0xF) +
                    (regs->af & FLAG_C) ? 1 : 0 > 0xF)
                    regs->af |= FLAG_H;
                else
                    regs->af &= ~FLAG_H;
            } else {
                if((*p1.u8 & 0xF) < (*p2.u8 & 0xF))
                    regs->af |= FLAG_H;
                else
                    regs->af &= ~FLAG_H;
            }
            if(i.op == SBC)
                scratch = *p1.u8 - (*p2.u8 + 
                    (regs->af & FLAG_C) ? 1 : 0);
            else
                scratch = *p1.u8 - *p2.u8;

            if(scratch < 0)
                regs->af |= FLAG_C;
            else
                regs->af &= ~FLAG_C;

            if(scratch == 0)
                regs->af |= FLAG_Z;
            else
                regs->af &= ~FLAG_Z;

            *p1.u8 = (uint8_t)scratch;
            break;
        case AND:
            regs->af |= FLAG_H;
            regs->af &= ~(FLAG_N | FLAG_C);

            *p1.u8 &= *p2.u8;

            if(*p1.u8 == 0)
                regs->af |= FLAG_Z;
            else
                regs->af &= ~FLAG_Z;
            break;
        case OR:
            regs->af &= ~(FLAG_N | FLAG_H | FLAG_C);

            *p1.u8 |= *p2.u8;

            if(*p1.u8 == 0)
                regs->af |= FLAG_Z;
            else
                regs->af &= ~FLAG_Z;
            break;
        case XOR:
            regs->af &= ~(FLAG_N | FLAG_H | FLAG_C);

            *p1.u8 ^= *p2.u8;

            if(*p1.u8 == 0)
                regs->af |= FLAG_Z;
            else
                regs->af &= ~FLAG_Z;
            break;
        case CP:
            regs->af |= FLAG_N;

            if((*p1.u8 & 0xF) < (*p2.u8 & 0xF))
                regs->af |= FLAG_H;
            else
                regs->af &= ~FLAG_H;

            scratch = *p1.u8 - *p2.u8;

            if(scratch < 0)
                regs->af |= FLAG_C;
            else
                regs->af &= ~FLAG_C;

            if(scratch == 0)
                regs->af |= FLAG_Z;
            else
                regs->af &= ~FLAG_Z;

            break;
        case RLCA:
            regs->af &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->af << 1;
            if(regs->af & 0x8000) {
                regs->af |= FLAG_C;
                scratch |= 0x0100;
            } else {
                regs->af &= ~FLAG_C;
                scratch &= ~0x0100;
            }

            regs->af &= 0x00FF;
            regs->af |= (scratch << 8) & 0xFF00;
            break;
        case RRCA:
            regs->af &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->af >> 1;
            if(regs->af & 0x0100) {
                regs->af |= FLAG_C;
                scratch |= 0x8000;
            } else {
                regs->af &= ~FLAG_C;
                scratch &= ~0x8000;
            }

            regs->af &= 0x00FF;
            regs->af |= (scratch << 8) & 0xFF00;
            break;
        case RLA:
            regs->af &= ~(FLAG_Z | FLAG_N | FLAG_H);
            scratch = regs->af << 1;
            if(regs->af & 0x8000)
                regs->af |= FLAG_C;
            else
                regs->af &= ~FLAG_C;

            regs->af &= 0x00FF;
            regs->af |= (scratch << 8) & 0xFF00;
            break;
        case RRA:
            regs->af &= ~(FLAG_Z | FLAG_N | FLAG_H);
            if(regs->af & 0x0100)
                regs->af |= FLAG_C;
            else
                regs->af &= ~FLAG_C;

            scratch = regs->af >> 1;
            regs->af &= 0x00FF;
            regs->af |= (scratch << 8) & 0xFF00;
            break;
        case DAA:
            scratch = regs->af >> 8;
            if((scratch & 0x0F) > 0x09 || regs->af & FLAG_H)
                scratch += 0x06;

            if((scratch & 0xF0) > 0x90 || regs->af & FLAG_C)
                scratch += 0x60;

            regs->af &= 0x00FF;
            regs->af |= scratch << 8;
            break;
        case CPL:
            scratch = regs->af >> 8;
            scratch = ~scratch;
            regs->af &= 0x00FF;
            regs->af |= scratch << 8;

            regs->af |= FLAG_N | FLAG_H;
            break;
        case SCF:
            regs->af |= FLAG_C;
            regs->af &= ~(FLAG_N | FLAG_H);
            break;
        case CCF:
            if(regs->af & FLAG_C)
                regs->af &= ~FLAG_C;
            else
                regs->af |= FLAG_C;
            break;
        case JR:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->af & FLAG_Z))
                        regs->pc += *p2.s8;
                    break;
                case F_NC:
                    if(!(regs->af & FLAG_C))
                        regs->pc += *p2.s8;
                    break;
                case F_Z:
                    if(regs->af & FLAG_Z)
                        regs->pc += *p2.s8;
                    break;
                case F_C:
                    if(regs->af & FLAG_C)
                        regs->pc += *p2.s8;
                    break;
                default:
                    break;
            }
            break;
        case JP:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->af & FLAG_Z))
                        regs->pc = *p2.u16;
                    break;
                case F_NC:
                    if(!(regs->af & FLAG_C))
                        regs->pc = *p2.u16;
                    break;
                case F_Z:
                    if(regs->af & FLAG_Z)
                        regs->pc = *p2.u16;
                    break;
                case F_C:
                    if(regs->af & FLAG_C)
                        regs->pc = *p2.u16;
                    break;
                default:
                    break;
            }
            break;
        case CALL:
            switch(i.p1) {
                case F_NZ:
                    if(!(regs->af & FLAG_Z))
                        goto _do_call;
                    break;
                case F_NC:
                    if(!(regs->af & FLAG_C))
                        goto _do_call;
                    break;
                case F_Z:
                    if(regs->af & FLAG_Z)
                        goto _do_call;
                    break;
                case F_C:
                    if(regs->af & FLAG_C)
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
                    if(!(regs->af & FLAG_Z))
                        goto _do_ret;
                    break;
                case F_NC:
                    if(!(regs->af & FLAG_C))
                        goto _do_ret;
                    break;
                case F_Z:
                    if(regs->af & FLAG_Z)
                        goto _do_ret;
                    break;
                case F_C:
                    if(regs->af & FLAG_C)
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
        default:
            break;
    }
    if(i.op != NOP)
        printf(
            "bc: %04x de: %04x hl: %04x af: %04x sp: %04x pc %04x ime: %d state: %d\r",
            regs->bc, regs->de, regs->hl, regs->af, regs->sp, regs->pc, regs->ime, regs->state
        );
}

void poweron(struct registers* regs) {
    // NOTE: skipping boot ROM
    *regs = (struct registers) {
        .bc = 0x0013,
        .de = 0x00D8,
        .hl = 0x014D,
        .af = 0x01B0,
        .sp = 0xFFFE,
        .pc = 0x0100,
    };
    init_regs();
    memwrite(REG_BANK, 0x01);
    memwrite(REG_DIV, 0xAB);
    memwrite(REG_P1, 0xCF);
}

void run(struct registers* regs) {
    while(1) {
        switch(regs->state) {
            case S_RUN:
                run_insn(regs);
                break;
            case S_STOP:
                // TODO: wait for joypad button
                break;
            case S_HALT:
                // TODO: wait for interrupt
                break;
        }
    }
}
#include <stdint.h>
#include <timer.h>
#include "memmap.h"

uint8_t
    reg_div, reg_tima,
    reg_tma, reg_tac;

static struct timer div_timer, tima_timer;

void run_timer() {
    if(timer_wait(&div_timer)) {
        timer_start(&div_timer, 256);
        reg_div++;
    }
    if(timer_wait(&tima_timer)) {
        unsigned int ticks;
        switch(reg_tac & 0x03) {
            case 0: ticks = 1024; break;
            case 1: ticks = 16; break;
            case 2: ticks = 64; break;
            case 3: ticks = 256; break;
        }
        timer_start(&tima_timer, ticks);
        if(reg_tac & 0x04) {
            if(reg_tima == 0xFF) {
                reg_tima = reg_tma;
                memwrite(REG_IF, memread(REG_IF) | 0x04);
            } else reg_tima++;
        }
    }
}
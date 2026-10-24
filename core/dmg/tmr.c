#include <stdint.h>
#include <timer.h>
#include "memmap.h"

uint16_t tmr_ctr;
uint8_t reg_tima, reg_tma, reg_tac;
bool wrote_tima;

static uint8_t prev_signal;
static uint16_t tac_bit[4] = {1 << 9, 1 << 3, 1 << 5, 1 << 7};

static struct timer reload_timer;

void tmr_do_edge() {
    uint8_t signal = (reg_tac & 0x04) ? (tmr_ctr & tac_bit[reg_tac & 0x03]) : 0;
    if(!signal && prev_signal) {
        if(++reg_tima == 0) timer_start(&reload_timer, 4);
    }
    prev_signal = signal;
}

void run_tmr() {
    if(timer_wait(&reload_timer)) {
        reg_tima = reg_tma;
        memwrite(REG_IF, memread(REG_IF) | 0x04);
    }
    tmr_ctr++;
    tmr_do_edge();
    if(wrote_tima) {
        timer_cancel(&reload_timer);
        wrote_tima = false;
    }
}
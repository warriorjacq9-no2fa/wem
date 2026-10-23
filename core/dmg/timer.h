#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern uint8_t
    reg_div, reg_tima,
    reg_tma, reg_tac;

void run_timer();

#endif
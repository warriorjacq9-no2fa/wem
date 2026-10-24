#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

extern uint16_t tmr_ctr;
extern uint8_t reg_tima, reg_tma, reg_tac;
extern bool wrote_tima;

void run_timer();
void tmr_do_edge();

#endif
#ifndef APU_H
#define APU_H

#include <stdint.h>

extern uint8_t
    reg_nr50, reg_nr51, reg_nr52,
    reg_nr10, reg_nr11, reg_nr12, reg_nr13, reg_nr14,
    reg_nr21, reg_nr22, reg_nr23, reg_nr24,
    reg_nr30, reg_nr31, reg_nr32, reg_nr33, reg_nr34,
    reg_nr41, reg_nr42, reg_nr43, reg_nr44;

void init_apu();
void run_apu();

#endif
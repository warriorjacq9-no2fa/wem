#ifndef APU_H
#define APU_H

#include <stdbool.h>
#include <stdint.h>

extern uint8_t
    reg_nr50, reg_nr51, reg_nr52,
    reg_nr10, reg_nr11, reg_nr12, reg_nr13, reg_nr14,
    reg_nr21, reg_nr22, reg_nr23, reg_nr24,
    reg_nr30, reg_nr31, reg_nr32, reg_nr33, reg_nr34,
    reg_nr41, reg_nr42, reg_nr43, reg_nr44;
extern uint8_t waveram[16];
extern bool
    wrote_nr14, wrote_nr24, wrote_nr34, wrote_nr44;

void apu_power_on();
void apu_power_off();
void init_apu();
void run_apu(void (*do_sample)(int8_t, int8_t));

#endif
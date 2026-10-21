#include <time.h>
#include "cpu.h"
#include "memmap.h"
#include "ppu.h"
#include "apu.h"

#define CLOCK_SPEED_HZ 4194304

static clock_t last_time;

void poweron(struct registers* regs) {
    *regs = (struct registers) {0};
    init_regs();
    init_ppu();
    init_apu();
}

void run(struct registers* regs, uint32_t* pixels, void (*do_sample)(int8_t)) {
    last_time = clock();
    while(1) {
        clock_t now = clock();
        double elapsed = (double)(now - last_time) / CLOCKS_PER_SEC;
        unsigned int elapsed_cycles = (unsigned int)(elapsed * CLOCK_SPEED_HZ);
        if(elapsed_cycles > 0) {
            for(unsigned int i = 0; i < elapsed_cycles; i++) {
                run_cpu(regs);
                run_ppu(pixels);
                run_apu(do_sample);
            }
            last_time = now;
        }
    }
}
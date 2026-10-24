#include "core.h"
#include <time.h>
#include "cpu.h"
#include "memmap.h"
#include "ppu.h"
#include "apu.h"
#include "tmr.h"

static uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

void poweron(struct registers* regs) {
    *regs = (struct registers) {0};
    init_regs();
    init_ppu();
    init_apu();
}

void run(struct registers* regs, uint32_t* pixels, void (*do_sample)(int8_t, int8_t)) {
    uint64_t last = now_ns();
    uint64_t frac = 0;

    while(1) {
        uint64_t now = now_ns();
        frac += (now - last) * CPU_HZ;
        last = now;

        uint64_t cycles = frac / 1000000000ull;
        frac %= 1000000000ull;

        if(cycles > CPU_HZ / 20) cycles = CPU_HZ / 20;

        for(uint64_t i = 0; i < cycles; i++) {
            run_tmr();
            run_cpu(regs);
            run_ppu(pixels);
            run_apu(do_sample);
        }
    }
}
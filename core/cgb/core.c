#include "cpu.h"
#include "memmap.h"
#include "ppu.h"

void poweron(struct registers* regs) {
    *regs = (struct registers) {0};
    init_regs();
    init_ppu();
}

void run(struct registers* regs, uint32_t* pixels) {
    while(1) {
        run_cpu(regs);
        run_ppu(pixels);
    }
}
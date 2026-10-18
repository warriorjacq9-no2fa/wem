#include "cpu.h"
#include "memmap.h"
#include "ppu.h"

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
    init_ppu();
}

void run(struct registers* regs, uint32_t* pixels) {
    while(1) {
        run_cpu(regs);
        run_ppu(pixels);
    }
}
#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include "../cpu.h"

#define ROM_SZ 32768
#define MEM_SZ 65536

void load_rom(uint8_t* data);
void poweron(struct registers* regs);
void run(struct registers* regs, uint32_t* pixels);

#endif
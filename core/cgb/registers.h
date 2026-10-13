#ifndef REGS_H
#define REGS_H

#include <stdint.h>

#define REG_BANK        0xFF50

#define REG_DMA         0xFF46

#define REG_PPU_LCDC    0xFF40
#define REG_PPU_STAT    0xFF41
#define REG_PPU_SCY     0xFF42
#define REG_PPU_SCX     0xFF43
#define REG_PPU_LY      0xFF44
#define REG_PPU_LYC     0xFF45

#define REG_P1          0xFF00

#define REG_SB          0xFF01
#define REG_SC          0xFF02

uint8_t read(uint16_t addr);
void write(uint16_t addr, uint8_t val);

#endif
#ifndef MEMMAP_H
#define MEMMAP_H

#include <stdint.h>

#define REG_P1          0xFF00

#define REG_SB          0xFF01
#define REG_SC          0xFF02

#define REG_DIV         0xFF04
#define REG_TIMA        0xFF05
#define REG_TMA         0xFF06
#define REG_TAC         0xFF07

#define REG_IF          0xFF0F

#define REG_PPU_LCDC    0xFF40
#define REG_PPU_STAT    0xFF41
#define REG_PPU_SCY     0xFF42
#define REG_PPU_SCX     0xFF43
#define REG_PPU_LY      0xFF44
#define REG_PPU_LYC     0xFF45

#define REG_DMA         0xFF46

#define REG_PPU_BGP     0xFF47
#define REG_PPU_OBP0    0xFF48
#define REG_PPU_OBP1    0xFF49
#define REG_PPU_WY      0xFF4A
#define REG_PPU_WX      0xFF4B

#define REG_BANK        0xFF50

#define REG_IE          0xFFFF

void init_regs();
uint8_t memread(uint16_t addr);
void memwrite(uint16_t addr, uint8_t val);

#endif
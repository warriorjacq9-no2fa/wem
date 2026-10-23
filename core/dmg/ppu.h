#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t
    reg_ppu_lcdc, reg_ppu_stat,
    reg_ppu_scy, reg_ppu_scx,
    reg_ppu_ly, reg_ppu_lyc,
    reg_ppu_bgp, reg_ppu_obp0, reg_ppu_obp1,
    reg_ppu_wy, reg_ppu_wx;
extern uint8_t vram[];
extern uint8_t oam[];

extern bool oam_block, vram_block;

void init_ppu();
void run_ppu(uint32_t* pixels);

#endif
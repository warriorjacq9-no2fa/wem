#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t
    reg_ppu_lcdc, reg_ppu_stat,
    reg_ppu_scy, reg_ppu_scx,
    reg_ppu_ly, reg_ppu_lyc,
    reg_ppu_bgp, reg_ppu_obp0, reg_ppu_obp1;

extern bool oam_block, vram_block;

#endif
#include "memmap.h"
#include <stdint.h>
#include <stdbool.h>
#include <immintrin.h>

#define S_HBLANK    0x00
#define S_VBLANK    0x01
#define S_OAM       0x02
#define S_DRAW      0x03

static uint8_t ly;
static uint16_t dot;
static uint16_t sprites[10];
static uint8_t bg_fifo[8];
static uint8_t spr_fifo[8];

static uint8_t
    reg_ppu_lcdc, reg_ppu_stat,
    reg_ppu_scy, reg_ppu_scx,
    reg_ppu_ly, reg_ppu_lyc,
    reg_ppu_bgp, reg_ppu_obp0, reg_ppu_obp1;

static bool oam_block, vram_block;

void init_ppu() {
    
}
static uint8_t idx, i;
void run_ppu(uint32_t* pixels) {
    if(!(reg_ppu_lcdc & 0x80)) {
        reg_ppu_stat &= ~0x03;
        reg_ppu_stat |= S_HBLANK;
        ly = 0;
        dot = 0;
        return;
    }
    switch(reg_ppu_stat & 0x03) {
        case S_OAM:
            oam_block = true;
            vram_block = false;
            if(dot >= 80) {
                reg_ppu_stat &= ~0x03;
                reg_ppu_stat |= S_DRAW;
                break;
            }
            if(dot == 0) {
                idx = 0;
                i = 0;
            } else if(dot % 2 == 1) {
                uint8_t y = memread(0xFE00 + i * 4);
                uint8_t height = (reg_ppu_lcdc & 0x04) ? 16 : 8;
                if(ly + 16 >= y && ly + 16 < y + height) {
                    sprites[idx] = 0xFE00 + i * 4;
                    idx++;
                    if(idx == 10) break;
                }
                i++;
            }
            break;
        case S_DRAW:
            if(dot >= 252) {
                reg_ppu_stat &= ~0x03;
                reg_ppu_stat |= S_HBLANK;
            }
            break;
        case S_HBLANK:
            if(dot >= 456) {
                // Write line
                dot = 0;
                ly++;
                if(ly == 144) {
                    // TODO: VBLANK IRQ
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_VBLANK;
                } else {
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                }
            }
            break;
        case S_VBLANK:
            if(dot >= 456) {
                dot = 0;
                ly++;
                if(ly == 154) {
                    // Write sprites, draw, reset FIFO
                    ly = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                }
            }
            break;
    }
    dot++;
}
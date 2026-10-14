#include "memmap.h"
#include <stdint.h>
#include <immintrin.h>

#define S_HBLANK    0x00
#define S_VBLANK    0x01
#define S_OAM       0x02
#define S_DRAW      0x03

static uint8_t line;
static uint16_t dot;
static uint16_t sprites[10];
static uint32_t fifo;

static uint8_t
    reg_ppu_lcdc, reg_ppu_stat,
    reg_ppu_scy, reg_ppu_scx,
    reg_ppu_ly, reg_ppu_lyc,
    reg_ppu_bgp, reg_ppu_obp0, reg_ppu_obp1;

void init_ppu() {
    
}

void run_ppu(uint32_t* pixels) {
    switch(reg_ppu_stat & 0x03) {
        case S_OAM:
            if(dot >= 80) {
                reg_ppu_stat &= ~0x03;
                reg_ppu_stat |= S_DRAW;
            }
            uint8_t idx = 0;
            for(int i = 0; i < 40; i++) {
                uint8_t y = memread(0xFE00 + i * 4) - 16;
                uint8_t height = (reg_ppu_lcdc & 0x04) ? 16 : 8;
                if(line >= y && line < y + height) {
                    sprites[idx] = 0xFE00 + i * 4;
                    idx++;
                    if(idx >= 10) break;
                }
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
                dot = 0;
                line++;
                if(line >= 144) {
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
                line++;
                if(line >= 154) {
                    line = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                }
            }
            break;
    }
    dot++;
}
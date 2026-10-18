#include "memmap.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define S_HBLANK    0x00
#define S_VBLANK    0x01
#define S_OAM       0x02
#define S_DRAW      0x03

/* LCDC register flags */
#define LCDC_EN         0x80 // Enable display
#define LCDC_WINTL      0x40 // Window tilemap select, 1: 0x9C00, 0: 0x9800
#define LCDC_WINEN      0x20 // Enable window
#define LCDC_TILEADDR   0x10 // Tile address mode, 1: 8000 method, 0: 8800 method
#define LCDC_BGTL       0x08 // Background tilemap select, 1: 0x9C00, 0: 0x9800
#define LCDC_SPRSZ      0x04 // Sprite size, 1: 1x2 tiles, 0: 1x1 tiles
#define LCDC_SPREN      0x02 // Enable sprites
#define LCDC_BGEN       0x01 // Enable background and window

/* Stat register flags */
#define STAT_INT_LYC    0x40
#define STAT_INT_OAM    0x20
#define STAT_INT_VBLANK 0x10
#define STAT_INT_HBLANK 0x08
#define STAT_LYC        0x04

/* Sprite flag bitmasks */
#define SF_PRIORITY 0x80 // 0: sprite above background, 1: background colors 1-3 overlay sprite
#define SF_YFLIP    0x40 // If set, flip sprite vertically
#define SF_XFLIP    0x20 // If set, flip sprite horizontally

static uint16_t dot;
static uint16_t sprites[10];
static uint16_t bg_fifo;
//static uint16_t spr_fifo;

uint8_t
    reg_ppu_lcdc, reg_ppu_stat,
    reg_ppu_scy, reg_ppu_scx,
    reg_ppu_ly, reg_ppu_lyc,
    reg_ppu_bgp, reg_ppu_obp0, reg_ppu_obp1;

bool oam_block, vram_block;

enum ppu_bgfetch {
    BGF_TILE,
    BGF_TILELO,
    BGF_TILEHI,
    BGF_PUSH,
} bgfetch_state;

void init_ppu() {
}

uint16_t get_tile(uint8_t tile) {
    if(reg_ppu_lcdc & LCDC_TILEADDR) {
        return 0x8000 + tile * 0x10;
    } else {
        return 0x9000 + (int8_t)tile * 0x10;
    }
}

uint16_t get_sprite_tile(uint8_t tile) {
    return 0x8000 + tile * 0x10;
}

uint16_t intl(uint8_t x, uint8_t y) {
    uint16_t res = 0;
    for (int i = 0; i < 8; i++) {
        res |= ((x & (1 << i)) << i);
        res |= ((y & (1 << i)) << (i + 1));
    }
    return res;
}

static uint8_t tile, tilehi, tilelo;
static uint8_t idx, xpos, wline;
static bool window;
void run_ppu(uint32_t* pixels) {
    if(!(reg_ppu_lcdc & LCDC_EN)) {
        reg_ppu_stat &= ~0x03;
        reg_ppu_stat |= S_HBLANK;
        reg_ppu_ly = 0;
        dot = 0;
#if DEBUG
    fprintf(stderr, "PPU disabled, dot: %d line: %d state: %d fifo: %04x\n",
        dot, reg_ppu_ly, reg_ppu_stat & 0x03, bg_fifo
    );
#endif
        return;
    }
    switch(reg_ppu_stat & 0x03) {
        case S_OAM:
            oam_block = true;
            vram_block = false;
            if(dot >= 80) {
                xpos = 0;
                reg_ppu_stat &= ~0x03;
                reg_ppu_stat |= S_DRAW;
                break;
            }
            if(dot % 2 == 0 && idx < 10) {
                uint8_t y = memread(0xFE00 + dot * 2);
                uint8_t x = memread(0xFE00 + dot * 2 + 1);
                uint8_t height = (reg_ppu_lcdc & LCDC_SPRSZ) ? 16 : 8;
                if(x >= 8 && reg_ppu_ly + 16 >= y && reg_ppu_ly + 16 < y + height) {
                    sprites[idx] = 0xFE00 + dot * 2;
                    idx++;
                }
            }
            break;
        case S_DRAW:
            oam_block = true;
            vram_block = true;
            switch(bgfetch_state) {
                case BGF_TILE:
                    if(!window) {
                        tile = memread((reg_ppu_lcdc & LCDC_BGTL ? 0x9C00 : 0x9800) + 
                            ((xpos + (reg_ppu_scx / 8 & 0x1F)) & 0x3FF) +
                            (32 * (((reg_ppu_ly + reg_ppu_scy) & 0xFF) / 8) & 0x3FF)
                        );
                    } else {
                        tile = memread((reg_ppu_lcdc & LCDC_WINTL ? 0x9C00 : 0x9800) + 
                            (xpos & 0x3FF) +
                            (32 * (wline / 8) & 0x3FF)
                        );
                    }
                    bgfetch_state = BGF_TILELO;
                    break;
                case BGF_TILELO:
                    tilelo = memread(get_tile(tile) + (window ?
                        2 * (wline % 8) :
                        2 * ((reg_ppu_ly + reg_ppu_scy) % 8))
                    );
                    bgfetch_state = BGF_TILEHI;
                    break;
                case BGF_TILEHI:
                    tilehi = memread(get_tile(tile) + (window ?
                        2 * (wline % 8) :
                        2 * ((reg_ppu_ly + reg_ppu_scy) % 8))
                        + 1
                    );
                    break;
                case BGF_PUSH:
                    if(bg_fifo != 0) break;
                    uint16_t row = intl(tilelo, tilehi);
                    for(int i = 0; i < 8; i++) {
                        uint8_t p_idx = (row & (0x03 << (i * 2))) >> (i * 2);
                        uint8_t pix = (reg_ppu_bgp & (0x03 << p_idx * 2)) >> p_idx * 2;
                        bg_fifo |= (pix & 0x03) << i * 2;
                    }

                    xpos++;
                    if(xpos == 160) {
                        reg_ppu_stat &= ~0x03;
                        reg_ppu_stat |= S_HBLANK;
                        if(reg_ppu_stat & STAT_INT_HBLANK)
                            memwrite(REG_IF, memread(REG_IF) | 0x02);
                    }
                    break;
            }
            break;
        case S_HBLANK:
            oam_block = false;
            vram_block = false;
            if(dot >= 456) {
                dot = 0;
                reg_ppu_ly++;
                if(reg_ppu_ly == 144) {
                    memwrite(REG_IF, memread(REG_IF) | 0x01);
                    wline = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_VBLANK;
                    if(reg_ppu_stat & STAT_INT_VBLANK)
                        memwrite(REG_IF, memread(REG_IF) | 0x02);
                } else {
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                    if(reg_ppu_stat & STAT_INT_OAM)
                        memwrite(REG_IF, memread(REG_IF) | 0x02);
                }
            }
            break;
        case S_VBLANK:
            if(dot >= 456) {
                dot = 0;
                reg_ppu_ly++;
                if(reg_ppu_ly == 154) {
                    reg_ppu_ly = 0;
                    idx = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                    if(reg_ppu_stat & STAT_INT_OAM)
                        memwrite(REG_IF, memread(REG_IF) | 0x02);
                }
            }
            break;
    }
    if(reg_ppu_ly == reg_ppu_lyc) {
        reg_ppu_stat |= STAT_LYC;
        if(reg_ppu_stat & STAT_INT_LYC) {
            memwrite(REG_IF, memread(REG_IF) | 0x02);
        }
    } else {
        reg_ppu_stat &= ~STAT_LYC;
    }
    dot++;
    // Push FIFO to LCD
    if(bg_fifo != 0) {
        uint8_t pix = (bg_fifo & 0x03) * (256 / 4);
        bg_fifo >>= 2;
        uint8_t x = xpos * 8 + dot % 8;
        pixels[reg_ppu_ly * 144 + x] = 0 | pix | pix << 8 | pix << 16;
    }
#if DEBUG
    fprintf(stderr, "PPU dot: %d line: %d stat: %02x fifo: %04x\n",
        dot, reg_ppu_ly, reg_ppu_stat, bg_fifo
    );
#endif
}
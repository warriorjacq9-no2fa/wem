#include "memmap.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
#define SF_PALETTE  0x10 // 0: OBP0, 1: OBP1

enum pix_layer {
    PL_BKG,
    PL_WIN,
    PL_SPR,
};

struct pixel {
    bool present;
    enum pix_layer layer;
    uint8_t color_idx;
    uint8_t spr_flags;
};

static uint16_t dot;
static uint8_t sprites[10];
static struct pixel bg_fifo[8];
static struct pixel spr_fifo[8];

uint8_t
    reg_ppu_lcdc, reg_ppu_stat,
    reg_ppu_scy, reg_ppu_scx,
    reg_ppu_ly, reg_ppu_lyc,
    reg_ppu_bgp, reg_ppu_obp0, reg_ppu_obp1,
    reg_ppu_wy, reg_ppu_wx;
uint8_t vram[8192];
uint8_t oam[160];

bool oam_block, vram_block;

enum ppu_fetch {
    F_TILE,
    F_TILELO,
    F_TILEHI,
    F_PUSH,
} fetch_state;

void init_ppu() {
}

static inline uint16_t get_tile(uint8_t tile) {
    if(reg_ppu_lcdc & LCDC_TILEADDR) {
        return tile * 0x10;
    } else {
        return 0x1000 + (int8_t)tile * 0x10;
    }
}

static inline uint16_t get_sprite_tile(uint8_t tile) {
    return tile * 0x10;
}

uint16_t intl(uint8_t x, uint8_t y) {
    uint16_t res = 0;
    for (int i = 0; i < 8; i++) {
        res |= ((x & (1 << i)) << i);
        res |= ((y & (1 << i)) << (i + 1));
    }
    return res;
}

static inline int fifo_len(struct pixel* fifo) {
    for(int i = 7; i >= 0; i--) {
        if(fifo[i].present) return i + 1;
    }
    return 0;
}

static uint8_t tile, tilehi, tilelo;
static uint8_t idx, xpos, tpos, wline;
static uint8_t discard;
static bool window, did_window;
static bool wy_latch;
static bool sprite;
static uint8_t s_idx;
void run_ppu(uint32_t* pixels) {
    if(!(reg_ppu_lcdc & LCDC_EN)) {
        window = false; did_window = false;
        wy_latch = false;
        wline = 0;
        oam_block = false;
        vram_block = false;
        memset(bg_fifo, 0, sizeof(bg_fifo));
        memset(spr_fifo, 0, sizeof(spr_fifo));
        reg_ppu_stat &= ~0x03;
        reg_ppu_stat |= S_OAM;
        reg_ppu_ly = 0;
        idx = 0;
        dot = 0;
#if DEBUG
    fprintf(stderr, "PPU disabled, dot: %d line: %d state: %d fifo len: %d\n",
        dot, reg_ppu_ly, reg_ppu_stat & 0x03, fifo_len(bg_fifo)
    );
#endif
        return;
    }
    switch(reg_ppu_stat & 0x03) {
        case S_OAM:
            oam_block = true;
            vram_block = false;
            if(dot >= 80) {
                window = false; did_window = false;
                xpos = 0; tpos = 0;
                discard = reg_ppu_scx % 8;
                memset(bg_fifo, 0, sizeof(bg_fifo));
                memset(spr_fifo, 0, sizeof(spr_fifo));
                fetch_state = F_TILE;
                reg_ppu_stat &= ~0x03;
                reg_ppu_stat |= S_DRAW;
                break;
            }
            if(dot % 2 == 0 && idx < 10) {
                uint8_t y = oam[dot * 2];
                uint8_t height = (reg_ppu_lcdc & LCDC_SPRSZ) ? 16 : 8;
                if(reg_ppu_ly + 16 >= y && reg_ppu_ly + 16 < y + height) {
                    sprites[idx] = dot / 2;
                    idx++;
                }
            }
            break;
        case S_DRAW:
            oam_block = true;
            vram_block = true;
            if(xpos >= 160) {
                reg_ppu_stat &= ~0x03;
                reg_ppu_stat |= S_HBLANK;
                if(reg_ppu_stat & STAT_INT_HBLANK) {
#if DEBUG
                    fprintf(stderr, "PPU: triggered HBLANK interrupt\n");
#endif
                    memwrite(REG_IF, memread(REG_IF) | 0x02);
                }
                break;
            }
            switch(fetch_state) {
                case F_TILE:
                    if(sprite) {
                        tile = oam[s_idx * 4 + 2];
                    } else if(window) {
                        tile = vram[(reg_ppu_lcdc & LCDC_WINTL ? 0x1C00 : 0x1800) + 
                            (tpos & 0x1F) + 32 * ((wline / 8) & 0x1F)
                        ];
                    } else {
                        uint16_t col = (tpos + (reg_ppu_scx / 8 & 0x1F)) & 0x1F;
                        uint16_t row = (((reg_ppu_ly + reg_ppu_scy) & 0xFF) / 8) & 0x1F;
                        tile = vram[(reg_ppu_lcdc & LCDC_BGTL ? 0x1C00 : 0x1800) + 
                            col + 32 * row
                        ];
                    }
                    if(!sprite) tpos++;
                    fetch_state = F_TILELO;
                    break;
                case F_TILELO:
                    if(sprite) {
                        uint8_t row = reg_ppu_ly + 16 - oam[s_idx * 4];
                        if(oam[s_idx * 4 + 3] & SF_YFLIP) {
                            if(reg_ppu_lcdc & LCDC_SPRSZ) {
                                row = (16 - 1) + row;
                            } else {
                                row = (8 - 1) + row;
                            }
                        }
                        tilelo = vram[get_sprite_tile(row >= 8 ? tile + 1 : tile) + 2 * (row % 8)];
                    } else if(window) {
                        tilelo = vram[get_tile(tile) + 2 * (wline % 8)];
                    } else {
                        tilelo = vram[get_tile(tile) + 2 * ((reg_ppu_ly + reg_ppu_scy) % 8)];
                    }
                    fetch_state = F_TILEHI;
                    break;
                case F_TILEHI:
                    if(sprite) {
                        uint8_t row = reg_ppu_ly + 16 - oam[s_idx * 4];
                        if(oam[s_idx * 4 + 3] & SF_YFLIP) {
                            if(reg_ppu_lcdc & LCDC_SPRSZ) {
                                row = 15 - row;
                            } else {
                                row = 7 - row;
                            }
                        }
                        tilehi = vram[get_sprite_tile(row >= 8 ? tile + 1 : tile) + 2 * (row % 8) + 1];
                    } else if(window) {
                        tilehi = vram[get_tile(tile) + 2 * (wline % 8) + 1];
                    } else {
                        tilehi = vram[get_tile(tile) + 2 * ((reg_ppu_ly + reg_ppu_scy) % 8) + 1];
                    }
                    fetch_state = F_PUSH;
                    break;
                case F_PUSH:
                    if(fifo_len(bg_fifo) != 0) break;
                    uint16_t row = intl(tilelo, tilehi);
                    if(sprite) {
                        for(int i = 0; i < 8; i++) {
                            uint8_t c_idx = (row & (0x03 << (i * 2))) >> (i * 2);
                            struct pixel* entry;
                            if(oam[s_idx * 4 + 3] & SF_XFLIP)
                                entry = &spr_fifo[i];
                            else
                                entry = &spr_fifo[7 - i];

                            entry->color_idx = c_idx;
                            entry->present = true;
                            entry->layer = PL_SPR;
                            entry->spr_flags = oam[s_idx * 4 + 3];
                        }
                        sprite = false;
                    } else {
                        for(int i = 0; i < 8; i++) {
                            uint8_t c_idx = (row & (0x03 << (i * 2))) >> (i * 2);
                            bg_fifo[7 - i].color_idx = c_idx;
                            bg_fifo[7 - i].present = true;
                            bg_fifo[7 - i].layer = window ? PL_WIN : PL_BKG;
                        }
                    }

                    fetch_state = F_TILE;
                    break;
            }
            if(!sprite) {
                for(int i = 0; i < idx; i++) {
                    if(oam[sprites[i] * 4 + 1] <= xpos + 8) {
                        s_idx = sprites[i];
                        sprites[i] = 0;
                        for(int j = i; j < idx - 1; j++) {
                            sprites[j] = sprites[j + 1];
                        }
                        idx--;
                        fetch_state = F_TILE;
                        sprite = true;
                        break;
                    }
                }
            }
            break;
        case S_HBLANK:
            oam_block = false;
            vram_block = false;
            if(dot >= 456) {
                dot = 0;
                reg_ppu_ly++;
                if(did_window) wline++;
                if(reg_ppu_ly == 144) {
#if DEBUG
                    fprintf(stderr, "PPU: triggered VBLANK interrupt\n");
#endif
                    memwrite(REG_IF, memread(REG_IF) | 0x01);
                    wline = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_VBLANK;
                    if(reg_ppu_stat & STAT_INT_VBLANK) {
#if DEBUG
                        fprintf(stderr, "PPU: triggered VBLANK STAT interrupt\n");
#endif
                        memwrite(REG_IF, memread(REG_IF) | 0x02);
                    }
                } else {
                    idx = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                    if(reg_ppu_stat & STAT_INT_OAM) {
#if DEBUG
                        fprintf(stderr, "PPU: triggered OAM interrupt\n");
#endif
                        memwrite(REG_IF, memread(REG_IF) | 0x02);
                    }
                }
            }
            break;
        case S_VBLANK:
            wline = 0;
            wy_latch = false;
            oam_block = false;
            vram_block = false;
            if(dot >= 456) {
                dot = 0;
                reg_ppu_ly++;
                if(reg_ppu_ly == 154) {
                    reg_ppu_ly = 0;
                    idx = 0;
                    reg_ppu_stat &= ~0x03;
                    reg_ppu_stat |= S_OAM;
                    if(reg_ppu_stat & STAT_INT_OAM) {
#if DEBUG
            fprintf(stderr, "PPU: triggered OAM interrupt\n");
#endif
                        memwrite(REG_IF, memread(REG_IF) | 0x02);
                    }
                }
            }
            break;
    }
    if(!wy_latch && reg_ppu_wy == reg_ppu_ly) wy_latch = true; 
    if(reg_ppu_ly == reg_ppu_lyc && !(reg_ppu_stat & STAT_LYC)) {
        reg_ppu_stat |= STAT_LYC;
        if(reg_ppu_stat & STAT_INT_LYC) {
#if DEBUG
            fprintf(stderr, "PPU: triggered LYC interrupt\n");
#endif
            memwrite(REG_IF, memread(REG_IF) | 0x02);
        }
    } else if(reg_ppu_ly != reg_ppu_lyc) {
        reg_ppu_stat &= ~STAT_LYC;
    }
    dot++;
    // Push FIFO to LCD
    if(fifo_len(bg_fifo) != 0) {
        if(!window && discard > 0) {
            discard--;
        } else {
            uint8_t color = (reg_ppu_bgp & (0x03 << bg_fifo[0].color_idx * 2)) >> bg_fifo[0].color_idx * 2;
            uint8_t pix = 255 - (color & 0x03) * 85;
            if(fifo_len(spr_fifo) != 0 && !sprite) {
                if(
                    spr_fifo[0].color_idx != 0 && (
                        bg_fifo[0].color_idx == 0 ||
                        !(spr_fifo[0].spr_flags & SF_PRIORITY)
                    )
                ) {
                    uint8_t pallete = (spr_fifo[0].spr_flags & SF_PALETTE) ? reg_ppu_obp1 : reg_ppu_obp0;
                    color = (pallete & (0x03 << spr_fifo[0].color_idx * 2)) >> spr_fifo[0].color_idx * 2;

                    uint8_t pix = 255 - (color & 0x03) * 85;
                    pixels[reg_ppu_ly * 160 + xpos] = 0xFF000000 | pix | pix << 8 | pix << 16;
                } else {
                    switch(bg_fifo[0].layer) {
                        case PL_BKG:
                            if(reg_ppu_lcdc & LCDC_BGEN)
                                pixels[reg_ppu_ly * 160 + xpos] = 0xFF000000 | pix | pix << 8 | pix << 16;
                            break;
                        case PL_WIN:
                            if((reg_ppu_lcdc & LCDC_BGEN) && (reg_ppu_lcdc & LCDC_WINEN)) {
                                did_window = true;
                                pixels[reg_ppu_ly * 160 + xpos] = 0xFF000000 | pix | pix << 8 | pix << 16;
                            }
                            break;
                        default:
                            break;
                    }
                }
                for(int i = 1; i < 8; i++) {
                    spr_fifo[i-1] = spr_fifo[i];
                }
                spr_fifo[7].present = false;
            } else {
                switch(bg_fifo[0].layer) {
                    case PL_BKG:
                        if(reg_ppu_lcdc & LCDC_BGEN)
                            pixels[reg_ppu_ly * 160 + xpos] = 0xFF000000 | pix | pix << 8 | pix << 16;
                        break;
                    case PL_WIN:
                        if((reg_ppu_lcdc & LCDC_BGEN) && (reg_ppu_lcdc & LCDC_WINEN)) {
                            did_window = true;
                            pixels[reg_ppu_ly * 160 + xpos] = 0xFF000000 | pix | pix << 8 | pix << 16;
                        }
                        break;
                    default:
                        break;
                }
            }
            xpos++;
        }

        for(int i = 1; i < 8; i++) {
            bg_fifo[i-1] = bg_fifo[i];
        }
        bg_fifo[7].present = false;

        if(reg_ppu_lcdc & LCDC_WINEN) {
            int win_x = (int)reg_ppu_wx - 7;
            if(!window && wy_latch && xpos >= win_x) {
                window = true;
                fetch_state = F_TILE;
                tpos = 0;
                memset(bg_fifo, 0, sizeof(bg_fifo));
            }
        }
    }
#if DEBUG
    fprintf(stderr, "PPU dot: %d line: %d xpos: %d stat: %02x bgfifo len: %d sprfifo len: %d fetch: %d %s\n",
        dot, reg_ppu_ly, xpos, reg_ppu_stat, fifo_len(bg_fifo), fifo_len(spr_fifo), fetch_state, sprite ? "sprite" : (window ? "window" : "background")
    );
#endif
}
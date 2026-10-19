#include "memmap.h"
#include <stdio.h>
#include <string.h>
#include "ppu.h"

static uint8_t reg_p1;
static uint8_t reg_sb, reg_sc;
static uint8_t
    reg_div, reg_tima,
    reg_tma, reg_tac;
static uint8_t reg_if;
static uint8_t reg_bank;
static uint8_t reg_dma;
static uint8_t reg_ie;

static uint8_t rom_00[16384];
static uint8_t rom_nn[16384];
static uint8_t vram[8192];
static uint8_t eram[8192];
static uint8_t wram0[4096];
static uint8_t wram1[4096];
static uint8_t oam[160];
static uint8_t hram[128];

static const uint8_t dmg_boot[]  = {
  0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 
  0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 
  0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 
  0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 
  0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 
  0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 
  0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, 
  0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 
  0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, 
  0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 
  0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 
  0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 
  0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 
  0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 
  0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, 
  0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50
};

void init_regs() {
    reg_bank    = 0xFE;
}

void load_rom(uint8_t *data) {
    memcpy(rom_00, data, 16384);
    memcpy(rom_nn, &data[16384], 16384);
}

uint8_t memread(uint16_t addr) {
    switch(addr) {
        case REG_P1:        return reg_p1 & 0b11001111;
        case REG_SB:        return reg_sb;
        case REG_SC:        return reg_sc;
        case REG_DIV:       return reg_div;
        case REG_TIMA:      return reg_tima;
        case REG_TMA:       return reg_tma;
        case REG_TAC:       return reg_tac;
        case REG_IF:        return reg_if;
        case REG_PPU_LCDC:  return reg_ppu_lcdc;
        case REG_PPU_STAT:  return reg_ppu_stat;
        case REG_PPU_SCY:   return reg_ppu_scy;
        case REG_PPU_SCX:   return reg_ppu_scx;
        case REG_PPU_LY:    return reg_ppu_ly;
        case REG_PPU_LYC:   return reg_ppu_lyc;
        case REG_PPU_BGP:   return reg_ppu_bgp;
        case REG_PPU_OBP0:  return reg_ppu_obp0;
        case REG_PPU_OBP1:  return reg_ppu_obp1;
        case REG_DMA:       return reg_dma;
        case REG_BANK:      return reg_bank;
        case REG_IE:        return reg_ie;
        default:
            if(addr < 0x4000) {
                if(!(reg_bank & 0x01) && (addr < 0x0100)) {
                    return dmg_boot[addr];
                } else {
                    return rom_00[addr];
                }
            } else if(addr < 0x8000)
                return rom_nn[addr - 0x4000];
            else if(addr < 0xA000) {
                if(vram_block) return 0xFF;
                else return vram[addr - 0x8000];
            } else if(addr < 0xC000)
                return eram[addr - 0xA000];
            else if(addr < 0xD000)
                return wram0[addr - 0xC000];
            else if(addr < 0xE000)
                return wram1[addr - 0xD000];
            else if(addr < 0xFE00)
                return memread(addr - 0x2000); // Echo RAM from 0xC000
            else if(addr < 0xFEA0) {
                if(oam_block) return 0xFF;
                else return oam[addr - 0xFE00];
            } else if(addr > 0xFF7F && addr < 0xFFFF)
                return hram[addr - 0xFF80];

            return 0;
    }
}

void memwrite(uint16_t addr, uint8_t val) {
    switch(addr) {
        case REG_P1:        reg_p1 = val & 0b00110000; break;
        case REG_SB:        reg_sb = val; break;
        case REG_SC:        reg_sc = val & 0b10000001; break;
        case REG_DIV:       reg_div = val; break;
        case REG_TIMA:      reg_tima = val; break;
        case REG_TMA:       reg_tma = val; break;
        case REG_TAC:       reg_tac = val; break;
        case REG_IF:        reg_if = val; break;
        case REG_PPU_LCDC:  reg_ppu_lcdc = val; fprintf(stderr, "LCDC set %02x\n", val); break;
        case REG_PPU_STAT:  reg_ppu_stat = val & 0b01111000; break;
        case REG_PPU_SCY:   reg_ppu_scy = val; break;
        case REG_PPU_SCX:   reg_ppu_scx = val; break;
        case REG_PPU_LY:    break;
        case REG_PPU_LYC:   reg_ppu_lyc = val; break;
        case REG_PPU_BGP:   reg_ppu_bgp = val; break;
        case REG_PPU_OBP0:  reg_ppu_obp0 = val; break;
        case REG_PPU_OBP1:  reg_ppu_obp1 = val; break;
        case REG_DMA:       reg_dma = val; break;
        case REG_BANK:      reg_bank = val & 0b00000001; break;
        case REG_IE:        reg_ie = val; break;
        default:
            if(addr > 0x7FFF && addr < 0xA000 && !vram_block)
                vram[addr - 0x8000] = val;
            else if(addr < 0xC000)
                eram[addr - 0xA000] = val;
            else if(addr < 0xD000)
                wram0[addr - 0xC000] = val;
            else if(addr < 0xE000)
                wram1[addr - 0xD000] = val;
            else if(addr < 0xFE00)
                memwrite(addr - 0x2000, val); // Echo RAM from 0xC000
            else if(addr < 0xFEA0 && !vram_block)
                oam[addr - 0xFE00] = val; // TODO: check PPU state
            else if(addr > 0xFF7F && addr < 0xFFFF)
                hram[addr - 0xFF80] = val;
    }
}
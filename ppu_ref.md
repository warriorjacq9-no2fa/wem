# Comprehensive DMG PPU Emulation Guide

This is a deep dive into implementing a cycle-accurate DMG PPU emulator. I'll cover the state machine, exact timings, the pixel fetching pipeline, and critical edge cases.

---

## Overall State Machine

The PPU cycles through **four modes** per scanline:

```
Mode 2 (OAM search)  → Mode 3 (Pixel transfer)  → Mode 0 (Hblank)
     80 cycles            ~168–172 cycles         ~204–208 cycles
     (lines 0–143)        (lines 0–143)           (lines 0–143)
                                                       ↓
                          Vblank (10 lines: 144–153, Mode 1)
```

**Mode 1** (Vblank) doesn't follow the normal scanline pattern; lines 144–153 each take 456 cycles and stay in Mode 1 the entire time.

---

## Mode 2: OAM Search (80 cycles)

During Mode 2, the PPU scans OAM ($FE00–$FE9F) to build a list of sprites visible on the current scanline.

### Algorithm
For each of the 40 sprite entries (0–39):
1. Read the sprite's Y coordinate from OAM
2. Check if the sprite's Y-range overlaps with the current scanline
3. If yes, add it to the sprite buffer for this line

### Y Coordinate Calculation
```
sprite_screen_y = oam[sprite_index].y - 16
sprite_height = LCDC.3 ? 16 : 8

if (ly >= sprite_screen_y && ly < sprite_screen_y + sprite_height)
    // Sprite is on this scanline
```

### Key Points
- **Only 10 sprites per scanline** can be drawn; if more than 10 sprites overlap a scanline, only the first 10 (by OAM index) are rendered
- The PPU doesn't fetch X coordinates during Mode 2; it only pre-filters by Y
- The X coordinates are used later during Mode 3 to determine the actual rendering order and clipping

---

## Mode 3: Pixel Transfer (Variable Duration: 168–172 cycles)

This is the most complex phase. The PPU generates 160 pixels (one per cycle, roughly) using a **fetch state machine**.

### The Fetch Machine

Mode 3 uses an internal **8-stage pipeline** that repeats to fetch and render each tile:

| Stage | Action | Duration |
|-------|--------|----------|
| **0** | Tile X increment (if needed) | 1 cycle |
| **1** | Fetch BG tile ID | 2 cycles |
| **2** | Fetch BG tile data (low) | 2 cycles |
| **3** | Fetch BG tile data (high) | 2 cycles |
| **4** | Fetch sprite tile data (low) | 2 cycles |
| **5** | Fetch sprite tile data (high) | 2 cycles |
| **6** | Push to pixel FIFO | 1 cycle |
| **7** | Push to pixel FIFO (continued) | 2 cycles |

Actually, this is simplified. Let me explain the **real** fetch machine:

### Real Fetch Machine (Detailed)

The PPU has an internal **pixel FIFO** (16 pixels max) and fetches 8-pixel tiles at a time. The machine cycles through these states:

```c
enum fetch_state {
    FETCH_TILE_ID = 0,     // 2 cycles: Read tile index from map
    FETCH_TILE_DATA_LOW,   // 2 cycles: Read tile data (low byte)
    FETCH_TILE_DATA_HIGH,  // 2 cycles: Read tile data (high byte)
    FETCH_SPRITE_DATA_LOW, // 2 cycles: Read sprite data (low byte, if applicable)
    FETCH_SPRITE_DATA_HIGH,// 2 cycles: Read sprite data (high byte, if applicable)
    PUSH                   // 2 cycles: Push 8 pixels to FIFO
};
```

Each state takes 2 cycles (except the first might stall). The machine repeats this sequence to fill the pixel FIFO.

### Pixel FIFO Management

- **FIFO capacity**: 16 pixels
- **FIFO is filled**: 8 pixels at a time from fetches
- **FIFO is drained**: 1 pixel per PPU cycle, sent to the LCD

The PPU can't output the next pixel until the FIFO has at least 1 pixel. If the FIFO empties (shouldn't happen in normal operation), the PPU stalls.

### Tile Map Fetching

When fetching a tile ID from the background/window map:

```
// Background
if using window && pixel_x >= wx:
    map_addr = (lcdc.6 ? 0x9C00 : 0x9800) + map_offset
    tile_x = (pixel_x - wx) / 8
    tile_y = (ly - wy) / 8
else:
    map_addr = (lcdc.3 ? 0x9C00 : 0x9800) + map_offset
    tile_x = (scx + pixel_x) / 8
    tile_y = (scy + ly) / 8

// Tile map is 32×32 tiles (1024 bytes)
map_offset = (tile_y & 31) * 32 + (tile_x & 31)
tile_id = vram[map_addr + map_offset]
```

### Tile Data Fetching

Tile data uses a 2-bit encoding per pixel. The **tile_data_select** bit (LCDC.4) chooses the addressing mode:

```
// If LCDC.4 = 0: Tile data at 0x8800–0x97FF (signed indexing)
// If LCDC.4 = 1: Tile data at 0x8000–0x8FFF (unsigned indexing)

if lcdc.4:
    tile_data_base = 0x8000
    tile_offset = tile_id * 16
else:
    tile_data_base = 0x9000
    tile_offset = ((int8_t)tile_id) * 16

// Each tile is 8 rows × 2 bytes per row
tile_y_in_tile = ly % 8  // or (ly - wy) % 8 for window
data_addr = tile_data_base + tile_offset + tile_y_in_tile * 2

low_byte = vram[data_addr]
high_byte = vram[data_addr + 1]
```

### Converting Tile Data to Pixels

Each row of a tile is 2 bytes encoding 8 pixels as 2-bit values:

```c
uint8_t low = vram[data_addr];
uint8_t high = vram[data_addr + 1];

// For each pixel 0–7 (left to right):
for (int px = 0; px < 8; px++) {
    int bit_pos = 7 - px;  // Bit 7 = pixel 0, Bit 0 = pixel 7
    int color_index = ((high >> bit_pos) & 1) << 1 | ((low >> bit_pos) & 1);
    // color_index is now 0–3
    
    // Look up in palette register
    int color = (bgp_register >> (color_index * 2)) & 3;
}
```

### Handling Window vs. Background

The **window** is a separate layer that:
- Starts rendering at position (WX – 7, WY)
- Uses its own tile map (selectable: $9800 or $9C00)
- Overrides the background for pixels where it's active
- Has its own internal X and Y counters

```c
// Window activation:
if (lcdc.5 && px >= wx - 7 && ly >= wy) {
    // Use window rendering instead of background
    window_x = px - (wx - 7);
    window_y = ly - wy;
    // Fetch from window map...
} else {
    // Use background rendering
}
```

**Important**: The window X counter increments even if the window isn't visible. Once WX changes mid-scanline, buggy behavior can occur—this is why some games disable the window entirely during rendering.

---

## Sprite Rendering During Mode 3

Sprites are fetched in parallel with the background/window rendering. The sprite rendering logic runs **outside** the main fetch machine.

### Building the Sprite List (from Mode 2)

During Mode 2, you collect all sprites whose Y-range includes the current line. Store them in a buffer with their **OAM indices** (0–39).

### Sprite Priority and X-Sorting

Sprites are rendered in a **specific order**:
1. Sort sprites by **X coordinate** (ascending)
2. If X coordinates are equal, use **OAM index** (ascending) as tiebreaker
3. The **first 10 by this order** are rendered; others are discarded

```c
// Pseudo-code for sorting (10 sprites max per line)
struct sprite {
    uint8_t x;
    uint8_t y;
    uint8_t tile;
    uint8_t flags;
    uint8_t oam_index;
};

// Sort by X, then by OAM index
std::sort(sprites_on_line.begin(), sprites_on_line.end(), 
    [](const sprite& a, const sprite& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.oam_index < b.oam_index;
    });

// Keep only the first 10
if (sprites_on_line.size() > 10) 
    sprites_on_line.resize(10);
```

### Fetching Sprite Tile Data

For each visible sprite:

```c
struct oam_entry {
    uint8_t y;      // Screen Y – 16
    uint8_t x;      // Screen X – 8
    uint8_t tile;   // Tile index
    uint8_t flags;  // Bit 7: priority, Bit 5: x-flip, Bit 6: y-flip, Bits 1–0: palette
};

// Determine which row of the sprite to fetch
uint8_t sprite_height = lcdc.2 ? 16 : 8;  // Bit 2 of LCDC
int sprite_y = oam.y - 16;
int row_in_sprite = ly - sprite_y;

// Handle Y-flip
if (oam.flags & 0x40)  // Y-flip bit
    row_in_sprite = sprite_height - 1 - row_in_sprite;

// Tile lookup
uint8_t tile_index = oam.tile;
if (sprite_height == 16) {
    // In 8×16 mode, ignore bit 0 of tile index for the base tile
    tile_index &= 0xFE;
    // Alternate between two tiles based on which half of sprite
    if (row_in_sprite >= 8)
        tile_index++;
}

// Fetch tile data (similar to background, always uses $8000–$8FFF)
int tile_offset = tile_index * 16 + (row_in_sprite % 8) * 2;
uint8_t tile_low = vram[0x8000 + tile_offset];
uint8_t tile_high = vram[0x8000 + tile_offset + 1];
```

### Blending Sprites with Background

For each pixel position, determine which sprite (if any) contributes:

```c
// For each output pixel:
for (int px = 0; px < 160; px++) {
    uint8_t bg_color = background_fifo[px];
    
    // Check each sprite (in rendering order)
    uint8_t sprite_color = 0;  // 0 = transparent
    bool sprite_priority_bg = false;
    bool sprite_found = false;
    
    for (auto& sprite : sprites_on_line) {
        int sprite_x = sprite.x - 8;
        int sprite_px = px - sprite_x;
        
        // Check if this sprite covers this pixel
        if (sprite_px >= 0 && sprite_px < 8) {
            // Fetch the color from this sprite's tile data
            int bit_pos = 7 - sprite_px;
            if (sprite.flags & 0x20)  // X-flip
                bit_pos = sprite_px;
            
            uint8_t color_index = ((sprite_high >> bit_pos) & 1) << 1 | 
                                  ((sprite_low >> bit_pos) & 1);
            
            if (color_index != 0) {  // 0 is transparent for sprites
                sprite_color = color_index;
                sprite_priority_bg = (sprite.flags & 0x80) != 0;  // Priority bit
                sprite_found = true;
                break;  // First sprite in order wins
            }
        }
    }
    
    // Determine final color
    uint8_t final_color;
    if (!sprite_found) {
        final_color = bg_color;
    } else if (!sprite_priority_bg) {
        // Sprite in front: use sprite color unless BG color is 0
        final_color = (bg_color == 0) ? sprite_color : bg_color;
    } else {
        // Sprite behind: always use BG color if non-zero
        final_color = (bg_color == 0) ? sprite_color : bg_color;
    }
    
    // Look up in palette and output
    uint8_t palette_reg = (sprite.flags & 0x10) ? obp1 : obp0;
    uint8_t lcd_color = (palette_reg >> (final_color * 2)) & 3;
    output_pixel(px, lcd_color);
}
```

---

## Mode 0: Hblank (~204–208 cycles)

Mode 0 is straightforward: do nothing. The PPU is idle, and the CPU can safely access VRAM and OAM.

The **duration varies** because Mode 3 (pixel transfer) is variable length (168–172 cycles). Hblank always occupies the remaining cycles to reach 456 per scanline.

---

## Vblank (Mode 1, 10 lines × 456 cycles)

During lines 144–153:
- Stay in **Mode 1** for all 456 cycles per line
- The PPU isn't fetching or rendering
- CPU has full access to VRAM and OAM
- **V-Blank interrupt** fires at the *start* of line 144

---

## Timing and Interrupt Quirks

### When Does Each Interrupt Fire?

**V-Blank Interrupt** (`$0040`):
- Fires on the **rising edge** of `ly == 144`
- More precisely: when `ly` transitions from 143 to 144
- If the interrupt is enabled and the `vblank` flag in STAT is set, the interrupt triggers

**STAT Interrupt** (`$0048`):
- Fires on rising edges of the STAT interrupt conditions
- Conditions are set in bits 3–6 of the STAT register:
  - **Bit 3**: H-Blank interrupt enable
  - **Bit 4**: V-Blank interrupt enable (redundant with $0040, but can double-fire)
  - **Bit 5**: OAM interrupt enable
  - **Bit 6**: LYC=LY interrupt enable

---

## VRAM and OAM Access Blocking

The CPU **cannot safely** access VRAM or OAM during certain modes:

| Mode | VRAM Access | OAM Access |
|------|-------------|-----------|
| **Mode 0 (Hblank)** | Safe | Safe |
| **Mode 1 (Vblank)** | Safe | Safe |
| **Mode 2 (OAM search)** | Safe | **BLOCKED** |
| **Mode 3 (Transfer)** | **BLOCKED** | **BLOCKED** |

If the CPU violates this (tries to write during Mode 2/3):
- Reads return 0xFF
- Writes are silently dropped (or partially corrupted, depending on exact cycle)
- Real hardware exhibits "VRAM corruption glitches" if abused

---

## Turning the LCD On/Off

When **LCDC.7 (LCD enable)** is set to 0:
- PPU immediately enters Mode 0
- `ly` is reset to 0
- The PPU doesn't advance to the next scanline until LCDC.7 is set to 1 again
- This allows the CPU to reset the display

---

## Register Details

### LY (Current Scanline, $FF44)
- Read-only
- Increments every 456 cycles
-
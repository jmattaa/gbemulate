#ifndef GBEMULATE_PPU_H
#define GBEMULATE_PPU_H

#include "types.h"

typedef struct {
    u8 ly;
    enum {
        OAM_SEARCH,
        PXL_TRANSFER,
        VBLANK,
        HBLANK,
    } mode;
    u8 npxl_scanline;
} ppu_t;

typedef struct gb_t gb_t;

void ppu_init(ppu_t *ppu);
void ppu_step(gb_t *gb);

#endif

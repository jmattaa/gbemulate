#ifndef GBEMULATE_PPU_H
#define GBEMULATE_PPU_H

#include "gb.h"
#include "types.h"

typedef struct {
    gb_t *gb;

    u8 ly;
    enum {
        OAM_SEARCH,
        PXL_TRANSFER,
        VBLANK,
        HBLANK,
    } mode;
    u8 npxl_scanline;
} ppu_t;

ppu_t *ppu_init(gb_t *gb);
void ppu_step(ppu_t *ppu);
void ppu_free(ppu_t *ppu);

#endif

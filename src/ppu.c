#include "ppu.h"
#include "gb.h"

void ppu_init(ppu_t *ppu) {
    ppu->ly = 0;
    ppu->mode = OAM_SEARCH;
    ppu->npxl_scanline = 0;
}

void ppu_step(gb_t *gb) {
    ppu_t *ppu = &gb->ppu;
    switch (ppu->mode) {
        case OAM_SEARCH:
            ppu->mode = PXL_TRANSFER;
            break;
        case PXL_TRANSFER:
            ppu->mode = HBLANK;
            break;
        case HBLANK:
            if (ppu->ly == 144)
                ppu->mode = VBLANK;
            else
                ppu->mode = OAM_SEARCH;
            break;
        case VBLANK:
            ppu->mode = OAM_SEARCH;
            break;
    }
}



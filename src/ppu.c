#include "ppu.h"
#include "gb.h"

void ppu_init(ppu_t *ppu) {
    ppu->ly = 0;
    ppu->mode = OAM_SEARCH;
    ppu->npxl_scanline = 0;
}

void ppu_step(gb_t *gb) {
    ppu_t *p = &gb->ppu;
    switch (p->mode) {
        case OAM_SEARCH:
            // scan the OAM from 0xfe00 to 0xfe9f to mix sprite pixels in the
            // current line later.

            if (p->ticks == 80)
                p->mode = PXL_TRANSFER;
            break;
        case PXL_TRANSFER:
            p->npxl_scanline++;
            if (p->npxl_scanline == 160)
                p->mode = HBLANK;
            break;
        case HBLANK:
            if (p->ticks == 456) {
                p->ticks = 0;
                p->ly++;
                if (p->ly == 144)
                    p->mode = VBLANK;
                else
                    p->mode = OAM_SEARCH;
            }
            break;
        case VBLANK:
            if (p->ticks == 456) {
                p->ticks = 0;
                p->ly++;
                if (p->ly == 153) {
                    p->ly = 0;
                    p->mode = OAM_SEARCH;
                }
            }
            break;
    }
}

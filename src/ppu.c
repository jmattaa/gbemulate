#include "ppu.h"
#include <stdlib.h>
#include "logger.h"

ppu_t *ppu_init(gb_t *gb) {
    ppu_t *ppu = malloc(sizeof(ppu_t));
    if (ppu == NULL) {
        log_error("Failed to allocate ppu_t (ppu struct)\n");
        return NULL;
    }

    ppu->gb = gb;
    ppu->ly = 0;
    ppu->mode = OAM_SEARCH;
    ppu->npxl_scanline = 0;

    return ppu;
}

void ppu_step(ppu_t *ppu) {
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

void ppu_free(ppu_t *ppu) { free(ppu); }

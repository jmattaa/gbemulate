#include "gb.h"
#include <stdlib.h>
#include "cpu.h"
#include "io.h"
#include "logger.h"

gb_t *gb_init(const char *fname) {
    gb_t *gb = malloc(sizeof(gb_t));
    if (gb == NULL) {
        log_error("Failed to allocate gb_t (gameboy struct)\n");
        return NULL;
    }

    gb->crom = io_freadb(fname, &gb->crom_size);
    if (gb->crom == NULL) {
        log_error("Failed to read ROM\n");
        free(gb);
        return NULL;
    }
    gb->mmap.cart = *(cmmap_t *)gb->crom;

    gb->cpu = cpu_init();
    if (gb->cpu == NULL) {
        free(gb->crom);
        free(gb);
        return NULL;
    }

    return gb;
}

void gb_clock_advance(gb_t *gb, u8 ticks) { gb->ticks += ticks; }

void gb_free(gb_t *gb) {
    cpu_free(gb->cpu);
    free(gb->crom);
    free(gb);
}

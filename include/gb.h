#ifndef GBEMULATE_GB_H
#define GBEMULATE_GB_H

#include <stddef.h>
#include "cpu.h"
#include "mmap.h"
#include "ppu.h"

struct gb_t {
    mmap_t mmap;
    cpu_t *cpu;
    ppu_t ppu;

    u32 ticks;

    char *crom; // cartridge rom
    size_t crom_size;
};

typedef struct gb_t gb_t;

gb_t *gb_init(const char *fname);
void gb_clock_advance(gb_t *gb, u8 ticks);
void gb_free(gb_t *gb);

#endif

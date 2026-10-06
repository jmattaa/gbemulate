#ifndef GBEMULATE_GB_H
#define GBEMULATE_GB_H

#include "cpu.h"
#include "mmap.h"
#include <stddef.h>

typedef struct {
  mmap_t mmap;
  cpu_t cpu;

  char *crom; // cartridge rom
  size_t crom_size;
} gb_t;

gb_t *gb_init(const char *fname);
void gb_free(gb_t *gb);

#endif

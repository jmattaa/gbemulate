#include "gb.h"
#include "io.h"
#include "logger.h"
#include <stdlib.h>

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

  return gb;
}

void gb_free(gb_t *gb) {
  free(gb->crom);
  free(gb);
}

#include "gb.h"
#include "logger.h"

int main(void) {
  gb_t *gb = gb_init("roms/tetris.gb");

  log_info("ROM SIZE: 0x%x\n", gb->crom_size);
  log_info("ROM title: %s\n", gb->mmap.cart.hdr.title);
  log_info("ROM type: 0x%x\n", gb->mmap.cart.hdr.cart_type);
  log_info("ROM size: 0x%x\n", gb->mmap.cart.hdr.rom_size);
  log_info("RAM size: 0x%x\n", gb->mmap.cart.hdr.ram_size);
  log_info("ROM version: 0x%x\n", gb->mmap.cart.hdr.rom_version);
  log_info("Licensee code: 0x%x\n", gb->mmap.cart.hdr.old_licensee);

  gb_free(gb);
  return 0;
}

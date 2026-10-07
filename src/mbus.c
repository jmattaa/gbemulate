#include "mbus.h"

u8 mbus_read(gb_t *gb, u8 addr) { return gb->mmap.mem[addr]; }
void mbus_write(gb_t *gb, u8 addr, u8 data) { gb->mmap.mem[addr] = data; }

#include "mbus.h"

u8 mbus_read(gb_t *gb, u16 addr) {
    if (addr == 0xff44) {
        return gb->ppu.ly;
    }
    return gb->mmap.mem[addr];
}

void mbus_write(gb_t *gb, u16 addr, u8 data) {
    if (addr == 0xff44)
        return; // don't write to ly
    gb->mmap.mem[addr] = data;
}

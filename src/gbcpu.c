#include "gbcpu.h"
#include "gbmmap.h"
#include "logger.h"

gbcpu_t gbcpu;

void gbcpu_init(void) { gbcpu.pc = 0x0100; }

void gbcpu_step(void)
{
    uint8_t op = gb_mmap.mem[gbcpu.pc++];

    // n8 little endian 8 bit value
    // n16 little endian 16 bit value
    // a8 little endian 8 bit address
    // a16 little endian 16 bit address
    switch (op)
    {
    case 0x00:
        // NOP
        break;
    case 0x01:
        // LD BC, n16 
        gbcpu.bc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0xc3:
        // JP a16
        gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    }
}

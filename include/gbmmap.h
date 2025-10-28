#ifndef GBEMULATE_GBMMAP_H
#define GBEMULATE_GBMMAP_H

#include "gbc.h"
#include <stdint.h>

/*
 * gb_mmap_t - Gameboy memory map
 */
typedef union
{
    uint8_t mem[0xffff];
    struct
    {
        union
        {
            struct
            {
                uint8_t rbank0[0x4000]; // 0x0000 - 0x3fff
                uint8_t
                    rbank1[0x4000]; // 0x4000 - 0x7fff // TODO: add mbc support
            };
            gb_cmmap_t cmmap; // 0x0000 - 0x7fff
        };
        uint8_t vram[0x2000];     // 0x8000 - 0x9fff
        uint8_t eram[0x2000];     // 0xa000 - 0xbfff // TODO: add mbc support
        uint8_t wram0[0x1000];    // 0xc000 - 0xcfff
        uint8_t wram1[0x1000];    // 0xd000 - 0xdfff
        uint8_t echo_ram[0x1e00]; // 0xe000 - 0xfdff
        uint8_t oam[0xa0];        // 0xfe00 - 0xfe9f
        uint8_t _unused[0x60];    // 0xfea0 - 0xfeff
        uint8_t io[0x80];         // 0xff00 - 0xff7f
        uint8_t hram[0x7f];       // 0xff80 - 0xfffe
        uint8_t ie;               // 0xffff
    } __attribute__((packed));
} gb_mmap_t;

extern gb_mmap_t gb_mmap;

#endif

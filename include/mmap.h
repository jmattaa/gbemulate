#ifndef GBEMULATE_MMAP_H
#define GBEMULATE_MMAP_H

#include "types.h"

typedef struct {
    u8 entry_point[4]; // 0x100 - 0x103 (most games fill this with a nop
                       // followed by a jp 0x0150 which is the start of the ROM)
    u8 logo[48];       // 0x104 - 0x133
    u8 title[16];      // 0x134 - 0x143
    u8 new_licensee[2]; // 0x144 - 0x145
    u8 sgb_flag;        // 0x146
    u8 cart_type;       // 0x147
    u8 rom_size;        // 0x148
    u8 ram_size;        // 0x149
    u8 dest_code;       // 0x14A
    u8 old_licensee;    // 0x14B
    u8 rom_version;     // 0x14C
    u8 hdr_checksum;    // 0x14D
    u8 gbl_checksum[2]; // 0x14E - 0x14F
} __attribute__((packed)) chdr_t;

typedef union {
    u8 data[0x8000]; // 32kib
    struct {
        u8 brom[0x100]; // 0x0000 - 0x00ff
        chdr_t hdr;
        u8 rom[0x4000]; // 0x0150 - 0x7fff
    };
} cmmap_t;

typedef union {
    u8 mem[0x10000];
    struct {
        union {
            struct {
                u8 rbank0[0x4000]; // 0x0000 - 0x3fff
                u8 rbank1[0x4000]; // 0x4000 - 0x7fff // TODO: add mbc support
            };
            cmmap_t cart; // 0x0000 - 0x7fff
        };
        u8 vram[0x2000];     // 0x8000 - 0x9fff
        u8 eram[0x2000];     // 0xa000 - 0xbfff // TODO: add mbc support
        u8 wram0[0x1000];    // 0xc000 - 0xcfff
        u8 wram1[0x1000];    // 0xd000 - 0xdfff
        u8 echo_ram[0x1e00]; // 0xe000 - 0xfdff
        u8 oam[0xa0];        // 0xfe00 - 0xfe9f
        u8 _unused[0x60];    // 0xfea0 - 0xfeff
        u8 io[0x80];         // 0xff00 - 0xff7f
        u8 hram[0x7f];       // 0xff80 - 0xfffe
        u8 ie;               // 0xffff
    } __attribute__((packed));
} mmap_t;

#endif

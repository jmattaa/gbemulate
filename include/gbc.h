#ifndef GBEMULATE_GBC_H
#define GBEMULATE_GBC_H

#include <stdint.h>

/*
 * gb_chdr_t - Gameboy Cartridge Header
 */
typedef struct
{
    uint8_t entry_point[4];   // 0x0100 - 0x0103
    uint8_t new_logo[48];     // 0x0104 - 0x0133
    uint8_t title[16];        // 0x0134 - 0x0143
    uint8_t new_licensee[2];  // 0x0144 - 0x0145
    uint8_t sgb_flag;         // 0x0146
    uint8_t cartridge_type;   // 0x0147
    uint8_t rom_size;         // 0x0148
    uint8_t ram_size;         // 0x0149
    uint8_t dest_code;        // 0x014A
    uint8_t old_licensee;     // 0x014B
    uint8_t mask_rom_nr;      // 0x014C
    uint8_t hdr_chksum;       // 0x014D
    uint8_t global_chksum[2]; // 0x014E-0x014F
} __attribute__((packed)) gb_chdr_t;

/*
 * gb_cmmap_t - Gameboy Cartridge memory map
 */
typedef union
{
    uint8_t data[0x8000];
    struct
    {
        uint8_t brom[0x100]; // 0x0000 - 0x00FF
        gb_chdr_t chdr;
        uint8_t rom[0x7eb0]; // 0x0150 - 0x7fff
    } __attribute__((packed));
} gb_cmmap_t;

/*
 * get the licensee name
 * chdr: Gameboy Cartridge Header
 * return:
 *      Name of the licensee
 */
const char *gbc_lic(gb_chdr_t *chdr);

#endif

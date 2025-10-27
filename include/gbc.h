#ifndef GBEMULATE_GBC_H
#define GBEMULATE_GBC_H

#include <stdint.h>

/*
 * gb_chdr_t - Gameboy Cartridge Header
 * 80 bytes
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
 * get the licensee name
 * chdr: Gameboy Cartridge Header
 * return:
 *      Name of the licensee
 */
const char *gbc_lic(gb_chdr_t *chdr);

/*
 * get the cartridge type name
 * chdr: Gameboy Cartridge Header
 * return:
 *      Name of the cartridge type or NULL if not found
 */
// const char *gbc_type_name(gb_chdr_t *chdr);

#endif

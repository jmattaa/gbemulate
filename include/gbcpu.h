#ifndef GBEMULATE_GBCPU_H
#define GBEMULATE_GBCPU_H

#include <stdint.h>

typedef struct
{
    unsigned _unused : 4; // bits 0-3, always 0
    unsigned c : 1;       // bit 4
    unsigned h : 1;       // bit 5
    unsigned n : 1;       // bit 6
    unsigned z : 1;       // bit 7
} __attribute__((packed)) gbcpu_flags_t;

typedef struct
{
    // registers
    // should they be lioke this or should it be a then f, b then c ...
    union
    {
        struct
        {
            gbcpu_flags_t f;
            uint8_t a;
        };
        uint16_t af;
    };
    union
    {
        struct
        {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };
    union
    {
        struct
        {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };
    union
    {
        struct
        {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };
    uint16_t sp;
    uint16_t pc;

    // some good shi we need
    uint8_t ime;
} __attribute__((packed)) gbcpu_t;

extern gbcpu_t gbcpu;

void gbcpu_init(void);
void gbcpu_step(void);

#endif

#ifndef GBEMULATE_GBCPU_H
#define GBEMULATE_GBCPU_H

#include <stdint.h>

typedef struct
{
    // registers
    union
   {
        struct
        {
            uint8_t f;
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
} __attribute__((packed)) gbcpu_t;

extern gbcpu_t gbcpu;

void gbcpu_init(void);
void gbcpu_step(void);

#endif

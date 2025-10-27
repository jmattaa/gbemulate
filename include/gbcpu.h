#ifndef GBEMULATE_GBCPU_H
#define GBEMULATE_GBCPU_H

#include <stdint.h>

typedef struct
{
    uint8_t a;
    uint8_t f;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t sp;
    uint16_t pc;
} __attribute__((packed)) gb_cpuregs_t;

#endif

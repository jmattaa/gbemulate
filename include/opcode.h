#ifndef GBEMULATE_OPCODE_H
#define GBEMULATE_OPCODE_H

#include "gb.h"

typedef struct {
    const char *name;
    void (*fn)(gb_t *gb);
} opcode_t;

u8 cpu_step(gb_t *gb);

#endif

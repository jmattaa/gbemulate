#ifndef GBEMULATE_CPU_H
#define GBEMULATE_CPU_H

#include "types.h"

typedef struct {
    union {
        struct {
            struct {
                u8 _ : 4; // bits 0-3, always 0
                u8 c : 1; // bit 4
                u8 h : 1; // bit 5
                u8 n : 1; // bit 6
                u8 z : 1; // bit 7
            } f;
            u8 a;
        };
        u16 af;
    };
    union {
        struct {
            u8 c;
            u8 b;
        };
        u16 bc;
    };
    union {
        struct {
            u8 e;
            u8 d;
        };
        u16 de;
    };
    union {
        struct {
            u8 l;
            u8 h;
        };
        u16 hl;
    };
    u16 sp;
    u16 pc;

    u8 ime;
    u8 halted;
    u8 stopped;
} cpu_t;

cpu_t *cpu_init(void);
void cpu_free(cpu_t *cpu);

#endif

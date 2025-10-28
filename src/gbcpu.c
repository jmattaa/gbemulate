#include "gbcpu.h"
#include "gbmmap.h"
#include "logger.h"
#include <stdint.h>

gbcpu_t gbcpu;

#define DEC_r8(reg)                                                            \
    reg--;                                                                     \
    gbcpu.f.z = reg == 0;                                                      \
    gbcpu.f.n = 1;                                                             \
    gbcpu.f.h = (reg >> 3) & 1

#define INC_r8(reg)                                                            \
    reg++;                                                                     \
    gbcpu.f.z = reg == 0;                                                      \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = (reg >> 3) & 1

#define ADD_RR_RR(r1, r2)                                                      \
    r1 += r2;                                                                  \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = (r1 >> 11) & 1;                                                \
    gbcpu.f.c = (r1 >> 15) & 1

void gbcpu_init(void) { gbcpu.pc = 0x0100; }

// https://gekkio.fi/files/gb-docs/gbctr.pdf
void gbcpu_step(void)
{
    uint8_t op = gb_mmap.mem[gbcpu.pc++];

    // n8 little endian 8 bit value
    // n16 little endian 16 bit value
    // a8 little endian 8 bit address
    // a16 little endian 16 bit address
    // e8 8bit signed value
    // [REG] - the value in the register REG
    switch (op)
    {
    case 0x00:
        // NOP
        break;
    case 0x01:
        // LD BC, n16
        gbcpu.bc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0x02:
        // LD [BC], A
        gb_mmap.mem[gbcpu.bc] = gbcpu.a;
        break;
    case 0x03:
        // INC BC
        gbcpu.bc++;
        break;
    case 0x04:
        // INC B
        INC_r8(gbcpu.b);
        break;
    case 0x05:
        // DEC B
        DEC_r8(gbcpu.b);
        break;
    case 0x06:
        // LD B, n8
        gbcpu.b = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x07:
        // RLCA
        // Rotate left circular (accumulator)
        // yeh idk what ts mean
        gbcpu.f.c = (gbcpu.a >> 7) & 1;
        gbcpu.a = (gbcpu.a << 1) | gbcpu.f.c;
        gbcpu.f.z = 0;
        gbcpu.f.n = 0;
        gbcpu.f.h = 0;
        break;
    case 0x08:
        // LD [a16], SP
        gb_mmap.mem[gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8)] =
            gbcpu.sp;
        break;
    case 0x09:
        // ADD HL, BC
        ADD_RR_RR(gbcpu.hl, gbcpu.bc);
        break;
    case 0x0a:
        // LD A, [BC]
        gbcpu.a = gb_mmap.mem[gbcpu.bc];
        break;
    case 0x0b:
        // DEC BC
        gbcpu.bc--;
        break;
    case 0x0c:
        // INC C
        INC_r8(gbcpu.c);
        break;
    case 0x0d:
        // DEC C
        DEC_r8(gbcpu.c);
        break;
    case 0x0e:
        // LD C, n8
        gbcpu.c = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x0f:
        // RRCA
        gbcpu.f.c = gbcpu.a & 1;
        gbcpu.a = gbcpu.a >> 1 | (gbcpu.f.c << 7);
        gbcpu.f.z = 0;
        gbcpu.f.n = 0;
        gbcpu.f.h = 0;
        break;

    case 0x10:
        // STOP
        break;
    case 0x11:
        // LD DE, n16
        gbcpu.de = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0x12:
        // LD [DE], A
        gb_mmap.mem[gbcpu.de] = gbcpu.a;
        break;
    case 0x13:
        // INC DE
        gbcpu.de++;
        break;
    case 0x14:
        // INC D
        INC_r8(gbcpu.d);
        break;
    case 0x15:
        // DEC D
        DEC_r8(gbcpu.d);
        break;
    case 0x16:
        // LD D, n8
        gbcpu.d = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x17:
    {
        // RLA
        // TODO: maybe without temp var???
        uint8_t c = gbcpu.f.c;
        gbcpu.f.c = (gbcpu.a >> 7) & 1;
        gbcpu.a = (gbcpu.a << 1) | c;
        gbcpu.f.z = 0;
        gbcpu.f.n = 0;
        gbcpu.f.h = 0;
    };
    break;
    case 0x18:
        // JR e8
        gbcpu.pc += (int8_t)gb_mmap.mem[gbcpu.pc];
        break;
    case 0x19:
        // ADD HL, DE
        ADD_RR_RR(gbcpu.hl, gbcpu.de);
        break;
    case 0x1a:
        // LD A, [DE]
        gbcpu.a = gb_mmap.mem[gbcpu.de];
        break;
    case 0x1b:
        // DEC DE
        gbcpu.de--;
        break;
    case 0x1c:
        // INC E
        INC_r8(gbcpu.e);
        break;
    case 0x1d:
        // DEC E
        DEC_r8(gbcpu.e);
        break;
    case 0x1e:
        // LD E, n8
        gbcpu.e = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x1f:
    {
        // RRA
        uint8_t c = gbcpu.f.c;
        gbcpu.f.c = (gbcpu.a >> 0) & 1;
        gbcpu.a = (gbcpu.a >> 1) | (c << 7);
        gbcpu.f.z = 0;
        gbcpu.f.n = 0;
        gbcpu.f.h = 0;
    };
    break;

    case 0x20:
        // JR NZ, e8
        if (!gbcpu.f.z)
            gbcpu.pc += (int8_t)gb_mmap.mem[gbcpu.pc];
        break;
    case 0x21:
        // LD HL, n16
        gbcpu.hl = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0x22:
        // LD [HL+], A
        gb_mmap.mem[gbcpu.hl++] = gbcpu.a;
        break;
    case 0x23:
        // INC HL
        gbcpu.hl++;
        break;
    case 0x24:
        // INC H
        INC_r8(gbcpu.h);
        break;
    case 0x25:
        // DEC H
        DEC_r8(gbcpu.h);
        break;
    case 0x26:
        // LD H, n8
        gbcpu.h = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x27:
        // DAA
        // https://github.com/rockytriton/LLD_gbemu/blob/e6be3433526a96401f7d42b653b37ab6a955415d/part9/lib/cpu_proc.c#L201
        {
            uint8_t u = 0;
            int fc = 0;

            if (gbcpu.f.h || (!gbcpu.f.n && (gbcpu.a & 0xF) > 9))
                u = 6;

            if (gbcpu.f.c || (!gbcpu.f.n && gbcpu.a > 0x99))
            {
                u |= 0x60;
                fc = 1;
            }

            gbcpu.a += gbcpu.f.n ? -u : u;

            gbcpu.f.z = gbcpu.a == 0;
            gbcpu.f.n = 0;
            gbcpu.f.h = 0;
            gbcpu.f.c = fc;
        };
        break;
    case 0x28:
        // JR Z, e8
        if (gbcpu.f.z)
            gbcpu.pc += (int8_t)gb_mmap.mem[gbcpu.pc];
        break;
    case 0x29:
        // ADD HL, HL
        ADD_RR_RR(gbcpu.hl, gbcpu.hl);
        break;
    case 0x2a:
        // LD A, [HL+]
        gbcpu.a = gb_mmap.mem[gbcpu.hl++];
        break;
    case 0x2b:
        // DEC HL
        gbcpu.hl--;
        break;
    case 0x2c:
        // INC L
        INC_r8(gbcpu.l);
        break;
    case 0x2d:
        // DEC L
        DEC_r8(gbcpu.l);
        break;
    case 0x2e:
        // LD L, n8
        gbcpu.l = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x2f:
        // CPL
        gbcpu.a = ~gbcpu.a;
        gbcpu.f.n = 1;
        gbcpu.f.h = 1;
        break;

    case 0x30:
        // JR NC, e8
        if (!gbcpu.f.c)
            gbcpu.pc += (int8_t)gb_mmap.mem[gbcpu.pc];
        break;
    case 0x31:
        // LD SP, n16
        gbcpu.sp = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0x32:
        // LD [HL-], A
        gb_mmap.mem[gbcpu.hl--] = gbcpu.a;
        break;
    case 0x33:
        // INC SP
        gbcpu.sp++;
        break;
    case 0x34:
        // INC [HL]
        INC_r8(gb_mmap.mem[gbcpu.hl]); // basically the same thing as inc a r8
                                       // but we're inc the value at the address
        break;
    case 0x35:
        // DEC [HL]
        DEC_r8(gb_mmap.mem[gbcpu.hl]); // same as above
        break;
    case 0x36:
        // LD [HL], n8
        gb_mmap.mem[gbcpu.hl] = gb_mmap.mem[gbcpu.pc++];
        break;
    case 0x37:
        // SCF
        gbcpu.f.c = 1;
        gbcpu.f.n = 0;
        gbcpu.f.h = 0;
        break;
    case 0x38:
        // JR C, e8
        if (gbcpu.c)
            gbcpu.pc += (int8_t)gb_mmap.mem[gbcpu.pc];
    case 0x39:
        // ADD HL, SP
        ADD_RR_RR(gbcpu.hl, gbcpu.sp);
        break;
    case 0x3a:
        // LD A, [HL-]
        gbcpu.a = gb_mmap.mem[gbcpu.hl--];
        break;
    case 0x3b:
        // DEC SP
        gbcpu.sp--;
        break;
    case 0x3c:
        // INC A
        INC_r8(gbcpu.a);
        break;
    case 0x3d:
        // DEC A
        DEC_r8(gbcpu.a);
        break;
    case 0x3e:
        // LD A, n8
        gbcpu.a = gb_mmap.mem[gbcpu.pc++];
    case 0x3f:
        // CCF
        gbcpu.f.n = 0;
        gbcpu.f.h = 0;
        gbcpu.f.c = !gbcpu.f.c;
        break;

    case 0x40:
        // LD B, B
        gbcpu.b = gbcpu.b;
        break;
    case 0x41:
        // LD B, C
        gbcpu.b = gbcpu.c;
        break;
    case 0x42:
        // LD B, D
        gbcpu.b = gbcpu.d;
        break;
    case 0x43:
        // LD B, E
        gbcpu.b = gbcpu.e;
        break;
    case 0x44:
        // LD B, H
        gbcpu.b = gbcpu.h;
        break;
    case 0x45:
        // LD B, L
        gbcpu.b = gbcpu.l;
        break;
    case 0x46:
        // LD B, [HL]
        gbcpu.b = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x47:
        // LD B, A
        gbcpu.b = gbcpu.a;
        break;
    case 0x48:
        // LD C, B
        gbcpu.c = gbcpu.b;
        break;
    case 0x49:
        // LD C, C
        gbcpu.c = gbcpu.c;
        break;
    case 0x4a:
        // LD C, D
        gbcpu.c = gbcpu.d;
        break;
    case 0x4b:
        // LD C, E
        gbcpu.c = gbcpu.e;
        break;
    case 0x4c:
        // LD C, H
        gbcpu.c = gbcpu.h;
        break;
    case 0x4d:
        // LD C, L
        gbcpu.c = gbcpu.l;
        break;
    case 0x4e:
        // LD C, [HL]
        gbcpu.c = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x4f:
        // LD C, A
        gbcpu.c = gbcpu.a;
        break;

    case 0x50:
        // LD D, B
        gbcpu.d = gbcpu.b;
        break;
    case 0x51:
        // LD D, C
        gbcpu.d = gbcpu.c;
        break;
    case 0x52:
        // LD D, D
        gbcpu.d = gbcpu.d;
        break;
    case 0x53:
        // LD D, E
        gbcpu.d = gbcpu.e;
        break;
    case 0x54:
        // LD D, H
        gbcpu.d = gbcpu.h;
        break;
    case 0x55:
        // LD D, L
        gbcpu.d = gbcpu.l;
        break;
    case 0x56:
        // LD D, [HL]
        gbcpu.d = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x57:
        // LD D, A
        gbcpu.d = gbcpu.a;
        break;
    case 0x58:
        // LD E, B
        gbcpu.e = gbcpu.b;
        break;
    case 0x59:
        // LD E, C
        gbcpu.e = gbcpu.c;
        break;
    case 0x5a:
        // LD E, D
        gbcpu.e = gbcpu.d;
        break;
    case 0x5b:
        // LD E, E
        gbcpu.e = gbcpu.e;
        break;
    case 0x5c:
        // LD E, H
        gbcpu.e = gbcpu.h;
        break;
    case 0x5d:
        // LD E, L
        gbcpu.e = gbcpu.l;
        break;
    case 0x5e:
        // LD E, [HL]
        gbcpu.e = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x5f:
        // LD E, A
        gbcpu.e = gbcpu.a;
        break;

    case 0xc3:
        // JP a16
        gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    }
}

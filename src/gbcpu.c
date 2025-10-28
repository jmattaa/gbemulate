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
    gbcpu.f.z = r1 == 0;                                                       \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = (r1 >> 11) & 1;                                                \
    gbcpu.f.c = (r1 >> 15) & 1

#define ADD_R8(r) ADD_RR_RR(gbcpu.a, r)

#define ADC_R8(r)                                                              \
    gbcpu.a += r + gbcpu.f.c;                                                  \
    gbcpu.f.z = gbcpu.a == 0;                                                  \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = (gbcpu.a >> 3) & 1;                                            \
    gbcpu.f.c = (gbcpu.a >> 7) & 1

#define SUB_R8(r)                                                              \
    gbcpu.a -= r;                                                              \
    gbcpu.f.z = gbcpu.a == 0;                                                  \
    gbcpu.f.n = 1;                                                             \
    gbcpu.f.h = (gbcpu.a >> 3) & 1;                                            \
    gbcpu.f.c = (gbcpu.a >> 7) & 1

#define SBC_R8(r)                                                              \
    gbcpu.a -= r - gbcpu.f.c;                                                  \
    gbcpu.f.z = gbcpu.a == 0;                                                  \
    gbcpu.f.n = 1;                                                             \
    gbcpu.f.h = (gbcpu.a >> 3) & 1;                                            \
    gbcpu.f.c = (gbcpu.a >> 7) & 1

#define AND_R8(r)                                                              \
    gbcpu.a &= r;                                                              \
    gbcpu.f.z = gbcpu.a == 0;                                                  \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = 1;                                                             \
    gbcpu.f.c = 0

#define XOR_R8(r)                                                              \
    gbcpu.a ^= r;                                                              \
    gbcpu.f.z = gbcpu.a == 0;                                                  \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = 0;                                                             \
    gbcpu.f.c = 0

#define OR_R8(r)                                                               \
    gbcpu.a |= r;                                                              \
    gbcpu.f.z = gbcpu.a == 0;                                                  \
    gbcpu.f.n = 0;                                                             \
    gbcpu.f.h = 0;                                                             \
    gbcpu.f.c = 0

#define CP_R8(r)                                                               \
    {                                                                          \
        uint8_t res = gbcpu.a - r;                                             \
        gbcpu.f.z = res == 0;                                                  \
        gbcpu.f.n = 1;                                                         \
        gbcpu.f.h = (res >> 3) & 1;                                            \
        gbcpu.f.c = (res >> 7) & 1;                                            \
    }

#define PUSH_RR(r)                                                             \
    gbcpu.sp--;                                                                \
    gb_mmap.mem[gbcpu.sp] = (r >> 8) & 0xff;                                   \
    gbcpu.sp--;                                                                \
    gb_mmap.mem[gbcpu.sp] = r & 0xff

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

    case 0x60:
        // LD H, B
        gbcpu.h = gbcpu.b;
        break;
    case 0x61:
        // LD H, C
        gbcpu.h = gbcpu.c;
        break;
    case 0x62:
        // LD H, D
        gbcpu.h = gbcpu.d;
        break;
    case 0x63:
        // LD H, E
        gbcpu.h = gbcpu.e;
        break;
    case 0x64:
        // LD H, H
        gbcpu.h = gbcpu.h;
        break;
    case 0x65:
        // LD H, L
        gbcpu.h = gbcpu.l;
        break;
    case 0x66:
        // LD H, [HL]
        gbcpu.h = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x67:
        // LD H, A
        gbcpu.h = gbcpu.a;
        break;
    case 0x68:
        // LD L, B
        gbcpu.l = gbcpu.b;
        break;
    case 0x69:
        // LD L, C
        gbcpu.l = gbcpu.c;
        break;
    case 0x6a:
        // LD L, D
        gbcpu.l = gbcpu.d;
        break;
    case 0x6b:
        // LD L, E
        gbcpu.l = gbcpu.e;
        break;
    case 0x6c:
        // LD L, H
        gbcpu.l = gbcpu.h;
        break;
    case 0x6d:
        // LD L, L
        gbcpu.l = gbcpu.l;
        break;
    case 0x6e:
        // LD L, [HL]
        gbcpu.l = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x6f:
        // LD L, A
        gbcpu.l = gbcpu.a;
        break;

    case 0x70:
        // LD [HL], B
        gb_mmap.mem[gbcpu.hl] = gbcpu.b;
        break;
    case 0x71:
        // LD [HL], C
        gb_mmap.mem[gbcpu.hl] = gbcpu.c;
        break;
    case 0x72:
        // LD [HL], D
        gb_mmap.mem[gbcpu.hl] = gbcpu.d;
        break;
    case 0x73:
        // LD [HL], E
        gb_mmap.mem[gbcpu.hl] = gbcpu.e;
        break;
    case 0x74:
        // LD [HL], H
        gb_mmap.mem[gbcpu.hl] = gbcpu.h;
        break;
    case 0x75:
        // LD [HL], L
        gb_mmap.mem[gbcpu.hl] = gbcpu.l;
        break;
    case 0x76:
        // HALT
        break;
    case 0x77:
        // LD [HL], A
        gb_mmap.mem[gbcpu.hl] = gbcpu.a;
        break;
    case 0x78:
        // LD A, B
        gbcpu.a = gbcpu.b;
        break;
    case 0x79:
        // LD A, C
        gbcpu.a = gbcpu.c;
        break;
    case 0x7a:
        // LD A, D
        gbcpu.a = gbcpu.d;
        break;
    case 0x7b:
        // LD A, E
        gbcpu.a = gbcpu.e;
        break;
    case 0x7c:
        // LD A, H
        gbcpu.a = gbcpu.h;
        break;
    case 0x7d:
        // LD A, L
        gbcpu.a = gbcpu.l;
        break;
    case 0x7e:
        // LD A, [HL]
        gbcpu.a = gb_mmap.mem[gbcpu.hl];
        break;
    case 0x7f:
        // LD A, A
        gbcpu.a = gbcpu.a;
        break;

    case 0x80:
        // ADD B
        ADD_R8(gbcpu.b);
        break;
    case 0x81:
        // ADD C
        ADD_R8(gbcpu.c);
        break;
    case 0x82:
        // ADD D
        ADD_R8(gbcpu.d);
        break;
    case 0x83:
        // ADD E
        ADD_R8(gbcpu.e);
        break;
    case 0x84:
        // ADD H
        ADD_R8(gbcpu.h);
        break;
    case 0x85:
        // ADD L
        ADD_R8(gbcpu.l);
        break;
    case 0x86:
        // ADD [HL]
        ADD_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0x87:
        // ADD A
        ADD_R8(gbcpu.a);
        break;
    case 0x88:
        // ADC B
        ADC_R8(gbcpu.b);
        break;
    case 0x89:
        // ADC C
        ADC_R8(gbcpu.c);
        break;
    case 0x8a:
        // ADC D
        ADC_R8(gbcpu.d);
        break;
    case 0x8b:
        // ADC E
        ADC_R8(gbcpu.e);
        break;
    case 0x8c:
        // ADC H
        ADC_R8(gbcpu.h);
        break;
    case 0x8d:
        // ADC L
        ADC_R8(gbcpu.l);
        break;
    case 0x8e:
        // ADC [HL]
        ADC_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0x8f:
        // ADC A
        ADC_R8(gbcpu.a);
        break;

    case 0x90:
        // SUB B
        SUB_R8(gbcpu.b);
        break;
    case 0x91:
        // SUB C
        SUB_R8(gbcpu.c);
        break;
    case 0x92:
        // SUB D
        SUB_R8(gbcpu.d);
        break;
    case 0x93:
        // SUB E
        SUB_R8(gbcpu.e);
        break;
    case 0x94:
        // SUB H
        SUB_R8(gbcpu.h);
        break;
    case 0x95:
        // SUB L
        SUB_R8(gbcpu.l);
        break;
    case 0x96:
        // SUB [HL]
        SUB_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0x97:
        // SUB A
        SUB_R8(gbcpu.a);
        break;
    case 0x98:
        // SBC B
        SBC_R8(gbcpu.b);
        break;
    case 0x99:
        // SBC C
        SBC_R8(gbcpu.c);
        break;
    case 0x9a:
        // SBC D
        SBC_R8(gbcpu.d);
        break;
    case 0x9b:
        // SBC E
        SBC_R8(gbcpu.e);
        break;
    case 0x9c:
        // SBC H
        SBC_R8(gbcpu.h);
        break;
    case 0x9d:
        // SBC L
        SBC_R8(gbcpu.l);
        break;
    case 0x9e:
        // SBC [HL]
        SBC_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0x9f:
        // SBC A
        SBC_R8(gbcpu.a);
        break;

    case 0xa0:
        // AND B
        AND_R8(gbcpu.b);
        break;
    case 0xa1:
        // AND C
        AND_R8(gbcpu.c);
        break;
    case 0xa2:
        // AND D
        AND_R8(gbcpu.d);
        break;
    case 0xa3:
        // AND E
        AND_R8(gbcpu.e);
        break;
    case 0xa4:
        // AND H
        AND_R8(gbcpu.h);
        break;
    case 0xa5:
        // AND L
        AND_R8(gbcpu.l);
        break;
    case 0xa6:
        // AND [HL]
        AND_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0xa7:
        // AND A
        AND_R8(gbcpu.a);
        break;
    case 0xa8:
        // XOR B
        XOR_R8(gbcpu.b);
        break;
    case 0xa9:
        // XOR C
        XOR_R8(gbcpu.c);
        break;
    case 0xaa:
        // XOR D
        XOR_R8(gbcpu.d);
        break;
    case 0xab:
        // XOR E
        XOR_R8(gbcpu.e);
        break;
    case 0xac:
        // XOR H
        XOR_R8(gbcpu.h);
        break;
    case 0xad:
        // XOR L
        XOR_R8(gbcpu.l);
        break;
    case 0xae:
        // XOR [HL]
        XOR_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0xaf:
        // XOR A
        XOR_R8(gbcpu.a);
        break;

    case 0xb0:
        // OR B
        OR_R8(gbcpu.b);
        break;
    case 0xb1:
        // OR C
        OR_R8(gbcpu.c);
        break;
    case 0xb2:
        // OR D
        OR_R8(gbcpu.d);
        break;
    case 0xb3:
        // OR E
        OR_R8(gbcpu.e);
        break;
    case 0xb4:
        // OR H
        OR_R8(gbcpu.h);
        break;
    case 0xb5:
        // OR L
        OR_R8(gbcpu.l);
        break;
    case 0xb6:
        // OR [HL]
        OR_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0xb7:
        // OR A
        OR_R8(gbcpu.a);
        break;
    case 0xb8:
        // CP B
        CP_R8(gbcpu.b);
        break;
    case 0xb9:
        // CP C
        CP_R8(gbcpu.c);
        break;
    case 0xba:
        // CP D
        CP_R8(gbcpu.d);
        break;
    case 0xbb:
        // CP E
        CP_R8(gbcpu.e);
        break;
    case 0xbc:
        // CP H
        CP_R8(gbcpu.h);
        break;
    case 0xbd:
        // CP L
        CP_R8(gbcpu.l);
        break;
    case 0xbe:
        // CP [HL]
        CP_R8(gb_mmap.mem[gbcpu.hl]);
        break;
    case 0xbf:
        // CP A
        CP_R8(gbcpu.a);
        break;

    case 0xc0:
        // RET NZ
        if (!gbcpu.f.z)
            gbcpu.pc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xc1:
        // POP BC
        gbcpu.bc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xc2:
        // JP NZ a16
        if (!gbcpu.f.z)
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0xc3:
        // JP a16
        gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0xc4:
        // CALL NZ a16
        if (!gbcpu.f.z)
        {
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        }
    case 0xc5:
        // PUSH BC
        PUSH_RR(gbcpu.bc);
        break;
    case 0xc6:
        // ADD n8
        ADD_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xc7:
        // RST 0x00
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x00;
        break;
    case 0xc8:
        // RET Z
        if (gbcpu.f.z)
            gbcpu.pc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xc9:
        // RET
        gbcpu.pc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xca:
        // JP Z, a16
        if (gbcpu.f.z)
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
        // 0xcb is done later
    case 0xcc:
        // CALL z, a16
        if (gbcpu.f.z)
        {
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        }
        break;
    case 0xcd:
        // CALL a16
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0xce:
        // ADC n8
        ADC_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xcf:
        // RST 0x08
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x08;
        break;

    case 0xd0:
        // RET NC
        if (!gbcpu.f.c)
            gbcpu.pc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xd1:
        // POP DE
        gbcpu.de = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xd2:
        // JP NC, a16
        if (!gbcpu.f.c)
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0xd3:
        // CRASCH
        break;
    case 0xd4:
        // CALL NC, a16
        if (!gbcpu.f.c)
        {
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        }
        break;
    case 0xd5:
        // PUSH DE
        PUSH_RR(gbcpu.de);
        break;
    case 0xd6:
        // SUB n8
        SUB_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xd7:
        // RST 0x10
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x10;
        break;
    case 0xd8:
        // RET C
        if (gbcpu.f.c)
            gbcpu.pc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xd9:
        // RETI
        gbcpu.ime = 1;
        gbcpu.pc = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xda:
        // JP C, a16
        if (gbcpu.f.c)
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        break;
    case 0xdb:
        // CRASCH
        break;
    case 0xdc:
        // CALL C, a16
        if (gbcpu.f.c)
        {
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
            gbcpu.sp--;
            gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
            gbcpu.pc = gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8);
        }
        break;
    case 0xdd:
        // CRASCH
        break;
    case 0xde:
        // SBC n8
        SBC_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xdf:
        // RST 0x18
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x18;
        break;

    case 0xe0:
        // LDH [a8], A
        gb_mmap.mem[0xff00 + gb_mmap.mem[gbcpu.pc++]] = gbcpu.a;
        break;
    case 0xe1:
        // POP HL
        gbcpu.hl = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xe2:
        // LD [C], A
        gb_mmap.mem[0xff00 + gbcpu.c] = gbcpu.a;
        break;
    case 0xe3:
        // CRASCH
        break;
    case 0xe4:
        // CRASCH
        break;
    case 0xe5:
        // PUSH HL
        PUSH_RR(gbcpu.hl);
        break;
    case 0xe6:
        // AND n8
        AND_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xe7:
        // RST 0x20
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x20;
        break;
    case 0xe8:
        // ADD SP, r8
        ADD_RR_RR(gbcpu.sp, gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xe9:
        // JP HL
        gbcpu.pc = gbcpu.hl;
        break;
    case 0xea:
        // LD [a16], A
        gb_mmap.mem[gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8)] =
            gbcpu.a;
        break;
    case 0xeb:
        // CRASCH
        break;
    case 0xec:
        // CRASCH
        break;
    case 0xed:
        // CRASCH
        break;
    case 0xee:
        // XOR n8
        XOR_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xef:
        // RST 0x28
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x28;
        break;

    case 0xf0:
        // LDH A, [a8]
        gbcpu.a = gb_mmap.mem[0xff00 + gb_mmap.mem[gbcpu.pc++]];
        break;
    case 0xf1:
        // POP AF
        gbcpu.af = gb_mmap.mem[gbcpu.sp++] | (gb_mmap.mem[gbcpu.sp++] << 8);
        break;
    case 0xf2:
        // LD A, [C]
        gbcpu.a = gb_mmap.mem[0xff00 + gbcpu.c];
        break;
    case 0xf3:
        // DI
        gbcpu.ime = 0;
        break;
    case 0xf4:
        // CRASCH
        break;
    case 0xf5:
        // PUSH AF
        PUSH_RR(gbcpu.af);
        break;
    case 0xf6:
        // OR n8
        OR_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xf7:
        // RST 0x30
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x30;
        break;
    case 0xf8:
        // LD HL, SP+e8
        gbcpu.hl = gbcpu.sp + (int8_t)gb_mmap.mem[gbcpu.pc++];
        gbcpu.f.z = 0;
        gbcpu.f.n = 0;
        gbcpu.f.h = (gbcpu.hl >> 11) & 1;
        gbcpu.f.c = (gbcpu.hl >> 15) & 1;
        break;
    case 0xf9:
        // LD SP, HL
        gbcpu.sp = gbcpu.hl;
        break;
    case 0xfa:
        // LD A, [a16]
        gbcpu.a =
            gb_mmap
                .mem[gb_mmap.mem[gbcpu.pc++] | (gb_mmap.mem[gbcpu.pc++] << 8)];
        break;
    case 0xfb:
        // EI
        gbcpu.ime = 1;
        break;
    case 0xfc:
        // CRASCH
        break;
    case 0xfd:
        // CRASCH
        break;
    case 0xfe:
        // CP n8
        CP_R8(gb_mmap.mem[gbcpu.pc++]);
        break;
    case 0xff:
        // RST 0x38
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc >> 8;
        gbcpu.sp--;
        gb_mmap.mem[gbcpu.sp] = gbcpu.pc & 0xff;
        gbcpu.pc = 0x38;
        break;

    case 0xcb:
        // CB prefix shi 😭
        break;
    }
}

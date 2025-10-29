#include "gbcpu.h"
#include "gbmmap.h"
#include <stdint.h>

gbcpu_t gbcpu;

#define INC_r8(reg)                                                            \
    do                                                                         \
    {                                                                          \
        uint8_t res = (reg) + 1;                                               \
        gbcpu.f.z = res == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = ((reg & 0xF) + 1) > 0xF;                                   \
        (reg) = res;                                                           \
    } while (0)

#define DEC_r8(reg)                                                            \
    do                                                                         \
    {                                                                          \
        uint8_t res = (reg) - 1;                                               \
        gbcpu.f.z = res == 0;                                                  \
        gbcpu.f.n = 1;                                                         \
        gbcpu.f.h = (reg & 0xF) == 0;                                          \
        (reg) = res;                                                           \
    } while (0)

#define ADD_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        uint16_t res = gbcpu.a + (r);                                          \
        gbcpu.f.z = ((uint8_t)res) == 0;                                       \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = ((gbcpu.a & 0xF) + ((r) & 0xF)) > 0xF;                     \
        gbcpu.f.c = res > 0xFF;                                                \
        gbcpu.a = (uint8_t)res;                                                \
    } while (0)

#define ADC_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        uint16_t res = gbcpu.a + (r) + gbcpu.f.c;                              \
        gbcpu.f.z = ((uint8_t)res) == 0;                                       \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = ((gbcpu.a & 0xF) + ((r) & 0xF) + gbcpu.f.c) > 0xF;         \
        gbcpu.f.c = res > 0xFF;                                                \
        gbcpu.a = (uint8_t)res;                                                \
    } while (0)

#define SUB_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        uint16_t res = gbcpu.a - (r);                                          \
        gbcpu.f.z = ((uint8_t)res) == 0;                                       \
        gbcpu.f.n = 1;                                                         \
        gbcpu.f.h = (gbcpu.a & 0xF) < ((r) & 0xF);                             \
        gbcpu.f.c = gbcpu.a < (r);                                             \
        gbcpu.a = (uint8_t)res;                                                \
    } while (0)

#define SBC_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        uint16_t res = gbcpu.a - (r) - gbcpu.f.c;                              \
        gbcpu.f.z = ((uint8_t)res) == 0;                                       \
        gbcpu.f.n = 1;                                                         \
        gbcpu.f.h = (gbcpu.a & 0xF) < (((r) & 0xF) + gbcpu.f.c);               \
        gbcpu.f.c = gbcpu.a < ((r) + gbcpu.f.c);                               \
        gbcpu.a = (uint8_t)res;                                                \
    } while (0)

#define CP_R8(r)                                                               \
    do                                                                         \
    {                                                                          \
        uint16_t res = gbcpu.a - (r);                                          \
        gbcpu.f.z = ((uint8_t)res) == 0;                                       \
        gbcpu.f.n = 1;                                                         \
        gbcpu.f.h = (gbcpu.a & 0xF) < ((r) & 0xF);                             \
        gbcpu.f.c = gbcpu.a < (r);                                             \
    } while (0)

#define ADD_RR_RR(r1, r2)                                                      \
    do                                                                         \
    {                                                                          \
        uint32_t res = (r1) + (r2);                                            \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = (((r1) & 0x0FFF) + ((r2) & 0x0FFF)) > 0x0FFF;              \
        gbcpu.f.c = res > 0xFFFF;                                              \
        (r1) = (uint16_t)res;                                                  \
    } while (0)

#define AND_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        gbcpu.a &= (r);                                                        \
        gbcpu.f.z = gbcpu.a == 0;                                              \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 1;                                                         \
        gbcpu.f.c = 0;                                                         \
    } while (0)

#define OR_R8(r)                                                               \
    do                                                                         \
    {                                                                          \
        gbcpu.a |= (r);                                                        \
        gbcpu.f.z = gbcpu.a == 0;                                              \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
        gbcpu.f.c = 0;                                                         \
    } while (0)

#define XOR_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        gbcpu.a ^= (r);                                                        \
        gbcpu.f.z = gbcpu.a == 0;                                              \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
        gbcpu.f.c = 0;                                                         \
    } while (0)

#define PUSH_RR(r)                                                             \
    do                                                                         \
    {                                                                          \
        gbcpu.sp--;                                                            \
        gb_mmap.mem[gbcpu.sp] = ((r) >> 8) & 0xFF;                             \
        gbcpu.sp--;                                                            \
        gb_mmap.mem[gbcpu.sp] = (r) & 0xFF;                                    \
    } while (0)

#define RLC_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        uint8_t c = ((r) >> 7) & 1;                                            \
        (r) = ((r) << 1) | c;                                                  \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
        gbcpu.f.c = c;                                                         \
    } while (0)

#define RRC_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        uint8_t c = (r) & 1;                                                   \
        (r) = ((r) >> 1) | (c << 7);                                           \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
        gbcpu.f.c = c;                                                         \
    } while (0)

#define RL_R8(r)                                                               \
    do                                                                         \
    {                                                                          \
        uint8_t c = gbcpu.f.c;                                                 \
        gbcpu.f.c = ((r) >> 7) & 1;                                            \
        (r) = ((r) << 1) | c;                                                  \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
    } while (0)

#define RR_R8(r)                                                               \
    do                                                                         \
    {                                                                          \
        uint8_t c = gbcpu.f.c;                                                 \
        gbcpu.f.c = (r) & 1;                                                   \
        (r) = ((r) >> 1) | (c << 7);                                           \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
    } while (0)

#define SLA_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        gbcpu.f.c = ((r) >> 7) & 1;                                            \
        (r) <<= 1;                                                             \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
    } while (0)

#define SRA_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        gbcpu.f.c = (r) & 1;                                                   \
        (r) = ((r) >> 1) | (r & 0x80);                                         \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
    } while (0)

#define SRL_R8(r)                                                              \
    do                                                                         \
    {                                                                          \
        gbcpu.f.c = (r) & 1;                                                   \
        (r) >>= 1;                                                             \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
    } while (0)

#define SWAP_R8(r)                                                             \
    do                                                                         \
    {                                                                          \
        (r) = ((r) << 4) | ((r) >> 4);                                         \
        gbcpu.f.z = (r) == 0;                                                  \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 0;                                                         \
        gbcpu.f.c = 0;                                                         \
    } while (0)

#define BIT(b, r)                                                              \
    do                                                                         \
    {                                                                          \
        gbcpu.f.z = !(((r) >> (b)) & 1);                                       \
        gbcpu.f.n = 0;                                                         \
        gbcpu.f.h = 1;                                                         \
    } while (0)

#define RES(b, r) ((r) &= ~(1 << (b)))
#define SET(b, r) ((r) |= (1 << (b)))

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
        op = gb_mmap.mem[gbcpu.pc++];
        switch (op)
        {
        case 0x00:
            // RLC B
            RLC_R8(gbcpu.b);
            break;
        case 0x01:
            // RLC C
            RLC_R8(gbcpu.c);
            break;
        case 0x02:
            // RLC D
            RLC_R8(gbcpu.d);
            break;
        case 0x03:
            // RLC E
            RLC_R8(gbcpu.e);
            break;
        case 0x04:
            // RLC H
            RLC_R8(gbcpu.h);
            break;
        case 0x05:
            // RLC L
            RLC_R8(gbcpu.l);
            break;
        case 0x06:
            // RLC [HL]
            RLC_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x07:
            // RLC A
            RLC_R8(gbcpu.a);
            break;
        case 0x08:
            // RRC B
            RRC_R8(gbcpu.b);
            break;
        case 0x09:
            // RRC C
            RRC_R8(gbcpu.c);
            break;
        case 0x0a:
            // RRC D
            RRC_R8(gbcpu.d);
            break;
        case 0x0b:
            // RRC E
            RRC_R8(gbcpu.e);
            break;
        case 0x0c:
            // RRC H
            RRC_R8(gbcpu.h);
            break;
        case 0x0d:
            // RRC L
            RRC_R8(gbcpu.l);
            break;
        case 0x0e:
            // RRC [HL]
            RRC_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x0f:
            // RRC A
            RRC_R8(gbcpu.a);
            break;

        case 0x10:
            // RL B
            RL_R8(gbcpu.b);
            break;
        case 0x11:
            // RL C
            RL_R8(gbcpu.c);
            break;
        case 0x12:
            // RL D
            RL_R8(gbcpu.d);
            break;
        case 0x13:
            // RL E
            RL_R8(gbcpu.e);
            break;
        case 0x14:
            // RL H
            RL_R8(gbcpu.h);
            break;
        case 0x15:
            // RL L
            RL_R8(gbcpu.l);
            break;
        case 0x16:
            // RL [HL]
            RL_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x17:
            // RL A
            RL_R8(gbcpu.a);
            break;
        case 0x18:
            // RR B
            RR_R8(gbcpu.b);
            break;
        case 0x19:
            // RR C
            RR_R8(gbcpu.c);
            break;
        case 0x1a:
            // RR D
            RR_R8(gbcpu.d);
            break;
        case 0x1b:
            // RR E
            RR_R8(gbcpu.e);
            break;
        case 0x1c:
            // RR H
            RR_R8(gbcpu.h);
            break;
        case 0x1d:
            // RR L
            RR_R8(gbcpu.l);
            break;
        case 0x1e:
            // RR [HL]
            RR_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x1f:
            // RR A
            RR_R8(gbcpu.a);
            break;

        case 0x20:
            // SLA B
            SLA_R8(gbcpu.b);
            break;
        case 0x21:
            // SLA C
            SLA_R8(gbcpu.c);
            break;
        case 0x22:
            // SLA D
            SLA_R8(gbcpu.d);
            break;
        case 0x23:
            // SLA E
            SLA_R8(gbcpu.e);
            break;
        case 0x24:
            // SLA H
            SLA_R8(gbcpu.h);
            break;
        case 0x25:
            // SLA L
            SLA_R8(gbcpu.l);
            break;
        case 0x26:
            // SLA [HL]
            SLA_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x27:
            // SLA A
            SLA_R8(gbcpu.a);
            break;
        case 0x28:
            // SRA B
            SRA_R8(gbcpu.b);
            break;
        case 0x29:
            // SRA C
            SRA_R8(gbcpu.c);
            break;
        case 0x2a:
            // SRA D
            SRA_R8(gbcpu.d);
            break;
        case 0x2b:
            // SRA E
            SRA_R8(gbcpu.e);
            break;
        case 0x2c:
            // SRA H
            SRA_R8(gbcpu.h);
            break;
        case 0x2d:
            // SRA L
            SRA_R8(gbcpu.l);
            break;
        case 0x2e:
            // SRA [HL]
            SRA_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x2f:
            // SRA A
            SRA_R8(gbcpu.a);
            break;

        case 0x30:
            // SWAP B
            SWAP_R8(gbcpu.b);
            break;
        case 0x31:
            // SWAP C
            SWAP_R8(gbcpu.c);
            break;
        case 0x32:
            // SWAP D
            SWAP_R8(gbcpu.d);
            break;
        case 0x33:
            // SWAP E
            SWAP_R8(gbcpu.e);
            break;
        case 0x34:
            // SWAP H
            SWAP_R8(gbcpu.h);
            break;
        case 0x35:
            // SWAP L
            SWAP_R8(gbcpu.l);
            break;
        case 0x36:
            // SWAP [HL]
            SWAP_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x37:
            // SWAP A
            SWAP_R8(gbcpu.a);
            break;
        case 0x38:
            // SRL B
            SRL_R8(gbcpu.b);
            break;
        case 0x39:
            // SRL C
            SRL_R8(gbcpu.c);
            break;
        case 0x3a:
            // SRL D
            SRL_R8(gbcpu.d);
            break;
        case 0x3b:
            // SRL E
            SRL_R8(gbcpu.e);
            break;
        case 0x3c:
            // SRL H
            SRL_R8(gbcpu.h);
            break;
        case 0x3d:
            // SRL L
            SRL_R8(gbcpu.l);
            break;
        case 0x3e:
            // SRL [HL]
            SRL_R8(gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x3f:
            // SRL A
            SRL_R8(gbcpu.a);
            break;

        case 0x40:
            // BIT 0, B
            BIT(0, gbcpu.b);
            break;
        case 0x41:
            // BIT 0, C
            BIT(0, gbcpu.c);
            break;
        case 0x42:
            // BIT 0, D
            BIT(0, gbcpu.d);
            break;
        case 0x43:
            // BIT 0, E
            BIT(0, gbcpu.e);
            break;
        case 0x44:
            // BIT 0, H
            BIT(0, gbcpu.h);
            break;
        case 0x45:
            // BIT 0, L
            BIT(0, gbcpu.l);
            break;
        case 0x46:
            // BIT 0, [HL]
            BIT(0, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x47:
            // BIT 0, A
            BIT(0, gbcpu.a);
            break;
        case 0x48:
            // BIT 1, B
            BIT(1, gbcpu.b);
            break;
        case 0x49:
            // BIT 1, C
            BIT(1, gbcpu.c);
            break;
        case 0x4a:
            // BIT 1, D
            BIT(1, gbcpu.d);
            break;
        case 0x4b:
            // BIT 1, E
            BIT(1, gbcpu.e);
            break;
        case 0x4c:
            // BIT 1, H
            BIT(1, gbcpu.h);
            break;
        case 0x4d:
            // BIT 1, L
            BIT(1, gbcpu.l);
            break;
        case 0x4e:
            // BIT 1, [HL]
            BIT(1, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x4f:
            // BIT 1, A
            BIT(1, gbcpu.a);
            break;

        case 0x50:
            // BIT 2, B
            BIT(2, gbcpu.b);
            break;
        case 0x51:
            // BIT 2, C
            BIT(2, gbcpu.c);
            break;
        case 0x52:
            // BIT 2, D
            BIT(2, gbcpu.d);
            break;
        case 0x53:
            // BIT 2, E
            BIT(2, gbcpu.e);
            break;
        case 0x54:
            // BIT 2, H
            BIT(2, gbcpu.h);
            break;
        case 0x55:
            // BIT 2, L
            BIT(2, gbcpu.l);
            break;
        case 0x56:
            // BIT 2, [HL]
            BIT(2, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x57:
            // BIT 2, A
            BIT(2, gbcpu.a);
            break;
        case 0x58:
            // BIT 3, B
            BIT(3, gbcpu.b);
            break;
        case 0x59:
            // BIT 3, C
            BIT(3, gbcpu.c);
            break;
        case 0x5a:
            // BIT 3, D
            BIT(3, gbcpu.d);
            break;
        case 0x5b:
            // BIT 3, E
            BIT(3, gbcpu.e);
            break;
        case 0x5c:
            // BIT 3, H
            BIT(3, gbcpu.h);
            break;
        case 0x5d:
            // BIT 3, L
            BIT(3, gbcpu.l);
            break;
        case 0x5e:
            // BIT 3, [HL]
            BIT(3, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x5f:
            // BIT 3, A
            BIT(3, gbcpu.a);
            break;

        case 0x60:
            // BIT 4, B
            BIT(4, gbcpu.b);
            break;
        case 0x61:
            // BIT 4, C
            BIT(4, gbcpu.c);
            break;
        case 0x62:
            // BIT 4, D
            BIT(4, gbcpu.d);
            break;
        case 0x63:
            // BIT 4, E
            BIT(4, gbcpu.e);
            break;
        case 0x64:
            // BIT 4, H
            BIT(4, gbcpu.h);
            break;
        case 0x65:
            // BIT 4, L
            BIT(4, gbcpu.l);
            break;
        case 0x66:
            // BIT 4, [HL]
            BIT(4, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x67:
            // BIT 4, A
            BIT(4, gbcpu.a);
            break;
        case 0x68:
            // BIT 5, B
            BIT(5, gbcpu.b);
            break;
        case 0x69:
            // BIT 5, C
            BIT(5, gbcpu.c);
            break;
        case 0x6a:
            // BIT 5, D
            BIT(5, gbcpu.d);
            break;
        case 0x6b:
            // BIT 5, E
            BIT(5, gbcpu.e);
            break;
        case 0x6c:
            // BIT 5, H
            BIT(5, gbcpu.h);
            break;
        case 0x6d:
            // BIT 5, L
            BIT(5, gbcpu.l);
            break;
        case 0x6e:
            // BIT 5, [HL]
            BIT(5, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x6f:
            // BIT 5, A
            BIT(5, gbcpu.a);
            break;

        case 0x70:
            // BIT 6, B
            BIT(6, gbcpu.b);
            break;
        case 0x71:
            // BIT 6, C
            BIT(6, gbcpu.c);
            break;
        case 0x72:
            // BIT 6, D
            BIT(6, gbcpu.d);
            break;
        case 0x73:
            // BIT 6, E
            BIT(6, gbcpu.e);
            break;
        case 0x74:
            // BIT 6, H
            BIT(6, gbcpu.h);
            break;
        case 0x75:
            // BIT 6, L
            BIT(6, gbcpu.l);
            break;
        case 0x76:
            // BIT 6, [HL]
            BIT(6, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x77:
            // BIT 6, A
            BIT(6, gbcpu.a);
            break;
        case 0x78:
            // BIT 7, B
            BIT(7, gbcpu.b);
            break;
        case 0x79:
            // BIT 7, C
            BIT(7, gbcpu.c);
            break;
        case 0x7a:
            // BIT 7, D
            BIT(7, gbcpu.d);
            break;
        case 0x7b:
            // BIT 7, E
            BIT(7, gbcpu.e);
            break;
        case 0x7c:
            // BIT 7, H
            BIT(7, gbcpu.h);
            break;
        case 0x7d:
            // BIT 7, L
            BIT(7, gbcpu.l);
            break;
        case 0x7e:
            // BIT 7, [HL]
            BIT(7, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x7f:
            // BIT 7, A
            BIT(7, gbcpu.a);
            break;

        case 0x80:
            // RES 0, B
            RES(0, gbcpu.b);
            break;
        case 0x81:
            // RES 0, C
            RES(0, gbcpu.c);
            break;
        case 0x82:
            // RES 0, D
            RES(0, gbcpu.d);
            break;
        case 0x83:
            // RES 0, E
            RES(0, gbcpu.e);
            break;
        case 0x84:
            // RES 0, H
            RES(0, gbcpu.h);
            break;
        case 0x85:
            // RES 0, L
            RES(0, gbcpu.l);
            break;
        case 0x86:
            // RES 0, [HL]
            RES(0, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x87:
            // RES 0, A
            RES(0, gbcpu.a);
            break;
        case 0x88:
            // RES 1, B
            RES(1, gbcpu.b);
            break;
        case 0x89:
            // RES 1, C
            RES(1, gbcpu.c);
            break;
        case 0x8a:
            // RES 1, D
            RES(1, gbcpu.d);
            break;
        case 0x8b:
            // RES 1, E
            RES(1, gbcpu.e);
            break;
        case 0x8c:
            // RES 1, H
            RES(1, gbcpu.h);
            break;
        case 0x8d:
            // RES 1, L
            RES(1, gbcpu.l);
            break;
        case 0x8e:
            // RES 1, [HL]
            RES(1, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x8f:
            // RES 1, A
            RES(1, gbcpu.a);
            break;

        case 0x90:
            // RES 2, B
            RES(2, gbcpu.b);
            break;
        case 0x91:
            // RES 2, C
            RES(2, gbcpu.c);
            break;
        case 0x92:
            // RES 2, D
            RES(2, gbcpu.d);
            break;
        case 0x93:
            // RES 2, E
            RES(2, gbcpu.e);
            break;
        case 0x94:
            // RES 2, H
            RES(2, gbcpu.h);
            break;
        case 0x95:
            // RES 2, L
            RES(2, gbcpu.l);
            break;
        case 0x96:
            // RES 2, [HL]
            RES(2, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x97:
            // RES 2, A
            RES(2, gbcpu.a);
            break;
        case 0x98:
            // RES 3, B
            RES(3, gbcpu.b);
            break;
        case 0x99:
            // RES 3, C
            RES(3, gbcpu.c);
            break;
        case 0x9a:
            // RES 3, D
            RES(3, gbcpu.d);
            break;
        case 0x9b:
            // RES 3, E
            RES(3, gbcpu.e);
            break;
        case 0x9c:
            // RES 3, H
            RES(3, gbcpu.h);
            break;
        case 0x9d:
            // RES 3, L
            RES(3, gbcpu.l);
            break;
        case 0x9e:
            // RES 3, [HL]
            RES(3, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0x9f:
            // RES 3, A
            RES(3, gbcpu.a);
            break;

        case 0xa0:
            // RES 4, B
            RES(4, gbcpu.b);
            break;
        case 0xa1:
            // RES 4, C
            RES(4, gbcpu.c);
            break;
        case 0xa2:
            // RES 4, D
            RES(4, gbcpu.d);
            break;
        case 0xa3:
            // RES 4, E
            RES(4, gbcpu.e);
            break;
        case 0xa4:
            // RES 4, H
            RES(4, gbcpu.h);
            break;
        case 0xa5:
            // RES 4, L
            RES(4, gbcpu.l);
            break;
        case 0xa6:
            // RES 4, [HL]
            RES(4, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xa7:
            // RES 4, A
            RES(4, gbcpu.a);
            break;
        case 0xa8:
            // RES 5, B
            RES(5, gbcpu.b);
            break;
        case 0xa9:
            // RES 5, C
            RES(5, gbcpu.c);
            break;
        case 0xaa:
            // RES 5, D
            RES(5, gbcpu.d);
            break;
        case 0xab:
            // RES 5, E
            RES(5, gbcpu.e);
            break;
        case 0xac:
            // RES 5, H
            RES(5, gbcpu.h);
            break;
        case 0xad:
            // RES 5, L
            RES(5, gbcpu.l);
            break;
        case 0xae:
            // RES 5, [HL]
            RES(5, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xaf:
            // RES 5, A
            RES(5, gbcpu.a);
            break;

        case 0xb0:
            // RES 6, B
            RES(6, gbcpu.b);
            break;
        case 0xb1:
            // RES 6, C
            RES(6, gbcpu.c);
            break;
        case 0xb2:
            // RES 6, D
            RES(6, gbcpu.d);
            break;
        case 0xb3:
            // RES 6, E
            RES(6, gbcpu.e);
            break;
        case 0xb4:
            // RES 6, H
            RES(6, gbcpu.h);
            break;
        case 0xb5:
            // RES 6, L
            RES(6, gbcpu.l);
            break;
        case 0xb6:
            // RES 6, [HL]
            RES(6, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xb7:
            // RES 6, A
            RES(6, gbcpu.a);
            break;
        case 0xb8:
            // RES 7, B
            RES(7, gbcpu.b);
            break;
        case 0xb9:
            // RES 7, C
            RES(7, gbcpu.c);
            break;
        case 0xba:
            // RES 7, D
            RES(7, gbcpu.d);
            break;
        case 0xbb:
            // RES 7, E
            RES(7, gbcpu.e);
            break;
        case 0xbc:
            // RES 7, H
            RES(7, gbcpu.h);
            break;
        case 0xbd:
            // RES 7, L
            RES(7, gbcpu.l);
            break;
        case 0xbe:
            // RES 7, [HL]
            RES(7, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xbf:
            // RES 7, A
            RES(7, gbcpu.a);
            break;

        case 0xc0:
            // SET 0, B
            SET(0, gbcpu.b);
            break;
        case 0xc1:
            // SET 0, C
            SET(0, gbcpu.c);
            break;
        case 0xc2:
            // SET 0, D
            SET(0, gbcpu.d);
            break;
        case 0xc3:
            // SET 0, E
            SET(0, gbcpu.e);
            break;
        case 0xc4:
            // SET 0, H
            SET(0, gbcpu.h);
            break;
        case 0xc5:
            // SET 0, L
            SET(0, gbcpu.l);
            break;
        case 0xc6:
            // SET 0, [HL]
            SET(0, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xc7:
            // SET 0, A
            SET(0, gbcpu.a);
            break;
        case 0xc8:
            // SET 1, B
            SET(1, gbcpu.b);
            break;
        case 0xc9:
            // SET 1, C
            SET(1, gbcpu.c);
            break;
        case 0xca:
            // SET 1, D
            SET(1, gbcpu.d);
            break;
        case 0xcb:
            // SET 1, E
            SET(1, gbcpu.e);
            break;
        case 0xcc:
            // SET 1, H
            SET(1, gbcpu.h);
            break;
        case 0xcd:
            // SET 1, L
            SET(1, gbcpu.l);
            break;
        case 0xce:
            // SET 1, [HL]
            SET(1, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xcf:
            // SET 1, A
            SET(1, gbcpu.a);
            break;

        case 0xd0:
            // SET 2, B
            SET(2, gbcpu.b);
            break;
        case 0xd1:
            // SET 2, C
            SET(2, gbcpu.c);
            break;
        case 0xd2:
            // SET 2, D
            SET(2, gbcpu.d);
            break;
        case 0xd3:
            // SET 2, E
            SET(2, gbcpu.e);
            break;
        case 0xd4:
            // SET 2, H
            SET(2, gbcpu.h);
            break;
        case 0xd5:
            // SET 2, L
            SET(2, gbcpu.l);
            break;
        case 0xd6:
            // SET 2, [HL]
            SET(2, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xd7:
            // SET 2, A
            SET(2, gbcpu.a);
            break;
        case 0xd8:
            // SET 3, B
            SET(3, gbcpu.b);
            break;
        case 0xd9:
            // SET 3, C
            SET(3, gbcpu.c);
            break;
        case 0xda:
            // SET 3, D
            SET(3, gbcpu.d);
            break;
        case 0xdb:
            // SET 3, E
            SET(3, gbcpu.e);
            break;
        case 0xdc:
            // SET 3, H
            SET(3, gbcpu.h);
            break;
        case 0xdd:
            // SET 3, L
            SET(3, gbcpu.l);
            break;
        case 0xde:
            // SET 3, [HL]
            SET(3, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xdf:
            // SET 3, A
            SET(3, gbcpu.a);
            break;

        case 0xe0:
            // SET 4, B
            SET(4, gbcpu.b);
            break;
        case 0xe1:
            // SET 4, C
            SET(4, gbcpu.c);
            break;
        case 0xe2:
            // SET 4, D
            SET(4, gbcpu.d);
            break;
        case 0xe3:
            // SET 4, E
            SET(4, gbcpu.e);
            break;
        case 0xe4:
            // SET 4, H
            SET(4, gbcpu.h);
            break;
        case 0xe5:
            // SET 4, L
            SET(4, gbcpu.l);
            break;
        case 0xe6:
            // SET 4, [HL]
            SET(4, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xe7:
            // SET 4, A
            SET(4, gbcpu.a);
            break;
        case 0xe8:
            // SET 5, B
            SET(5, gbcpu.b);
            break;
        case 0xe9:
            // SET 5, C
            SET(5, gbcpu.c);
            break;
        case 0xea:
            // SET 5, D
            SET(5, gbcpu.d);
            break;
        case 0xeb:
            // SET 5, E
            SET(5, gbcpu.e);
            break;
        case 0xec:
            // SET 5, H
            SET(5, gbcpu.h);
            break;
        case 0xed:
            // SET 5, L
            SET(5, gbcpu.l);
            break;
        case 0xee:
            // SET 5, [HL]
            SET(5, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xef:
            // SET 5, A
            SET(5, gbcpu.a);
            break;

        case 0xf0:
            // SET 6, B
            SET(6, gbcpu.b);
            break;
        case 0xf1:
            // SET 6, C
            SET(6, gbcpu.c);
            break;
        case 0xf2:
            // SET 6, D
            SET(6, gbcpu.d);
            break;
        case 0xf3:
            // SET 6, E
            SET(6, gbcpu.e);
            break;
        case 0xf4:
            // SET 6, H
            SET(6, gbcpu.h);
            break;
        case 0xf5:
            // SET 6, L
            SET(6, gbcpu.l);
            break;
        case 0xf6:
            // SET 6, [HL]
            SET(6, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xf7:
            // SET 6, A
            SET(6, gbcpu.a);
            break;
        case 0xf8:
            // SET 7, B
            SET(7, gbcpu.b);
            break;
        case 0xf9:
            // SET 7, C
            SET(7, gbcpu.c);
            break;
        case 0xfa:
            // SET 7, D
            SET(7, gbcpu.d);
            break;
        case 0xfb:
            // SET 7, E
            SET(7, gbcpu.e);
            break;
        case 0xfc:
            // SET 7, H
            SET(7, gbcpu.h);
            break;
        case 0xfd:
            // SET 7, L
            SET(7, gbcpu.l);
            break;
        case 0xfe:
            // SET 7, [HL]
            SET(7, gb_mmap.mem[gbcpu.hl]);
            break;
        case 0xff:
            // SET 7, A
            SET(7, gbcpu.a);
            break;
        }
        break;
    }
}

#include "opcode.h"
#include "gb.h"
#include "logger.h"
#include "mbus.h"
#include "types.h"

static const opcode_t opcodes[0x100];
static const opcode_t cb_opcodes[0x100];

u8 cpu_step(gb_t *gb) {
    if (gb->cpu->halted || gb->cpu->stopped) {
        gb_clock_advance(gb, 4);
        return 1;
    }

    u8 opcode = mbus_read(gb, gb->cpu->pc++);
    opcode_t op = opcodes[opcode];
    if (!op.fn) {
        log_error("Unknown opcode 0x%x (%s) at 0x%x\n", opcode, op.name,
                  gb->cpu->pc - 1);
        return 0;
    }

    op.fn(gb);
    return 1;
}

#define ld_r16_n16(r16)                                                        \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        r16 = mbus_read(gb, gb->cpu->pc++);                                    \
        gb_clock_advance(gb, 4);                                               \
        r16 |= mbus_read(gb, gb->cpu->pc++) << 8;                              \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define ld_r8_n8(r8)                                                           \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        r8 = mbus_read(gb, gb->cpu->pc++);                                     \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define ld_ar16_r8(r16, r8)                                                    \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, r16, r8);                                               \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define ld_r8_ar16(r8, r16)                                                    \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        r8 = mbus_read(gb, r16);                                               \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define ld_adra16_r16(r16)                                                     \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        u8 _lo = mbus_read(gb, gb->cpu->pc++);                                 \
        gb_clock_advance(gb, 4);                                               \
        u8 _hi = mbus_read(gb, gb->cpu->pc++);                                 \
        u16 _addr = (u16)_lo | ((u16)_hi << 8);                                \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, _addr, (u8)((u16)(r16) & 0xFF));                        \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, _addr + 1, (u8)(((u16)(r16) >> 8) & 0xFF));             \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define inc_r16(r16)                                                           \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        r16++;                                                                 \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define dec_r16(r16)                                                           \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        r16--;                                                                 \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define inc_r8(r8)                                                             \
    do {                                                                       \
        r8++;                                                                  \
        gb->cpu->f.z = r8 == 0;                                                \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = (r8 & 0xf) == 0;                                        \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define dec_r8(r8)                                                             \
    do {                                                                       \
        r8--;                                                                  \
        gb->cpu->f.z = r8 == 0;                                                \
        gb->cpu->f.n = 1;                                                      \
        gb->cpu->f.h = (r8 & 0xf) == 0xf;                                      \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define add_r16_r16(rdest, rsrc)                                               \
    do {                                                                       \
        u16 _a = (u16)(rdest);                                                 \
        u16 _b = (u16)(rsrc);                                                  \
        u32 _result = (u32)_a + (u32)_b;                                       \
                                                                               \
        gb_clock_advance(gb, 4);                                               \
        rdest = (u16)_result;                                                  \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = ((_a & 0x0FFF) + (_b & 0x0FFF)) > 0x0FFF;               \
        gb->cpu->f.c = _result > 0xFFFF;                                       \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define rlc_r8(r8)                                                             \
    do {                                                                       \
        u8 carry = r8 >> 7;                                                    \
        r8 = (r8 << 1) | carry;                                                \
        gb->cpu->f.z = r8 == 0;                                                \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = 0;                                                      \
        gb->cpu->f.c = carry;                                                  \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define rrc_r8(r8)                                                             \
    do {                                                                       \
        u8 carry = r8 & 1;                                                     \
        r8 = (r8 >> 1) | (carry << 7);                                         \
        gb->cpu->f.z = r8 == 0;                                                \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = 0;                                                      \
        gb->cpu->f.c = carry;                                                  \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define push_r16(r16)                                                          \
    do {                                                                       \
        gb->cpu->sp--;                                                         \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, gb->cpu->sp, (u8)(((u16)(r16) >> 8) & 0xFF));           \
        gb->cpu->sp--;                                                         \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, gb->cpu->sp, (u8)((u16)(r16) & 0xFF));                  \
        gb_clock_advance(gb, 4);                                               \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define pop_r16(r16)                                                           \
    do {                                                                       \
        u8 _lo = mbus_read(gb, gb->cpu->sp++);                                 \
        gb_clock_advance(gb, 4);                                               \
        u8 _hi = mbus_read(gb, gb->cpu->sp++);                                 \
        gb_clock_advance(gb, 4);                                               \
        (r16) = (u16)_lo | ((u16)_hi << 8);                                    \
        gb_clock_advance(gb, 4);                                               \
    } while (0)

#define jr_e8(cond)                                                            \
    do {                                                                       \
        gb_clock_advance(gb, 4);                                               \
        s8 _e = (s8)mbus_read(gb, gb->cpu->pc++);                              \
        gb_clock_advance(gb, 4);                                               \
        if (cond) {                                                            \
            gb->cpu->pc = (u16)(gb->cpu->pc + _e);                             \
            gb_clock_advance(gb, 4);                                           \
        }                                                                      \
    } while (0)

#define alu_add(v)                                                             \
    do {                                                                       \
        u8 _v = (u8)(v);                                                       \
        u16 _res = (u16)gb->cpu->a + _v;                                       \
        gb->cpu->f.h = ((gb->cpu->a & 0x0F) + (_v & 0x0F)) > 0x0F;             \
        gb->cpu->f.c = _res > 0xFF;                                            \
        gb->cpu->a = (u8)_res;                                                 \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
    } while (0)

#define alu_adc(v)                                                             \
    do {                                                                       \
        u8 _v = (u8)(v);                                                       \
        u8 _c = gb->cpu->f.c;                                                  \
        u16 _res = (u16)gb->cpu->a + _v + _c;                                  \
        gb->cpu->f.h = ((gb->cpu->a & 0x0F) + (_v & 0x0F) + _c) > 0x0F;        \
        gb->cpu->f.c = _res > 0xFF;                                            \
        gb->cpu->a = (u8)_res;                                                 \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
    } while (0)

#define alu_sub(v)                                                             \
    do {                                                                       \
        u8 _v = (u8)(v);                                                       \
        gb->cpu->f.h = (gb->cpu->a & 0x0F) < (_v & 0x0F);                      \
        gb->cpu->f.c = gb->cpu->a < _v;                                        \
        gb->cpu->a -= _v;                                                      \
        gb->cpu->f.n = 1;                                                      \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
    } while (0)

#define alu_sbc(v)                                                             \
    do {                                                                       \
        u8 _v = (u8)(v);                                                       \
        u8 _c = gb->cpu->f.c;                                                  \
        s16 _res = (s16)gb->cpu->a - _v - _c;                                  \
        gb->cpu->f.h = ((gb->cpu->a & 0x0F) - (_v & 0x0F) - _c) < 0;           \
        gb->cpu->f.c = _res < 0;                                               \
        gb->cpu->a = (u8)_res;                                                 \
        gb->cpu->f.n = 1;                                                      \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
    } while (0)

#define alu_and(v)                                                             \
    do {                                                                       \
        gb->cpu->a &= (u8)(v);                                                 \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = 1;                                                      \
        gb->cpu->f.c = 0;                                                      \
    } while (0)

#define alu_xor(v)                                                             \
    do {                                                                       \
        gb->cpu->a ^= (u8)(v);                                                 \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = 0;                                                      \
        gb->cpu->f.c = 0;                                                      \
    } while (0)

#define alu_or(v)                                                              \
    do {                                                                       \
        gb->cpu->a |= (u8)(v);                                                 \
        gb->cpu->f.z = gb->cpu->a == 0;                                        \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = 0;                                                      \
        gb->cpu->f.c = 0;                                                      \
    } while (0)

#define alu_cp(v)                                                              \
    do {                                                                       \
        u8 _v = (u8)(v);                                                       \
        gb->cpu->f.z = gb->cpu->a == _v;                                       \
        gb->cpu->f.n = 1;                                                      \
        gb->cpu->f.h = (gb->cpu->a & 0x0F) < (_v & 0x0F);                      \
        gb->cpu->f.c = gb->cpu->a < _v;                                        \
    } while (0)

// LD r, r' register copies ---------------------------------------------------
#define ld_r8_r8(d, s) static void ld_##d##_##s(gb_t *gb) {                     \
        gb_clock_advance(gb, 4);                                               \
        gb->cpu->d = gb->cpu->s;                                               \
    }

#define ld_r8_hl(d) static void ld_##d##_hl(gb_t *gb) {                        \
        gb_clock_advance(gb, 4);                                               \
        gb->cpu->d = mbus_read(gb, gb->cpu->hl);                               \
        gb_clock_advance(gb, 4);                                               \
    }

#define ld_hl_r8(s) static void ld_hl_##s(gb_t *gb) {                          \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, gb->cpu->hl, gb->cpu->s);                               \
        gb_clock_advance(gb, 4);                                               \
    }

#define ALL_LD_DST(dst)                                                        \
    ld_r8_r8(dst, b) ld_r8_r8(dst, c) ld_r8_r8(dst, d) ld_r8_r8(dst, e)        \
        ld_r8_r8(dst, h) ld_r8_r8(dst, l) ld_r8_hl(dst) ld_r8_r8(dst, a)

ALL_LD_DST(b)
ALL_LD_DST(c)
ALL_LD_DST(d)
ALL_LD_DST(e)
ALL_LD_DST(h)
ALL_LD_DST(l)

ld_hl_r8(b) ld_hl_r8(c) ld_hl_r8(d) ld_hl_r8(e) ld_hl_r8(h) ld_hl_r8(l)
    ld_hl_r8(a)

ALL_LD_DST(a)

// ALU register/HL/n8 ---------------------------------------------------------
#define DEF_ALU_R8(op, d) static void op##_##d(gb_t *gb) {                     \
        alu_##op(gb->cpu->d);                                                  \
        gb_clock_advance(gb, 4);                                               \
    }

#define DEF_ALU_HL(op) static void op##_hl(gb_t *gb) {                         \
        alu_##op(mbus_read(gb, gb->cpu->hl));                                  \
        gb_clock_advance(gb, 8);                                               \
    }

#define DEF_ALU_N8(op) static void op##_n8(gb_t *gb) {                         \
        gb_clock_advance(gb, 4);                                               \
        alu_##op(mbus_read(gb, gb->cpu->pc++));                                \
        gb_clock_advance(gb, 4);                                               \
    }

#define ALL_ALU(op)                                                            \
    DEF_ALU_R8(op, b) DEF_ALU_R8(op, c) DEF_ALU_R8(op, d) DEF_ALU_R8(op, e)    \
        DEF_ALU_R8(op, h) DEF_ALU_R8(op, l) DEF_ALU_HL(op) DEF_ALU_R8(op, a)   \
            DEF_ALU_N8(op)

ALL_ALU(add)
ALL_ALU(adc)
ALL_ALU(sub)
ALL_ALU(sbc)
ALL_ALU(and)
ALL_ALU(xor)
ALL_ALU(or)
ALL_ALU(cp)

// 8-bit loads ----------------------------------------------------------------
static void nop(gb_t *gb) { gb_clock_advance(gb, 4); }
static void ld_bc_n16(gb_t *gb) { ld_r16_n16(gb->cpu->bc); }
static void ld_abc_a(gb_t *gb) { ld_ar16_r8(gb->cpu->bc, gb->cpu->a); }
static void inc_bc(gb_t *gb) { inc_r16(gb->cpu->bc); }
static void inc_b(gb_t *gb) { inc_r8(gb->cpu->b); }
static void dec_b(gb_t *gb) { dec_r8(gb->cpu->b); }
static void ld_b_n8(gb_t *gb) { ld_r8_n8(gb->cpu->b); }
static void rlca(gb_t *gb) {
    rlc_r8(gb->cpu->a);
    gb->cpu->f.z = 0; // non 0xcb rotates always set z to 0
}
static void ld_addra16_sp(gb_t *gb) { ld_adra16_r16(gb->cpu->sp); }
static void add_hl_bc(gb_t *gb) { add_r16_r16(gb->cpu->hl, gb->cpu->bc); }
static void ld_a_abc(gb_t *gb) { ld_r8_ar16(gb->cpu->a, gb->cpu->bc); }
static void dec_bc(gb_t *gb) { dec_r16(gb->cpu->bc); }
static void inc_c(gb_t *gb) { inc_r8(gb->cpu->c); }
static void dec_c(gb_t *gb) { dec_r8(gb->cpu->c); }
static void ld_c_n8(gb_t *gb) { ld_r8_n8(gb->cpu->c); }
static void rrca(gb_t *gb) {
    rrc_r8(gb->cpu->a);
    gb->cpu->f.z = 0; // non 0xcb rotates always set z to 0
}
static void stop_n8(gb_t *gb) {
    gb_clock_advance(gb, 4);
    mbus_read(gb, gb->cpu->pc++);
    // TODO SET STOPPED STATE
    gb->cpu->stopped = 1;
}
static void ld_de_n16(gb_t *gb) { ld_r16_n16(gb->cpu->de); }
static void ld_ade_a(gb_t *gb) { ld_ar16_r8(gb->cpu->de, gb->cpu->a); }
static void inc_de(gb_t *gb) { inc_r16(gb->cpu->de); }
static void inc_d(gb_t *gb) { inc_r8(gb->cpu->d); }
static void dec_d(gb_t *gb) { dec_r8(gb->cpu->d); }
static void ld_d_n8(gb_t *gb) { ld_r8_n8(gb->cpu->d); }
static void rla(gb_t *gb) {
    u8 c = gb->cpu->a >> 7;
    gb->cpu->a = (gb->cpu->a << 1) | gb->cpu->f.c;
    gb->cpu->f.z = 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = c;
    gb_clock_advance(gb, 4);
}
static void add_hl_de(gb_t *gb) { add_r16_r16(gb->cpu->hl, gb->cpu->de); }
static void ld_a_ade(gb_t *gb) { ld_r8_ar16(gb->cpu->a, gb->cpu->de); }
static void dec_de(gb_t *gb) { dec_r16(gb->cpu->de); }
static void inc_e(gb_t *gb) { inc_r8(gb->cpu->e); }
static void dec_e(gb_t *gb) { dec_r8(gb->cpu->e); }
static void ld_e_n8(gb_t *gb) { ld_r8_n8(gb->cpu->e); }
static void rra(gb_t *gb) {
    u8 c = gb->cpu->a & 1;
    gb->cpu->a = (gb->cpu->a >> 1) | (gb->cpu->f.c << 7);
    gb->cpu->f.z = 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = c;
    gb_clock_advance(gb, 4);
}
static void jr_nz(gb_t *gb) { jr_e8(!gb->cpu->f.z); }
static void jr(gb_t *gb) { jr_e8(1); }
static void ld_hl_n16(gb_t *gb) { ld_r16_n16(gb->cpu->hl); }
static void ld_ahli_a(gb_t *gb) {
    ld_ar16_r8(gb->cpu->hl, gb->cpu->a);
    gb->cpu->hl++;
}
static void inc_hl(gb_t *gb) { inc_r16(gb->cpu->hl); }
static void inc_h(gb_t *gb) { inc_r8(gb->cpu->h); }
static void dec_h(gb_t *gb) { dec_r8(gb->cpu->h); }
static void ld_h_n8(gb_t *gb) { ld_r8_n8(gb->cpu->h); }
static void daa(gb_t *gb) {
    u8 adjust = 0;
    if (gb->cpu->f.h || (gb->cpu->a & 0x0F) > 0x09)
        adjust |= 0x06;
    if (gb->cpu->f.c || gb->cpu->a > 0x99) {
        adjust |= 0x60;
        gb->cpu->f.c = 1;
    }
    gb->cpu->a =
        (u8)(gb->cpu->f.n ? gb->cpu->a - adjust : gb->cpu->a + adjust);
    gb->cpu->f.z = gb->cpu->a == 0;
    gb->cpu->f.h = 0;
    gb_clock_advance(gb, 4);
}
static void jr_z(gb_t *gb) { jr_e8(gb->cpu->f.z); }
static void add_hl_hl(gb_t *gb) { add_r16_r16(gb->cpu->hl, gb->cpu->hl); }
static void ld_a_ahli(gb_t *gb) {
    ld_r8_ar16(gb->cpu->a, gb->cpu->hl);
    gb->cpu->hl++;
}
static void dec_hl(gb_t *gb) { dec_r16(gb->cpu->hl); }
static void inc_l(gb_t *gb) { inc_r8(gb->cpu->l); }
static void dec_l(gb_t *gb) { dec_r8(gb->cpu->l); }
static void ld_l_n8(gb_t *gb) { ld_r8_n8(gb->cpu->l); }
static void cpl(gb_t *gb) {
    gb->cpu->a = ~gb->cpu->a;
    gb->cpu->f.n = 1;
    gb->cpu->f.h = 1;
    gb_clock_advance(gb, 4);
}
static void jr_nc(gb_t *gb) { jr_e8(!gb->cpu->f.c); }
static void ld_sp_n16(gb_t *gb) { ld_r16_n16(gb->cpu->sp); }
static void ld_ahld_a(gb_t *gb) {
    ld_ar16_r8(gb->cpu->hl, gb->cpu->a);
    gb->cpu->hl--;
}
static void inc_sp(gb_t *gb) { inc_r16(gb->cpu->sp); }
static void inc_ahl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    v++;
    mbus_write(gb, gb->cpu->hl, v);
    gb->cpu->f.z = v == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = (v & 0xf) == 0;
    gb_clock_advance(gb, 12);
}
static void dec_ahl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    v--;
    mbus_write(gb, gb->cpu->hl, v);
    gb->cpu->f.z = v == 0;
    gb->cpu->f.n = 1;
    gb->cpu->f.h = (v & 0xf) == 0xf;
    gb_clock_advance(gb, 12);
}
static void ld_ahl_n8(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 v = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    mbus_write(gb, gb->cpu->hl, v);
    gb_clock_advance(gb, 4);
}
static void scf(gb_t *gb) {
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = 1;
    gb_clock_advance(gb, 4);
}
static void jr_c(gb_t *gb) { jr_e8(gb->cpu->f.c); }
static void add_hl_sp(gb_t *gb) { add_r16_r16(gb->cpu->hl, gb->cpu->sp); }
static void ld_a_ahld(gb_t *gb) {
    ld_r8_ar16(gb->cpu->a, gb->cpu->hl);
    gb->cpu->hl--;
}
static void dec_sp(gb_t *gb) { dec_r16(gb->cpu->sp); }
static void inc_a(gb_t *gb) { inc_r8(gb->cpu->a); }
static void dec_a(gb_t *gb) { dec_r8(gb->cpu->a); }
static void ld_a_n8(gb_t *gb) { ld_r8_n8(gb->cpu->a); }
static void ccf(gb_t *gb) {
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = !gb->cpu->f.c;
    gb_clock_advance(gb, 4);
}
static void halt(gb_t *gb) {
    gb->cpu->halted = 1;
    gb_clock_advance(gb, 4);
}

// Stack / jump / call / return ----------------------------------------------
static void push_bc(gb_t *gb) { push_r16(gb->cpu->bc); }
static void push_de(gb_t *gb) { push_r16(gb->cpu->de); }
static void push_hl(gb_t *gb) { push_r16(gb->cpu->hl); }
static void push_af(gb_t *gb) { push_r16(gb->cpu->af); }
static void pop_bc(gb_t *gb) { pop_r16(gb->cpu->bc); }
static void pop_de(gb_t *gb) { pop_r16(gb->cpu->de); }
static void pop_hl(gb_t *gb) { pop_r16(gb->cpu->hl); }
static void pop_af(gb_t *gb) { pop_r16(gb->cpu->af); }

#define DEF_JP_CC(name, cond)                                                  \
    static void name(gb_t *gb) {                                               \
        gb_clock_advance(gb, 4);                                               \
        u8 lo = mbus_read(gb, gb->cpu->pc++);                                  \
        gb_clock_advance(gb, 4);                                               \
        u8 hi = mbus_read(gb, gb->cpu->pc++);                                  \
        gb_clock_advance(gb, 4);                                               \
        if (cond) {                                                            \
            gb->cpu->pc = (u16)lo | ((u16)hi << 8);                            \
            gb_clock_advance(gb, 4);                                           \
        }                                                                      \
    }

DEF_JP_CC(jp_nz, !gb->cpu->f.z)
DEF_JP_CC(jp_z, gb->cpu->f.z)
DEF_JP_CC(jp_nc, !gb->cpu->f.c)
DEF_JP_CC(jp_c, gb->cpu->f.c)

static void jp_a16(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 lo = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    u8 hi = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    gb->cpu->pc = (u16)lo | ((u16)hi << 8);
    gb_clock_advance(gb, 4);
}
static void jp_hl(gb_t *gb) {
    gb->cpu->pc = gb->cpu->hl;
    gb_clock_advance(gb, 4);
}
static void ld_sp_hl(gb_t *gb) {
    gb->cpu->sp = gb->cpu->hl;
    gb_clock_advance(gb, 4);
    gb_clock_advance(gb, 4);
}

#define DEF_CALL_CC(name, cond)                                                \
    static void name(gb_t *gb) {                                               \
        gb_clock_advance(gb, 4);                                               \
        u8 lo = mbus_read(gb, gb->cpu->pc++);                                  \
        gb_clock_advance(gb, 4);                                               \
        u8 hi = mbus_read(gb, gb->cpu->pc++);                                  \
        gb_clock_advance(gb, 4);                                               \
        if (cond) {                                                            \
            gb->cpu->sp--;                                                     \
            gb_clock_advance(gb, 4);                                           \
            mbus_write(gb, gb->cpu->sp, (gb->cpu->pc >> 8) & 0xFF);            \
            gb->cpu->sp--;                                                     \
            gb_clock_advance(gb, 4);                                           \
            mbus_write(gb, gb->cpu->sp, gb->cpu->pc & 0xFF);                   \
            gb->cpu->pc = (u16)lo | ((u16)hi << 8);                            \
            gb_clock_advance(gb, 4);                                           \
        }                                                                      \
    }

DEF_CALL_CC(call_nz, !gb->cpu->f.z)
DEF_CALL_CC(call_z, gb->cpu->f.z)
DEF_CALL_CC(call_nc, !gb->cpu->f.c)
DEF_CALL_CC(call_c, gb->cpu->f.c)

static void call_a16(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 lo = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    u8 hi = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    gb->cpu->sp--;
    gb_clock_advance(gb, 4);
    mbus_write(gb, gb->cpu->sp, (gb->cpu->pc >> 8) & 0xFF);
    gb->cpu->sp--;
    gb_clock_advance(gb, 4);
    mbus_write(gb, gb->cpu->sp, gb->cpu->pc & 0xFF);
    gb->cpu->pc = (u16)lo | ((u16)hi << 8);
    gb_clock_advance(gb, 4);
}

static void ret(gb_t *gb) {
    u8 lo = mbus_read(gb, gb->cpu->sp++);
    gb_clock_advance(gb, 4);
    u8 hi = mbus_read(gb, gb->cpu->sp++);
    gb_clock_advance(gb, 4);
    gb->cpu->pc = (u16)lo | ((u16)hi << 8);
    gb_clock_advance(gb, 4);
    gb_clock_advance(gb, 4);
}

#define DEF_RET_CC(name, cond)                                                 \
    static void name(gb_t *gb) {                                               \
        gb_clock_advance(gb, 4);                                               \
        if (cond) {                                                            \
            u8 lo = mbus_read(gb, gb->cpu->sp++);                              \
            gb_clock_advance(gb, 4);                                           \
            u8 hi = mbus_read(gb, gb->cpu->sp++);                              \
            gb_clock_advance(gb, 4);                                           \
            gb->cpu->pc = (u16)lo | ((u16)hi << 8);                            \
            gb_clock_advance(gb, 4);                                           \
            gb_clock_advance(gb, 4);                                           \
        }                                                                      \
    }

DEF_RET_CC(ret_nz, !gb->cpu->f.z)
DEF_RET_CC(ret_z, gb->cpu->f.z)
DEF_RET_CC(ret_nc, !gb->cpu->f.c)
DEF_RET_CC(ret_c, gb->cpu->f.c)

static void reti(gb_t *gb) {
    ret(gb);
    gb->cpu->ime = 1;
}

#define DEF_RST(name, addr)                                                    \
    static void name(gb_t *gb) {                                               \
        gb->cpu->sp--;                                                         \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, gb->cpu->sp, (gb->cpu->pc >> 8) & 0xFF);                \
        gb->cpu->sp--;                                                         \
        gb_clock_advance(gb, 4);                                               \
        mbus_write(gb, gb->cpu->sp, gb->cpu->pc & 0xFF);                       \
        gb->cpu->pc = (u16)(addr);                                             \
        gb_clock_advance(gb, 4);                                               \
        gb_clock_advance(gb, 4);                                               \
    }

DEF_RST(rst_00, 0x00)
DEF_RST(rst_08, 0x08)
DEF_RST(rst_10, 0x10)
DEF_RST(rst_18, 0x18)
DEF_RST(rst_20, 0x20)
DEF_RST(rst_28, 0x28)
DEF_RST(rst_30, 0x30)
DEF_RST(rst_38, 0x38)

// Interrupts / misc ----------------------------------------------------------
static void di(gb_t *gb) {
    gb->cpu->ime = 0;
    gb_clock_advance(gb, 4);
}
static void ei(gb_t *gb) {
    // TODO: take effect after the following instruction
    gb->cpu->ime = 1;
    gb_clock_advance(gb, 4);
}

// I/O and 16-bit address loads ------------------------------------------------
static void ldh_n8_a(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 off = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    mbus_write(gb, 0xFF00 + off, gb->cpu->a);
    gb_clock_advance(gb, 4);
}
static void ldh_c_a(gb_t *gb) {
    mbus_write(gb, 0xFF00 + gb->cpu->c, gb->cpu->a);
    gb_clock_advance(gb, 4);
    gb_clock_advance(gb, 4);
}
static void ld_aa16_a(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 lo = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    u8 hi = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    mbus_write(gb, (u16)lo | ((u16)hi << 8), gb->cpu->a);
    gb_clock_advance(gb, 4);
}
static void ldh_a_n8(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 off = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    gb->cpu->a = mbus_read(gb, 0xFF00 + off);
    gb_clock_advance(gb, 4);
}
static void ldh_a_c(gb_t *gb) {
    gb->cpu->a = mbus_read(gb, 0xFF00 + gb->cpu->c);
    gb_clock_advance(gb, 4);
    gb_clock_advance(gb, 4);
}
static void ld_a_aa16(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 lo = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    u8 hi = mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    gb->cpu->a = mbus_read(gb, (u16)lo | ((u16)hi << 8));
    gb_clock_advance(gb, 4);
}

// SP + e8 --------------------------------------------------------------------
static void add_sp_e8(gb_t *gb) {
    gb_clock_advance(gb, 4);
    s8 e = (s8)mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    u16 e16 = (u16)e;
    u16 sp = gb->cpu->sp;
    gb->cpu->f.z = 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = ((sp & 0x0F) + (e16 & 0x0F)) > 0x0F;
    gb->cpu->f.c = ((sp & 0xFF) + (e16 & 0xFF)) > 0xFF;
    gb->cpu->sp = (u16)(sp + e16);
    gb_clock_advance(gb, 4);
    gb_clock_advance(gb, 4);
}
static void ld_hl_spplus8(gb_t *gb) {
    gb_clock_advance(gb, 4);
    s8 e = (s8)mbus_read(gb, gb->cpu->pc++);
    gb_clock_advance(gb, 4);
    u16 e16 = (u16)e;
    u16 sp = gb->cpu->sp;
    gb->cpu->f.z = 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = ((sp & 0x0F) + (e16 & 0x0F)) > 0x0F;
    gb->cpu->f.c = ((sp & 0xFF) + (e16 & 0xFF)) > 0xFF;
    gb->cpu->hl = (u16)(sp + e16);
    gb_clock_advance(gb, 4);
    gb_clock_advance(gb, 4);
}

// CB prefix ------------------------------------------------------------------
static void cb_prefix(gb_t *gb) {
    gb_clock_advance(gb, 4);
    u8 opcode = mbus_read(gb, gb->cpu->pc++);
    opcode_t op = cb_opcodes[opcode];
    if (!op.fn) {
        log_error("Unknown cb opcode 0x%x (%s)\n", opcode, op.name);
        return;
    }
    op.fn(gb);
}

// CB rotate / shift ----------------------------------------------------------
static void cb_rlc(gb_t *gb, u8 *r) {
    u8 v = *r;
    *r = (v << 1) | (v >> 7);
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v >> 7;
    gb_clock_advance(gb, 4);
}
static void cb_rrc(gb_t *gb, u8 *r) {
    u8 v = *r;
    *r = (v >> 1) | (v << 7);
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 4);
}
static void cb_rl(gb_t *gb, u8 *r) {
    u8 v = *r;
    u8 c = gb->cpu->f.c;
    *r = (v << 1) | c;
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v >> 7;
    gb_clock_advance(gb, 4);
}
static void cb_rr(gb_t *gb, u8 *r) {
    u8 v = *r;
    u8 c = gb->cpu->f.c;
    *r = (v >> 1) | (c << 7);
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 4);
}
static void cb_sla(gb_t *gb, u8 *r) {
    u8 v = *r;
    *r = v << 1;
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v >> 7;
    gb_clock_advance(gb, 4);
}
static void cb_sra(gb_t *gb, u8 *r) {
    u8 v = *r;
    *r = (v >> 1) | (v & 0x80);
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 4);
}
static void cb_swap(gb_t *gb, u8 *r) {
    u8 v = *r;
    *r = (v << 4) | (v >> 4);
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = 0;
    gb_clock_advance(gb, 4);
}
static void cb_srl(gb_t *gb, u8 *r) {
    u8 v = *r;
    *r = v >> 1;
    gb->cpu->f.z = *r == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 4);
}

#define DEF_CB_R(op, reg) static void op##_##reg(gb_t *gb) {                   \
        cb_##op(gb, &gb->cpu->reg);                                            \
    }

#define ALL_CB_R(op)                                                           \
    DEF_CB_R(op, b) DEF_CB_R(op, c) DEF_CB_R(op, d) DEF_CB_R(op, e)            \
        DEF_CB_R(op, h) DEF_CB_R(op, l) DEF_CB_R(op, a)

ALL_CB_R(rlc)
ALL_CB_R(rrc)
ALL_CB_R(rl)
ALL_CB_R(rr)
ALL_CB_R(sla)
ALL_CB_R(sra)
ALL_CB_R(swap)
ALL_CB_R(srl)

static void rlc_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 c = v >> 7;
    v = (v << 1) | c;
    mbus_write(gb, gb->cpu->hl, v);
    gb->cpu->f.z = v == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = c;
    gb_clock_advance(gb, 12);
}
static void rrc_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 c = v & 1;
    v = (v >> 1) | (c << 7);
    mbus_write(gb, gb->cpu->hl, v);
    gb->cpu->f.z = v == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = c;
    gb_clock_advance(gb, 12);
}
static void rl_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 c = gb->cpu->f.c;
    u8 nv = (v << 1) | c;
    mbus_write(gb, gb->cpu->hl, nv);
    gb->cpu->f.z = nv == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v >> 7;
    gb_clock_advance(gb, 12);
}
static void rr_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 c = gb->cpu->f.c;
    u8 nv = (v >> 1) | (c << 7);
    mbus_write(gb, gb->cpu->hl, nv);
    gb->cpu->f.z = nv == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 12);
}
static void sla_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 nv = v << 1;
    mbus_write(gb, gb->cpu->hl, nv);
    gb->cpu->f.z = nv == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v >> 7;
    gb_clock_advance(gb, 12);
}
static void sra_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 nv = (v >> 1) | (v & 0x80);
    mbus_write(gb, gb->cpu->hl, nv);
    gb->cpu->f.z = nv == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 12);
}
static void swap_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 nv = (v << 4) | (v >> 4);
    mbus_write(gb, gb->cpu->hl, nv);
    gb->cpu->f.z = nv == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = 0;
    gb_clock_advance(gb, 12);
}
static void srl_hl(gb_t *gb) {
    u8 v = mbus_read(gb, gb->cpu->hl);
    u8 nv = v >> 1;
    mbus_write(gb, gb->cpu->hl, nv);
    gb->cpu->f.z = nv == 0;
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 0;
    gb->cpu->f.c = v & 1;
    gb_clock_advance(gb, 12);
}

// CB bit operations ----------------------------------------------------------
static void cb_bit(gb_t *gb, u8 *r, u8 bit) {
    gb->cpu->f.z = !((*r >> bit) & 1);
    gb->cpu->f.n = 0;
    gb->cpu->f.h = 1;
    gb_clock_advance(gb, 4);
}
static void cb_res(gb_t *gb, u8 *r, u8 bit) {
    *r &= ~(1u << bit);
    gb_clock_advance(gb, 4);
}
static void cb_set(gb_t *gb, u8 *r, u8 bit) {
    *r |= (1u << bit);
    gb_clock_advance(gb, 4);
}

#define DEF_CB_BIT(bit, reg) static void bit_##bit##_##reg(gb_t *gb) {         \
        cb_bit(gb, &gb->cpu->reg, bit);                                        \
    }
#define DEF_CB_BIT_HL(bit) static void bit_##bit##_hl(gb_t *gb) {              \
        u8 v = mbus_read(gb, gb->cpu->hl);                                     \
        gb->cpu->f.z = !((v >> bit) & 1);                                      \
        gb->cpu->f.n = 0;                                                      \
        gb->cpu->f.h = 1;                                                      \
        gb_clock_advance(gb, 8);                                               \
    }
#define DEF_CB_RES(bit, reg) static void res_##bit##_##reg(gb_t *gb) {         \
        cb_res(gb, &gb->cpu->reg, bit);                                        \
    }
#define DEF_CB_RES_HL(bit) static void res_##bit##_hl(gb_t *gb) {              \
        u8 v = mbus_read(gb, gb->cpu->hl);                                     \
        v &= ~(1u << bit);                                                     \
        mbus_write(gb, gb->cpu->hl, v);                                        \
        gb_clock_advance(gb, 12);                                              \
    }
#define DEF_CB_SET(bit, reg) static void set_##bit##_##reg(gb_t *gb) {         \
        cb_set(gb, &gb->cpu->reg, bit);                                        \
    }
#define DEF_CB_SET_HL(bit) static void set_##bit##_hl(gb_t *gb) {              \
        u8 v = mbus_read(gb, gb->cpu->hl);                                     \
        v |= (1u << bit);                                                      \
        mbus_write(gb, gb->cpu->hl, v);                                        \
        gb_clock_advance(gb, 12);                                              \
    }

#define ALL_CB_BIT(bit)                                                        \
    DEF_CB_BIT(bit, b) DEF_CB_BIT(bit, c) DEF_CB_BIT(bit, d) DEF_CB_BIT(bit, e) \
        DEF_CB_BIT(bit, h) DEF_CB_BIT(bit, l) DEF_CB_BIT_HL(bit)                \
            DEF_CB_BIT(bit, a)
#define ALL_CB_RES(bit)                                                        \
    DEF_CB_RES(bit, b) DEF_CB_RES(bit, c) DEF_CB_RES(bit, d) DEF_CB_RES(bit, e) \
        DEF_CB_RES(bit, h) DEF_CB_RES(bit, l) DEF_CB_RES_HL(bit)                \
            DEF_CB_RES(bit, a)
#define ALL_CB_SET(bit)                                                        \
    DEF_CB_SET(bit, b) DEF_CB_SET(bit, c) DEF_CB_SET(bit, d) DEF_CB_SET(bit, e) \
        DEF_CB_SET(bit, h) DEF_CB_SET(bit, l) DEF_CB_SET_HL(bit)                \
            DEF_CB_SET(bit, a)

ALL_CB_BIT(0) ALL_CB_BIT(1) ALL_CB_BIT(2) ALL_CB_BIT(3) ALL_CB_BIT(4)
    ALL_CB_BIT(5) ALL_CB_BIT(6) ALL_CB_BIT(7)
ALL_CB_RES(0) ALL_CB_RES(1) ALL_CB_RES(2) ALL_CB_RES(3) ALL_CB_RES(4)
    ALL_CB_RES(5) ALL_CB_RES(6) ALL_CB_RES(7)
ALL_CB_SET(0) ALL_CB_SET(1) ALL_CB_SET(2) ALL_CB_SET(3) ALL_CB_SET(4)
    ALL_CB_SET(5) ALL_CB_SET(6) ALL_CB_SET(7)

// n8 little endian 8 bit value
// n16 little endian 16 bit value
// a8 little endian 8 bit address
// a16 little endian 16 bit address
// e8 8bit signed value
// [REG] - the value in the register REG
static const opcode_t opcodes[0x100] = {
    [0x00] = {"nop", nop},
    [0x01] = {"ld bc, n16", ld_bc_n16},
    [0x02] = {"ld [bc], a", ld_abc_a},
    [0x03] = {"inc bc", inc_bc},
    [0x04] = {"inc b", inc_b},
    [0x05] = {"dec b", dec_b},
    [0x06] = {"ld b, n8", ld_b_n8},
    [0x07] = {"rlca", rlca},
    [0x08] = {"ld [a16], sp", ld_addra16_sp},
    [0x09] = {"add hl, bc", add_hl_bc},
    [0x0a] = {"ld a, [bc]", ld_a_abc},
    [0x0b] = {"dec bc", dec_bc},
    [0x0c] = {"inc c", inc_c},
    [0x0d] = {"dec c", dec_c},
    [0x0e] = {"ld c, n8", ld_c_n8},
    [0x0f] = {"rrca", rrca},
    [0x10] = {"stop n8", stop_n8},
    [0x11] = {"ld de, n16", ld_de_n16},
    [0x12] = {"ld [de], a", ld_ade_a},
    [0x13] = {"inc de", inc_de},
    [0x14] = {"inc d", inc_d},
    [0x15] = {"dec d", dec_d},
    [0x16] = {"ld d, n8", ld_d_n8},
    [0x17] = {"rla", rla},
    [0x18] = {"jr e8", jr},
    [0x19] = {"add hl, de", add_hl_de},
    [0x1a] = {"ld a, [de]", ld_a_ade},
    [0x1b] = {"dec de", dec_de},
    [0x1c] = {"inc e", inc_e},
    [0x1d] = {"dec e", dec_e},
    [0x1e] = {"ld e n8", ld_e_n8},
    [0x1f] = {"rra", rra},
    [0x20] = {"jr nz, e8", jr_nz},
    [0x21] = {"ld hl, n16", ld_hl_n16},
    [0x22] = {"ld [hli], a", ld_ahli_a},
    [0x23] = {"inc hl", inc_hl},
    [0x24] = {"inc h", inc_h},
    [0x25] = {"dec h", dec_h},
    [0x26] = {"ld h, n8", ld_h_n8},
    [0x27] = {"daa", daa},
    [0x28] = {"jr z, e8", jr_z},
    [0x29] = {"add hl, hl", add_hl_hl},
    [0x2a] = {"ld a, [hli]", ld_a_ahli},
    [0x2b] = {"dec hl", dec_hl},
    [0x2c] = {"inc l", inc_l},
    [0x2d] = {"dec l", dec_l},
    [0x2e] = {"ld l, n8", ld_l_n8},
    [0x2f] = {"cpl", cpl},
    [0x30] = {"jr nc, e8", jr_nc},
    [0x31] = {"ld sp, n16", ld_sp_n16},
    [0x32] = {"ld [hld], a", ld_ahld_a},
    [0x33] = {"inc sp", inc_sp},
    [0x34] = {"inc [hl]", inc_ahl},
    [0x35] = {"dec [hl]", dec_ahl},
    [0x36] = {"ld [hl], n8", ld_ahl_n8},
    [0x37] = {"scf", scf},
    [0x38] = {"jr c, e8", jr_c},
    [0x39] = {"add hl, sp", add_hl_sp},
    [0x3a] = {"ld a, [hld]", ld_a_ahld},
    [0x3b] = {"dec sp", dec_sp},
    [0x3c] = {"inc a", inc_a},
    [0x3d] = {"dec a", dec_a},
    [0x3e] = {"ld a, n8", ld_a_n8},
    [0x3f] = {"ccf", ccf},
    [0x40] = {"ld b, b", ld_b_b},
    [0x41] = {"ld b, c", ld_b_c},
    [0x42] = {"ld b, d", ld_b_d},
    [0x43] = {"ld b, e", ld_b_e},
    [0x44] = {"ld b, h", ld_b_h},
    [0x45] = {"ld b, l", ld_b_l},
    [0x46] = {"ld b, [hl]", ld_b_hl},
    [0x47] = {"ld b, a", ld_b_a},
    [0x48] = {"ld c, b", ld_c_b},
    [0x49] = {"ld c, c", ld_c_c},
    [0x4a] = {"ld c, d", ld_c_d},
    [0x4b] = {"ld c, e", ld_c_e},
    [0x4c] = {"ld c, h", ld_c_h},
    [0x4d] = {"ld c, l", ld_c_l},
    [0x4e] = {"ld c, [hl]", ld_c_hl},
    [0x4f] = {"ld c, a", ld_c_a},
    [0x50] = {"ld d, b", ld_d_b},
    [0x51] = {"ld d, c", ld_d_c},
    [0x52] = {"ld d, d", ld_d_d},
    [0x53] = {"ld d, e", ld_d_e},
    [0x54] = {"ld d, h", ld_d_h},
    [0x55] = {"ld d, l", ld_d_l},
    [0x56] = {"ld d, [hl]", ld_d_hl},
    [0x57] = {"ld d, a", ld_d_a},
    [0x58] = {"ld e, b", ld_e_b},
    [0x59] = {"ld e, c", ld_e_c},
    [0x5a] = {"ld e, d", ld_e_d},
    [0x5b] = {"ld e, e", ld_e_e},
    [0x5c] = {"ld e, h", ld_e_h},
    [0x5d] = {"ld e, l", ld_e_l},
    [0x5e] = {"ld e, [hl]", ld_e_hl},
    [0x5f] = {"ld e, a", ld_e_a},
    [0x60] = {"ld h, b", ld_h_b},
    [0x61] = {"ld h, c", ld_h_c},
    [0x62] = {"ld h, d", ld_h_d},
    [0x63] = {"ld h, e", ld_h_e},
    [0x64] = {"ld h, h", ld_h_h},
    [0x65] = {"ld h, l", ld_h_l},
    [0x66] = {"ld h, [hl]", ld_h_hl},
    [0x67] = {"ld h, a", ld_h_a},
    [0x68] = {"ld l, b", ld_l_b},
    [0x69] = {"ld l, c", ld_l_c},
    [0x6a] = {"ld l, d", ld_l_d},
    [0x6b] = {"ld l, e", ld_l_e},
    [0x6c] = {"ld l, h", ld_l_h},
    [0x6d] = {"ld l, l", ld_l_l},
    [0x6e] = {"ld l, [hl]", ld_l_hl},
    [0x6f] = {"ld l, a", ld_l_a},
    [0x70] = {"ld [hl], b", ld_hl_b},
    [0x71] = {"ld [hl], c", ld_hl_c},
    [0x72] = {"ld [hl], d", ld_hl_d},
    [0x73] = {"ld [hl], e", ld_hl_e},
    [0x74] = {"ld [hl], h", ld_hl_h},
    [0x75] = {"ld [hl], l", ld_hl_l},
    [0x76] = {"halt", halt},
    [0x77] = {"ld [hl], a", ld_hl_a},
    [0x78] = {"ld a, b", ld_a_b},
    [0x79] = {"ld a, c", ld_a_c},
    [0x7a] = {"ld a, d", ld_a_d},
    [0x7b] = {"ld a, e", ld_a_e},
    [0x7c] = {"ld a, h", ld_a_h},
    [0x7d] = {"ld a, l", ld_a_l},
    [0x7e] = {"ld a, [hl]", ld_a_hl},
    [0x7f] = {"ld a, a", ld_a_a},
    [0x80] = {"add a, b", add_b},
    [0x81] = {"add a, c", add_c},
    [0x82] = {"add a, d", add_d},
    [0x83] = {"add a, e", add_e},
    [0x84] = {"add a, h", add_h},
    [0x85] = {"add a, l", add_l},
    [0x86] = {"add a, [hl]", add_hl},
    [0x87] = {"add a, a", add_a},
    [0x88] = {"adc a, b", adc_b},
    [0x89] = {"adc a, c", adc_c},
    [0x8a] = {"adc a, d", adc_d},
    [0x8b] = {"adc a, e", adc_e},
    [0x8c] = {"adc a, h", adc_h},
    [0x8d] = {"adc a, l", adc_l},
    [0x8e] = {"adc a, [hl]", adc_hl},
    [0x8f] = {"adc a, a", adc_a},
    [0x90] = {"sub b", sub_b},
    [0x91] = {"sub c", sub_c},
    [0x92] = {"sub d", sub_d},
    [0x93] = {"sub e", sub_e},
    [0x94] = {"sub h", sub_h},
    [0x95] = {"sub l", sub_l},
    [0x96] = {"sub [hl]", sub_hl},
    [0x97] = {"sub a", sub_a},
    [0x98] = {"sbc a, b", sbc_b},
    [0x99] = {"sbc a, c", sbc_c},
    [0x9a] = {"sbc a, d", sbc_d},
    [0x9b] = {"sbc a, e", sbc_e},
    [0x9c] = {"sbc a, h", sbc_h},
    [0x9d] = {"sbc a, l", sbc_l},
    [0x9e] = {"sbc a, [hl]", sbc_hl},
    [0x9f] = {"sbc a, a", sbc_a},
    [0xa0] = {"and b", and_b},
    [0xa1] = {"and c", and_c},
    [0xa2] = {"and d", and_d},
    [0xa3] = {"and e", and_e},
    [0xa4] = {"and h", and_h},
    [0xa5] = {"and l", and_l},
    [0xa6] = {"and [hl]", and_hl},
    [0xa7] = {"and a", and_a},
    [0xa8] = {"xor b", xor_b},
    [0xa9] = {"xor c", xor_c},
    [0xaa] = {"xor d", xor_d},
    [0xab] = {"xor e", xor_e},
    [0xac] = {"xor h", xor_h},
    [0xad] = {"xor l", xor_l},
    [0xae] = {"xor [hl]", xor_hl},
    [0xaf] = {"xor a", xor_a},
    [0xb0] = {"or b", or_b},
    [0xb1] = {"or c", or_c},
    [0xb2] = {"or d", or_d},
    [0xb3] = {"or e", or_e},
    [0xb4] = {"or h", or_h},
    [0xb5] = {"or l", or_l},
    [0xb6] = {"or [hl]", or_hl},
    [0xb7] = {"or a", or_a},
    [0xb8] = {"cp b", cp_b},
    [0xb9] = {"cp c", cp_c},
    [0xba] = {"cp d", cp_d},
    [0xbb] = {"cp e", cp_e},
    [0xbc] = {"cp h", cp_h},
    [0xbd] = {"cp l", cp_l},
    [0xbe] = {"cp [hl]", cp_hl},
    [0xbf] = {"cp a", cp_a},
    [0xc0] = {"ret nz", ret_nz},
    [0xc1] = {"pop bc", pop_bc},
    [0xc2] = {"jp nz, a16", jp_nz},
    [0xc3] = {"jp a16", jp_a16},
    [0xc4] = {"call nz, a16", call_nz},
    [0xc5] = {"push bc", push_bc},
    [0xc6] = {"add a, n8", add_n8},
    [0xc7] = {"rst 00h", rst_00},
    [0xc8] = {"ret z", ret_z},
    [0xc9] = {"ret", ret},
    [0xca] = {"jp z, a16", jp_z},
    [0xcb] = {"cb prefix", cb_prefix},
    [0xcc] = {"call z, a16", call_z},
    [0xcd] = {"call a16", call_a16},
    [0xce] = {"adc a, n8", adc_n8},
    [0xcf] = {"rst 08h", rst_08},
    [0xd0] = {"ret nc", ret_nc},
    [0xd1] = {"pop de", pop_de},
    [0xd2] = {"jp nc, a16", jp_nc},
    [0xd3] = {"illegal", NULL},
    [0xd4] = {"call nc, a16", call_nc},
    [0xd5] = {"push de", push_de},
    [0xd6] = {"sub n8", sub_n8},
    [0xd7] = {"rst 10h", rst_10},
    [0xd8] = {"ret c", ret_c},
    [0xd9] = {"reti", reti},
    [0xda] = {"jp c, a16", jp_c},
    [0xdb] = {"illegal", NULL},
    [0xdc] = {"call c, a16", call_c},
    [0xdd] = {"illegal", NULL},
    [0xde] = {"sbc a, n8", sbc_n8},
    [0xdf] = {"rst 18h", rst_18},
    [0xe0] = {"ldh [n8], a", ldh_n8_a},
    [0xe1] = {"pop hl", pop_hl},
    [0xe2] = {"ldh [c], a", ldh_c_a},
    [0xe3] = {"illegal", NULL},
    [0xe4] = {"illegal", NULL},
    [0xe5] = {"push hl", push_hl},
    [0xe6] = {"and n8", and_n8},
    [0xe7] = {"rst 20h", rst_20},
    [0xe8] = {"add sp, e8", add_sp_e8},
    [0xe9] = {"jp [hl]", jp_hl},
    [0xea] = {"ld [a16], a", ld_aa16_a},
    [0xeb] = {"illegal", NULL},
    [0xec] = {"illegal", NULL},
    [0xed] = {"illegal", NULL},
    [0xee] = {"xor n8", xor_n8},
    [0xef] = {"rst 28h", rst_28},
    [0xf0] = {"ldh a, [n8]", ldh_a_n8},
    [0xf1] = {"pop af", pop_af},
    [0xf2] = {"ldh a, [c]", ldh_a_c},
    [0xf3] = {"di", di},
    [0xf4] = {"illegal", NULL},
    [0xf5] = {"push af", push_af},
    [0xf6] = {"or n8", or_n8},
    [0xf7] = {"rst 30h", rst_30},
    [0xf8] = {"ld hl, sp+e8", ld_hl_spplus8},
    [0xf9] = {"ld sp, hl", ld_sp_hl},
    [0xfa] = {"ld a, [a16]", ld_a_aa16},
    [0xfb] = {"ei", ei},
    [0xfc] = {"illegal", NULL},
    [0xfd] = {"illegal", NULL},
    [0xfe] = {"cp n8", cp_n8},
    [0xff] = {"rst 38h", rst_38},
};

static const opcode_t cb_opcodes[0x100] = {
    [0x00] = {"rlc b", rlc_b},
    [0x01] = {"rlc c", rlc_c},
    [0x02] = {"rlc d", rlc_d},
    [0x03] = {"rlc e", rlc_e},
    [0x04] = {"rlc h", rlc_h},
    [0x05] = {"rlc l", rlc_l},
    [0x06] = {"rlc [hl]", rlc_hl},
    [0x07] = {"rlc a", rlc_a},
    [0x08] = {"rrc b", rrc_b},
    [0x09] = {"rrc c", rrc_c},
    [0x0a] = {"rrc d", rrc_d},
    [0x0b] = {"rrc e", rrc_e},
    [0x0c] = {"rrc h", rrc_h},
    [0x0d] = {"rrc l", rrc_l},
    [0x0e] = {"rrc [hl]", rrc_hl},
    [0x0f] = {"rrc a", rrc_a},
    [0x10] = {"rl b", rl_b},
    [0x11] = {"rl c", rl_c},
    [0x12] = {"rl d", rl_d},
    [0x13] = {"rl e", rl_e},
    [0x14] = {"rl h", rl_h},
    [0x15] = {"rl l", rl_l},
    [0x16] = {"rl [hl]", rl_hl},
    [0x17] = {"rl a", rl_a},
    [0x18] = {"rr b", rr_b},
    [0x19] = {"rr c", rr_c},
    [0x1a] = {"rr d", rr_d},
    [0x1b] = {"rr e", rr_e},
    [0x1c] = {"rr h", rr_h},
    [0x1d] = {"rr l", rr_l},
    [0x1e] = {"rr [hl]", rr_hl},
    [0x1f] = {"rr a", rr_a},
    [0x20] = {"sla b", sla_b},
    [0x21] = {"sla c", sla_c},
    [0x22] = {"sla d", sla_d},
    [0x23] = {"sla e", sla_e},
    [0x24] = {"sla h", sla_h},
    [0x25] = {"sla l", sla_l},
    [0x26] = {"sla [hl]", sla_hl},
    [0x27] = {"sla a", sla_a},
    [0x28] = {"sra b", sra_b},
    [0x29] = {"sra c", sra_c},
    [0x2a] = {"sra d", sra_d},
    [0x2b] = {"sra e", sra_e},
    [0x2c] = {"sra h", sra_h},
    [0x2d] = {"sra l", sra_l},
    [0x2e] = {"sra [hl]", sra_hl},
    [0x2f] = {"sra a", sra_a},
    [0x30] = {"swap b", swap_b},
    [0x31] = {"swap c", swap_c},
    [0x32] = {"swap d", swap_d},
    [0x33] = {"swap e", swap_e},
    [0x34] = {"swap h", swap_h},
    [0x35] = {"swap l", swap_l},
    [0x36] = {"swap [hl]", swap_hl},
    [0x37] = {"swap a", swap_a},
    [0x38] = {"srl b", srl_b},
    [0x39] = {"srl c", srl_c},
    [0x3a] = {"srl d", srl_d},
    [0x3b] = {"srl e", srl_e},
    [0x3c] = {"srl h", srl_h},
    [0x3d] = {"srl l", srl_l},
    [0x3e] = {"srl [hl]", srl_hl},
    [0x3f] = {"srl a", srl_a},
    [0x40] = {"bit 0, b", bit_0_b},
    [0x41] = {"bit 0, c", bit_0_c},
    [0x42] = {"bit 0, d", bit_0_d},
    [0x43] = {"bit 0, e", bit_0_e},
    [0x44] = {"bit 0, h", bit_0_h},
    [0x45] = {"bit 0, l", bit_0_l},
    [0x46] = {"bit 0, [hl]", bit_0_hl},
    [0x47] = {"bit 0, a", bit_0_a},
    [0x48] = {"bit 1, b", bit_1_b},
    [0x49] = {"bit 1, c", bit_1_c},
    [0x4a] = {"bit 1, d", bit_1_d},
    [0x4b] = {"bit 1, e", bit_1_e},
    [0x4c] = {"bit 1, h", bit_1_h},
    [0x4d] = {"bit 1, l", bit_1_l},
    [0x4e] = {"bit 1, [hl]", bit_1_hl},
    [0x4f] = {"bit 1, a", bit_1_a},
    [0x50] = {"bit 2, b", bit_2_b},
    [0x51] = {"bit 2, c", bit_2_c},
    [0x52] = {"bit 2, d", bit_2_d},
    [0x53] = {"bit 2, e", bit_2_e},
    [0x54] = {"bit 2, h", bit_2_h},
    [0x55] = {"bit 2, l", bit_2_l},
    [0x56] = {"bit 2, [hl]", bit_2_hl},
    [0x57] = {"bit 2, a", bit_2_a},
    [0x58] = {"bit 3, b", bit_3_b},
    [0x59] = {"bit 3, c", bit_3_c},
    [0x5a] = {"bit 3, d", bit_3_d},
    [0x5b] = {"bit 3, e", bit_3_e},
    [0x5c] = {"bit 3, h", bit_3_h},
    [0x5d] = {"bit 3, l", bit_3_l},
    [0x5e] = {"bit 3, [hl]", bit_3_hl},
    [0x5f] = {"bit 3, a", bit_3_a},
    [0x60] = {"bit 4, b", bit_4_b},
    [0x61] = {"bit 4, c", bit_4_c},
    [0x62] = {"bit 4, d", bit_4_d},
    [0x63] = {"bit 4, e", bit_4_e},
    [0x64] = {"bit 4, h", bit_4_h},
    [0x65] = {"bit 4, l", bit_4_l},
    [0x66] = {"bit 4, [hl]", bit_4_hl},
    [0x67] = {"bit 4, a", bit_4_a},
    [0x68] = {"bit 5, b", bit_5_b},
    [0x69] = {"bit 5, c", bit_5_c},
    [0x6a] = {"bit 5, d", bit_5_d},
    [0x6b] = {"bit 5, e", bit_5_e},
    [0x6c] = {"bit 5, h", bit_5_h},
    [0x6d] = {"bit 5, l", bit_5_l},
    [0x6e] = {"bit 5, [hl]", bit_5_hl},
    [0x6f] = {"bit 5, a", bit_5_a},
    [0x70] = {"bit 6, b", bit_6_b},
    [0x71] = {"bit 6, c", bit_6_c},
    [0x72] = {"bit 6, d", bit_6_d},
    [0x73] = {"bit 6, e", bit_6_e},
    [0x74] = {"bit 6, h", bit_6_h},
    [0x75] = {"bit 6, l", bit_6_l},
    [0x76] = {"bit 6, [hl]", bit_6_hl},
    [0x77] = {"bit 6, a", bit_6_a},
    [0x78] = {"bit 7, b", bit_7_b},
    [0x79] = {"bit 7, c", bit_7_c},
    [0x7a] = {"bit 7, d", bit_7_d},
    [0x7b] = {"bit 7, e", bit_7_e},
    [0x7c] = {"bit 7, h", bit_7_h},
    [0x7d] = {"bit 7, l", bit_7_l},
    [0x7e] = {"bit 7, [hl]", bit_7_hl},
    [0x7f] = {"bit 7, a", bit_7_a},
    [0x80] = {"res 0, b", res_0_b},
    [0x81] = {"res 0, c", res_0_c},
    [0x82] = {"res 0, d", res_0_d},
    [0x83] = {"res 0, e", res_0_e},
    [0x84] = {"res 0, h", res_0_h},
    [0x85] = {"res 0, l", res_0_l},
    [0x86] = {"res 0, [hl]", res_0_hl},
    [0x87] = {"res 0, a", res_0_a},
    [0x88] = {"res 1, b", res_1_b},
    [0x89] = {"res 1, c", res_1_c},
    [0x8a] = {"res 1, d", res_1_d},
    [0x8b] = {"res 1, e", res_1_e},
    [0x8c] = {"res 1, h", res_1_h},
    [0x8d] = {"res 1, l", res_1_l},
    [0x8e] = {"res 1, [hl]", res_1_hl},
    [0x8f] = {"res 1, a", res_1_a},
    [0x90] = {"res 2, b", res_2_b},
    [0x91] = {"res 2, c", res_2_c},
    [0x92] = {"res 2, d", res_2_d},
    [0x93] = {"res 2, e", res_2_e},
    [0x94] = {"res 2, h", res_2_h},
    [0x95] = {"res 2, l", res_2_l},
    [0x96] = {"res 2, [hl]", res_2_hl},
    [0x97] = {"res 2, a", res_2_a},
    [0x98] = {"res 3, b", res_3_b},
    [0x99] = {"res 3, c", res_3_c},
    [0x9a] = {"res 3, d", res_3_d},
    [0x9b] = {"res 3, e", res_3_e},
    [0x9c] = {"res 3, h", res_3_h},
    [0x9d] = {"res 3, l", res_3_l},
    [0x9e] = {"res 3, [hl]", res_3_hl},
    [0x9f] = {"res 3, a", res_3_a},
    [0xa0] = {"res 4, b", res_4_b},
    [0xa1] = {"res 4, c", res_4_c},
    [0xa2] = {"res 4, d", res_4_d},
    [0xa3] = {"res 4, e", res_4_e},
    [0xa4] = {"res 4, h", res_4_h},
    [0xa5] = {"res 4, l", res_4_l},
    [0xa6] = {"res 4, [hl]", res_4_hl},
    [0xa7] = {"res 4, a", res_4_a},
    [0xa8] = {"res 5, b", res_5_b},
    [0xa9] = {"res 5, c", res_5_c},
    [0xaa] = {"res 5, d", res_5_d},
    [0xab] = {"res 5, e", res_5_e},
    [0xac] = {"res 5, h", res_5_h},
    [0xad] = {"res 5, l", res_5_l},
    [0xae] = {"res 5, [hl]", res_5_hl},
    [0xaf] = {"res 5, a", res_5_a},
    [0xb0] = {"res 6, b", res_6_b},
    [0xb1] = {"res 6, c", res_6_c},
    [0xb2] = {"res 6, d", res_6_d},
    [0xb3] = {"res 6, e", res_6_e},
    [0xb4] = {"res 6, h", res_6_h},
    [0xb5] = {"res 6, l", res_6_l},
    [0xb6] = {"res 6, [hl]", res_6_hl},
    [0xb7] = {"res 6, a", res_6_a},
    [0xb8] = {"res 7, b", res_7_b},
    [0xb9] = {"res 7, c", res_7_c},
    [0xba] = {"res 7, d", res_7_d},
    [0xbb] = {"res 7, e", res_7_e},
    [0xbc] = {"res 7, h", res_7_h},
    [0xbd] = {"res 7, l", res_7_l},
    [0xbe] = {"res 7, [hl]", res_7_hl},
    [0xbf] = {"res 7, a", res_7_a},
    [0xc0] = {"set 0, b", set_0_b},
    [0xc1] = {"set 0, c", set_0_c},
    [0xc2] = {"set 0, d", set_0_d},
    [0xc3] = {"set 0, e", set_0_e},
    [0xc4] = {"set 0, h", set_0_h},
    [0xc5] = {"set 0, l", set_0_l},
    [0xc6] = {"set 0, [hl]", set_0_hl},
    [0xc7] = {"set 0, a", set_0_a},
    [0xc8] = {"set 1, b", set_1_b},
    [0xc9] = {"set 1, c", set_1_c},
    [0xca] = {"set 1, d", set_1_d},
    [0xcb] = {"set 1, e", set_1_e},
    [0xcc] = {"set 1, h", set_1_h},
    [0xcd] = {"set 1, l", set_1_l},
    [0xce] = {"set 1, [hl]", set_1_hl},
    [0xcf] = {"set 1, a", set_1_a},
    [0xd0] = {"set 2, b", set_2_b},
    [0xd1] = {"set 2, c", set_2_c},
    [0xd2] = {"set 2, d", set_2_d},
    [0xd3] = {"set 2, e", set_2_e},
    [0xd4] = {"set 2, h", set_2_h},
    [0xd5] = {"set 2, l", set_2_l},
    [0xd6] = {"set 2, [hl]", set_2_hl},
    [0xd7] = {"set 2, a", set_2_a},
    [0xd8] = {"set 3, b", set_3_b},
    [0xd9] = {"set 3, c", set_3_c},
    [0xda] = {"set 3, d", set_3_d},
    [0xdb] = {"set 3, e", set_3_e},
    [0xdc] = {"set 3, h", set_3_h},
    [0xdd] = {"set 3, l", set_3_l},
    [0xde] = {"set 3, [hl]", set_3_hl},
    [0xdf] = {"set 3, a", set_3_a},
    [0xe0] = {"set 4, b", set_4_b},
    [0xe1] = {"set 4, c", set_4_c},
    [0xe2] = {"set 4, d", set_4_d},
    [0xe3] = {"set 4, e", set_4_e},
    [0xe4] = {"set 4, h", set_4_h},
    [0xe5] = {"set 4, l", set_4_l},
    [0xe6] = {"set 4, [hl]", set_4_hl},
    [0xe7] = {"set 4, a", set_4_a},
    [0xe8] = {"set 5, b", set_5_b},
    [0xe9] = {"set 5, c", set_5_c},
    [0xea] = {"set 5, d", set_5_d},
    [0xeb] = {"set 5, e", set_5_e},
    [0xec] = {"set 5, h", set_5_h},
    [0xed] = {"set 5, l", set_5_l},
    [0xee] = {"set 5, [hl]", set_5_hl},
    [0xef] = {"set 5, a", set_5_a},
    [0xf0] = {"set 6, b", set_6_b},
    [0xf1] = {"set 6, c", set_6_c},
    [0xf2] = {"set 6, d", set_6_d},
    [0xf3] = {"set 6, e", set_6_e},
    [0xf4] = {"set 6, h", set_6_h},
    [0xf5] = {"set 6, l", set_6_l},
    [0xf6] = {"set 6, [hl]", set_6_hl},
    [0xf7] = {"set 6, a", set_6_a},
    [0xf8] = {"set 7, b", set_7_b},
    [0xf9] = {"set 7, c", set_7_c},
    [0xfa] = {"set 7, d", set_7_d},
    [0xfb] = {"set 7, e", set_7_e},
    [0xfc] = {"set 7, h", set_7_h},
    [0xfd] = {"set 7, l", set_7_l},
    [0xfe] = {"set 7, [hl]", set_7_hl},
    [0xff] = {"set 7, a", set_7_a},
};

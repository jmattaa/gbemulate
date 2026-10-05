package gb

cpu_regs :: struct {
	using _: struct #raw_union {
		using _: struct {
			f: bit_field (u8) {
				_unused: u8 | 4, // 4 bits (bits 0-3)
				c:       u8 | 1, // 1 bit  (bit 4)
				h:       u8 | 1, // 1 bit  (bit 5)
				n:       u8 | 1, // 1 bit  (bit 6)
				z:       u8 | 1, // 1 bit  (bit 7)
			},
			a: u8,
		},
		af:      u16,
	},
	using _: struct #raw_union {
		using _: struct {
			c: u8,
			b: u8,
		},
		bc:      u16,
	},
	using _: struct #raw_union {
		using _: struct {
			e: u8,
			d: u8,
		},
		de:      u16,
	},
	using _: struct #raw_union {
		using _: struct {
			l: u8,
			h: u8,
		},
		hl:      u16,
	},
	sp:      u16,
	pc:      u16,
}

dbg_setup_regs :: proc() -> cpu_regs {
	return cpu_regs{af = 0xaea0, bc = 0x0013, de = 0x00d8, hl = 0x014d, sp = 0xfffe, pc = 0x100}
}

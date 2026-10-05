package gb

chdr_t :: struct #packed {
	nop:          [4]u8, // 0x100 - 0x103
	logo:         [48]u8, // 0x104 - 0x133
	title:        [16]u8, // 0x134 - 0x143
	new_licensee: [2]u8, // 0x144 - 0x145
	sgb_flag:     u8, // 0x146
	cart_type:    u8, // 0x147
	rom_size:     u8, // 0x148
	ram_size:     u8, // 0x149
	dest_code:    u8, // 0x14A
	old_licensee: u8, // 0x14B
	rom_version:  u8, // 0x14C
	hdr_checksum: u8, // 0x14D
	gbl_checksum: [2]u8, // 0x14E - 0x14F
}

cmmap_t :: struct #raw_union {
	data:    [0x8000]u8, // 32kb
	using _: struct #packed {
        brom: [0x100]u8, // 0x0000 - 0x00ff
        chdr: chdr_t,
        ehdr: [0x100]u8, // 0x0150 - 0x7fff
	},
}

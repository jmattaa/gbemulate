package main

import "core:fmt"
import "gb"
import "utils"
import rl "vendor:raylib"

main :: proc() {
	rom_size: i64
	rom := utils.fread("roms/tetris.gb", &rom_size)
	if rom == nil {
		fmt.println("Failed to read ROM")
		return
	}
	defer delete(rom)

	fmt.println("ROM size:", rom_size)
	chdr := cast(^gb.chdr)(raw_data(rom[0x100:]))
	fmt.println("ROM title:", string(chdr.title[:]))
    fmt.println("ROM type:", chdr.cart_type)
    fmt.println("ROM size:", chdr.rom_size)
    fmt.println("RAM size:", chdr.ram_size)
    fmt.println("ROM version:", chdr.rom_version)

    cpu_regs := gb.dbg_setup_regs()

    fmt.println("f: ", cpu_regs.f)
    fmt.println("a: ", cpu_regs.a)
    fmt.println("af: ", cpu_regs.af)
    fmt.println("c: ", cpu_regs.c)
    fmt.println("b: ", cpu_regs.b)
    fmt.println("bc: ", cpu_regs.bc)
    fmt.println("d: ", cpu_regs.d)
    fmt.println("e: ", cpu_regs.e)
    fmt.println("de: ", cpu_regs.de)
    fmt.println("h: ", cpu_regs.h)
    fmt.println("l: ", cpu_regs.l)
    fmt.println("hl: ", cpu_regs.hl)
    fmt.println("sp: ", cpu_regs.sp)
    fmt.println("pc: ", cpu_regs.pc)

// 	rl.SetConfigFlags({.WINDOW_UNDECORATED})
// 	rl.InitWindow(utils.GB_WIDTH * utils.RES_MULT, utils.GB_HEIGHT * utils.RES_MULT, "gbemulate")
// 	defer rl.CloseWindow()
// 
// 	for !rl.WindowShouldClose() {
// 		rl.BeginDrawing()
// 		rl.ClearBackground({139, 172, 15, 255})
// 		rl.EndDrawing()
// 	}
}

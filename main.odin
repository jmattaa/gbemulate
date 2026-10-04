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

	rl.SetConfigFlags({.WINDOW_UNDECORATED})
	rl.InitWindow(utils.GB_WIDTH * utils.RES_MULT, utils.GB_HEIGHT * utils.RES_MULT, "gbemulate")
	defer rl.CloseWindow()

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		rl.EndDrawing()
	}
}

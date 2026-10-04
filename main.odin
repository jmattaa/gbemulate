package main

import "core:fmt"
import "utils"
import rl "vendor:raylib"

main :: proc() {
	size: i64
	rom := utils.fread("roms/tetris.gb", &size)
	if rom == nil {
		fmt.println("Failed to read ROM")
		return
	}
	defer delete(rom)

	rl.SetConfigFlags({.WINDOW_UNDECORATED})
	rl.InitWindow(utils.GB_WIDTH * utils.RES_MULT, utils.GB_HEIGHT * utils.RES_MULT, "gbemulate")
	defer rl.CloseWindow()

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		rl.EndDrawing()
	}
}

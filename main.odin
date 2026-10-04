package main

import rl "vendor:raylib"

main :: proc() {
    rl.SetConfigFlags({.WINDOW_UNDECORATED})
    rl.InitWindow(800, 600, "gbemulate")
    defer rl.CloseWindow()

    for !rl.WindowShouldClose() {
        rl.BeginDrawing() 
        defer rl.EndDrawing()

        rl.ClearBackground(rl.RAYWHITE)
    }
}

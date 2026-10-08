#include <raylib.h>
#include "constatns.h"
#include "gb.h"
#include "logger.h"
#include "opcode.h"

int main(void) {
    gb_t *gb = gb_init("roms/tests/cpu_instrs.gb");
    if (gb == NULL)
        return 1;

    log_info("ROM SIZE: 0x%x\n", gb->crom_size);
    log_info("ROM title: %s\n", gb->mmap.cart.hdr.title);
    log_info("ROM type: 0x%x\n", gb->mmap.cart.hdr.cart_type);
    log_info("ROM size: 0x%x\n", gb->mmap.cart.hdr.rom_size);
    log_info("RAM size: 0x%x\n", gb->mmap.cart.hdr.ram_size);
    log_info("ROM version: 0x%x\n", gb->mmap.cart.hdr.rom_version);
    log_info("Licensee code: 0x%x\n", gb->mmap.cart.hdr.old_licensee);

    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(GB_WIDTH * GB_PXL_MUL, GB_HEIGHT * GB_PXL_MUL, "gbemulate");

    while (!WindowShouldClose()) {
        if (!cpu_step(gb))
            break;

        BeginDrawing();
        ClearBackground((Color){139, 172, 15, 255});
        EndDrawing();
    }

    CloseWindow();

    gb_free(gb);
    return 0;
}

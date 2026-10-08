#include "constatns.h"
#include "gb.h"
#include "gfx.h"
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

    gfx_ctx_t *gfx_ctx = gfx_init(GB_WIDTH, GB_HEIGHT, "gbemulate");
    if (gfx_ctx == NULL) {
        gb_free(gb);
        log_fatal(1, "Failed to initialize graphics\n");
    }

    u8 running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = 0;
        }

        if (!cpu_step(gb))
            running = 0;
        gfx_clear(gfx_ctx, (SDL_Color){139, 172, 15, 255});
        gfx_update(gfx_ctx);
    }

    gfx_free(gfx_ctx);
    gb_free(gb);
    return 0;
}

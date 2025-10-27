#include "constants.h"
#include "gbc.h"
#include "io.h"
#include "logger.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

#define SDL_Panic(...)                                                         \
    {                                                                          \
        printf("SDL error: %s\n", SDL_GetError());                             \
        __VA_ARGS__;                                                           \
        exit(1);                                                               \
    }

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        log_error("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
        SDL_Panic(SDL_Quit());

    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;

    if (!SDL_CreateWindowAndRenderer("GBEmulate", GB_WIDTH * RES_MULTIPLIER,
                                     GB_HEIGHT * RES_MULTIPLIER,
                                     SDL_WINDOW_BORDERLESS, &win, &ren))
        SDL_Panic(SDL_DestroyWindow(win); SDL_DestroyRenderer(ren); SDL_Quit());

    size_t rom_size = 0;
    char *rom = io_freadb(argv[1], &rom_size);
    if (rom == NULL)
    {
        SDL_DestroyWindow(win);
        SDL_DestroyRenderer(ren);
        SDL_Quit();
        return 1;
    }
    log_info("ROM size: %d bytes\n", rom_size);
    gb_cmmap_t *cmmap = (gb_cmmap_t *)rom;
    log_info("ROM title: %.16s\n", cmmap->chdr.title);
    log_info("ROM size: %d\n", cmmap->chdr.rom_size);
    log_info("RAM size: %d\n", cmmap->chdr.ram_size);
    log_info("Licensee: %s\n", gbc_lic(&cmmap->chdr));

    int running = 1;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                running = 0;
                break;
            }
        }

        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
        SDL_Delay(10);
    }

    free(rom);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

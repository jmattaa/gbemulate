#ifndef GBEMULATE_GFX_H
#define GBEMULATE_GFX_H

#include <SDL3/SDL.h>
#include "constatns.h"
#include "types.h"

const SDL_Color PALETTE[4] = {{0xe0, 0xf0, 0xe7, 0xff},
                              {0x8b, 0xa3, 0x94, 0xff},
                              {0x55, 0x64, 0x5a, 0xff},
                              {0x34, 0x3d, 0x37, 0xff}};

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Texture *tex;

    u8 buffer[GB_WIDTH * GB_HEIGHT * 4];
    u32 offset;
} gfx_ctx_t;

gfx_ctx_t *gfx_init(u8 w, u8 h, const char *title);

void gfx_clear(gfx_ctx_t *ctx, SDL_Color c);
void gfx_write(gfx_ctx_t *ctx, u8 ncol);
void gfx_vblank(gfx_ctx_t *ctx);
void gfx_update(gfx_ctx_t *ctx);

void gfx_free(gfx_ctx_t *ctx);

#endif

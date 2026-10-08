#ifndef GBEMULATE_GFX_H
#define GBEMULATE_GFX_H

#include <SDL3/SDL.h>
#include "types.h"

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Texture *tex;
} gfx_ctx_t;

gfx_ctx_t *gfx_init(u8 w, u8 h, const char *title);

void gfx_clear(gfx_ctx_t *ctx, SDL_Color c);
void gfx_pxl(gfx_ctx_t *ctx, u8 x, u8 y,  SDL_Color c);
void gfx_update(gfx_ctx_t *ctx);

void gfx_free(gfx_ctx_t *ctx);

#endif

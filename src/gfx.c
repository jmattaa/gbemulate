#include "gfx.h"
#include <stdlib.h>
#include "logger.h"

gfx_ctx_t *gfx_init(u8 w, u8 h, const char *title) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error("Failed to initialize SDL\n");
        log_error("%s\n", SDL_GetError());
        return NULL;
    }

    gfx_ctx_t *ctx = malloc(sizeof(gfx_ctx_t));
    if (ctx == NULL) {
        log_error("Failed to allocate gfx_ctx_t (graphics context)\n");
        log_error("%s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    SDL_CreateWindowAndRenderer(title, w, h, SDL_WINDOW_BORDERLESS, &ctx->win,
                                &ctx->ren);
    if (ctx->win == NULL || ctx->ren == NULL) {
        log_error(
            "Failed to create window or renderer (or both 🤷‍♂️)\n");
        log_error("%s\n", SDL_GetError());
        free(ctx);
        SDL_Quit();
        return NULL;
    }

    ctx->tex = SDL_CreateTexture(ctx->ren, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_SetTextureScaleMode(ctx->tex, SDL_SCALEMODE_NEAREST);
    if (ctx->tex == NULL) {
        log_error("Failed to create texture\n");
        log_error("%s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->ren);
        SDL_DestroyWindow(ctx->win);
        free(ctx);
        SDL_Quit();
        return NULL;
    }
    SDL_RaiseWindow(ctx->win);

    return ctx;
}

void gfx_clear(gfx_ctx_t *ctx, SDL_Color c) {
    SDL_SetRenderDrawColor(ctx->ren, c.r, c.g, c.b, c.a);
    SDL_RenderClear(ctx->ren);
}

void gfx_pxl(gfx_ctx_t *ctx, u8 x, u8 y, SDL_Color c) {
    SDL_SetRenderDrawColor(ctx->ren, c.r, c.g, c.b, c.a);
    SDL_RenderPoint(ctx->ren, x, y);
}

void gfx_update(gfx_ctx_t *ctx) { SDL_RenderPresent(ctx->ren); }

void gfx_free(gfx_ctx_t *ctx) {
    SDL_DestroyTexture(ctx->tex);
    SDL_DestroyRenderer(ctx->ren);
    SDL_DestroyWindow(ctx->win);
    SDL_Quit();
    free(ctx);
}

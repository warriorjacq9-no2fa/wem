#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>

int graphics_run(int width, int height, uint32_t* pixelbuf) {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    if(!SDL_CreateWindowAndRenderer(
            "wem",
            width * 4,
            height * 4,
            SDL_WINDOW_RESIZABLE,
            &window,
            &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s",
                SDL_GetError());
        SDL_Quit();
        return -1;
    }

    if(!SDL_SetRenderLogicalPresentation(
            renderer,
            width,
            height,
            SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
        SDL_Log("SDL_SetRenderLogicalPresentation failed: %s",
                SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if(!texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if(!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)) {
        SDL_Log("SDL_SetTextureScaleMode failed: %s", SDL_GetError());
    }

    const SDL_PixelFormatDetails *format =
        SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_ARGB8888);

    if(!format) {
        SDL_Log("SDL_GetPixelFormatDetails failed: %s",
                SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        if(!running) {
            break;
        }

        void *texturePixels = NULL;
        int pitch = 0;

        if(!SDL_LockTexture(
                texture, NULL, &texturePixels, &pitch)) {
            SDL_Log("SDL_LockTexture failed: %s", SDL_GetError());
            break;
        }

        uint8_t *dst = (uint8_t *)texturePixels;

        for (int y = 0; y < height; y++) {
            uint32_t *row = (uint32_t *)(dst + y * pitch);

            for (int x = 0; x < width; x++) {
                uint32_t p = pixelbuf[y * width + x];

                row[x] = SDL_MapRGBA(
                    format, NULL,
                    p & 0xFF,
                    (p >> 8) & 0xFF,
                    (p >> 16) & 0xFF,
                    (p >> 24) & 0xFF
                );
            }
        }

        SDL_UnlockTexture(texture);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderTexture(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
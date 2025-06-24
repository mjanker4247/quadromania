#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>  // for dirname()
#ifdef __APPLE__
#include <mach-o/dyld.h>  // for _NSGetExecutablePath
#endif
#include <unistd.h>  // for readlink

#include "graphics/ttf_font.h"
#include "utils/logger.h"

static TTF_Font *game_font = NULL;
static Uint16 font_height = 0;

bool TTF_Font_Init(void)
{
    // Try loading system fonts in order of preference
    const char *font_names[] = {
        // macOS system fonts
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/STHeiti Light.ttc",
        "/System/Library/Fonts/STHeiti Medium.ttc",
        // Linux system fonts
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/droid/DroidSansFallback.ttf",
        // Windows system fonts (if running under Wine)
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\simhei.ttf",
        NULL
    };

    game_font = NULL;
    for (int i = 0; font_names[i] != NULL; i++) {
        game_font = TTF_OpenFont(font_names[i], 16);  // Adjust size as needed
        if (game_font != NULL) {
            DEBUG_PRINT("Successfully loaded system font: %s", font_names[i]);
            break;
        }
    }

    if (game_font == NULL) {
        // If no system fonts were found, try loading from the data directory as fallback
        char font_path[1024];
        char exec_path[1024];
        char *exec_dir;
        char *source_dir;

        #ifdef __APPLE__
            uint32_t size = sizeof(exec_path);
            if (_NSGetExecutablePath(exec_path, &size) == 0) {
                exec_dir = dirname(exec_path);
                source_dir = dirname(exec_dir);
            } else {
                source_dir = ".";
            }
        #else
            ssize_t count = readlink("/proc/self/exe", exec_path, sizeof(exec_path));
            if (count != -1) {
                exec_dir = dirname(exec_path);
                source_dir = dirname(exec_dir);
            } else {
                source_dir = ".";
            }
        #endif

        snprintf(font_path, sizeof(font_path), "%s/data/fonts/DejaVuSans.ttf", source_dir);
        game_font = TTF_OpenFont(font_path, 16);
        if (game_font == NULL) {
            fprintf(stderr, "Failed to load any font: %s\n", TTF_GetError());
            return (false);
        }
        DEBUG_PRINT("Loaded fallback font from data directory");
    }

    font_height = TTF_FontHeight(game_font);
    DEBUG_PRINT("Font loaded successfully, height: %d", font_height);

    return true;
}

void TTF_Font_CleanUp(void)
{
    if (game_font != NULL) {
        TTF_CloseFont(game_font);
        game_font = NULL;
    }
}

void TTF_Font_DrawText(SDL_Renderer *renderer, Uint16 x, Uint16 y, char *text)
{
    // First draw a semi-transparent dark background
    SDL_Rect bg;
    bg.x = x - 5;  // Add some padding
    bg.y = y - 2;
    bg.w = TTF_Font_GetTextWidth(text) + 10;  // Add padding on both sides
    bg.h = font_height + 4;  // Add padding top and bottom
    
    // Set blend mode for transparency
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    // Draw dark background (R=0, G=0, B=0, A=180)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &bg);
    
    // Reset blend mode and color
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    // Create text surface
    SDL_Surface *text_surface = TTF_RenderText_Solid(game_font, text, (SDL_Color){255, 255, 255, 255});
    if (text_surface == NULL) {
        DEBUG_PRINT("Failed to render text: %s", TTF_GetError());
        return;
    }
    
    // Create texture from surface
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (text_texture == NULL) {
        DEBUG_PRINT("Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(text_surface);
        return;
    }
    
    // Draw the text
    SDL_Rect text_rect = {x, y, text_surface->w, text_surface->h};
    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
    
    // Clean up
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

Uint16 TTF_Font_GetTextWidth(char *text)
{
    int w, h;
    TTF_SizeText(game_font, text, &w, &h);
    return (Uint16)w;
}

Uint16 TTF_Font_GetHeight(void)
{
    return font_height;
} 
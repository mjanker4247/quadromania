/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: fonts.c - Simplified TTF font system implementation
 * last Modified: 2024-12-19
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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

#include "graphics/fonts.h"
#include "graphics/renderer.h"
#include "utils/logger.h"

static TTF_Font *game_font = NULL;
static Uint16 font_height = 0;

bool Font_Init(void)
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
		game_font = TTF_OpenFont(font_names[i], 32);  // Adjust size as needed
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
		game_font = TTF_OpenFont(font_path, 32);
		if (game_font == NULL) {
			fprintf(stderr, "Failed to load any font: %s\n", TTF_GetError());
			return false;
		}
		DEBUG_PRINT("Loaded fallback font from data directory");
	}

	font_height = TTF_FontHeight(game_font);
	DEBUG_PRINT("Font loaded successfully, height: %d", font_height);

	return true;
}

void Font_CleanUp(void)
{
	if (game_font != NULL) {
		TTF_CloseFont(game_font);
		game_font = NULL;
	}
}

/* Unified font drawing function to eliminate code duplication */
static void Font_DrawTextInternal(SDL_Renderer *renderer, Uint16 x, Uint16 y, const char *text, SDL_Color color)
{
	if (!text || !renderer || !game_font) {
		return;
	}
	
	// Draw semi-transparent dark background
	SDL_Rect bg;
	bg.x = x - 5;
	bg.y = y - 2;
	bg.w = Font_GetTextWidth(text) + 10;
	bg.h = font_height + 10;
	
	// Set blend mode for transparency
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
	SDL_RenderFillRect(renderer, &bg);
	
	// Reset blend mode
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	
	// Create text surface with specified color
	SDL_Surface *text_surface = TTF_RenderText_Solid(game_font, text, color);
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

void Font_DrawText(SDL_Renderer *renderer, Uint16 x, Uint16 y, const char *text)
{
	Font_DrawTextInternal(renderer, x, y, text, (SDL_Color){255, 255, 255, 255});
}

void Font_DrawTextWithColor(SDL_Renderer *renderer, Uint16 x, Uint16 y, const char *text, SDL_Color color)
{
	Font_DrawTextInternal(renderer, x, y, text, color);
}

void Font_DrawCenteredText(SDL_Renderer *renderer, Uint16 y, const char *text)
{
	if (!text || !renderer) {
		return;
	}
	
	Uint16 text_width = Font_GetTextWidth(text);
	Uint16 screen_width = Graphics_GetScreenWidth();
	Uint16 x = (screen_width - text_width) / 2;
	
	Font_DrawText(renderer, x, y, text);
}

Uint16 Font_GetTextWidth(const char *text)
{
	if (!text || !game_font) {
		return 0;
	}
	
	int w, h;
	TTF_SizeText(game_font, text, &w, &h);
	return (Uint16)w;
}

Uint16 Font_GetHeight(void)
{
	return font_height;
}

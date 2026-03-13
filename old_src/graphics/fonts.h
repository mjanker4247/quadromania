/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: fonts.h - Simplified TTF font system
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

#ifndef FONTS_H
#define FONTS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Initialize the font system
bool Font_Init(void);

// Clean up the font system
void Font_CleanUp(void);

// Draw text at specified position
void Font_DrawText(SDL_Renderer *renderer, Uint16 x, Uint16 y, const char *text);

// Draw text with custom color
void Font_DrawTextWithColor(SDL_Renderer *renderer, Uint16 x, Uint16 y, const char *text, SDL_Color color);

// Draw centered text at specified Y position
void Font_DrawCenteredText(SDL_Renderer *renderer, Uint16 y, const char *text);

// Get text width in pixels
Uint16 Font_GetTextWidth(const char *text);

// Get font height in pixels
Uint16 Font_GetHeight(void);

#endif /* FONTS_H */

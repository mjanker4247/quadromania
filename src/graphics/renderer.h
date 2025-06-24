/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: graphics.h - header file for the graphics API
 * last Modified: 05.03.2010 : 18:09
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "common/version.h"

	/************
	 * DEFINES  *
	 ************/

	#define NR_OF_TEXTURES 	10
	#define NR_OF_DOTS     	5
	#define GFXPREFIX ""

	/**********
	 * MACROS *
	 **********/
	#define SCREEN_WIDTH 	Graphics_GetScreenWidth()
	#define SCREEN_HEIGHT 	Graphics_GetScreenHeight()

	/**************
	 * PROTOTYPES *
	 **************/

	void Graphics_DrawBackground(Uint8);
	void Graphics_DrawDot(Uint16, Uint16, Uint8);
	void Graphics_DrawOuterFrame(void);
	void Graphics_DrawText(Uint16, Uint16, char *);
	void Graphics_DrawTitle(void);
	void Graphics_DrawInstructions(void);
	void Graphics_DrawWinMessage(void);
	void Graphics_DrawGameoverMessage(void);
	void Graphics_ListHighscores(Uint16 nr_of_table);
	bool Graphics_Init(bool);
	void Graphics_UpdateScreen(void);
	void Graphics_CleanUp(void);
	Uint16 Graphics_GetDotWidth(void);
	Uint16 Graphics_GetDotHeight(void);
	Uint16 Graphics_GetScreenWidth(void);
	Uint16 Graphics_GetScreenHeight(void);
	Uint16 Graphics_GetFontHeight(void);
	Uint16 Graphics_ScaleX(Uint16 logical_x);
	Uint16 Graphics_ScaleY(Uint16 logical_y);
	Uint16 Graphics_ScaleWidth(Uint16 logical_width);
	Uint16 Graphics_ScaleHeight(Uint16 logical_height);
	void Graphics_WindowToLogical(int window_x, int window_y, int *logical_x, int *logical_y);
	SDL_Surface* Graphics_LoadGraphicsResource(char*);
	void Graphics_SetWindowIcon(void);

	/**
	 * Get the SDL renderer for use by other systems
	 * @return Pointer to the SDL renderer
	 */
	SDL_Renderer* Graphics_GetRenderer(void);

	/**
	 * This function draws the given highscore table and its associated entries.
	 */

#ifdef __cplusplus
}
#endif

#endif

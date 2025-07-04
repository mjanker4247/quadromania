/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: quadromania.c - handles the game logic and the playfield
 * last Modified: 06.03.2010 : 11:28
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

/* for data types... */
#include <SDL2/SDL.h>
#include "utils/random.h"
#include "graphics/renderer.h"
#include "core/game.h"
#include <stdbool.h>
#include "common/version.h"

/*************
 * CONSTANTS *
 *************/

/* these constants define how many permutations of tiles will happen at level start */
const Uint8 Quadromania_BaseOfRotations  = 56;
const Uint8 Quadromania_ModifierPerLevel = 13;
/* base color used - all tiles have to revert to this color*/
const Uint8 Quadromania_Basecolor        = 0;


/* data structures... */
static Uint8 playfield[18][13];
/* flags for the actual game state */
static Uint8 rotations, backgroundart;
static Uint16 turns, limit;

/* Optimization: Simple state tracking for turn-based rendering */
static struct {
    bool needs_redraw;
    Uint8 last_playfield[18][13];
} render_state = {true, {0}};


/*************
 * FUNCTIONS *
 *************/

/* function that clears the playfield */
void Quadromania_ClearPlayfield()
{
	Uint8 i, j;
	for (i = 0; i < 18; i++)
		for (j = 0; j < 13; j++)
			playfield[i][j] = Quadromania_Basecolor;
	
	/* Mark for redraw in turn-based game */
	render_state.needs_redraw = true;
}

/* this function initializes the playfield and rotates squares around
 basically preparing the playfield for a game to be played */
void Quadromania_InitPlayfield(Uint16 initialrotations, Uint8 maxrotations)
{
	Uint16 i;

	rotations = maxrotations;
	backgroundart = ((Uint32) Random_GetRandom() % 10);
	Quadromania_ClearPlayfield();

	/* rotate the squares....*/
	for (i = 1; i < initialrotations; ++i)
	{
		Quadromania_Rotate(((Random_GetRandom() % 16) + 1), ((Random_GetRandom() % 11) + 1));
	}

	turns=0;
	limit=initialrotations*maxrotations;

}

/* this method rotates the dots around a central point...
 actually the key algorithm in a Quadromania game ;)
 border and range checking has to be done in the calling routine... */
void Quadromania_Rotate(Uint32 x, Uint32 y)
{
	Uint8 i, j;
	
	/* For turn-based games, we only need to mark that something changed */
	render_state.needs_redraw = true;
	
	for (i = x - 1; i < (x + 2); ++i)
		for (j = y - 1; j < (y + 2); ++j)
		{
			playfield[i][j]++;
			if (playfield[i][j] > rotations)
				playfield[i][j] = Quadromania_Basecolor;
		}

	turns++;
}

/* this method/procedure draws the complete playfield */
void Quadromania_DrawPlayfield()
{
	Uint16 i, j;
	char txt[512];

	/* For turn-based games, only redraw if something changed */
	if (!render_state.needs_redraw) {
		/* Just update the status text which changes every turn */
		sprintf(txt,"Used turns: %d",turns);
		Graphics_DrawText(0,0,txt);
		sprintf(txt,"Limit: %d",limit);
		Graphics_DrawText(((Graphics_GetScreenWidth() *2) / 3),0,txt);
		sprintf(txt,"%s",VERSION);
		Graphics_DrawText(0, (Graphics_GetScreenHeight() - Graphics_GetFontHeight()), txt);
		return;
	}

	Graphics_DrawBackground(backgroundart);
	Graphics_DrawOuterFrame();

	/* Draw the entire playfield */
	for (i = 0; i < 18; i++) {
		for (j = 0; j < 13; j++) {
			Graphics_DrawDot(i * Graphics_GetDotWidth() + Graphics_GetDotWidth(), 
			                j * Graphics_GetDotHeight() + Graphics_GetDotHeight(), 
			                playfield[i][j]);
		}
	}

	/* draw status line */
	sprintf(txt,"Used turns: %d",turns);
	Graphics_DrawText(0,0,txt);
	sprintf(txt,"Limit: %d",limit);
	Graphics_DrawText(((Graphics_GetScreenWidth() *2) / 3),0,txt);
	sprintf(txt,"%s",VERSION);
	Graphics_DrawText(0, (Graphics_GetScreenHeight() - Graphics_GetFontHeight()), txt);
	
	/* Mark as drawn */
	render_state.needs_redraw = false;
}

/* this function tells you wether you have won or not... */
bool Quadromania_IsGameWon()
{
	bool ok = true;
	Uint8 i, j;

	for (i = 0; i < 18; i++)
	{
		for (j = 0; j < 13; j++)
		{
			if (playfield[i][j] != Quadromania_Basecolor)
			{
				ok = false;
				break;
			}
		}
		/* break out of the loops if not won.... */
		if (ok == false)
			break;
	}

	return (ok);
}

/* this function tells if the player hit the turn limit */
bool Quadromania_IsTurnLimithit()
{
    return((bool)(turns>limit));
}

/* Return percentage of solutions (turns to limit ratio)*100 */
Uint32 Quadromania_GetPercentOfSolution()
{
	if(Quadromania_IsTurnLimithit()==true)
	{
		return(0);
	}
	else
	{
		return((Uint32)(((limit-turns)*10000)/turns));
	}
}

/* this function calculates the amount of initial rotations for a given start level... */
Uint16 Quadromania_GetRotationsPerLevel(Uint8 level)
{
	return (Quadromania_BaseOfRotations + level * Quadromania_ModifierPerLevel);
}

/* Turn-based game optimizations */
void Quadromania_ForceRedraw(void)
{
	render_state.needs_redraw = true;
}

bool Quadromania_NeedsRedraw(void)
{
	return render_state.needs_redraw;
}


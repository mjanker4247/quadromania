/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: gui.c - handles drawing the GUI and dialogues to the screen + verifies input on the GUI
 * last Modified: 06.0232010 : 11:35
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

#include <stdio.h>

#include "datatypes.h"
#include "gui.h"
#include "graphics.h"
#include "event.h"
#include "quadromania.h"

/*******************************
 * data structures for the GUI *
 *******************************/

Uint16 GUI_MenuPosition[MAX_NR_OF_MENU_ENTRIES];

/**************************************
 * FUNCTIONS for drawing GUI elements *
 **************************************/

/* draw the mainmenu with all screen elements like logos, decals and game configuration on screen */
void GUI_DrawMainmenu( Uint8 nr_of_dots, Uint8 selected_level)
{
	Uint8 i;
	const Uint16 menu_column = GUI_GetMenuColumnLeft();
	char nstr[20];

	if (GUI_MenuPosition[MENU_START_GAME] == 0)
	{
		GUI_InitMenuCoordinates();
	}

	Graphics_DrawBackground(9);
	Graphics_DrawOuterFrame();
	Graphics_DrawTitle();

	Graphics_DrawText(menu_column, GUI_MenuPosition[MENU_START_GAME],
			"Start the game");
	Graphics_DrawText(menu_column,
			GUI_MenuPosition[MENU_CHANGE_NR_OF_COLORS], "Select colors");

	for (i = 0; i < nr_of_dots; ++i)
		Graphics_DrawDot(((SCREEN_WIDTH * 450) / 640) + i
				* Graphics_GetDotWidth(),
				GUI_MenuPosition[MENU_CHANGE_NR_OF_COLORS], i);

	Graphics_DrawText(menu_column,
			GUI_MenuPosition[MENU_CHANGE_NR_OF_ROTATIONS],
			"Select initial turns");
	sprintf(nstr, "%d", Quadromania_GetRotationsPerLevel(selected_level));
	Graphics_DrawText( ((SCREEN_WIDTH * 480) / 640),
			GUI_MenuPosition[MENU_CHANGE_NR_OF_ROTATIONS], nstr);

	Graphics_DrawText(menu_column, GUI_MenuPosition[MENU_HIGHSCORES],
				"Highscores");

	Graphics_DrawText(menu_column, GUI_MenuPosition[MENU_INSTRUCTIONS],
			"Instructions");

	Graphics_DrawText(menu_column, GUI_MenuPosition[MENU_QUIT], "Quit");

	Graphics_UpdateScreen();
	return;
}

/* get the left x coordinate of menu points */
Uint16 GUI_GetMenuColumnLeft()
{
	return ((SCREEN_WIDTH * 48) / 320);
}

/* get the right x coordinate of menu points */
Uint16 GUI_GetMenuColumnRight()
{
	return (SCREEN_WIDTH - GUI_GetMenuColumnLeft());
}

/* initialize position list of menu entries */
void GUI_InitMenuCoordinates()
{
	const Uint16 start_pos = ((SCREEN_HEIGHT * 240) / 480);
	const Uint16 offset_per_line = Graphics_GetFontHeight() + (SCREEN_HEIGHT
			/ 120);

	GUI_MenuPosition[MENU_START_GAME] = start_pos;
	GUI_MenuPosition[MENU_CHANGE_NR_OF_COLORS] = start_pos + offset_per_line;
	GUI_MenuPosition[MENU_CHANGE_NR_OF_ROTATIONS] = start_pos + offset_per_line
			* 2;
	GUI_MenuPosition[MENU_HIGHSCORES] = start_pos + offset_per_line
				* 3;
	GUI_MenuPosition[MENU_INSTRUCTIONS] = start_pos + offset_per_line * 4;
	GUI_MenuPosition[MENU_QUIT] = start_pos + offset_per_line * 5;
	return;
}

/* determines the currently pointed to mainmenu entry via the Event interface */
tGUI_MenuEntries GUI_GetClickedMenuEntry()
{
	const Uint16 font_height = Graphics_GetFontHeight();
	const Uint16 mouse_y = Event_GetMouseY();

	if ((Event_GetMouseX() > GUI_GetMenuColumnLeft()) && (Event_GetMouseX() < GUI_GetMenuColumnRight()))
	{
		if ((mouse_y > GUI_MenuPosition[MENU_START_GAME]) && (mouse_y
				< GUI_MenuPosition[MENU_START_GAME] + font_height))
		{
			return MENU_START_GAME;
		}
		else if ((mouse_y > GUI_MenuPosition[MENU_CHANGE_NR_OF_COLORS])
				&& (mouse_y < GUI_MenuPosition[MENU_CHANGE_NR_OF_COLORS]
						+ font_height))
		{
			return MENU_CHANGE_NR_OF_COLORS;
		}
		else if ((mouse_y > GUI_MenuPosition[MENU_CHANGE_NR_OF_ROTATIONS])
				&& (mouse_y < GUI_MenuPosition[MENU_CHANGE_NR_OF_ROTATIONS]
						+ font_height))
		{
			return MENU_CHANGE_NR_OF_ROTATIONS;
		}
		else if ((mouse_y > GUI_MenuPosition[MENU_HIGHSCORES]) && (mouse_y
						< GUI_MenuPosition[MENU_HIGHSCORES] + font_height))
		{
			return MENU_HIGHSCORES;
		}
		else if ((mouse_y > GUI_MenuPosition[MENU_INSTRUCTIONS]) && (mouse_y
				< GUI_MenuPosition[MENU_INSTRUCTIONS] + font_height))
		{
			return MENU_INSTRUCTIONS;
		}
		else if ((mouse_y > GUI_MenuPosition[MENU_QUIT]) && (mouse_y
				< GUI_MenuPosition[MENU_QUIT] + font_height))
		{
			return MENU_QUIT;
		}
		else
		{
			return MENU_UNDEFINED;
		}
	}
	else
	{
		return MENU_UNDEFINED;
	}
}

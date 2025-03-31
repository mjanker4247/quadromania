/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: main.c - the main module handling input and game control
 * last Modified: 18.11.2010 : 18:36
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
#include <string.h>
#include <stdlib.h>

#include "datatypes.h"
#include "random.h"
#include "graphics.h"
#include "sound.h"
#include "quadromania.h"
#include "highscore.h"
#include "boolean.h"
#include "SFont.h"
#include "debug.h"

#include "main.h"
#include "event.h"
#include "gui.h"

/* the main function - program execution starts here... */
int main(int argc, char *argv[])
{
	BOOLEAN fullscreen = FALSE;
	BOOLEAN debug = FALSE;

	/* parse command line arguments... */
	while (argc > 1)
	{
		if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--fullscreen") == 0)
		{
			fullscreen = TRUE;
		}
		else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0)
		{
			debug = TRUE;
		}
		else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			fprintf(stderr, "Quadromania v%s\n", VERSION);
			fprintf(stderr, "Copyright (c) 2002-2010 by Matthias Arndt / ASM Software\n");
			fprintf(stderr, "2925 Modified by Marco Janker\n");
			fprintf(stderr, "This program is free software under the GNU General Public License\n\n");
			fprintf(stderr, "Usage: %s [options]\n", argv[0]);
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "  -f, --fullscreen    Run in fullscreen mode\n");
			fprintf(stderr, "  -d, --debug         Enable debug output\n");
			fprintf(stderr, "  -h, --help          Show this help message\n");
			return 0;
		}
		argc--;
		argv++;
	}

	/* Initialize debug system */
	DEBUG_INIT(debug);

	/* initialize game engine... */
	if (InitGameEngine(fullscreen))
	{
		/* initialize event handler */
		Event_Init();
		Quadromania_ClearPlayfield();
		MainHandler();
	}
	else
	{
		return (1);
	}

	return (0);
}

/**
 * The game engine and its subcomponents are initialized from this function.
 * @returns whether the initialization was successfull
 */
BOOLEAN InitGameEngine(BOOLEAN fullscreen)
{
	/* initialize random number generator... */
	Random_InitSeed();
	/* load highscores from disk */
	Highscore_LoadTable();
	/* initialize sound system */
	Sound_Init();
	/* initialize graphics module... */
	if(Graphics_Init(fullscreen))
	{
		/* initialize event handler */
		Event_Init();
		Quadromania_ClearPlayfield();
		return (TRUE);
	}
	else
	{
		return(FALSE);
	}
}

/**
 * The main handling function implements the title screen and game management via a statemachine.
 * Transitions of the state machine are initiated from user input.
 */
void MainHandler()
{
	enum GAMESTATE status, oldstatus; /* for the event driven automata... */

	tGUI_MenuEntries menu;  /* the current selected menu entry */

	Uint8 maxrotations = 1;        /* setup variable for the maximum amount of possible colors      */
	Uint8 level = 1;               /* game level - to setup the desired amount of initial rotations */
	Uint32 score = 0;              /* the score calculated from turn to limit ratio at game over    */
	Uint16 highscore_position = 0; /* possible highscore list entry position                        */

	char *highscore_entry;

	status = UNINITIALIZED;
	oldstatus = status;
	/* the main loop - event and automata driven :) */
	do
	{
		menu = MENU_UNDEFINED; /* safe guard menu selection */
		/* Event reading and parsing.... */
		Event_ProcessInput();
		if (Event_GetDpadUp() == TRUE)
		{
			fprintf(stderr,"n\n");
			Event_DebounceDpad();
			while(Event_IsDpadPressed() == TRUE);
		}
		if (Event_GetDpadButton() == TRUE)
		{
			fprintf(stderr,"DPAD BUTTON\n");
			Event_DebounceDpad();
		}
		if (Event_IsESCPressed() == TRUE)
		{
			if ((status == GAME) || (status == SHOW_HIGHSCORES))
			{
				/* is there a game running? if yes then back to title screen...*/
				status = TITLE;
			}
			else
			{
				status = QUIT;
			}
			Event_DebounceKeys();
		}
		if (Event_QuitRequested() == TRUE)
		{
			status = QUIT;
		}

		/* act upon the state of the event driven automata... */
		switch (status)
		{
		case SETUPCHANGED:
		case TITLE:
			if (oldstatus != status) /* recently switched to the title screen? */
			{
				/* then we have to redraw it....*/
				GUI_DrawMainmenu(maxrotations + 1, level);
				if (status == SETUPCHANGED)
					status = TITLE;
				oldstatus = status;
			}

			/* check for clicks in the menu */
			if (Event_MouseClicked() == TRUE)
			{
				if (Event_GetMouseButton() == 1)
				{
					menu = GUI_GetClickedMenuEntry();
					switch (menu)
					{
					case MENU_START_GAME:
						/* "start a new game" ? */
						Sound_PlayEffect(SOUND_MENU);
						status = GAME;
						Quadromania_InitPlayfield(
								Quadromania_GetRotationsPerLevel(level),
								maxrotations);
						Quadromania_DrawPlayfield();
						Graphics_UpdateScreen();
						break;
					case MENU_CHANGE_NR_OF_COLORS:
						/* "Select Colors" ? */
						Sound_PlayEffect(SOUND_MENU);
						status = SETUPCHANGED;
						++maxrotations;
						if (maxrotations > 4)
							maxrotations = 1;
						break;

					case MENU_CHANGE_NR_OF_ROTATIONS:
						/* "Select number of rotations" ? */
						Sound_PlayEffect(SOUND_MENU);
						status = SETUPCHANGED;
						++level;
						if (level > HIGHSCORE_NR_OF_TABLES)
							level = 1;
						break;
					case MENU_INSTRUCTIONS:
						/* "Instructions" ? */
						Sound_PlayEffect(SOUND_MENU);
						status = INSTRUCTIONS;
						break;
					case MENU_HIGHSCORES:
						/* Highscores? */
						Sound_PlayEffect(SOUND_MENU);
						status = SHOW_HIGHSCORES;
						break;
					case MENU_QUIT:
						Sound_PlayEffect(SOUND_MENU);
						status = QUIT;
						break;
					default:
						/* undefined menu entry */
						break;
					};
				}
				Event_DebounceMouse();
			}
			break;
		case INSTRUCTIONS:
			/* shall we show the INSTRUCTIONS screen? */
			if (oldstatus != status)
			{
				oldstatus = status;
				/* redraw instructions screen */
				Graphics_DrawInstructions();

			}
			if (Event_MouseClicked() == TRUE)
			{
				if ((Event_GetMouseButton() == 1) && (Event_GetMouseY()
						> (SCREEN_HEIGHT - Graphics_GetFontHeight())))
				{
					Event_DebounceMouse();
					status = TITLE;
				}
			}
			break;
		case GAME:
			/* is there a game running? */
			if (oldstatus != status)
				oldstatus = status;

			/* mousebutton clicked?*/
			if (Event_MouseClicked() == TRUE)
			{
				if (Event_GetMouseButton() == 1)
				{
					Uint16 xraster, yraster;
					xraster = (Uint16) ((Event_GetMouseX()
							- Graphics_GetDotWidth()) / Graphics_GetDotWidth());
					yraster = (Uint16) ((Event_GetMouseY()
							- Graphics_GetDotHeight())
							/ Graphics_GetDotHeight());

					DEBUG_PRINT("Click at %d,%d", xraster, yraster);

					/* valid click on playfield? */
					if ((xraster > 0) && (xraster < 17) && (yraster > 0)
							&& (yraster < 12))
					{
						/* then rotate the correct 3x3 part... */
						Quadromania_Rotate(xraster, yraster);
						Quadromania_DrawPlayfield();
						Graphics_UpdateScreen();

						/* update score */
						score = Quadromania_GetPercentOfSolution();
						DEBUG_PRINT("Score: %d%%", score);

						/* make noise */
						Sound_PlayEffect(SOUND_TURN);
						/* check for unsuccessful end*/
						if (Quadromania_IsTurnLimithit())
							status = GAMEOVER;

						/* check for successful game end... */
						if (Quadromania_IsGameWon())
							status = WON; /* if yes (board cleared to red) - well go to end screen :) */
					}

				}
				Event_DebounceMouse();
			}

			break;
		case WON:
			if (status != oldstatus)
			{
				oldstatus = status;
				Graphics_DrawWinMessage();
				Sound_PlayEffect(SOUND_WIN);
			}

			if (Event_MouseClicked() == TRUE)
			{
				Event_DebounceMouse();
				status = HIGHSCORE_ENTRY;
			}
			break;

		case GAMEOVER:
			if (status != oldstatus)
			{
				oldstatus = status;
				Graphics_DrawGameoverMessage();
				Sound_PlayEffect(SOUND_LOOSE);
			}

			if (Event_MouseClicked() == TRUE)
			{
				Event_DebounceMouse();
				status = SHOW_HIGHSCORES; /* no highscore entry in case of all turns are used up */
			}
			break;
		case HIGHSCORE_ENTRY:
			if((highscore_position = Highscore_GetPosition(level-1,score)) != HIGHSCORE_NO_ENTRY)
			{
				highscore_entry = Highscore_GetNameFromTimestamp();
				Highscore_EnterScore(level-1, score, highscore_entry , highscore_position);
				DEBUG_PRINT("Highscore: %d, Position %d", score, highscore_position);
				status = SHOW_HIGHSCORES;
			}
			else
			{
				status = TITLE;
			}
			break;
		case SHOW_HIGHSCORES:
			if (status != oldstatus)
			{
				oldstatus = status;
				Graphics_ListHighscores(level-1);
			}

			if (Event_MouseClicked() == TRUE)
			{
				Event_DebounceMouse();
				status = TITLE;
			}
			break;
		case QUIT:
			/* so you want to quit? */
			break;
		default:
			/* unknown or undefined state - then go to the title screen...*/
			status = TITLE;
			oldstatus = NONE;
			break;
		}

	} while (status != QUIT);

	/* save highscores at game end */
	Highscore_SaveTable();

}

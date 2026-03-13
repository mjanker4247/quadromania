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
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <SDL2/SDL.h>
#include "utils/random.h"
#include "graphics/renderer.h"
#include "audio/sound.h"
#include "core/game.h"
#include "data/highscore.h"
#include "graphics/fonts.h"
#include "utils/logger.h"
#include "core/state_manager.h"

#include "main.h"
#include "input/events.h"
#include "graphics/ui.h"

/* the main function - program execution starts here... */
int main(int argc, char *argv[])
{
	LOG_INFO("Quadromania starting up (version: %s)", VERSION);

	// Hardcoded options
	bool fullscreen = false; // Set to true for fullscreen
	bool debug_logging = true;

	// Initialize logger system with hardcoded defaults
	logger_init(debug_logging, false, true, LOG_LEVEL_INFO, NULL, 0, 5, false);

	// initialize game engine...
	if (InitGameEngine(fullscreen))
	{
		// initialize state manager
		game_state_context_t state_context;
		state_manager_init(&state_context);

		// Main game loop using simplified state manager
		MainHandler(&state_context);

		// Clean up state manager
		state_manager_cleanup(&state_context);
		LOG_DEBUG("State manager cleaned up");
	}
	else
	{
		LOG_ERROR("Failed to initialize game engine");
		return (1);
	}

	// Cleanup audio system
	Sound_Exit();
	LOG_INFO("Quadromania shutting down");
	return (0);
}

/**
 * The game engine and its subcomponents are initialized from this function.
 * @returns whether the initialization was successfull
 */
bool InitGameEngine(bool fullscreen)
{
	LOG_INFO("Initializing game engine (fullscreen: %s)", fullscreen ? "true" : "false");
	
	/* initialize random number generator... */
	Random_InitSeed();
	LOG_DEBUG("Random number generator initialized");
	
	/* load highscores from disk */
	Highscore_LoadTable();
	LOG_DEBUG("Highscore table loaded");
	
	/* initialize sound system */
	Sound_Init();
	LOG_DEBUG("Audio system initialized");
	
	/* initialize graphics module... */
	if(Graphics_Init(fullscreen))
	{
		LOG_INFO("Graphics system initialized successfully");
		/* Show mouse cursor and disable relative mouse mode */
		SDL_ShowCursor(SDL_ENABLE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
		/* initialize event handler */
		Event_Init();
		LOG_DEBUG("Event handler initialized");
		Quadromania_ClearPlayfield();
		LOG_DEBUG("Playfield cleared");
		return (true);
	}
	else
	{
		LOG_ERROR("Failed to initialize graphics system");
		return(false);
	}
}

/**
 * The main handling function implements the game loop using the simplified state manager.
 * @param context Pointer to the game state context
 */
void MainHandler(game_state_context_t *context)
{
	if (!context) return;
	
	LOG_INFO("Starting main game loop");
	
	/* main game loop */
	while (true) {
		/* Process input events */
		Event_ProcessInput();
		
		/* Update game state - returns false if should quit */
		if (!state_manager_update(context)) {
			DEBUG_PRINT("State manager requested quit");
			break;
		}
		
		/* For responsive input, use a shorter delay */
		SDL_Delay(16); /* 60 FPS for responsive input */
	}
	
	LOG_INFO("Game loop ended");
}

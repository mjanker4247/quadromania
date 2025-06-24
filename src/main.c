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

#include <SDL.h>
#include "utils/random.h"
#include "graphics/renderer.h"
#include "audio/sound.h"
#include "core/game.h"
#include "data/highscore.h"
#include "data/config.h"
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
	
	config_t *config = config_init();
	if (!config) {
		LOG_ERROR("Failed to initialize configuration");
		fprintf(stderr, "Failed to initialize configuration\n");
		return 1;
	}
	LOG_DEBUG("Configuration structure initialized");

	/* First pass: look for --config option */
	char *config_file = NULL;
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--config") == 0) {
			config_file = argv[i + 1];
			break;
		}
	}
	
	if (!config_file) {
		/* Use default config file in executable directory */
		config_file = config_get_default_path();
		if (!config_file) {
			LOG_ERROR("Failed to get default config path");
			fprintf(stderr, "Failed to get default config path\n");
			config_free(config);
			return 1;
		}
	}
	LOG_INFO("Using config file: %s", config_file);
	
	/* Always create or update the config file to ensure all options are present */
	config_create_or_update(config_file);
	LOG_DEBUG("Config file created/updated");
	
	/* Load configuration from file */
	config_load(config, config_file);
	LOG_DEBUG("Configuration loaded from file");
	
	/* Parse command line arguments (overrides config file) */
	config_parse_args(config, argc, argv);
	LOG_DEBUG("Command line arguments parsed");

	/* Initialize logger system with enhanced defaults */
	if (config->debug || config->log_to_file || config->log_level != LOG_LEVEL_INFO || 
		config->max_file_size > 0 || !config->log_to_stderr) {
		logger_init(config->debug || config->log_level >= LOG_LEVEL_DEBUG, 
				   config->log_to_file, config->log_to_stderr, 
				   config->log_level, config->log_filename, config->max_file_size, 
				   config->max_files, config->log_overwrite);
	} else {
		/* Enable basic logging for development */
		logger_init(true, false, true, LOG_LEVEL_INFO, NULL, 0, 5, false);
	}

	/* initialize game engine... */
	if (InitGameEngine(config->fullscreen))
	{
		/* initialize state manager */
		game_state_context_t state_context;
		state_manager_init(&state_context);
		
		/* Start with title screen */
		state_manager_transition_to(&state_context, GAME_STATE_TITLE);
		
		/* Main game loop using state manager */
		MainHandler(&state_context);
		
		/* Clean up state manager */
		state_manager_cleanup(&state_context);
		LOG_DEBUG("State manager cleaned up");
	}
	else
	{
		LOG_ERROR("Failed to initialize game engine");
		config_free(config);
		return (1);
	}

	config_free(config);
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
	LOG_DEBUG("Sound system initialized");
	
	/* initialize graphics module... */
	if(Graphics_Init(fullscreen))
	{
		LOG_INFO("Graphics system initialized successfully");
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
 * The main handling function implements the game loop using the state manager.
 * @param context Pointer to the game state context
 */
void MainHandler(game_state_context_t *context)
{
	if (!context) return;
	
	LOG_INFO("Starting main game loop");
	
	/* Main game loop */
	while (state_manager_process_transitions(context)) {
		state_manager_update(context);
		state_manager_render(context);
		
		/* Small delay to prevent excessive CPU usage */
		SDL_Delay(16); /* ~60 FPS */
	}
	
	LOG_INFO("Main game loop ended");
}

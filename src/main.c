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

#include "common/datatypes.h"
#include "utils/random.h"
#include "graphics/renderer.h"
#include "audio/sound.h"
#include "core/game.h"
#include "data/highscore.h"
#include "graphics/fonts.h"
#include "utils/logger.h"

#include "main.h"
#include "input/events.h"
#include "graphics/ui.h"

/* Helper: trim whitespace */
static char *trim(char *str) {
	char *end;
	while (isspace((unsigned char)*str)) str++;
	if (*str == 0) return str;
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;
	end[1] = '\0';
	return str;
}

/* Helper: get executable directory */
static char* get_executable_dir(void) {
	static char exec_path[1024];
	static char *exec_dir = NULL;
	
	if (exec_dir == NULL) {
#ifdef __APPLE__
		uint32_t size = sizeof(exec_path);
		if (_NSGetExecutablePath(exec_path, &size) == 0) {
			exec_dir = dirname(exec_path);
		} else {
			exec_dir = ".";
		}
#else
		ssize_t count = readlink("/proc/self/exe", exec_path, sizeof(exec_path));
		if (count != -1) {
			exec_dir = dirname(exec_path);
		} else {
			exec_dir = ".";
		}
#endif
	}
	return exec_dir;
}

/* Helper: create or update config file with missing options */
static void create_or_update_config_file(const char *filename) {
	FILE *f = fopen(filename, "r");
	bool options_present[8] = {false}; // Track which options are present
	char line[256];
	char *option_names[] = {"fullscreen", "debug", "log_file", "log_level", 
						   "log_max_size", "log_max_files", "log_to_stderr", "log_overwrite"};
	char *option_defaults[] = {"false", "false", "quadromania.log", "info", 
							  "1024", "1", "false", "true"};
	
	// Read existing config and check which options are present
	if (f) {
		while (fgets(line, sizeof(line), f)) {
			char *eq = strchr(line, '=');
			if (eq) {
				*eq = 0;
				char *key = trim(line);
				for (int i = 0; i < 8; i++) {
					if (strcasecmp(key, option_names[i]) == 0) {
						options_present[i] = true;
						break;
					}
				}
			}
		}
		fclose(f);
	}
	
	// Open file in append mode to add missing options
	f = fopen(filename, "a");
	if (!f) return;
	
	// Add header if file was empty
	if (!options_present[0] && !options_present[1] && !options_present[2]) {
		fprintf(f, "# Quadromania Configuration File\n");
		fprintf(f, "# Lines starting with # are comments\n\n");
	}
	
	// Add missing options
	bool added_any = false;
	for (int i = 0; i < 8; i++) {
		if (!options_present[i]) {
			if (i == 0) fprintf(f, "# Display settings\n");
			else if (i == 2) fprintf(f, "\n# Debug settings\n");
			fprintf(f, "%s=%s\n", option_names[i], option_defaults[i]);
			added_any = true;
		}
	}
	
	fclose(f);
	
	if (added_any) {
		fprintf(stderr, "Updated config file: %s (added missing options)\n", filename);
	} else if (!options_present[0]) {
		fprintf(stderr, "Created new config file: %s\n", filename);
	}
}

/* Helper: parse config file */
static void parse_config_file(const char *filename,
							 bool *fullscreen,
							 bool *debug,
							 char **log_filename,
							 int *log_level,
							 size_t *max_file_size,
							 int *max_files,
							 bool *log_to_file,
							 bool *log_to_stderr,
							 bool *log_overwrite) {
	FILE *f = fopen(filename, "r");
	if (!f) return;
	char line[256];
	while (fgets(line, sizeof(line), f)) {
		char *eq, *key, *val;
		if (line[0] == '#' || line[0] == ';' || line[0] == '\n') continue;
		eq = strchr(line, '=');
		if (!eq) continue;
		*eq = 0;
		key = trim(line);
		val = trim(eq + 1);
		if (strcasecmp(key, "fullscreen") == 0) {
			*fullscreen = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
		} else if (strcasecmp(key, "debug") == 0) {
			*debug = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
		} else if (strcasecmp(key, "log_file") == 0) {
			*log_filename = strdup(val);
			*log_to_file = true;
		} else if (strcasecmp(key, "log_level") == 0) {
			if (strcasecmp(val, "error") == 0) *log_level = LOG_LEVEL_ERROR;
			else if (strcasecmp(val, "warn") == 0) *log_level = LOG_LEVEL_WARN;
			else if (strcasecmp(val, "info") == 0) *log_level = LOG_LEVEL_INFO;
			else if (strcasecmp(val, "debug") == 0) *log_level = LOG_LEVEL_DEBUG;
			else if (strcasecmp(val, "trace") == 0) *log_level = LOG_LEVEL_TRACE;
		} else if (strcasecmp(key, "log_max_size") == 0) {
			*max_file_size = (size_t)atol(val);
		} else if (strcasecmp(key, "log_max_files") == 0) {
			*max_files = atoi(val);
		} else if (strcasecmp(key, "log_to_stderr") == 0) {
			*log_to_stderr = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
		} else if (strcasecmp(key, "log_overwrite") == 0) {
			*log_overwrite = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
		}
	}
	fclose(f);
}

/* the main function - program execution starts here... */
int main(int argc, char *argv[])
{
	bool fullscreen = false;
	bool debug = false;
	char *log_filename = NULL;
	int log_level = LOG_LEVEL_DEBUG;
	size_t max_file_size = 0;
	int max_files = 5;
	bool log_to_file = false;
	bool log_to_stderr = true;
	bool log_overwrite = false;
	char *config_file = NULL;

	/* First pass: look for --config option */
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--config") == 0) {
			config_file = argv[i + 1];
			break;
		}
	}
	if (!config_file) {
		/* Use default config file in executable directory */
		char default_config_path[1024];
		snprintf(default_config_path, sizeof(default_config_path), "%s/quadromania.cfg", get_executable_dir());
		
		/* Always create or update the config file to ensure all options are present */
		create_or_update_config_file(default_config_path);
		config_file = strdup(default_config_path);
	}
	if (config_file) {
		parse_config_file(config_file, &fullscreen, &debug, &log_filename, &log_level, &max_file_size, &max_files, &log_to_file, &log_to_stderr, &log_overwrite);
	}

	/* parse command line arguments... */
	while (argc > 1)
	{
		if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--fullscreen") == 0)
		{
			fullscreen = true;
		}
		else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0)
		{
			debug = true;
		}
		else if (strcmp(argv[1], "--log-file") == 0 && argc > 2)
		{
			if (log_filename) free(log_filename);
			log_filename = strdup(argv[2]);
			log_to_file = true;
			argc--;
			argv++;
		}
		else if (strcmp(argv[1], "--log-level") == 0 && argc > 2)
		{
			if (strcmp(argv[2], "error") == 0) log_level = LOG_LEVEL_ERROR;
			else if (strcmp(argv[2], "warn") == 0) log_level = LOG_LEVEL_WARN;
			else if (strcmp(argv[2], "info") == 0) log_level = LOG_LEVEL_INFO;
			else if (strcmp(argv[2], "debug") == 0) log_level = LOG_LEVEL_DEBUG;
			else if (strcmp(argv[2], "trace") == 0) log_level = LOG_LEVEL_TRACE;
			argc--;
			argv++;
		}
		else if (strcmp(argv[1], "--log-max-size") == 0 && argc > 2)
		{
			max_file_size = (size_t)atol(argv[2]);
			argc--;
			argv++;
		}
		else if (strcmp(argv[1], "--log-max-files") == 0 && argc > 2)
		{
			max_files = atoi(argv[2]);
			argc--;
			argv++;
		}
		else if (strcmp(argv[1], "--no-stderr") == 0)
		{
			log_to_stderr = false;
		}
		else if (strcmp(argv[1], "--log-overwrite") == 0)
		{
			log_overwrite = true;
		}
		else if (strcmp(argv[1], "--config") == 0 && argc > 2)
		{
			/* already handled above */
			argc--;
			argv++;
		}
		else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			fprintf(stderr, "Quadromania v%s\n", VERSION);
			fprintf(stderr, "Copyright (c) 2002-2010 by Matthias Arndt / ASM Software\n");
			fprintf(stderr, "2925 Modified by Marco Janker\n");
			fprintf(stderr, "This program is free software under the GNU General Public License\n\n");
			fprintf(stderr, "Usage: %s [options]\n", argv[0]);
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "  --config <file>          Load parameters from config file\n");
			fprintf(stderr, "  -f, --fullscreen         Run in fullscreen mode\n");
			fprintf(stderr, "  -d, --debug              Enable debug output (to stderr)\n");
			fprintf(stderr, "      --log-file <file>    Log debug output to file\n");
			fprintf(stderr, "      --log-level <level>  Set log level (error, warn, info, debug, trace)\n");
			fprintf(stderr, "      --log-max-size <n>   Max log file size in bytes before rotation\n");
			fprintf(stderr, "      --log-max-files <n>  Number of rotated log files to keep\n");
			fprintf(stderr, "      --no-stderr          Do not log to stderr\n");
			fprintf(stderr, "      --log-overwrite      Overwrite log file at startup\n");
			fprintf(stderr, "  -h, --help               Show this help message\n");
			fprintf(stderr, "\nConfig file format (key=value, one per line):\n");
			fprintf(stderr, "  fullscreen=true|false\n  debug=true|false\n  log_file=quadromania.log\n  log_level=info|debug|warn|error|trace\n  log_max_size=1000000\n  log_max_files=3\n  log_to_stderr=true|false\n  log_overwrite=true|false\n");
			return 0;
		}
		argc--;
		argv++;
	}

	/* Initialize logger system */
	if (log_to_file || log_level != LOG_LEVEL_DEBUG || max_file_size > 0 || !log_to_stderr) {
		/* If log_filename is relative, make it absolute by prepending executable directory */
		if (log_filename && log_filename[0] != '/' && log_filename[0] != '\\') {
			char full_log_path[1024];
			snprintf(full_log_path, sizeof(full_log_path), "%s/%s", get_executable_dir(), log_filename);
			free(log_filename);
			log_filename = strdup(full_log_path);
		}
		logger_init(debug, log_to_file, log_to_stderr, log_level, log_filename, max_file_size, max_files, log_overwrite);
	} else {
		DEBUG_INIT(debug);
	}

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
bool InitGameEngine(bool fullscreen)
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
		return (true);
	}
	else
	{
		return(false);
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
		if (Event_GetDpadUp() == true)
		{
			fprintf(stderr,"n\n");
			Event_DebounceDpad();
			while(Event_IsDpadPressed() == true);
		}
		if (Event_GetDpadButton() == true)
		{
			fprintf(stderr,"DPAD BUTTON\n");
			Event_DebounceDpad();
		}
		if (Event_IsESCPressed() == true)
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
		if (Event_QuitRequested() == true)
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
			if (Event_MouseClicked() == true)
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
			if (Event_MouseClicked() == true)
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
			if (Event_MouseClicked() == true)
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

			if (Event_MouseClicked() == true)
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

			if (Event_MouseClicked() == true)
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

			if (Event_MouseClicked() == true)
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

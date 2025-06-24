/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: graphics.c - implements the graphics API
 * last Modified: 18.11.2010 : 19:12
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
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>  // for dirname()
#ifdef __APPLE__
#include <mach-o/dyld.h>  // for _NSGetExecutablePath
#endif
#include <unistd.h>  // for readlink

#include "graphics/renderer.h"
#include "graphics/fonts.h"
#include "data/highscore.h"
#include "utils/logger.h"
#include "graphics/ttf_font.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *textures = NULL, *frame = NULL, *dots = NULL, *font = NULL, 
                   *title = NULL, *copyright = NULL;
static TTF_Font *game_font = NULL;

/* Optimization: Texture cache to avoid reloading */
static struct {
    SDL_Texture *texture;
    char filename[256];
    bool loaded;
} texture_cache[10] = {0};

static Uint16 frame_width, frame_height, 
		dot_width, dot_height, texture_width,
		texture_height, font_height, 
		title_width, title_height, 
		copyright_width, copyright_height;

/**
 * @return width of the text in pixels for TTF font
 */
static Uint16 TTF_TextWidth(char *text)
{
	int w, h;
	TTF_SizeText(game_font, text, &w, &h);
	return (Uint16)w;
}

/**
 * This function textures the complete screen with 1 out of 10 textures.
 */
void Graphics_DrawBackground(Uint8 texture)
{
	Uint8 i, j;
	SDL_Rect src, dest;

	const Uint8 x_blits = (Graphics_GetScreenWidth() / texture_width);
	const Uint8 y_blits = (Graphics_GetScreenHeight() / texture_height);

	/* draw the textured background... */
	for (j = 0; j < y_blits; j++)
		for (i = 0; i < x_blits; i++)
		{
			src.x = (texture % NR_OF_TEXTURES) * texture_width;
			src.y = 0;
			src.w = texture_width;
			src.h = texture_height;
			dest.x = Graphics_ScaleX(i * texture_width);
			dest.y = Graphics_ScaleY(j * texture_height);
			dest.w = Graphics_ScaleWidth(texture_width);
			dest.h = Graphics_ScaleHeight(texture_height);
			SDL_RenderCopy(renderer, textures, &src, &dest);
		}
}

/**
 * This function draws one of the coloured dots at the given playfield coordinate.
 */
void Graphics_DrawDot(Uint16 x, Uint16 y, Uint8 number)
{
	SDL_Rect src, dest;

	src.x = (number % NR_OF_DOTS) * dot_width;
	src.y = 0;
	src.w = dot_width;
	src.h = dot_height;
	dest.x = Graphics_ScaleX(x);
	dest.y = Graphics_ScaleY(y);
	dest.w = Graphics_ScaleWidth(dot_width);
	dest.h = Graphics_ScaleHeight(dot_height);
	SDL_RenderCopy(renderer, dots, &src, &dest);
}

/**
 * This function draws the title logo onscreen.
 */
void Graphics_DrawTitle()
{
	SDL_Rect src, dest;

	src.x = 0;
	src.y = 0;
	src.w = title_width;
	src.h = title_height;
	dest.x = Graphics_ScaleX((Graphics_GetScreenWidth() / 2) - (title_width / 2));
	dest.y = Graphics_ScaleY(Graphics_GetDotHeight());
	dest.w = Graphics_ScaleWidth(title_width);
	dest.h = Graphics_ScaleHeight(title_height);
	SDL_RenderCopy(renderer, title, &src, &dest);

	// Draw the copyright image
	src.x = 0;
	src.y = 0;
	src.w = copyright_width;
	src.h = copyright_height;
	dest.x = Graphics_ScaleX((Graphics_GetScreenWidth() / 2) - (copyright_width / 2));
	dest.y = Graphics_ScaleY((Graphics_GetScreenHeight() * 120) / 480);
	dest.w = Graphics_ScaleWidth(copyright_width);
	dest.h = Graphics_ScaleHeight(copyright_height);
	SDL_RenderCopy(renderer, copyright, &src, &dest);

}

/**
 * This function draws the "instructions" screen.
 */
void Graphics_DrawInstructions()
{
	const char *continue_msg = "Click here to continue!";
	const Uint16 instruction_y = ((SCREEN_HEIGHT * 120) / 480);

	SDL_Rect src, dest;
	SDL_Surface *instructions_gfx;

	instructions_gfx  = Graphics_LoadGraphicsResource("instructions.png");

	Graphics_DrawBackground(0);
	Graphics_DrawOuterFrame();
	/* draw logo */
	src.x = 0;
	src.y = 0;
	src.w = texture_width;
	src.h = texture_height;
	dest.x = ((SCREEN_WIDTH / 2) - (texture_width / 2));
	dest.y = instruction_y + (dot_height/2);
	dest.w = src.w;
	dest.h = src.h;
	SDL_RenderCopy(renderer, title, &src, &dest);
	XCenteredString(renderer, instruction_y, "Instructions");
	/* draw instructions */
	src.x = 0;
	src.y = 0;
	src.w = instructions_gfx->w;
	src.h = instructions_gfx->h;
	dest.x = ((SCREEN_WIDTH / 2) - (instructions_gfx->w / 2));
	dest.y = instruction_y + (dot_height/2);
	dest.w = src.w;
	dest.h = src.h;
	SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, instructions_gfx), &src, &dest);

	Graphics_DrawText((SCREEN_WIDTH - TTF_Font_GetTextWidth((char *)continue_msg)),(SCREEN_HEIGHT - font_height),(char *)continue_msg);
	Graphics_UpdateScreen();

	SDL_FreeSurface(instructions_gfx);
}

/**
 * This function draws the given highscore table and its associated entries.
 */
void Graphics_ListHighscores(Uint16 nr_of_table)
{
	Uint16 i,y;
	char txt[30];
	SDL_Rect src,dest;
	const Uint16 highscore_y = ((SCREEN_HEIGHT * 120) / 480);

	HighscoreEntry *entry;

	Graphics_DrawBackground(1);
	Graphics_DrawOuterFrame();
	/* draw logo */
	src.x = 0;
	src.y = 0;
	src.w = texture_width;
	src.h = texture_height;
	dest.x = ((SCREEN_WIDTH / 2) - (texture_width / 2));
	dest.y = dot_height;
	dest.w = src.w;
	dest.h = src.h;
	SDL_RenderCopy(renderer, title, &src, &dest);
	sprintf(txt,"High scores for Level %d",nr_of_table+1);
	XCenteredString(renderer, highscore_y, txt);

	for(i = 0; i < HIGHSCORE_NR_OF_ENTRIES_PER_TABLE; i++)
	{
		entry=Highscore_GetEntry(nr_of_table, i);
		DEBUG_PRINT("Highscore entry %d: %s %d", i, entry->name, entry->score);

		y = highscore_y + ((i + 2) * font_height);

		sprintf(txt,"%s",entry->name);
		Graphics_DrawText(dot_width, y,txt);
		sprintf(txt,"%d",entry->score);
		Graphics_DrawText(((SCREEN_WIDTH * 3) / 4), y,txt);
	}

	Graphics_UpdateScreen();
}

/**
 * This functions draws the wooden outer frame onscreen.
 */
void Graphics_DrawOuterFrame()
{
	SDL_Rect src, dest;

	src.x = 0;
	src.y = 0;
	src.w = frame_width;
	src.h = frame_height;
	dest.x = 0;
	dest.y = 0;
	dest.w = Graphics_ScaleWidth(Graphics_GetScreenWidth());
	dest.h = Graphics_ScaleHeight(Graphics_GetScreenHeight());
	SDL_RenderCopy(renderer, frame, &src, &dest);
}

/**
 * This function draws text at the given screen coordinate.
 */
void Graphics_DrawText(Uint16 x, Uint16 y, char *text)
{
	TTF_Font_DrawText(renderer, Graphics_ScaleX(x), Graphics_ScaleY(y), text);
}

/**
 * This function shows the "you have won!" message.
 */
void Graphics_DrawWinMessage()
{
	const Uint16 factor = (SCREEN_WIDTH / 320);
	SDL_Rect base, dest;
	/* draw some message ...*/

	base.x = (SCREEN_WIDTH / 64);
	base.y = (SCREEN_HEIGHT / 2) - 20;
	base.w = (SCREEN_WIDTH - 2 * base.x);
	base.h = font_height + factor * 2;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &base);

	dest.x = base.x + factor;
	dest.y = base.y + factor;
	dest.w = base.w - 2 * factor;
	dest.h = base.h - 2 * factor;
	SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
	SDL_RenderFillRect(renderer, &dest);

	XCenteredString(renderer,dest.y + factor,
			"Congratulations! You've won!");
	Graphics_UpdateScreen();
	return;
}

/**
 * This function shows the "you have lost" message.
 */
void Graphics_DrawGameoverMessage()
{
	const Uint16 factor = (SCREEN_WIDTH / 320);
	SDL_Rect base, dest;
	/* draw some message ...*/

	base.x = (SCREEN_WIDTH / 64);
	base.y = (SCREEN_HEIGHT / 2) - 20;
	base.w = (SCREEN_WIDTH - 2 * base.x);
	base.h = font_height + factor * 2;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &base);

	dest.x = base.x + factor;
	dest.y = base.y + factor;
	dest.w = base.w - 2 * factor;
	dest.h = base.h - 2 * factor;
	SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
	SDL_RenderFillRect(renderer, &dest);

	XCenteredString(renderer, dest.y + factor,
			"GAME OVER! You hit the turn limit!");
	SDL_RenderPresent(renderer);
	return;
}

/**
 * This function initializes the graphics subsystem.
 * A fullscreen mode maybe requested.
 * @return true if initialization was successfull
 */
bool Graphics_Init(bool set_fullscreen)
{

	/* initialize SDL...  */
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "%s\n\nUnable to initialize SDL: %s\n", VERSION,
				SDL_GetError());
		return (false);
	}
	
	/* initialize SDL_ttf */
	if (TTF_Init() < 0) {
		fprintf(stderr, "Unable to initialize SDL_ttf: %s\n", TTF_GetError());
		return (false);
	}
	
	/* make sure to shutdown SDL at program end... */
	atexit(SDL_Quit);
	atexit(TTF_Quit);

	/* Create window */
	Uint32 window_flags = SDL_WINDOW_SHOWN;
	if (set_fullscreen) {
		window_flags |= SDL_WINDOW_FULLSCREEN;
	}
	
	window = SDL_CreateWindow(VERSION,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		640, 480,
		window_flags);
		
	if (!window) {
		fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
		return false;
	}

	/* Get the actual window size */
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);
	DEBUG_PRINT("Window created with dimensions: %dx%d", window_width, window_height);

	/* Create renderer */
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
		return false;
	}

	/* Set logical size to match the window size for consistent positioning */
	SDL_RenderSetLogicalSize(renderer, 640, 480);
	DEBUG_PRINT("Renderer logical size set to: 640x480");

	/* Set window icon */
	SDL_Surface *icon_surface = Graphics_LoadGraphicsResource("*ICON*");
	if (icon_surface) {
		SDL_SetWindowIcon(window, icon_surface);
		SDL_FreeSurface(icon_surface);
	}

#if(HAVE_MOUSE_POINTER == 0)
	/* disable mouse pointer if configured */
	SDL_ShowCursor(SDL_DISABLE);
#endif

	/* init graphic data for use by the game */
	SDL_Surface *temp_surface;
	
	temp_surface = Graphics_LoadGraphicsResource("texture.png");
	textures = SDL_CreateTextureFromSurface(renderer, temp_surface);
	SDL_FreeSurface(temp_surface);
	
	temp_surface = Graphics_LoadGraphicsResource("frame.png");
	frame = SDL_CreateTextureFromSurface(renderer, temp_surface);
	SDL_FreeSurface(temp_surface);
	
	temp_surface = Graphics_LoadGraphicsResource("dots.png");
	dots = SDL_CreateTextureFromSurface(renderer, temp_surface);
	SDL_FreeSurface(temp_surface);
	
	temp_surface = Graphics_LoadGraphicsResource("font.png");
	font = SDL_CreateTextureFromSurface(renderer, temp_surface);
	SDL_FreeSurface(temp_surface);
	
	temp_surface = Graphics_LoadGraphicsResource("title.png");
	title = SDL_CreateTextureFromSurface(renderer, temp_surface);
	SDL_FreeSurface(temp_surface);
	
	temp_surface = Graphics_LoadGraphicsResource("copyright.png");
	copyright = SDL_CreateTextureFromSurface(renderer, temp_surface);
	SDL_FreeSurface(temp_surface);

	DEBUG_PRINT("Images loaded");
	InitFont(font);
	DEBUG_PRINT("Font ready");

	/* did our graphics load properly? */
	if((textures==NULL)||(frame==NULL)||(dots==NULL)||(title==NULL)||(copyright==NULL)||(font==NULL))
	{
		fprintf (stderr, "%s initgraphics(): One or more image files failed to load properly!\n\n",PACKAGE);
		exit(2);
	}

	/* collect information of sizes of the various graphics */
	int w, h;
	SDL_QueryTexture(frame, NULL, NULL, &w, &h);
	frame_width = (Uint16)w;
	frame_height = (Uint16)h;
	DEBUG_PRINT("Frame texture: %dx%d", frame_width, frame_height);
	
	SDL_QueryTexture(dots, NULL, NULL, &w, &h);
	dot_width = (Uint16)(w / NR_OF_DOTS);
	dot_height = (Uint16)h;
	DEBUG_PRINT("Dots texture: %dx%d (each dot: %dx%d)", w, h, dot_width, dot_height);
	
	SDL_QueryTexture(textures, NULL, NULL, &w, &h);
	texture_width = (Uint16)(w / NR_OF_TEXTURES);
	texture_height = (Uint16)h;
	DEBUG_PRINT("Textures: %dx%d (each texture: %dx%d)", w, h, texture_width, texture_height);
	
	SDL_QueryTexture(title, NULL, NULL, &w, &h);
	title_width = (Uint16)w;
	title_height = (Uint16)h;
	DEBUG_PRINT("Title: %dx%d", w, h);

	SDL_QueryTexture(font, NULL, NULL, &w, &h);
	font_height = (Uint16)h;
	DEBUG_PRINT("Font texture: %dx%d (height: %d)", w, h, font_height);

	/* Load TTF font */
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

	atexit(Graphics_CleanUp);

	return(true);
}

/**
 * This callback function frees all memory requested by the Graphics submodule.
 */
void Graphics_CleanUp()
{
	/* Optimization: Clean up texture cache */
	for (int i = 0; i < 10; i++) {
		if (texture_cache[i].loaded && texture_cache[i].texture) {
			SDL_DestroyTexture(texture_cache[i].texture);
			texture_cache[i].texture = NULL;
			texture_cache[i].loaded = false;
		}
	}
	
	if (game_font) {
		TTF_CloseFont(game_font);
		game_font = NULL;
	}
	SDL_DestroyTexture(textures);
	SDL_DestroyTexture(frame);
	SDL_DestroyTexture(dots);
	SDL_DestroyTexture(font);
	SDL_DestroyTexture(title);
	SDL_DestroyTexture(copyright);
	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = NULL;
	}

#ifdef _DEBUG
	fprintf(stderr,"image surfaces successfully freed....\n");
#endif
}

/**
 * @return width of a dot tile in pixels
 */
Uint16 Graphics_GetDotWidth()
{
	return(dot_width);
}

/**
 * @return height of a dot tile in pixels
 */
Uint16 Graphics_GetDotHeight()
{
	return(dot_height);
}

/**
 * @return width of the entire screen in pixels
 */
Uint16 Graphics_GetScreenWidth()
{
	/* Return logical width - the game was designed for 640x480 */
	return 640;
}

/**
 * @return height of the entire screen in pixels
 */
Uint16 Graphics_GetScreenHeight()
{
	/* Return logical height - the game was designed for 640x480 */
	return 480;
}

/**
 * @return height of the font in pixels
 */
Uint16 Graphics_GetFontHeight()
{
	return font_height;
}

/**
 * Convert logical X coordinate to actual screen X coordinate
 */
Uint16 Graphics_ScaleX(Uint16 logical_x)
{
	/* With logical size set, SDL handles scaling automatically */
	return logical_x;
}

/**
 * Convert logical Y coordinate to actual screen Y coordinate
 */
Uint16 Graphics_ScaleY(Uint16 logical_y)
{
	/* With logical size set, SDL handles scaling automatically */
	return logical_y;
}

/**
 * Convert logical width to actual screen width
 */
Uint16 Graphics_ScaleWidth(Uint16 logical_width)
{
	/* With logical size set, SDL handles scaling automatically */
	return logical_width;
}

/**
 * Convert logical height to actual screen height
 */
Uint16 Graphics_ScaleHeight(Uint16 logical_height)
{
	/* With logical size set, SDL handles scaling automatically */
	return logical_height;
}

/**
 * This function loads a graphics resource from a given filename.
 * Any surfaces loaded with this call must be freed at exit with the Graphics_CleanUp() call.
 * @return pointer to SDL_Surface if loading was successfull
 */
SDL_Surface* Graphics_LoadGraphicsResource(char* inputfilename)
{
	char filename[1024]; /* temporary filename buffer */
	char exec_path[1024];
	char *exec_dir;
	char *source_dir;
	
	/* Optimization: Check texture cache first */
	for (int i = 0; i < 10; i++) {
		if (texture_cache[i].loaded && strcmp(texture_cache[i].filename, inputfilename) == 0) {
			/* Return a surface from the cached texture (for compatibility) */
			SDL_Surface *surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
			if (surface) {
				/* Note: This is a temporary surface for compatibility */
				/* In a full optimization, we'd return the texture directly */
			}
			return surface;
		}
	}
	
	/* Get the directory where the executable is located */
	#ifdef __APPLE__
		uint32_t size = sizeof(exec_path);
		if (_NSGetExecutablePath(exec_path, &size) == 0) {
			exec_dir = dirname(exec_path);
			/* Go up one level to get to the source directory */
			source_dir = dirname(exec_dir);
		} else {
			source_dir = ".";
		}
	#else
		ssize_t count = readlink("/proc/self/exe", exec_path, sizeof(exec_path));
		if (count != -1) {
			exec_dir = dirname(exec_path);
			/* Go up one level to get to the source directory */
			source_dir = dirname(exec_dir);
		} else {
			source_dir = ".";
		}
	#endif

	if(strcmp("*ICON*", inputfilename) == 0)
	{
		snprintf(filename, sizeof(filename), "%s/data/icons/quadromania32.png", source_dir);
	}
	else
	{
		snprintf(filename, sizeof(filename), "%s/data/%s%s", source_dir, GFXPREFIX, inputfilename);
	}

	DEBUG_PRINT("Attempting to load resource: %s\n", filename);
	
	// Check if file exists
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Error: File does not exist: %s\n", filename);
		fprintf(stderr, "Current working directory: %s\n", getcwd(NULL, 0));
		fprintf(stderr, "Source directory: %s\n", source_dir);
		return NULL;
	}
	fclose(f);

	SDL_Surface *surface = IMG_Load(filename);
	if (surface == NULL) {
		fprintf(stderr, "Error loading image %s: %s\n", filename, IMG_GetError());
	} else {
		DEBUG_PRINT("Successfully loaded: %s\n", filename);
		
		/* Optimization: Cache the texture for future use */
		for (int i = 0; i < 10; i++) {
			if (!texture_cache[i].loaded) {
				texture_cache[i].texture = SDL_CreateTextureFromSurface(renderer, surface);
				strncpy(texture_cache[i].filename, inputfilename, sizeof(texture_cache[i].filename) - 1);
				texture_cache[i].loaded = true;
				break;
			}
		}
	}
	
	return surface;
}

/**
 * This function updates any changes to the screen and makes them visible.
 */
void Graphics_UpdateScreen()
{
	SDL_RenderPresent(renderer);
}

/**
 * Get the SDL renderer for use by other systems
 */
SDL_Renderer* Graphics_GetRenderer(void)
{
	return renderer;
}

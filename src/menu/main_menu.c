/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: main_menu.c - Main menu implementation using menu manager
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
#include <stdlib.h>
#include <string.h>
#include "menu/main_menu.h"
#include "input/menu_input.h"
#include "graphics/renderer.h"
#include "audio/sound.h"
#include "core/state_manager.h"
#include "core/game.h"
#include "data/highscore.h"
#include "utils/logger.h"

// Forward declarations for callback functions
static void start_game_callback(void* data);
static void change_colors_callback(void* data);
static void change_rotations_callback(void* data);
static void instructions_callback(void* data);
static void highscores_callback(void* data);
static void quit_callback(void* data);

/**
 * Create and initialize the main menu
 */
MainMenu* MainMenu_Create(game_state_context_t* context)
{
    if (!context) {
        LOG_ERROR("Cannot create main menu: invalid context");
        return NULL;
    }
    
    MainMenu* main_menu = (MainMenu*)malloc(sizeof(MainMenu));
    if (!main_menu) {
        LOG_ERROR("Failed to allocate main menu");
        return NULL;
    }
    
    memset(main_menu, 0, sizeof(MainMenu));
    main_menu->game_context = context;
    main_menu->nr_of_dots = context->max_rotations + 1;
    main_menu->selected_level = context->level;
    main_menu->show_debug = false;
    
    // Create menu manager
    main_menu->menu = MenuManager_Create();
    if (!main_menu->menu) {
        LOG_ERROR("Failed to create menu manager");
        free(main_menu);
        return NULL;
    }
    
    // Set menu position
    MenuManager_SetPosition(main_menu->menu, 48, 200);
    MenuManager_SetSpacing(main_menu->menu, 10);
    
    // Add menu items with callbacks
    MenuManager_AddItem(main_menu->menu, "Start the game", start_game_callback, main_menu);
    MenuManager_AddItem(main_menu->menu, "Select colors", change_colors_callback, main_menu);
    MenuManager_AddItem(main_menu->menu, "Select initial turns", change_rotations_callback, main_menu);
    MenuManager_AddItem(main_menu->menu, "Highscores", highscores_callback, main_menu);
    MenuManager_AddItem(main_menu->menu, "Instructions", instructions_callback, main_menu);
    MenuManager_AddItem(main_menu->menu, "Quit", quit_callback, main_menu);
    
    // Debug: Print all menu item positions
    DEBUG_PRINT("Main menu created with %d items:", MenuManager_GetItemCount(main_menu->menu));
    for (int i = 0; i < MenuManager_GetItemCount(main_menu->menu); i++) {
        const MenuItem* item = MenuManager_GetItem(main_menu->menu, i);
        if (item) {
            DEBUG_PRINT("  Item %d: '%s' at %d,%d (size: %dx%d)", 
                       i, item->text, item->x, item->y, item->width, item->height);
        }
    }
    
    LOG_INFO("Main menu created successfully with %d items", MenuManager_GetItemCount(main_menu->menu));
    return main_menu;
}

/**
 * Destroy the main menu and free resources
 */
void MainMenu_Destroy(MainMenu* main_menu)
{
    if (!main_menu) return;
    
    if (main_menu->menu) {
        MenuManager_Destroy(main_menu->menu);
    }
    
    free(main_menu);
    LOG_INFO("Main menu destroyed");
}

/**
 * Update the main menu (handle input, update state)
 */
void MainMenu_Update(MainMenu* main_menu)
{
    if (!main_menu) return;
    
    // Update menu items if game state changed
    if (main_menu->nr_of_dots != main_menu->game_context->max_rotations + 1 ||
        main_menu->selected_level != main_menu->game_context->level) {
        MainMenu_UpdateItems(main_menu);
    }
}

/**
 * Render the main menu
 */
void MainMenu_Render(MainMenu* main_menu)
{
    if (!main_menu) return;
    
    // Draw background and title
    Graphics_DrawBackground(9);
    Graphics_DrawOuterFrame();
    Graphics_DrawTitle();
    
    // Draw colored dots for color selection
    Uint8 i;
    for (i = 0; i < main_menu->nr_of_dots; ++i) {
        Graphics_DrawDot(((Graphics_GetScreenWidth() * 450) / 640) + i * Graphics_GetDotWidth(),
                        main_menu->menu->items[1].y, i);  // Draw dots next to "Select colors"
    }
    
    // Draw rotation count
    char nstr[20];
    sprintf(nstr, "%d", Quadromania_GetRotationsPerLevel(main_menu->selected_level));
    Graphics_DrawText(((Graphics_GetScreenWidth() * 480) / 640),
                     main_menu->menu->items[2].y, nstr);  // Draw count next to "Select initial turns"
    
    // Render menu items
    MenuManager_Render(main_menu->menu);
    
    // Render debug information if enabled
    if (main_menu->show_debug) {
        MenuManager_RenderDebug(main_menu->menu);
    }
    
    Graphics_UpdateScreen();
}

/**
 * Handle SDL events for the main menu
 */
MenuEvent MainMenu_HandleEvent(MainMenu* main_menu, const SDL_Event* event)
{
    if (!main_menu || !event) {
        MenuEvent empty_event = {MENU_EVENT_NONE, -1, 0, 0, NULL};
        return empty_event;
    }
    
    return MenuInput_ProcessEvent(event, main_menu->menu);
}

/**
 * Handle mouse click directly (for use with old event system)
 */
MenuEvent MainMenu_HandleClick(MainMenu* main_menu, Uint16 mouse_x, Uint16 mouse_y)
{
    if (!main_menu) {
        MenuEvent empty_event = {MENU_EVENT_NONE, -1, 0, 0, NULL};
        return empty_event;
    }
    
    DEBUG_PRINT("MainMenu_HandleClick: coordinates %d,%d", mouse_x, mouse_y);
    return MenuManager_HandleClick(main_menu->menu, mouse_x, mouse_y);
}

/**
 * Update menu items based on game state
 */
void MainMenu_UpdateItems(MainMenu* main_menu)
{
    if (!main_menu) return;
    
    main_menu->nr_of_dots = main_menu->game_context->max_rotations + 1;
    main_menu->selected_level = main_menu->game_context->level;
    
    DEBUG_PRINT("Updated main menu: dots=%d, level=%d", main_menu->nr_of_dots, main_menu->selected_level);
}

/**
 * Toggle debug rendering
 */
void MainMenu_ToggleDebug(MainMenu* main_menu)
{
    if (!main_menu) return;
    
    main_menu->show_debug = !main_menu->show_debug;
    DEBUG_PRINT("Debug rendering %s", main_menu->show_debug ? "enabled" : "disabled");
}

// Callback functions for menu items

static void start_game_callback(void* data)
{
    MainMenu* main_menu = (MainMenu*)data;
    if (!main_menu) return;
    
    LOG_INFO("Starting game from main menu");
    Sound_PlayEffect(SOUND_MENU);
    
    state_manager_transition_to(main_menu->game_context, GAME_STATE_GAME);
    Quadromania_InitPlayfield(
        Quadromania_GetRotationsPerLevel(main_menu->game_context->level),
        main_menu->game_context->max_rotations);
    Quadromania_DrawPlayfield();
    Graphics_UpdateScreen();
}

static void change_colors_callback(void* data)
{
    MainMenu* main_menu = (MainMenu*)data;
    if (!main_menu) return;
    
    LOG_INFO("Changing colors from main menu");
    Sound_PlayEffect(SOUND_MENU);
    
    state_manager_transition_to(main_menu->game_context, GAME_STATE_SETUP_CHANGED);
    ++main_menu->game_context->max_rotations;
    if (main_menu->game_context->max_rotations > 4) {
        main_menu->game_context->max_rotations = 1;
    }
}

static void change_rotations_callback(void* data)
{
    MainMenu* main_menu = (MainMenu*)data;
    if (!main_menu) return;
    
    LOG_INFO("Changing rotations from main menu");
    Sound_PlayEffect(SOUND_MENU);
    
    state_manager_transition_to(main_menu->game_context, GAME_STATE_SETUP_CHANGED);
    ++main_menu->game_context->level;
    if (main_menu->game_context->level > HIGHSCORE_NR_OF_TABLES) {
        main_menu->game_context->level = 1;
    }
}

static void instructions_callback(void* data)
{
    MainMenu* main_menu = (MainMenu*)data;
    if (!main_menu) return;
    
    LOG_INFO("Showing instructions from main menu");
    Sound_PlayEffect(SOUND_MENU);
    
    state_manager_transition_to(main_menu->game_context, GAME_STATE_INSTRUCTIONS);
}

static void highscores_callback(void* data)
{
    MainMenu* main_menu = (MainMenu*)data;
    if (!main_menu) return;
    
    LOG_INFO("Showing highscores from main menu");
    Sound_PlayEffect(SOUND_MENU);
    
    state_manager_transition_to(main_menu->game_context, GAME_STATE_SHOW_HIGHSCORES);
}

static void quit_callback(void* data)
{
    MainMenu* main_menu = (MainMenu*)data;
    if (!main_menu) return;
    
    LOG_INFO("Quitting from main menu");
    Sound_PlayEffect(SOUND_MENU);
    
    state_manager_transition_to(main_menu->game_context, GAME_STATE_QUIT);
} 
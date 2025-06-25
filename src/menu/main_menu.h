/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: main_menu.h - Main menu implementation using menu manager
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

#ifndef __MAIN_MENU_H
#define __MAIN_MENU_H

#include "menu/menu_manager.h"
#include "core/state_manager.h"
#include "core/game.h"

/**
 * Main menu structure containing game state and menu manager
 */
typedef struct {
    MenuManager* menu;
    game_state_context_t* game_context;
    Uint8 nr_of_dots;
    Uint8 selected_level;
    bool show_debug;
} MainMenu;

/**
 * Create and initialize the main menu
 */
MainMenu* MainMenu_Create(game_state_context_t* context);

/**
 * Destroy the main menu and free resources
 */
void MainMenu_Destroy(MainMenu* main_menu);

/**
 * Update the main menu (handle input, update state)
 */
void MainMenu_Update(MainMenu* main_menu);

/**
 * Render the main menu
 */
void MainMenu_Render(MainMenu* main_menu);

/**
 * Handle SDL events for the main menu
 */
MenuEvent MainMenu_HandleEvent(MainMenu* main_menu, const SDL_Event* event);

/**
 * Handle mouse click directly (for use with old event system)
 */
MenuEvent MainMenu_HandleClick(MainMenu* main_menu, Uint16 mouse_x, Uint16 mouse_y);

/**
 * Update menu items based on game state
 */
void MainMenu_UpdateItems(MainMenu* main_menu);

/**
 * Toggle debug rendering
 */
void MainMenu_ToggleDebug(MainMenu* main_menu);

#endif /* __MAIN_MENU_H */ 
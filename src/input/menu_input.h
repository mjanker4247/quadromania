/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: menu_input.h - Menu input event handling
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

#ifndef __MENU_INPUT_H
#define __MENU_INPUT_H

#include <SDL2/SDL.h>
#include "menu/menu_manager.h"

/**
 * Process SDL events for menu input
 */
MenuEvent MenuInput_ProcessEvent(const SDL_Event* event, MenuManager* menu);

/**
 * Check if an SDL event is a menu click
 */
bool MenuInput_IsMenuClick(const SDL_Event* event);

/**
 * Check if an SDL event is a menu hover
 */
bool MenuInput_IsMenuHover(const SDL_Event* event);

/**
 * Convert SDL mouse coordinates to logical coordinates
 */
void MenuInput_ConvertCoordinates(int window_x, int window_y, int* logical_x, int* logical_y);

#endif /* __MENU_INPUT_H */ 
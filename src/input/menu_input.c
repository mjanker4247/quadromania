/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: menu_input.c - Menu input event handling implementation
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
#include "input/menu_input.h"
#include "graphics/renderer.h"
#include "utils/logger.h"

/**
 * Process SDL events for menu input
 */
MenuEvent MenuInput_ProcessEvent(const SDL_Event* event, MenuManager* menu)
{
    MenuEvent menu_event = {MENU_EVENT_NONE, -1, 0, 0, NULL};
    
    if (!event || !menu) {
        return menu_event;
    }
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                int logical_x, logical_y;
                MenuInput_ConvertCoordinates(event->button.x, event->button.y, &logical_x, &logical_y);
                menu_event = MenuManager_HandleClick(menu, logical_x, logical_y);
            }
            break;
            
        case SDL_MOUSEMOTION:
            if (MenuInput_IsMenuHover(event)) {
                int logical_x, logical_y;
                MenuInput_ConvertCoordinates(event->motion.x, event->motion.y, &logical_x, &logical_y);
                menu_event = MenuManager_HandleHover(menu, logical_x, logical_y);
            }
            break;
            
        default:
            break;
    }
    
    return menu_event;
}

/**
 * Check if an SDL event is a menu click
 */
bool MenuInput_IsMenuClick(const SDL_Event* event)
{
    return event && event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT;
}

/**
 * Check if an SDL event is a menu hover
 */
bool MenuInput_IsMenuHover(const SDL_Event* event)
{
    return event && event->type == SDL_MOUSEMOTION;
}

/**
 * Convert SDL mouse coordinates to logical coordinates
 */
void MenuInput_ConvertCoordinates(int window_x, int window_y, int* logical_x, int* logical_y)
{
    if (!logical_x || !logical_y) return;
    
    Graphics_WindowToLogical(window_x, window_y, logical_x, logical_y);
    
    DEBUG_PRINT("Coordinate conversion: window(%d,%d) -> logical(%d,%d)", 
                window_x, window_y, *logical_x, *logical_y);
} 
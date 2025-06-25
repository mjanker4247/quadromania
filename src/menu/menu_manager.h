/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: menu_manager.h - Modular menu management system
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

#ifndef __MENU_MANAGER_H
#define __MENU_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define MAX_MENU_ENTRIES 20

/**
 * Menu item structure containing text, position, and callback
 */
typedef struct {
    const char* text;
    Uint16 x, y;
    Uint16 width, height;
    bool is_clickable;
    bool is_visible;
    void (*callback)(void* context);
    void* callback_data;
} MenuItem;

/**
 * Menu manager structure containing all menu items and state
 */
typedef struct {
    MenuItem items[MAX_MENU_ENTRIES];
    int item_count;
    bool is_visible;
    void* context;
    Uint16 x, y;  // Base position for the menu
    Uint16 spacing;  // Spacing between items
} MenuManager;

/**
 * Menu event types for input handling
 */
typedef enum {
    MENU_EVENT_NONE,
    MENU_EVENT_CLICK,
    MENU_EVENT_HOVER,
    MENU_EVENT_LEAVE
} MenuEventType;

/**
 * Menu event structure for input events
 */
typedef struct {
    MenuEventType type;
    int item_index;
    Uint16 x, y;
    const char* item_text;
} MenuEvent;

/**
 * Core menu manager functions
 */
MenuManager* MenuManager_Create(void);
void MenuManager_Destroy(MenuManager* manager);
void MenuManager_AddItem(MenuManager* manager, const char* text, void (*callback)(void*), void* data);
void MenuManager_SetPosition(MenuManager* manager, Uint16 x, Uint16 y);
void MenuManager_SetSpacing(MenuManager* manager, Uint16 spacing);
void MenuManager_Show(MenuManager* manager);
void MenuManager_Hide(MenuManager* manager);
bool MenuManager_IsVisible(MenuManager* manager);

/**
 * Input handling functions
 */
MenuEvent MenuManager_HandleClick(MenuManager* manager, Uint16 mouse_x, Uint16 mouse_y);
MenuEvent MenuManager_HandleHover(MenuManager* manager, Uint16 mouse_x, Uint16 mouse_y);

/**
 * Rendering functions
 */
void MenuManager_Render(MenuManager* manager);
void MenuManager_RenderDebug(MenuManager* manager);  // Debug rendering for click areas

/**
 * Utility functions
 */
int MenuManager_GetItemCount(MenuManager* manager);
const MenuItem* MenuManager_GetItem(MenuManager* manager, int index);
void MenuManager_Clear(MenuManager* manager);

#endif /* __MENU_MANAGER_H */ 
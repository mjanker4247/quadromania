/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: menu_manager.c - Modular menu management system implementation
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
#include "menu/menu_manager.h"
#include "graphics/renderer.h"
#include "graphics/ttf_font.h"
#include "utils/logger.h"

/**
 * Create a new menu manager
 */
MenuManager* MenuManager_Create(void)
{
    MenuManager* manager = (MenuManager*)malloc(sizeof(MenuManager));
    if (!manager) {
        LOG_ERROR("Failed to allocate menu manager");
        return NULL;
    }
    
    memset(manager, 0, sizeof(MenuManager));
    manager->is_visible = true;
    manager->spacing = 10;
    manager->x = 48;  // Default position
    manager->y = 200;
    
    LOG_INFO("Menu manager created successfully");
    return manager;
}

/**
 * Destroy a menu manager and free all resources
 */
void MenuManager_Destroy(MenuManager* manager)
{
    if (!manager) return;
    
    MenuManager_Clear(manager);
    free(manager);
    LOG_INFO("Menu manager destroyed");
}

/**
 * Add a menu item to the manager
 */
void MenuManager_AddItem(MenuManager* manager, const char* text, void (*callback)(void*), void* data)
{
    if (!manager || !text || manager->item_count >= MAX_MENU_ENTRIES) {
        LOG_ERROR("Cannot add menu item: invalid parameters or full menu");
        return;
    }
    
    MenuItem* item = &manager->items[manager->item_count];
    item->text = text;
    item->callback = callback;
    item->callback_data = data;
    item->is_clickable = true;
    item->is_visible = true;
    
    // Calculate position based on base position and item count
    item->x = manager->x;
    item->y = manager->y + (manager->item_count * (Graphics_GetFontHeight() + manager->spacing));
    
    // Calculate width and height based on text
    item->width = TTF_Font_GetTextWidth((char*)text);
    item->height = Graphics_GetFontHeight();
    
    manager->item_count++;
    
    DEBUG_PRINT("Added menu item '%s' at position %d,%d (size: %dx%d)", 
                text, item->x, item->y, item->width, item->height);
}

/**
 * Set the base position for the menu
 */
void MenuManager_SetPosition(MenuManager* manager, Uint16 x, Uint16 y)
{
    if (!manager) return;
    
    manager->x = x;
    manager->y = y;
    
    // Update all item positions
    for (int i = 0; i < manager->item_count; i++) {
        MenuItem* item = &manager->items[i];
        item->x = x;
        item->y = y + (i * (Graphics_GetFontHeight() + manager->spacing));
    }
    
    DEBUG_PRINT("Menu position set to %d,%d", x, y);
}

/**
 * Set spacing between menu items
 */
void MenuManager_SetSpacing(MenuManager* manager, Uint16 spacing)
{
    if (!manager) return;
    
    manager->spacing = spacing;
    
    // Update all item positions with new spacing
    for (int i = 0; i < manager->item_count; i++) {
        MenuItem* item = &manager->items[i];
        item->y = manager->y + (i * (Graphics_GetFontHeight() + spacing));
    }
    
    DEBUG_PRINT("Menu spacing set to %d", spacing);
}

/**
 * Show the menu
 */
void MenuManager_Show(MenuManager* manager)
{
    if (!manager) return;
    manager->is_visible = true;
    DEBUG_PRINT("Menu shown");
}

/**
 * Hide the menu
 */
void MenuManager_Hide(MenuManager* manager)
{
    if (!manager) return;
    manager->is_visible = false;
    DEBUG_PRINT("Menu hidden");
}

/**
 * Check if menu is visible
 */
bool MenuManager_IsVisible(MenuManager* manager)
{
    return manager && manager->is_visible;
}

/**
 * Handle mouse click on menu
 */
MenuEvent MenuManager_HandleClick(MenuManager* manager, Uint16 mouse_x, Uint16 mouse_y)
{
    MenuEvent event = {MENU_EVENT_NONE, -1, mouse_x, mouse_y, NULL};
    
    if (!manager || !manager->is_visible) {
        DEBUG_PRINT("Menu click ignored: manager=%p, visible=%s", 
                   manager, manager ? (manager->is_visible ? "true" : "false") : "NULL");
        return event;
    }
    
    DEBUG_PRINT("Processing menu click at %d,%d (menu has %d items)", mouse_x, mouse_y, manager->item_count);
    
    for (int i = 0; i < manager->item_count; i++) {
        MenuItem* item = &manager->items[i];
        
        if (!item->is_visible || !item->is_clickable) {
            DEBUG_PRINT("Skipping item %d: visible=%s, clickable=%s", 
                       i, item->is_visible ? "true" : "false", item->is_clickable ? "true" : "false");
            continue;
        }
        
        DEBUG_PRINT("Checking item %d: '%s' at %d,%d (size: %dx%d)", 
                   i, item->text, item->x, item->y, item->width, item->height);
        
        // Check if click is within item bounds
        if (mouse_x >= item->x && mouse_x <= item->x + item->width &&
            mouse_y >= item->y && mouse_y <= item->y + item->height) {
            
            event.type = MENU_EVENT_CLICK;
            event.item_index = i;
            event.item_text = item->text;
            
            DEBUG_PRINT("Menu click detected on item %d: '%s'", i, item->text);
            
            // Execute callback if available
            if (item->callback) {
                DEBUG_PRINT("Executing callback for item %d", i);
                item->callback(item->callback_data);
            } else {
                DEBUG_PRINT("No callback for item %d", i);
            }
            
            break;
        } else {
            DEBUG_PRINT("Click outside item %d bounds: mouse(%d,%d) not in (%d,%d,%d,%d)", 
                       i, mouse_x, mouse_y, item->x, item->y, item->x + item->width, item->y + item->height);
        }
    }
    
    if (event.type == MENU_EVENT_NONE) {
        DEBUG_PRINT("No menu item clicked");
    }
    
    return event;
}

/**
 * Handle mouse hover on menu
 */
MenuEvent MenuManager_HandleHover(MenuManager* manager, Uint16 mouse_x, Uint16 mouse_y)
{
    MenuEvent event = {MENU_EVENT_NONE, -1, mouse_x, mouse_y, NULL};
    
    if (!manager || !manager->is_visible) {
        return event;
    }
    
    for (int i = 0; i < manager->item_count; i++) {
        MenuItem* item = &manager->items[i];
        
        if (!item->is_visible) continue;
        
        // Check if hover is within item bounds
        if (mouse_x >= item->x && mouse_x <= item->x + item->width &&
            mouse_y >= item->y && mouse_y <= item->y + item->height) {
            
            event.type = MENU_EVENT_HOVER;
            event.item_index = i;
            event.item_text = item->text;
            break;
        }
    }
    
    return event;
}

/**
 * Render the menu
 */
void MenuManager_Render(MenuManager* manager)
{
    if (!manager || !manager->is_visible) return;
    
    for (int i = 0; i < manager->item_count; i++) {
        MenuItem* item = &manager->items[i];
        
        if (!item->is_visible) continue;
        
        Graphics_DrawText(item->x, item->y, (char*)item->text);
    }
}

/**
 * Render debug information (click areas)
 */
void MenuManager_RenderDebug(MenuManager* manager)
{
    if (!manager || !manager->is_visible) return;
    
    for (int i = 0; i < manager->item_count; i++) {
        MenuItem* item = &manager->items[i];
        
        if (!item->is_visible) continue;
        
        // Draw debug rectangle around click area
        SDL_Rect debug_rect = {
            item->x - 2, item->y - 2,
            item->width + 4, item->height + 4
        };
        
        // Set debug color (semi-transparent red)
        SDL_SetRenderDrawBlendMode(Graphics_GetRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Graphics_GetRenderer(), 255, 0, 0, 128);
        SDL_RenderFillRect(Graphics_GetRenderer(), &debug_rect);
        SDL_SetRenderDrawBlendMode(Graphics_GetRenderer(), SDL_BLENDMODE_NONE);
    }
}

/**
 * Get the number of items in the menu
 */
int MenuManager_GetItemCount(MenuManager* manager)
{
    return manager ? manager->item_count : 0;
}

/**
 * Get a specific menu item
 */
const MenuItem* MenuManager_GetItem(MenuManager* manager, int index)
{
    if (!manager || index < 0 || index >= manager->item_count) {
        return NULL;
    }
    return &manager->items[index];
}

/**
 * Clear all menu items
 */
void MenuManager_Clear(MenuManager* manager)
{
    if (!manager) return;
    
    memset(manager->items, 0, sizeof(manager->items));
    manager->item_count = 0;
    
    DEBUG_PRINT("Menu cleared");
} 
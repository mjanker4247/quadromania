/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: keyboard_handler.h - Platform-agnostic keyboard input handling
 * last Modified: 2024-12-19
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

#ifndef __KEYBOARD_HANDLER_H
#define __KEYBOARD_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include "input_manager.h"

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * Keyboard state structure
 */
typedef struct
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool button;
    bool escape;
    bool quit;
} KeyboardState;

/**
 * Key mapping configuration
 */
typedef struct
{
    uint32_t up_key;
    uint32_t down_key;
    uint32_t left_key;
    uint32_t right_key;
    uint32_t button_key;
    uint32_t escape_key;
    uint32_t quit_key;
    uint32_t volume_up_key;
    uint32_t volume_down_key;
} KeyMapping;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the keyboard handler
 * @param mapping Key mapping configuration
 * @return true on success, false on failure
 */
bool KeyboardHandler_Init(const KeyMapping* mapping);

/**
 * Shutdown the keyboard handler
 */
void KeyboardHandler_Shutdown(void);

/**
 * Process keyboard events
 * @param event Input event to process
 * @param dpad_state Pointer to dpad state to update
 * @return true if event was handled
 */
bool KeyboardHandler_ProcessEvent(const InputEvent* event, DpadState* dpad_state);

/**
 * Get current keyboard state
 * @return Pointer to current keyboard state
 */
const KeyboardState* KeyboardHandler_GetState(void);

/**
 * Reset keyboard state
 */
void KeyboardHandler_ResetState(void);

/**
 * Set key mapping
 * @param mapping New key mapping
 */
void KeyboardHandler_SetKeyMapping(const KeyMapping* mapping);

/**
 * Get current key mapping
 * @return Pointer to current key mapping
 */
const KeyMapping* KeyboardHandler_GetKeyMapping(void);

/**
 * Check if a specific key is pressed
 * @param key_code Key code to check
 * @return true if key is pressed
 */
bool KeyboardHandler_IsKeyPressed(uint32_t key_code);

/**
 * Get default key mapping for the current platform
 * @return Default key mapping
 */
KeyMapping KeyboardHandler_GetDefaultMapping(void);

#endif /* __KEYBOARD_HANDLER_H */ 
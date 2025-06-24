/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: mouse_handler.h - Platform-agnostic mouse input handling
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

#ifndef __MOUSE_HANDLER_H
#define __MOUSE_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include "input_manager.h"

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * Mouse button configuration
 */
typedef struct
{
    uint8_t left_button;
    uint8_t right_button;
    uint8_t middle_button;
} MouseButtonConfig;

/**
 * Mouse sensitivity configuration
 */
typedef struct
{
    float x_sensitivity;
    float y_sensitivity;
    bool invert_y;
} MouseSensitivityConfig;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the mouse handler
 * @param button_config Mouse button configuration
 * @param sensitivity_config Mouse sensitivity configuration
 * @return true on success, false on failure
 */
bool MouseHandler_Init(const MouseButtonConfig* button_config, 
                      const MouseSensitivityConfig* sensitivity_config);

/**
 * Shutdown the mouse handler
 */
void MouseHandler_Shutdown(void);

/**
 * Process mouse events
 * @param event Input event to process
 * @param mouse_state Pointer to mouse state to update
 * @return true if event was handled
 */
bool MouseHandler_ProcessEvent(const InputEvent* event, MouseState* mouse_state);

/**
 * Get current mouse state
 * @return Pointer to current mouse state
 */
const MouseState* MouseHandler_GetState(void);

/**
 * Reset mouse state
 */
void MouseHandler_ResetState(void);

/**
 * Set mouse button configuration
 * @param button_config New button configuration
 */
void MouseHandler_SetButtonConfig(const MouseButtonConfig* button_config);

/**
 * Get current mouse button configuration
 * @return Pointer to current button configuration
 */
const MouseButtonConfig* MouseHandler_GetButtonConfig(void);

/**
 * Set mouse sensitivity configuration
 * @param sensitivity_config New sensitivity configuration
 */
void MouseHandler_SetSensitivityConfig(const MouseSensitivityConfig* sensitivity_config);

/**
 * Get current mouse sensitivity configuration
 * @return Pointer to current sensitivity configuration
 */
const MouseSensitivityConfig* MouseHandler_GetSensitivityConfig(void);

/**
 * Get default mouse button configuration for the current platform
 * @return Default button configuration
 */
MouseButtonConfig MouseHandler_GetDefaultButtonConfig(void);

/**
 * Get default mouse sensitivity configuration for the current platform
 * @return Default sensitivity configuration
 */
MouseSensitivityConfig MouseHandler_GetDefaultSensitivityConfig(void);

/**
 * Check if a specific mouse button is pressed
 * @param button Button number to check
 * @return true if button is pressed
 */
bool MouseHandler_IsButtonPressed(uint8_t button);

/**
 * Get mouse position
 * @param x Pointer to store X coordinate
 * @param y Pointer to store Y coordinate
 */
void MouseHandler_GetPosition(uint16_t* x, uint16_t* y);

/**
 * Set mouse position (for relative positioning)
 * @param x X coordinate
 * @param y Y coordinate
 */
void MouseHandler_SetPosition(uint16_t x, uint16_t y);

#endif /* __MOUSE_HANDLER_H */ 
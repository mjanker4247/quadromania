/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: joystick_handler.h - Platform-agnostic joystick input handling
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

#ifndef __JOYSTICK_HANDLER_H
#define __JOYSTICK_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include "input_manager.h"

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * Joystick state structure
 */
typedef struct
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool button;
    bool escape;
} JoystickState;

/**
 * Joystick axis configuration
 */
typedef struct
{
    uint8_t x_axis;
    uint8_t y_axis;
    int16_t deadzone;
    int16_t threshold;
} JoystickAxisConfig;

/**
 * Joystick button configuration
 */
typedef struct
{
    uint8_t up_button;
    uint8_t down_button;
    uint8_t left_button;
    uint8_t right_button;
    uint8_t fire_button;
    uint8_t escape_button;
    uint8_t volume_up_button;
    uint8_t volume_down_button;
} JoystickButtonConfig;

/**
 * Joystick platform types
 */
typedef enum
{
    JOYSTICK_PLATFORM_DEFAULT,
    JOYSTICK_PLATFORM_GP2X,
    JOYSTICK_PLATFORM_ANDROID,
    JOYSTICK_PLATFORM_IOS,
    JOYSTICK_PLATFORM_WINDOWS,
    JOYSTICK_PLATFORM_LINUX,
    JOYSTICK_PLATFORM_MACOS,
    JOYSTICK_PLATFORM_COUNT
} JoystickPlatformType;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the joystick handler
 * @param platform_type Platform type for configuration
 * @param axis_config Axis configuration
 * @param button_config Button configuration
 * @return true on success, false on failure
 */
bool JoystickHandler_Init(JoystickPlatformType platform_type,
                         const JoystickAxisConfig* axis_config,
                         const JoystickButtonConfig* button_config);

/**
 * Shutdown the joystick handler
 */
void JoystickHandler_Shutdown(void);

/**
 * Process joystick events
 * @param event Input event to process
 * @param dpad_state Pointer to dpad state to update
 * @return true if event was handled
 */
bool JoystickHandler_ProcessEvent(const InputEvent* event, DpadState* dpad_state);

/**
 * Get current joystick state
 * @return Pointer to current joystick state
 */
const JoystickState* JoystickHandler_GetState(void);

/**
 * Reset joystick state
 */
void JoystickHandler_ResetState(void);

/**
 * Set joystick axis configuration
 * @param axis_config New axis configuration
 */
void JoystickHandler_SetAxisConfig(const JoystickAxisConfig* axis_config);

/**
 * Get current joystick axis configuration
 * @return Pointer to current axis configuration
 */
const JoystickAxisConfig* JoystickHandler_GetAxisConfig(void);

/**
 * Set joystick button configuration
 * @param button_config New button configuration
 */
void JoystickHandler_SetButtonConfig(const JoystickButtonConfig* button_config);

/**
 * Get current joystick button configuration
 * @return Pointer to current button configuration
 */
const JoystickButtonConfig* JoystickHandler_GetButtonConfig(void);

/**
 * Get default axis configuration for the given platform
 * @param platform_type Platform type
 * @return Default axis configuration
 */
JoystickAxisConfig JoystickHandler_GetDefaultAxisConfig(JoystickPlatformType platform_type);

/**
 * Get default button configuration for the given platform
 * @param platform_type Platform type
 * @return Default button configuration
 */
JoystickButtonConfig JoystickHandler_GetDefaultButtonConfig(JoystickPlatformType platform_type);

/**
 * Check if a specific joystick button is pressed
 * @param button Button number to check
 * @return true if button is pressed
 */
bool JoystickHandler_IsButtonPressed(uint8_t button);

/**
 * Get joystick axis value
 * @param axis Axis number
 * @return Axis value (-32768 to 32767)
 */
int16_t JoystickHandler_GetAxisValue(uint8_t axis);

/**
 * Set joystick platform type
 * @param platform_type New platform type
 */
void JoystickHandler_SetPlatformType(JoystickPlatformType platform_type);

/**
 * Get current joystick platform type
 * @return Current platform type
 */
JoystickPlatformType JoystickHandler_GetPlatformType(void);

/**
 * Detect and set the appropriate platform type automatically
 * @return Detected platform type
 */
JoystickPlatformType JoystickHandler_DetectPlatform(void);

/**
 * Check if joystick is connected
 * @return true if joystick is connected
 */
bool JoystickHandler_IsConnected(void);

/**
 * Get number of connected joysticks
 * @return Number of connected joysticks
 */
int JoystickHandler_GetConnectedCount(void);

#endif /* __JOYSTICK_HANDLER_H */ 
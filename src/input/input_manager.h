/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: input_manager.h - Unified input management interface
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

#ifndef __INPUT_MANAGER_H
#define __INPUT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * This data structure represents the collect input and state of the mouse/stylus input device.
 */
typedef struct
{
    uint16_t x, y;     /** mouse coordinates in pixels */
    uint8_t button;    /** mouse button number */
    bool clicked;      /** mouse clicked? */
} MouseState;

/**
 * This data structure represents the state of the virtual directional pad.
 * Each member describes whether the direction or firebutton is pressed.
 */
typedef struct
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool button;
} DpadState;

/**
 * Input device types for configuration
 */
typedef enum
{
    INPUT_DEVICE_MOUSE,
    INPUT_DEVICE_JOYSTICK,
    INPUT_DEVICE_TOUCH,
    INPUT_DEVICE_COUNT
} InputDeviceType;

/**
 * Input event types
 */
typedef enum
{
    INPUT_EVENT_NONE,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_DOWN,
    INPUT_EVENT_MOUSE_UP,
    INPUT_EVENT_JOYSTICK_AXIS,
    INPUT_EVENT_JOYSTICK_BUTTON_DOWN,
    INPUT_EVENT_JOYSTICK_BUTTON_UP,
    INPUT_EVENT_QUIT,
    INPUT_EVENT_COUNT
} InputEventType;

/**
 * Unified input event structure
 */
typedef struct
{
    InputEventType type;
    union
    {
        struct
        {
            uint16_t x, y;
            uint8_t button;
        } mouse;
        struct
        {
            uint8_t axis;
            int16_t value;
        } joystick_axis;
        struct
        {
            uint8_t button;
        } joystick_button;
    } data;
} InputEvent;

/**
 * Input manager configuration
 */
typedef struct
{
    bool enable_mouse;
    bool enable_joystick;
    bool enable_touch;
    uint8_t debounce_time;
} InputConfig;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize input manager
 * @param config Input configuration
 * @return true on success, false on failure
 */
bool InputManager_Init(const InputConfig* config);

/**
 * Set the renderer for coordinate conversion
 * @param renderer Renderer pointer (platform-specific type)
 */
void InputManager_SetRenderer(void* renderer);

/**
 * Shutdown input manager
 */
void InputManager_Shutdown(void);

/**
 * Process all pending input events
 * This should be called once per frame
 */
void InputManager_ProcessEvents(void);

/**
 * Get the current mouse state
 * @return Pointer to current mouse state
 */
const MouseState* InputManager_GetMouseState(void);

/**
 * Get the current unified directional pad state
 * @return Pointer to current dpad state
 */
const DpadState* InputManager_GetDpadState(void);

/**
 * Check if quit was requested
 * @return true if quit was requested
 */
bool InputManager_IsQuitRequested(void);

/**
 * Debounce mouse input
 */
void InputManager_DebounceMouse(void);

/**
 * Debounce directional pad input
 */
void InputManager_DebounceDpad(void);

/**
 * Check if any directional pad input is pressed
 * @return true if any dpad input is active
 */
bool InputManager_IsDpadPressed(void);

/**
 * Get the next input event from the queue
 * @param event Pointer to store the event
 * @return true if an event was retrieved, false if queue is empty
 */
bool InputManager_GetNextEvent(InputEvent* event);

/**
 * Enable or disable a specific input device
 * @param device_type Type of device to configure
 * @param enabled true to enable, false to disable
 */
void InputManager_SetDeviceEnabled(InputDeviceType device_type, bool enabled);

/**
 * Get whether a specific input device is enabled
 * @param device_type Type of device to check
 * @return true if device is enabled
 */
bool InputManager_IsDeviceEnabled(InputDeviceType device_type);

#endif /* __INPUT_MANAGER_H */ 
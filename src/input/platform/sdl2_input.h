/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sdl2_input.h - SDL2 platform-specific input implementation
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

#ifndef __SDL2_INPUT_H
#define __SDL2_INPUT_H

#include <SDL2/SDL.h>
#include "../../input/platform_input.h"

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * SDL2-specific input event structure
 */
typedef struct
{
    SDL_Event sdl_event;
    InputEvent unified_event;
} SDL2InputEvent;

/**
 * SDL2 joystick device information
 */
typedef struct
{
    SDL_Joystick* joystick;
    char name[256];
    bool enabled;
    int button_count;
    int axis_count;
    int hat_count;
} SDL2JoystickDevice;

/**
 * SDL2 input system state
 */
typedef struct
{
    SDL2JoystickDevice* joysticks;
    int joystick_count;
    bool initialized;
    PlatformInputConfig config;
} SDL2InputState;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize SDL2 input system
 * @param config Platform input configuration
 * @return true on success, false on failure
 */
bool SDL2Input_Init(const PlatformInputConfig* config);

/**
 * Shutdown SDL2 input system
 */
void SDL2Input_Shutdown(void);

/**
 * Process SDL2 events and convert to unified events
 * @param event_buffer Buffer to store converted events
 * @param buffer_size Size of the event buffer
 * @return Number of events processed
 */
int SDL2Input_ProcessEvents(PlatformInputEvent* event_buffer, int buffer_size);

/**
 * Convert SDL2 event to unified event
 * @param sdl_event SDL2 event
 * @param unified_event Pointer to store unified event
 * @return true if conversion was successful
 */
bool SDL2Input_ConvertEvent(const SDL_Event* sdl_event, InputEvent* unified_event);

/**
 * Get SDL2 key code for unified key code
 * @param unified_key_code Unified key code
 * @return SDL2 key code
 */
SDL_Keycode SDL2Input_GetSDLKeyCode(uint32_t unified_key_code);

/**
 * Get unified key code for SDL2 key code
 * @param sdl_key_code SDL2 key code
 * @return Unified key code
 */
uint32_t SDL2Input_GetUnifiedKeyCode(SDL_Keycode sdl_key_code);

/**
 * Get SDL2 input capabilities
 * @return SDL2 input capabilities
 */
PlatformInputCapabilities SDL2Input_GetCapabilities(void);

/**
 * Detect SDL2 platform and return appropriate configuration
 * @return Detected SDL2 input configuration
 */
PlatformInputConfig SDL2Input_DetectPlatform(void);

/**
 * Check if SDL2 supports specific input feature
 * @param feature Feature to check
 * @return true if feature is supported
 */
bool SDL2Input_SupportsFeature(const char* feature);

/**
 * Get SDL2 input device name
 * @param device_type Type of input device
 * @param device_index Device index
 * @return Device name or NULL if not available
 */
const char* SDL2Input_GetDeviceName(InputDeviceType device_type, int device_index);

/**
 * Get SDL2 input device count
 * @param device_type Type of input device
 * @return Number of available devices
 */
int SDL2Input_GetDeviceCount(InputDeviceType device_type);

/**
 * Enable or disable SDL2 input device
 * @param device_type Type of input device
 * @param device_index Device index
 * @param enabled true to enable, false to disable
 * @return true on success, false on failure
 */
bool SDL2Input_SetDeviceEnabled(InputDeviceType device_type, int device_index, bool enabled);

/**
 * Check if SDL2 input device is enabled
 * @param device_type Type of input device
 * @param device_index Device index
 * @return true if device is enabled
 */
bool SDL2Input_IsDeviceEnabled(InputDeviceType device_type, int device_index);

/**
 * Get SDL2 joystick device by index
 * @param index Joystick index
 * @return Pointer to joystick device or NULL if not found
 */
SDL2JoystickDevice* SDL2Input_GetJoystickDevice(int index);

/**
 * Initialize SDL2 joystick devices
 * @return true on success, false on failure
 */
bool SDL2Input_InitJoysticks(void);

/**
 * Open SDL2 joystick by index
 * @param index Joystick index
 * @return true on success, false on failure
 */
bool SDL2Input_OpenJoystick(int index);

/**
 * Shutdown SDL2 joystick devices
 */
void SDL2Input_ShutdownJoysticks(void);

/**
 * Handle SDL2 joystick addition event
 * @param device_index Device index of added joystick
 */
void SDL2Input_HandleJoystickAdded(int device_index);

/**
 * Handle SDL2 joystick removal event
 * @param device_index Device index of removed joystick
 */
void SDL2Input_HandleJoystickRemoved(int device_index);

#endif /* __SDL2_INPUT_H */ 
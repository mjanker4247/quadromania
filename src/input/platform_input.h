/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: platform_input.h - Platform abstraction for input handling
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

#ifndef __PLATFORM_INPUT_H
#define __PLATFORM_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include "input_manager.h"

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * Platform-specific input event structure
 */
typedef struct
{
    uint32_t platform_event_type;
    void* platform_event_data;
    InputEvent unified_event;
} PlatformInputEvent;

/**
 * Platform input capabilities
 */
typedef struct
{
    bool has_mouse;
    bool has_joystick;
    bool has_touch;
    bool has_accelerometer;
    bool has_gyroscope;
    int max_joysticks;
    int max_touch_points;
} PlatformInputCapabilities;

/**
 * Platform input configuration
 */
typedef struct
{
    PlatformInputCapabilities capabilities;
    bool enable_raw_input;
    bool enable_relative_mouse;
    bool enable_touch_emulation;
} PlatformInputConfig;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize platform-specific input system
 * @param config Platform input configuration
 * @return true on success, false on failure
 */
bool PlatformInput_Init(const PlatformInputConfig* config);

/**
 * Set the renderer for coordinate conversion (platform-specific)
 * @param renderer Renderer pointer (platform-specific type)
 */
void PlatformInput_SetRenderer(void* renderer);

/**
 * Shutdown platform-specific input system
 */
void PlatformInput_Shutdown(void);

/**
 * Get platform input capabilities
 * @return Platform input capabilities
 */
PlatformInputCapabilities PlatformInput_GetCapabilities(void);

/**
 * Process platform-specific events and convert to unified events
 * @param event_buffer Buffer to store converted events
 * @param buffer_size Size of the event buffer
 * @return Number of events processed
 */
int PlatformInput_ProcessEvents(PlatformInputEvent* event_buffer, int buffer_size);

/**
 * Convert platform-specific event to unified event
 * @param platform_event Platform-specific event
 * @param unified_event Pointer to store unified event
 * @return true if conversion was successful
 */
bool PlatformInput_ConvertEvent(const void* platform_event, InputEvent* unified_event);

/**
 * Set platform-specific input configuration
 * @param config New platform input configuration
 */
void PlatformInput_SetConfig(const PlatformInputConfig* config);

/**
 * Get current platform input configuration
 * @return Current platform input configuration
 */
PlatformInputConfig PlatformInput_GetConfig(void);

/**
 * Detect current platform and return appropriate configuration
 * @return Detected platform input configuration
 */
PlatformInputConfig PlatformInput_DetectPlatform(void);

/**
 * Check if platform supports specific input feature
 * @param feature Feature to check (e.g., "raw_input", "touch", etc.)
 * @return true if feature is supported
 */
bool PlatformInput_SupportsFeature(const char* feature);

/**
 * Get platform-specific input device name
 * @param device_type Type of input device
 * @param device_index Device index
 * @return Device name or NULL if not available
 */
const char* PlatformInput_GetDeviceName(InputDeviceType device_type, int device_index);

/**
 * Get platform-specific input device count
 * @param device_type Type of input device
 * @return Number of available devices
 */
int PlatformInput_GetDeviceCount(InputDeviceType device_type);

/**
 * Enable or disable platform-specific input device
 * @param device_type Type of input device
 * @param device_index Device index
 * @param enabled true to enable, false to disable
 * @return true on success, false on failure
 */
bool PlatformInput_SetDeviceEnabled(InputDeviceType device_type, int device_index, bool enabled);

/**
 * Check if platform-specific input device is enabled
 * @param device_type Type of input device
 * @param device_index Device index
 * @return true if device is enabled
 */
bool PlatformInput_IsDeviceEnabled(InputDeviceType device_type, int device_index);

#endif /* __PLATFORM_INPUT_H */ 
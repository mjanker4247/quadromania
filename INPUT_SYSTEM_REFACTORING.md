# Input System Refactoring

This document describes the refactored input system that splits the original monolithic `events.c` into separate, modular input handlers with a unified input manager.

## Architecture Overview

The new input system consists of several layers:

### 1. Unified Input Manager (`input_manager.h/c`)
- **Purpose**: Provides a single, clean interface for all input handling
- **Features**: 
  - Coordinates all input handlers
  - Maintains unified input state
  - Handles event queuing and processing
  - Provides device enable/disable functionality
  - Abstracts platform-specific details

### 2. Input Handlers
Each input handler is responsible for a specific input device type:

#### Keyboard Handler (`keyboard_handler.h/c`)
- **Purpose**: Platform-agnostic keyboard input handling
- **Features**:
  - Configurable key mappings
  - Platform-specific default configurations
  - Key state tracking
  - Event processing for keyboard events

#### Mouse Handler (`mouse_handler.h/c`)
- **Purpose**: Platform-agnostic mouse input handling
- **Features**:
  - Configurable button mappings
  - Sensitivity and inversion settings
  - Mouse state tracking
  - Event processing for mouse events

#### Joystick Handler (`joystick_handler.h/c`)
- **Purpose**: Platform-agnostic joystick input handling
- **Features**:
  - Platform-specific configurations (GP2X, Android, iOS, etc.)
  - Configurable axis and button mappings
  - Deadzone and threshold settings
  - Event processing for joystick events

### 3. Platform Abstraction Layer

#### Platform Input (`platform_input.h/c`)
- **Purpose**: Abstract interface for platform-specific input implementations
- **Features**:
  - Platform capability detection
  - Event conversion between platform and unified formats
  - Device management
  - Feature support checking

#### SDL2 Implementation (`platform/sdl2_input.h/c`)
- **Purpose**: SDL2-specific input implementation
- **Features**:
  - SDL2 event processing
  - Joystick device management
  - Key code mapping
  - Platform detection

### 4. Compatibility Layer (`events_compat.h/c`)
- **Purpose**: Maintains backward compatibility with existing code
- **Features**:
  - Implements the old event API using the new input system
  - Allows gradual migration from old to new API
  - Preserves existing function signatures

## Key Benefits

### 1. Modularity
- Each input device type has its own handler
- Easy to add new input devices or modify existing ones
- Clear separation of concerns

### 2. Platform Abstraction
- Platform-specific details are isolated in platform implementations
- Easy to add support for new platforms
- Consistent API across different platforms

### 3. Configuration
- Configurable key mappings, button mappings, and sensitivity settings
- Platform-specific default configurations
- Runtime configuration changes

### 4. Extensibility
- Easy to add new input features (touch, accelerometer, etc.)
- Event queuing system for complex input scenarios
- Support for multiple input devices simultaneously

### 5. Backward Compatibility
- Existing code continues to work without changes
- Gradual migration path to new API
- No breaking changes

## Usage

### For New Code (Recommended)

```c
#include "input/input_manager.h"

// Initialize input manager
InputConfig config = {
    .enable_keyboard = true,
    .enable_mouse = true,
    .enable_joystick = true,
    .enable_touch = false,
    .debounce_time = 20
};

if (!InputManager_Init(&config)) {
    // Handle initialization error
}

// Main game loop
while (!InputManager_IsQuitRequested()) {
    // Process all input events
    InputManager_ProcessEvents();
    
    // Get current input states
    const MouseState* mouse = InputManager_GetMouseState();
    const DpadState* dpad = InputManager_GetDpadState();
    
    // Use input states for game logic
    if (dpad->up) {
        // Handle up input
    }
    
    if (mouse->clicked) {
        // Handle mouse click
    }
    
    // Check for specific events
    InputEvent event;
    while (InputManager_GetNextEvent(&event)) {
        switch (event.type) {
        case INPUT_EVENT_KEY_DOWN:
            // Handle key press
            break;
        case INPUT_EVENT_MOUSE_DOWN:
            // Handle mouse press
            break;
        }
    }
}

// Cleanup
InputManager_Shutdown();
```

### For Existing Code (Compatibility)

```c
#include "input/events_compat.h"

// Initialize (same as before)
Event_Init();

// Main game loop (same as before)
while (!Event_QuitRequested()) {
    Event_ProcessInput();
    
    // All existing functions work the same
    if (Event_GetDpadUp()) {
        // Handle up input
    }
    
    if (Event_MouseClicked()) {
        // Handle mouse click
    }
}
```

## Migration Guide

### Phase 1: Use Compatibility Layer
1. Replace `#include "input/events.h"` with `#include "input/events_compat.h"`
2. No other code changes needed
3. Test that everything works as before

### Phase 2: Gradual Migration
1. Start using the new input manager API for new features
2. Gradually replace old event calls with new API calls
3. Use both APIs simultaneously during transition

### Phase 3: Complete Migration
1. Replace all old event API calls with new input manager API
2. Remove compatibility layer includes
3. Use only the new input system

## Platform Support

### Current Platforms
- **SDL2**: Full support with joystick, keyboard, and mouse
- **GP2X**: Specialized button mappings and configurations
- **Generic**: Default configurations for unknown platforms

### Adding New Platforms
1. Create new platform implementation in `platform/` directory
2. Implement platform-specific event conversion
3. Add platform detection logic
4. Update CMakeLists.txt to include new files

## Configuration

### Keyboard Configuration
```c
KeyMapping mapping = {
    .up_key = 0x26,        // Arrow Up
    .down_key = 0x28,      // Arrow Down
    .left_key = 0x25,      // Arrow Left
    .right_key = 0x27,     // Arrow Right
    .button_key = 0x11,    // Ctrl
    .escape_key = 0x1B,    // Escape
    .quit_key = 0x1B,      // Escape
    .volume_up_key = 0x6B, // Keypad +
    .volume_down_key = 0x6D // Keypad -
};

KeyboardHandler_SetKeyMapping(&mapping);
```

### Mouse Configuration
```c
MouseButtonConfig button_config = {
    .left_button = 1,
    .right_button = 3,
    .middle_button = 2
};

MouseSensitivityConfig sensitivity_config = {
    .x_sensitivity = 1.0f,
    .y_sensitivity = 1.0f,
    .invert_y = false
};

MouseHandler_SetButtonConfig(&button_config);
MouseHandler_SetSensitivityConfig(&sensitivity_config);
```

### Joystick Configuration
```c
JoystickAxisConfig axis_config = {
    .x_axis = 0,
    .y_axis = 1,
    .deadzone = 16000,
    .threshold = 16000
};

JoystickButtonConfig button_config = {
    .up_button = 0,
    .down_button = 1,
    .left_button = 2,
    .right_button = 3,
    .fire_button = 4,
    .escape_button = 5
};

JoystickHandler_SetAxisConfig(&axis_config);
JoystickHandler_SetButtonConfig(&button_config);
```

## File Structure

```
src/input/
├── events.h/c                    # Original event API (deprecated)
├── events_compat.h/c             # Compatibility layer
├── input_manager.h/c             # Unified input manager
├── keyboard_handler.h/c          # Keyboard input handler
├── mouse_handler.h/c             # Mouse input handler
├── joystick_handler.h/c          # Joystick input handler
├── platform_input.h/c            # Platform abstraction
└── platform/
    └── sdl2_input.h/c            # SDL2 platform implementation
```

## Future Enhancements

### Planned Features
- Touch input support
- Accelerometer/gyroscope support
- Haptic feedback
- Input recording and playback
- Custom input mapping UI
- Network input (for multiplayer)

### Potential Platforms
- Android (native)
- iOS (native)
- Web (WebAssembly)
- Console platforms (PlayStation, Xbox, Nintendo)

## Testing

The new input system includes comprehensive testing capabilities:
- Input event validation
- Platform detection testing
- Configuration validation
- Performance benchmarking
- Memory leak detection

## Performance Considerations

- Event queuing system prevents input loss
- Efficient state management
- Minimal memory footprint
- Optimized for real-time applications
- Configurable debouncing

## Troubleshooting

### Common Issues
1. **Input not working**: Check device enable/disable settings
2. **Wrong key mappings**: Verify platform-specific configurations
3. **Performance issues**: Adjust debounce timers and event buffer sizes
4. **Platform detection**: Ensure platform-specific code is properly included

### Debug Features
- Comprehensive logging
- Input state inspection
- Event tracing
- Performance profiling
- Memory usage monitoring 
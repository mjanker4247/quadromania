# State Manager Refactoring

## Overview

This document describes the refactoring of the game state logic from `main.c` into a dedicated state management system. The goal was to separate state-specific logic and implement proper state transitions.

## Changes Made

### 1. Created State Manager Module

#### New Files:

- `src/core/state_manager.h` - Header file defining the state management interface
- `src/core/state_manager.c` - Implementation of the state management system

#### Key Components:

**State Enumeration:**

```c
typedef enum {
    GAME_STATE_UNINITIALIZED,
    GAME_STATE_NONE,
    GAME_STATE_TITLE,
    GAME_STATE_INSTRUCTIONS,
    GAME_STATE_SETUP_CHANGED,
    GAME_STATE_GAME,
    GAME_STATE_WON,
    GAME_STATE_GAMEOVER,
    GAME_STATE_HIGHSCORE_ENTRY,
    GAME_STATE_SHOW_HIGHSCORES,
    GAME_STATE_QUIT
} game_state_t;
```

**State Context Structure:**

```c
typedef struct {
    game_state_t current_state;
    game_state_t previous_state;
    bool state_changed;
    
    /* Game configuration */
    Uint8 max_rotations;
    Uint8 level;
    Uint32 score;
    Uint16 highscore_position;
    
    /* State-specific data */
    char *highscore_entry;
} game_state_context_t;
```

### 2. State-Specific Logic Separation

Each game state now has its own dedicated handler function:

- `state_manager_handle_title()` - Title screen logic
- `state_manager_handle_instructions()` - Instructions screen logic
- `state_manager_handle_game()` - Active game logic
- `state_manager_handle_won()` - Win screen logic
- `state_manager_handle_gameover()` - Game over screen logic
- `state_manager_handle_highscore_entry()` - Highscore entry logic
- `state_manager_handle_show_highscores()` - Highscore display logic

### 3. State Transition Management

**Centralized Transition Logic:**

- `state_manager_transition_to()` - Handles state transitions
- `state_manager_process_transitions()` - Processes input events and determines transitions
- `state_manager_update()` - Updates the current state

**Proper State Change Detection:**

- Tracks previous state to detect state changes
- Only redraws screens when state actually changes
- Maintains state-specific data across transitions

### 4. Refactored Main Loop

**Before (main.c):**

- Large switch statement with all state logic mixed together
- Global variables for game state
- Inline state transition logic
- Difficult to maintain and extend

**After (main.c):**

- Clean main loop using state manager
- Separated concerns: initialization, state management, cleanup
- Modular and extensible design

```c
void MainHandler(game_state_context_t *context)
{
    if (!context) return;
    
    /* Main game loop */
    while (state_manager_process_transitions(context)) {
        state_manager_update(context);
        state_manager_render(context);
        
        /* Small delay to prevent excessive CPU usage */
        SDL_Delay(16); /* ~60 FPS */
    }
}
```

### 5. Updated Build System

- Added state manager sources to `CMakeLists.txt`
- Integrated with existing modular structure
- Maintains proper include paths and dependencies

## Benefits

### 1. **Separation of Concerns**

- Each state has its own dedicated handler
- State logic is isolated and easier to understand
- Clear boundaries between different game states

### 2. **Improved Maintainability**

- Easy to add new states
- State-specific logic is contained
- Reduced complexity in main loop

### 3. **Better State Management**

- Proper state transition tracking
- State change detection for efficient rendering
- Centralized state context management

### 4. **Enhanced Extensibility**

- New states can be added without modifying existing code
- State-specific data is properly managed
- Clear interface for state operations

### 5. **Code Organization**

- Modular design following existing project structure
- Consistent with other subsystems (graphics, audio, input)
- Clear separation between core game logic and state management

## State Flow

```
UNINITIALIZED → TITLE → [GAME/INSTRUCTIONS/HIGHSCORES]
    ↓
TITLE → GAME → [WON/GAMEOVER]
    ↓
WON → HIGHSCORE_ENTRY → SHOW_HIGHSCORES → TITLE
    ↓
GAMEOVER → SHOW_HIGHSCORES → TITLE
    ↓
[Any State] → QUIT (via ESC or quit request)
```

## Usage

The state manager is automatically initialized in `main()` and used throughout the game loop. No changes are required to existing game logic - the refactoring is transparent to the rest of the codebase.

## Future Enhancements

1. **State History** - Track state transition history for debugging
2. **State Validation** - Validate state transitions
3. **State Persistence** - Save/restore game state
4. **State Animation** - Smooth transitions between states
5. **State Events** - Event-driven state changes

## Testing

The refactored code has been tested and:
- ✅ Builds successfully
- ✅ Runs without errors
- ✅ Maintains all original functionality
- ✅ Preserves game state transitions
- ✅ Handles input correctly 
/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: state_manager.h - header file for the game state management module
 * last Modified: 2024
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

#ifndef __STATE_MANAGER_H
#define __STATE_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "graphics/ui.h"

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

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

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the state manager
 * @param context Pointer to the state context to initialize
 */
void state_manager_init(game_state_context_t *context);

/**
 * Clean up the state manager
 * @param context Pointer to the state context to clean up
 */
void state_manager_cleanup(game_state_context_t *context);

/**
 * Process state transitions based on events
 * @param context Pointer to the state context
 * @return true if the game should continue, false if it should quit
 */
bool state_manager_process_transitions(game_state_context_t *context);

/**
 * Update the current state
 * @param context Pointer to the state context
 */
void state_manager_update(game_state_context_t *context);

/**
 * Render the current state
 * @param context Pointer to the state context
 */
void state_manager_render(game_state_context_t *context);

/**
 * Handle title screen state
 * @param context Pointer to the state context
 */
void state_manager_handle_title(game_state_context_t *context);

/**
 * Handle instructions screen state
 * @param context Pointer to the state context
 */
void state_manager_handle_instructions(game_state_context_t *context);

/**
 * Handle game state
 * @param context Pointer to the state context
 */
void state_manager_handle_game(game_state_context_t *context);

/**
 * Handle won state
 * @param context Pointer to the state context
 */
void state_manager_handle_won(game_state_context_t *context);

/**
 * Handle game over state
 * @param context Pointer to the state context
 */
void state_manager_handle_gameover(game_state_context_t *context);

/**
 * Handle highscore entry state
 * @param context Pointer to the state context
 */
void state_manager_handle_highscore_entry(game_state_context_t *context);

/**
 * Handle show highscores state
 * @param context Pointer to the state context
 */
void state_manager_handle_show_highscores(game_state_context_t *context);

/**
 * Transition to a new state
 * @param context Pointer to the state context
 * @param new_state The new state to transition to
 */
void state_manager_transition_to(game_state_context_t *context, game_state_t new_state);

#endif /* __STATE_MANAGER_H */ 
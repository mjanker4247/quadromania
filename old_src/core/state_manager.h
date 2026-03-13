/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: state_manager.h - Simplified game state management
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

#ifndef __STATE_MANAGER_H
#define __STATE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

typedef enum {
    GAME_STATE_TITLE,
    GAME_STATE_INSTRUCTIONS,
    GAME_STATE_GAME,
    GAME_STATE_WON,
    GAME_STATE_GAMEOVER,
    GAME_STATE_HIGHSCORES,
    GAME_STATE_QUIT
} game_state_t;

typedef struct {
    game_state_t current_state;
    
    /* Game configuration */
    uint8_t max_rotations;
    uint8_t level;
    uint32_t score;
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
 * Update the current state and handle input
 * @param context Pointer to the state context
 * @return true if the game should continue, false if it should quit
 */
bool state_manager_update(game_state_context_t *context);

/**
 * Change to a new state
 * @param context Pointer to the state context
 * @param new_state The new state to change to
 */
void state_manager_change_state(game_state_context_t *context, game_state_t new_state);

#endif /* __STATE_MANAGER_H */ 
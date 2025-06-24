/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: state_manager.c - implementation of the game state management module
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

#include "state_manager.h"
#include "core/game.h"
#include "graphics/renderer.h"
#include "graphics/ui.h"
#include "audio/sound.h"
#include "data/highscore.h"
#include "input/events.h"
#include "utils/logger.h"

void state_manager_init(game_state_context_t *context)
{
    if (!context) return;
    
    context->current_state = GAME_STATE_UNINITIALIZED;
    context->previous_state = GAME_STATE_NONE;
    context->state_changed = false;
    
    /* Initialize game configuration */
    context->max_rotations = 1;
    context->level = 1;
    context->score = 0;
    context->highscore_position = 0;
    context->highscore_entry = NULL;
}

void state_manager_cleanup(game_state_context_t *context)
{
    if (!context) return;
    
    if (context->highscore_entry) {
        free(context->highscore_entry);
        context->highscore_entry = NULL;
    }
}

void state_manager_transition_to(game_state_context_t *context, game_state_t new_state)
{
    if (!context) return;
    
    context->previous_state = context->current_state;
    context->current_state = new_state;
    context->state_changed = true;
}

bool state_manager_process_transitions(game_state_context_t *context)
{
    if (!context) return false;
    
    /* Process input events */
    Event_ProcessInput();
    
    /* Handle global events that can cause state transitions */
    if (Event_IsESCPressed()) {
        if (context->current_state == GAME_STATE_GAME || 
            context->current_state == GAME_STATE_SHOW_HIGHSCORES) {
            state_manager_transition_to(context, GAME_STATE_TITLE);
        } else {
            state_manager_transition_to(context, GAME_STATE_QUIT);
        }
        Event_DebounceKeys();
    }
    
    if (Event_QuitRequested()) {
        state_manager_transition_to(context, GAME_STATE_QUIT);
    }
    
    return context->current_state != GAME_STATE_QUIT;
}

void state_manager_update(game_state_context_t *context)
{
    if (!context) return;
    
    switch (context->current_state) {
        case GAME_STATE_TITLE:
        case GAME_STATE_SETUP_CHANGED:
            state_manager_handle_title(context);
            break;
        case GAME_STATE_INSTRUCTIONS:
            state_manager_handle_instructions(context);
            break;
        case GAME_STATE_GAME:
            state_manager_handle_game(context);
            break;
        case GAME_STATE_WON:
            state_manager_handle_won(context);
            break;
        case GAME_STATE_GAMEOVER:
            state_manager_handle_gameover(context);
            break;
        case GAME_STATE_HIGHSCORE_ENTRY:
            state_manager_handle_highscore_entry(context);
            break;
        case GAME_STATE_SHOW_HIGHSCORES:
            state_manager_handle_show_highscores(context);
            break;
        case GAME_STATE_QUIT:
            /* Save highscores before quitting */
            Highscore_SaveTable();
            break;
        default:
            /* Unknown state - go to title screen */
            state_manager_transition_to(context, GAME_STATE_TITLE);
            break;
    }
}

void state_manager_render(game_state_context_t *context)
{
    if (!context) return;
    
    /* State-specific rendering is handled in the update functions */
    /* This function can be used for common rendering tasks if needed */
}

void state_manager_handle_title(game_state_context_t *context)
{
    if (context->state_changed) {
        /* Redraw the title screen */
        GUI_DrawMainmenu(context->max_rotations + 1, context->level);
        if (context->current_state == GAME_STATE_SETUP_CHANGED) {
            state_manager_transition_to(context, GAME_STATE_TITLE);
        }
        context->state_changed = false;
    }
    
    /* Check for menu clicks */
    if (Event_MouseClicked()) {
        if (Event_GetMouseButton() == 1) {
            tGUI_MenuEntries menu = GUI_GetClickedMenuEntry();
            
            switch (menu) {
                case MENU_START_GAME:
                    Sound_PlayEffect(SOUND_MENU);
                    state_manager_transition_to(context, GAME_STATE_GAME);
                    Quadromania_InitPlayfield(
                        Quadromania_GetRotationsPerLevel(context->level),
                        context->max_rotations);
                    Quadromania_DrawPlayfield();
                    Graphics_UpdateScreen();
                    break;
                    
                case MENU_CHANGE_NR_OF_COLORS:
                    Sound_PlayEffect(SOUND_MENU);
                    state_manager_transition_to(context, GAME_STATE_SETUP_CHANGED);
                    ++context->max_rotations;
                    if (context->max_rotations > 4) {
                        context->max_rotations = 1;
                    }
                    break;
                    
                case MENU_CHANGE_NR_OF_ROTATIONS:
                    Sound_PlayEffect(SOUND_MENU);
                    state_manager_transition_to(context, GAME_STATE_SETUP_CHANGED);
                    ++context->level;
                    if (context->level > HIGHSCORE_NR_OF_TABLES) {
                        context->level = 1;
                    }
                    break;
                    
                case MENU_INSTRUCTIONS:
                    Sound_PlayEffect(SOUND_MENU);
                    state_manager_transition_to(context, GAME_STATE_INSTRUCTIONS);
                    break;
                    
                case MENU_HIGHSCORES:
                    Sound_PlayEffect(SOUND_MENU);
                    state_manager_transition_to(context, GAME_STATE_SHOW_HIGHSCORES);
                    break;
                    
                case MENU_QUIT:
                    Sound_PlayEffect(SOUND_MENU);
                    state_manager_transition_to(context, GAME_STATE_QUIT);
                    break;
                    
                default:
                    /* Undefined menu entry */
                    break;
            }
        }
        Event_DebounceMouse();
    }
}

void state_manager_handle_instructions(game_state_context_t *context)
{
    if (context->state_changed) {
        Graphics_DrawInstructions();
        context->state_changed = false;
    }
    
    if (Event_MouseClicked()) {
        if (Event_GetMouseButton() == 1 && 
            Event_GetMouseY() > (SCREEN_HEIGHT - Graphics_GetFontHeight())) {
            Event_DebounceMouse();
            state_manager_transition_to(context, GAME_STATE_TITLE);
        }
    }
}

void state_manager_handle_game(game_state_context_t *context)
{
    if (context->state_changed) {
        context->state_changed = false;
    }
    
    /* Handle mouse clicks for game interaction */
    if (Event_MouseClicked()) {
        if (Event_GetMouseButton() == 1) {
            Uint16 xraster, yraster;
            xraster = (Uint16)((Event_GetMouseX() - Graphics_GetDotWidth()) / Graphics_GetDotWidth());
            yraster = (Uint16)((Event_GetMouseY() - Graphics_GetDotHeight()) / Graphics_GetDotHeight());
            
            DEBUG_PRINT("Click at %d,%d", xraster, yraster);
            
            /* Valid click on playfield? */
            if (xraster > 0 && xraster < 17 && yraster > 0 && yraster < 12) {
                /* Rotate the correct 3x3 part */
                Quadromania_Rotate(xraster, yraster);
                Quadromania_DrawPlayfield();
                Graphics_UpdateScreen();
                
                /* Update score */
                context->score = Quadromania_GetPercentOfSolution();
                DEBUG_PRINT("Score: %d%%", context->score);
                
                /* Play sound effect */
                Sound_PlayEffect(SOUND_TURN);
                
                /* Check for game end conditions */
                if (Quadromania_IsTurnLimithit()) {
                    state_manager_transition_to(context, GAME_STATE_GAMEOVER);
                } else if (Quadromania_IsGameWon()) {
                    state_manager_transition_to(context, GAME_STATE_WON);
                }
            }
        }
        Event_DebounceMouse();
    }
}

void state_manager_handle_won(game_state_context_t *context)
{
    if (context->state_changed) {
        Graphics_DrawWinMessage();
        Sound_PlayEffect(SOUND_WIN);
        context->state_changed = false;
    }
    
    if (Event_MouseClicked()) {
        Event_DebounceMouse();
        state_manager_transition_to(context, GAME_STATE_HIGHSCORE_ENTRY);
    }
}

void state_manager_handle_gameover(game_state_context_t *context)
{
    if (context->state_changed) {
        Graphics_DrawGameoverMessage();
        Sound_PlayEffect(SOUND_LOOSE);
        context->state_changed = false;
    }
    
    if (Event_MouseClicked()) {
        Event_DebounceMouse();
        state_manager_transition_to(context, GAME_STATE_SHOW_HIGHSCORES);
    }
}

void state_manager_handle_highscore_entry(game_state_context_t *context)
{
    context->highscore_position = Highscore_GetPosition(context->level - 1, context->score);
    
    if (context->highscore_position != HIGHSCORE_NO_ENTRY) {
        context->highscore_entry = Highscore_GetNameFromTimestamp();
        Highscore_EnterScore(context->level - 1, context->score, 
                            context->highscore_entry, context->highscore_position);
        DEBUG_PRINT("Highscore: %d, Position %d", context->score, context->highscore_position);
        state_manager_transition_to(context, GAME_STATE_SHOW_HIGHSCORES);
    } else {
        state_manager_transition_to(context, GAME_STATE_TITLE);
    }
}

void state_manager_handle_show_highscores(game_state_context_t *context)
{
    if (context->state_changed) {
        Graphics_ListHighscores(context->level - 1);
        context->state_changed = false;
    }
    
    if (Event_MouseClicked()) {
        Event_DebounceMouse();
        state_manager_transition_to(context, GAME_STATE_TITLE);
    }
} 
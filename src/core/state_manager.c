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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/state_manager.h"
#include "core/game.h"
#include "graphics/renderer.h"
#include "graphics/ui.h"
#include "input/events.h"
#include "audio/sound.h"
#include "data/highscore.h"
#include "menu/main_menu.h"
#include "utils/logger.h"

void state_manager_init(game_state_context_t *context)
{
    if (!context) return;
    
    memset(context, 0, sizeof(game_state_context_t));
    context->current_state = GAME_STATE_TITLE;
    context->state_changed = true;
    context->max_rotations = 1;
    context->level = 1;
    context->score = 0;
    context->highscore_position = HIGHSCORE_NO_ENTRY;
    context->highscore_entry = NULL;
    
    // Create main menu instance
    context->main_menu = MainMenu_Create(context);
    if (!context->main_menu) {
        LOG_ERROR("Failed to create main menu for state manager");
    }
    
    LOG_INFO("State manager initialized");
}

void state_manager_cleanup(game_state_context_t *context)
{
    if (!context) return;
    
    // Destroy main menu
    if (context->main_menu) {
        MainMenu_Destroy(context->main_menu);
        context->main_menu = NULL;
    }
    
    if (context->highscore_entry) {
        free(context->highscore_entry);
        context->highscore_entry = NULL;
    }
    
    LOG_INFO("State manager cleaned up");
}

void state_manager_transition_to(game_state_context_t *context, game_state_t new_state)
{
    if (!context) return;
    
    LOG_DEBUG("State transition: %d -> %d", context->current_state, new_state);
    
    context->previous_state = context->current_state;
    context->current_state = new_state;
    context->state_changed = true;
}

bool state_manager_process_transitions(game_state_context_t *context)
{
    if (!context) return false;
    
    /* Process input events */
    Event_ProcessInput();
    
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
    
    /* For turn-based games, only render when state changes or explicitly requested */
    if (context->state_changed) {
        /* State-specific rendering is handled in the update functions */
        /* This function can be used for common rendering tasks if needed */
        context->state_changed = false;
    }
}

void state_manager_handle_title(game_state_context_t *context)
{
    if (!context || !context->main_menu) return;
    
    if (context->state_changed) {
        /* Redraw the title screen using the new menu system */
        MainMenu_Render(context->main_menu);
        if (context->current_state == GAME_STATE_SETUP_CHANGED) {
            state_manager_transition_to(context, GAME_STATE_TITLE);
        }
        context->state_changed = false;
    }
    
    /* Update menu state */
    MainMenu_Update(context->main_menu);
    
    /* Handle mouse clicks through the new menu system */
    if (Event_MouseClicked()) {
        DEBUG_PRINT("Mouse clicked detected in title state");
        if (Event_GetMouseButton() == 1) {
            DEBUG_PRINT("Left mouse button detected");
            
            Uint16 mouse_x = Event_GetMouseX();
            Uint16 mouse_y = Event_GetMouseY();
            DEBUG_PRINT("Mouse coordinates: %d,%d", mouse_x, mouse_y);
            
            // Handle click directly with menu manager
            MenuEvent menu_event = MainMenu_HandleClick(context->main_menu, mouse_x, mouse_y);
            DEBUG_PRINT("Menu event: type=%d, item=%d, text='%s'", 
                       menu_event.type, menu_event.item_index, 
                       menu_event.item_text ? menu_event.item_text : "NULL");
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
                
                /* For turn-based games, only redraw if something changed */
                if (Quadromania_NeedsRedraw()) {
                    Quadromania_DrawPlayfield();
                    Graphics_UpdateScreen();
                }
                
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
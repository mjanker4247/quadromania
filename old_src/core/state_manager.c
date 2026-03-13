/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: state_manager.c - Simplified game state management implementation
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

#include "state_manager.h"
#include "core/game.h"
#include "graphics/renderer.h"
#include "graphics/ui.h"
#include "audio/sound.h"
#include "data/highscore.h"
#include "input/events.h"
#include "utils/logger.h"

/* Forward declarations for static functions */
static bool handle_title_state(game_state_context_t *context);
static bool handle_instructions_state(game_state_context_t *context);
static bool handle_game_state(game_state_context_t *context);
static bool handle_won_state(game_state_context_t *context);
static bool handle_gameover_state(game_state_context_t *context);
static bool handle_highscores_state(game_state_context_t *context);

void state_manager_init(game_state_context_t *context)
{
    if (!context) return;
    
    context->current_state = GAME_STATE_TITLE;
    context->max_rotations = 1;
    context->level = 1;
    context->score = 0;
    
    LOG_INFO("State manager initialized");
}

void state_manager_cleanup(game_state_context_t *context)
{
    if (!context) return;
    
    /* Save highscores before cleanup */
    Highscore_SaveTable();
    LOG_INFO("State manager cleaned up");
}

void state_manager_change_state(game_state_context_t *context, game_state_t new_state)
{
    if (!context) return;
    
    LOG_DEBUG("State change: %d -> %d", context->current_state, new_state);
    context->current_state = new_state;
}

bool state_manager_update(game_state_context_t *context)
{
    if (!context) return false;
    
    /* Check for quit request */
    if (Event_QuitRequested()) {
        return false;
    }
    
    /* Handle current state */
    switch (context->current_state) {
        case GAME_STATE_TITLE:
            return handle_title_state(context);
            
        case GAME_STATE_INSTRUCTIONS:
            return handle_instructions_state(context);
            
        case GAME_STATE_GAME:
            return handle_game_state(context);
            
        case GAME_STATE_WON:
            return handle_won_state(context);
            
        case GAME_STATE_GAMEOVER:
            return handle_gameover_state(context);
            
        case GAME_STATE_HIGHSCORES:
            return handle_highscores_state(context);
            
        case GAME_STATE_QUIT:
            return false;
            
        default:
            context->current_state = GAME_STATE_TITLE;
            return true;
    }
}

/* Title screen state */
static bool handle_title_state(game_state_context_t *context)
{
    uint16_t mouse_x = Event_GetMouseX();
    uint16_t mouse_y = Event_GetMouseY();
    tGUI_MenuEntries hovered_entry = GUI_GetMenuEntryAtPosition(mouse_x, mouse_y);

    /* Draw menu */
    GUI_DrawMainmenu(context->max_rotations + 1, context->level, hovered_entry);

    /* Handle click */
    if (Event_MouseClicked() && Event_GetMouseButton() == 1) {
        tGUI_MenuEntries clicked_entry = hovered_entry;
        DEBUG_PRINT("Menu entry clicked: %d", clicked_entry);
        
        switch (clicked_entry) {
            case MENU_START_GAME:
                Sound_PlayEffect(SOUND_MENU);
                Quadromania_InitPlayfield(
                    Quadromania_GetRotationsPerLevel(context->level),
                    context->max_rotations);
                Quadromania_DrawPlayfield();
                Graphics_UpdateScreen();
                context->current_state = GAME_STATE_GAME;
                break;
                
            case MENU_CHANGE_NR_OF_COLORS:
                Sound_PlayEffect(SOUND_MENU);
                ++context->max_rotations;
                if (context->max_rotations > 4) context->max_rotations = 1;
                break;
                
            case MENU_CHANGE_NR_OF_ROTATIONS:
                Sound_PlayEffect(SOUND_MENU);
                ++context->level;
                if (context->level > HIGHSCORE_NR_OF_TABLES) context->level = 1;
                break;
                
            case MENU_INSTRUCTIONS:
                Sound_PlayEffect(SOUND_MENU);
                Graphics_DrawInstructions();
                context->current_state = GAME_STATE_INSTRUCTIONS;
                break;
                
            case MENU_HIGHSCORES:
                Sound_PlayEffect(SOUND_MENU);
                Graphics_ListHighscores(context->level - 1);
                context->current_state = GAME_STATE_HIGHSCORES;
                break;
                
            case MENU_QUIT:
                Sound_PlayEffect(SOUND_MENU);
                context->current_state = GAME_STATE_QUIT;
                break;
                
            default:
                break;
        }
        Event_ResetClicked();
    }
    
    return true;
}

/* Instructions screen state */
static bool handle_instructions_state(game_state_context_t *context)
{
    if (Event_MouseClicked()) {
        if (Event_GetMouseButton() == 1 && 
            Event_GetMouseY() > (SCREEN_HEIGHT - Graphics_GetFontHeight())) {
            context->current_state = GAME_STATE_TITLE;
        }
    }
    
    return true;
}

/* Game state */
static bool handle_game_state(game_state_context_t *context)
{
    /* Handle mouse clicks for game interaction */
    if (Event_MouseClicked()) {
        if (Event_GetMouseButton() == 1) {
            uint16_t xraster, yraster;
            xraster = (uint16_t)((Event_GetMouseX() - Graphics_GetDotWidth()) / Graphics_GetDotWidth());
            yraster = (uint16_t)((Event_GetMouseY() - Graphics_GetDotHeight()) / Graphics_GetDotHeight());

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
                    Graphics_DrawGameoverMessage();
                    Sound_PlayEffect(SOUND_LOOSE);
                    context->current_state = GAME_STATE_GAMEOVER;
                } else if (Quadromania_IsGameWon()) {
                    Graphics_DrawWinMessage();
                    Sound_PlayEffect(SOUND_WIN);
                    context->current_state = GAME_STATE_WON;
                }
            }
        }
        Event_ResetClicked();
    }
    
    return true;
}

/* Won state */
static bool handle_won_state(game_state_context_t *context)
{
    if (Event_MouseClicked()) {
        /* Check if it's a highscore */
        uint16_t position = Highscore_GetPosition(context->level - 1, context->score);
        if (position != HIGHSCORE_NO_ENTRY) {
            char *name = Highscore_GetNameFromTimestamp();
            Highscore_EnterScore(context->level - 1, context->score, name, position);
            DEBUG_PRINT("Highscore: %d, Position %d", context->score, position);
        }
        
        /* Show highscores */
        Graphics_ListHighscores(context->level - 1);
        context->current_state = GAME_STATE_HIGHSCORES;
    }
    
    return true;
}

/* Game over state */
static bool handle_gameover_state(game_state_context_t *context)
{
    if (Event_MouseClicked()) {
        /* Show highscores */
        Graphics_ListHighscores(context->level - 1);
        context->current_state = GAME_STATE_HIGHSCORES;
    }
    
    return true;
}

/* Highscores state */
static bool handle_highscores_state(game_state_context_t *context)
{
    if (Event_MouseClicked()) {
        context->current_state = GAME_STATE_TITLE;
    }
    
    return true;
} 
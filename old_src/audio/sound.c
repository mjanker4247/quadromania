/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sound.c - Simplified sound and music API implementation
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

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/sound.h"
#include "utils/logger.h"

/* Audio system state */
static bool sound_initialized = false;
static Uint8 sound_volume = 100;

/* Sound effect chunks */
static Mix_Chunk *sound_menu = NULL;
static Mix_Chunk *sound_turn = NULL;
static Mix_Chunk *sound_win = NULL;
static Mix_Chunk *sound_loose = NULL;

/* Background music */
static Mix_Music *music = NULL;

/* Audio configuration constants */
static const int audio_rate = 44100;
static const Uint16 audio_format = AUDIO_S16SYS;
static const int audio_channels = 2;
static const int audio_buffers = 4096;

/**
 * Initialize the audio system
 */
void Sound_Init(void)
{
	if (sound_initialized) {
		LOG_WARN("Audio system already initialized");
		return;
	}

	/* Initialize SDL audio subsystem */
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		LOG_ERROR("Failed to initialize SDL audio subsystem: %s", SDL_GetError());
		return;
	}

	/* Open audio device */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		LOG_ERROR("Unable to open audio: %s", Mix_GetError());
		return;
	}

	/* Load sound effects */
	sound_menu = Mix_LoadWAV("data/sound/menu.wav");
	sound_turn = Mix_LoadWAV("data/sound/turn.wav");
	sound_win = Mix_LoadWAV("data/sound/win.wav");
	sound_loose = Mix_LoadWAV("data/sound/loose.wav");

	if (!sound_menu || !sound_turn || !sound_win || !sound_loose) {
		LOG_ERROR("Failed to load sound effects: %s", Mix_GetError());
		/* Clean up any loaded sounds */
		if (sound_menu) Mix_FreeChunk(sound_menu);
		if (sound_turn) Mix_FreeChunk(sound_turn);
		if (sound_win) Mix_FreeChunk(sound_win);
		if (sound_loose) Mix_FreeChunk(sound_loose);
		Mix_CloseAudio();
		return;
	}

	/* Set default volume */
	Uint8 calculated_volume = (Uint8)(((Uint16)sound_volume * SDL_MIX_MAXVOLUME) / 100);
	Mix_Volume(-1, calculated_volume);

	/* Load and play background music */
	music = Mix_LoadMUS("data/sound/music.ogg");
	if (music != NULL) {
		Mix_PlayMusic(music, -1);
		LOG_INFO("Background music loaded and started");
	} else {
		LOG_WARN("Failed to load music: %s", Mix_GetError());
	}

	sound_initialized = true;
	LOG_INFO("Audio system initialized successfully");
}

/**
 * Play a sound effect
 */
void Sound_PlayEffect(SoundEffect snd)
{
	if (!sound_initialized) {
		return;
	}

	Mix_Chunk *chunk = NULL;
	
	switch (snd) {
		case SOUND_MENU:
			chunk = sound_menu;
			break;
		case SOUND_TURN:
			chunk = sound_turn;
			break;
		case SOUND_WIN:
			chunk = sound_win;
			break;
		case SOUND_LOOSE:
			chunk = sound_loose;
			break;
		default:
			LOG_WARN("Unknown sound effect: %d", snd);
			return;
	}

	if (chunk) {
		Mix_PlayChannel(-1, chunk, 0);
	}
}

/**
 * Set audio volume (0-100)
 */
void Sound_SetVolume(Uint8 volume)
{
	if (!sound_initialized) {
		return;
	}

	sound_volume = volume;
	Uint8 calculated_volume = (Uint8)(((Uint16)sound_volume * SDL_MIX_MAXVOLUME) / 100);
	Mix_Volume(-1, calculated_volume);
	
	LOG_DEBUG("Audio volume set to %d%%", volume);
}

/**
 * Clean up audio system
 */
void Sound_Exit(void)
{
	if (!sound_initialized) {
		return;
	}

	/* Stop and free music */
	Mix_HaltMusic();
	if (music) {
		Mix_FreeMusic(music);
		music = NULL;
	}

	/* Free sound effects */
	if (sound_menu) {
		Mix_FreeChunk(sound_menu);
		sound_menu = NULL;
	}
	if (sound_turn) {
		Mix_FreeChunk(sound_turn);
		sound_turn = NULL;
	}
	if (sound_win) {
		Mix_FreeChunk(sound_win);
		sound_win = NULL;
	}
	if (sound_loose) {
		Mix_FreeChunk(sound_loose);
		sound_loose = NULL;
	}

	/* Close audio system */
	Mix_CloseAudio();
	
	sound_initialized = false;
	LOG_INFO("Audio system cleaned up");
}

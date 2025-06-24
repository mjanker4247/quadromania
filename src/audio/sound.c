/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sound.c - implements the sound and music API
 * last Modified: 03.07.2010 : 17:49
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/sound.h"
#include "utils/logger.h"

#include <SDL2/SDL_mixer.h>

/* data structures... */
bool sound_initialized = false;
/* the current mixer volume */
Uint8 sound_volume;

/* actual mixer chunks for our sound effects */
Mix_Chunk *sound_menu = NULL;
Mix_Chunk *sound_turn = NULL;
Mix_Chunk *sound_win = NULL;
Mix_Chunk *sound_loose = NULL;
/* the current music to play */
Mix_Music *music = NULL;

/*************
 * CONSTANTS *
 *************/

/* volume in percent */
const Uint8 Sound_VolumeDefault = 100;
/* sample rate */
const int audio_rate = 44100;
/* audio data format for internal use */
const Uint16 audio_format = AUDIO_S16SYS; /* 16-bit stereo */
/* number of channels */
const int audio_channels = 2;
/* size of audio buffers */
const int audio_buffers = 4096;

/*************
 * FUNCTIONS *
 *************/

/* initialize sound subsystem */
void Sound_Init()
{

	SDL_InitSubSystem(SDL_INIT_AUDIO);

	/* open audio devices and setup basic parameters */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
	{
		fprintf(stderr, "Unable to open audio!\n");
	}
	else
	{
		/* load actual sound files.... */
		sound_menu = Mix_LoadWAV("data/sound/menu.wav");
		sound_turn = Mix_LoadWAV("data/sound/turn.wav");
		sound_win = Mix_LoadWAV("data/sound/win.wav");
		sound_loose = Mix_LoadWAV("data/sound/loose.wav");

		/* set default volume */
		sound_volume = Sound_VolumeDefault;
		Sound_SetVolume(sound_volume);

		/* load and play music */
		music = Mix_LoadMUS("data/sound/music.ogg");
		if (music != NULL)
			Mix_PlayMusic(music, -1);

		sound_initialized = true;
	}

	atexit(Sound_Exit);

	DEBUG_PRINT("Audio initialized");

	return;
}

/* terminate sound subsystem properly */
void Sound_Exit()
{
	/* stop playing music */
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music = NULL;

	Mix_CloseAudio();
	sound_initialized = false;
	return;
}

/* play a sound by symbolic name */
void Sound_PlayEffect(SoundEffect snd)
{
	if (sound_initialized == true)
	{
		switch (snd)
		{
		case SOUND_MENU:
			Mix_PlayChannel(-1, sound_menu, 0);
			break;
		case SOUND_TURN:
			Mix_PlayChannel(-1, sound_turn, 0);
			break;
		case SOUND_WIN:
			Mix_PlayChannel(-1, sound_win, 0);
			break;
		case SOUND_LOOSE:
			Mix_PlayChannel(-1, sound_loose, 0);
			break;
		default:
			/* do nothing */
			break;
		}
	}
	return;
}

/* set the actual sound mixer volume according to the given volume in percent */
void Sound_SetVolume(Uint8 volume)
{
	Uint8 calculated_volume;
	if (volume > 100)
		volume = 100;
	calculated_volume = (Uint8) (((Uint16) volume * SDL_MIX_MAXVOLUME) / 100);
	Mix_Volume(-1, calculated_volume);
	return;
}

/* increase sound mixer volume by 10% */
void Sound_IncreaseVolume()
{
	if (sound_volume < 100)
		sound_volume = sound_volume + 10;
	Sound_SetVolume(sound_volume);
	DEBUG_PRINT("Volume increased to %d%%", sound_volume);
	return;
}

/* decrease sound mixer volume by 10% */
void Sound_DecreaseVolume()
{
	if (sound_volume > 0)
		sound_volume = sound_volume - 10;
	DEBUG_PRINT("Volume decreased to %d%%", sound_volume);
	Sound_SetVolume(sound_volume);
	return;
}

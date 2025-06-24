/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sound.h - header file for the sound and music API
 * last Modified: 29.06.2010 : 19:23
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

#ifndef __SOUND_H
#define __SOUND_H

#include <SDL2/SDL.h>
#include <stdbool.h>

	/**************************
	 * DATA TYPE DECLARATIONS *
     **************************/
	typedef enum
	{
		SOUND_MENU  = 1,
		SOUND_TURN  = 2,
		SOUND_WIN   = 3,
		SOUND_LOOSE = 4
	} SoundEffect;

	/* Audio command types for thread-safe communication */
	typedef enum
	{
		AUDIO_CMD_PLAY_EFFECT,
		AUDIO_CMD_SET_VOLUME,
		AUDIO_CMD_INCREASE_VOLUME,
		AUDIO_CMD_DECREASE_VOLUME,
		AUDIO_CMD_STOP_MUSIC,
		AUDIO_CMD_PLAY_MUSIC,
		AUDIO_CMD_SHUTDOWN
	} AudioCommandType;

	/* Audio command structure for thread communication */
	typedef struct
	{
		AudioCommandType type;
		union {
			SoundEffect effect;
			Uint8 volume;
			bool play_music;
		} data;
	} AudioCommand;

	/* Audio thread state */
	typedef struct
	{
		bool initialized;
		bool running;
		SDL_Thread *thread;
		SDL_mutex *command_mutex;
		SDL_cond *command_cond;
		AudioCommand command_queue[32];
		int queue_head;
		int queue_tail;
		int queue_size;
	} AudioThreadState;

	/**************
	 * PROTOTYPES *
	 **************/
	void Sound_Init(void);
	void Sound_PlayEffect(SoundEffect snd);
	void Sound_Exit(void);
	void Sound_SetVolume(Uint8 volume);
	void Sound_IncreaseVolume(void);
	void Sound_DecreaseVolume(void);
	
	/* Thread-safe audio functions */
	bool Sound_InitThreaded(void);
	void Sound_ShutdownThreaded(void);
	int Sound_AudioThread(void *data);
	bool Sound_QueueCommand(AudioCommand *cmd);
	
	/* Audio thread management and statistics */
	bool Sound_IsThreaded(void);
	int Sound_GetQueueSize(void);
	void Sound_GetAudioStats(int *queue_size, int *processed_commands, float *avg_latency);
	
	/* Test function */
	void Sound_TestThreadedSystem(void);

#endif /* __SOUND_H */

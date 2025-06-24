/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: sound.c - implements the sound and music API with threaded audio processing
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

/* Threaded audio system state */
static AudioThreadState audio_thread = {0};

/* Audio statistics for monitoring - simplified for turn-based games */
static struct {
	int total_commands_processed;
	int commands_dropped;
} audio_stats = {0};

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

/**
 * Initialize the threaded audio system
 */
bool Sound_InitThreaded(void)
{
	if (audio_thread.initialized) {
		LOG_WARN("Audio thread already initialized");
		return true;
	}

	/* Initialize SDL audio subsystem */
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		LOG_ERROR("Failed to initialize SDL audio subsystem: %s", SDL_GetError());
		return false;
	}

	/* Open audio devices and setup basic parameters */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		LOG_ERROR("Unable to open audio: %s", Mix_GetError());
		return false;
	}

	/* Load sound effects with error checking */
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
		return false;
	}

	/* Set default volume */
	sound_volume = Sound_VolumeDefault;
	Uint8 calculated_volume = (Uint8)(((Uint16)sound_volume * SDL_MIX_MAXVOLUME) / 100);
	Mix_Volume(-1, calculated_volume);

	/* Load music */
	music = Mix_LoadMUS("data/sound/music.ogg");
	if (music != NULL) {
		Mix_PlayMusic(music, -1);
	} else {
		LOG_WARN("Failed to load music: %s", Mix_GetError());
	}

	/* Initialize thread synchronization primitives */
	audio_thread.command_mutex = SDL_CreateMutex();
	audio_thread.command_cond = SDL_CreateCond();
	
	if (!audio_thread.command_mutex || !audio_thread.command_cond) {
		LOG_ERROR("Failed to create audio thread synchronization primitives");
		/* Clean up */
		Mix_CloseAudio();
		return false;
	}

	/* Initialize command queue */
	audio_thread.queue_head = 0;
	audio_thread.queue_tail = 0;
	audio_thread.queue_size = 0;
	audio_thread.running = true;
	audio_thread.initialized = true;

	/* Initialize statistics */
	memset(&audio_stats, 0, sizeof(audio_stats));

	/* Create audio thread */
	audio_thread.thread = SDL_CreateThread(Sound_AudioThread, "AudioThread", NULL);
	if (!audio_thread.thread) {
		LOG_ERROR("Failed to create audio thread: %s", SDL_GetError());
		/* Clean up */
		SDL_DestroyMutex(audio_thread.command_mutex);
		SDL_DestroyCond(audio_thread.command_cond);
		Mix_CloseAudio();
		return false;
	}

	LOG_INFO("Threaded audio system initialized successfully");
	return true;
}

/**
 * Audio processing thread function
 */
int Sound_AudioThread(void *data)
{
	LOG_INFO("Audio thread started");
	
	while (audio_thread.running) {
		AudioCommand cmd;
		bool has_command = false;
		
		/* Wait for commands with timeout */
		SDL_LockMutex(audio_thread.command_mutex);
		while (audio_thread.queue_size == 0 && audio_thread.running) {
			SDL_CondWaitTimeout(audio_thread.command_cond, audio_thread.command_mutex, 100);
		}
		
		if (audio_thread.queue_size > 0) {
			/* Get command from queue */
			cmd = audio_thread.command_queue[audio_thread.queue_head];
			audio_thread.queue_head = (audio_thread.queue_head + 1) % 32;
			audio_thread.queue_size--;
			has_command = true;
		}
		SDL_UnlockMutex(audio_thread.command_mutex);
		
		if (has_command) {
			/* Process audio command */
			switch (cmd.type) {
				case AUDIO_CMD_PLAY_EFFECT:
					switch (cmd.data.effect) {
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
							break;
					}
					audio_stats.total_commands_processed++;
					break;
					
				case AUDIO_CMD_SET_VOLUME:
					{
						Uint8 calculated_volume = (Uint8)(((Uint16)cmd.data.volume * SDL_MIX_MAXVOLUME) / 100);
						Mix_Volume(-1, calculated_volume);
						sound_volume = cmd.data.volume;
						audio_stats.total_commands_processed++;
					}
					break;
					
				case AUDIO_CMD_INCREASE_VOLUME:
					if (sound_volume < 100) {
						sound_volume += 10;
						Uint8 calculated_volume = (Uint8)(((Uint16)sound_volume * SDL_MIX_MAXVOLUME) / 100);
						Mix_Volume(-1, calculated_volume);
					}
					audio_stats.total_commands_processed++;
					break;
					
				case AUDIO_CMD_DECREASE_VOLUME:
					if (sound_volume > 0) {
						sound_volume -= 10;
						Uint8 calculated_volume = (Uint8)(((Uint16)sound_volume * SDL_MIX_MAXVOLUME) / 100);
						Mix_Volume(-1, calculated_volume);
					}
					audio_stats.total_commands_processed++;
					break;
					
				case AUDIO_CMD_STOP_MUSIC:
					Mix_HaltMusic();
					audio_stats.total_commands_processed++;
					break;
					
				case AUDIO_CMD_PLAY_MUSIC:
					if (music && cmd.data.play_music) {
						Mix_PlayMusic(music, -1);
					}
					audio_stats.total_commands_processed++;
					break;
					
				case AUDIO_CMD_SHUTDOWN:
					audio_thread.running = false;
					break;
					
				default:
					break;
			}
		}
	}
	
	LOG_INFO("Audio thread terminated after processing %d commands", audio_stats.total_commands_processed);
	return 0;
}

/**
 * Queue an audio command for processing by the audio thread
 */
bool Sound_QueueCommand(AudioCommand *cmd)
{
	if (!audio_thread.initialized || !cmd) {
		return false;
	}
	
	SDL_LockMutex(audio_thread.command_mutex);
	
	if (audio_thread.queue_size >= 32) {
		SDL_UnlockMutex(audio_thread.command_mutex);
		audio_stats.commands_dropped++;
		LOG_WARN("Audio command queue full, dropping command (dropped: %d)", audio_stats.commands_dropped);
		return false;
	}
	
	audio_thread.command_queue[audio_thread.queue_tail] = *cmd;
	audio_thread.queue_tail = (audio_thread.queue_tail + 1) % 32;
	audio_thread.queue_size++;
	
	SDL_CondSignal(audio_thread.command_cond);
	SDL_UnlockMutex(audio_thread.command_mutex);
	
	return true;
}

/**
 * Shutdown the threaded audio system
 */
void Sound_ShutdownThreaded(void)
{
	if (!audio_thread.initialized) {
		return;
	}
	
	/* Signal shutdown to audio thread */
	AudioCommand shutdown_cmd = {AUDIO_CMD_SHUTDOWN, {0}};
	Sound_QueueCommand(&shutdown_cmd);
	
	/* Wait for thread to terminate */
	if (audio_thread.thread) {
		int thread_result;
		SDL_WaitThread(audio_thread.thread, &thread_result);
		audio_thread.thread = NULL;
	}
	
	/* Clean up synchronization primitives */
	if (audio_thread.command_mutex) {
		SDL_DestroyMutex(audio_thread.command_mutex);
		audio_thread.command_mutex = NULL;
	}
	
	if (audio_thread.command_cond) {
		SDL_DestroyCond(audio_thread.command_cond);
		audio_thread.command_cond = NULL;
	}
	
	/* Clean up audio resources */
	Mix_HaltMusic();
	if (music) {
		Mix_FreeMusic(music);
		music = NULL;
	}
	
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
	
	Mix_CloseAudio();
	
	/* Reset state */
	memset(&audio_thread, 0, sizeof(audio_thread));
	sound_initialized = false;
	
	LOG_INFO("Threaded audio system shutdown complete");
}

/* initialize sound subsystem (legacy function - now uses threaded version) */
void Sound_Init()
{
	sound_initialized = Sound_InitThreaded();
	if (sound_initialized) {
		atexit(Sound_Exit);
		LOG_INFO("Audio initialized");
	} else {
		LOG_ERROR("Failed to initialize audio system");
	}
}

/* terminate sound subsystem properly (legacy function) */
void Sound_Exit()
{
	Sound_ShutdownThreaded();
}

/* play a sound by symbolic name (thread-safe) */
void Sound_PlayEffect(SoundEffect snd)
{
	if (!sound_initialized) {
		return;
	}
	
	AudioCommand cmd = {AUDIO_CMD_PLAY_EFFECT, {.effect = snd}};
	Sound_QueueCommand(&cmd);
}

/* set the actual sound mixer volume according to the given volume in percent (thread-safe) */
void Sound_SetVolume(Uint8 volume)
{
	if (!sound_initialized) {
		return;
	}
	
	AudioCommand cmd = {AUDIO_CMD_SET_VOLUME, {.volume = volume}};
	Sound_QueueCommand(&cmd);
}

/* increase sound mixer volume by 10% (thread-safe) */
void Sound_IncreaseVolume()
{
	if (!sound_initialized) {
		return;
	}
	
	AudioCommand cmd = {AUDIO_CMD_INCREASE_VOLUME, {0}};
	Sound_QueueCommand(&cmd);
	LOG_DEBUG("Volume increase queued");
}

/* decrease sound mixer volume by 10% (thread-safe) */
void Sound_DecreaseVolume()
{
	if (!sound_initialized) {
		return;
	}
	
	AudioCommand cmd = {AUDIO_CMD_DECREASE_VOLUME, {0}};
	Sound_QueueCommand(&cmd);
	LOG_DEBUG("Volume decrease queued");
}

/**
 * Check if audio system is running in threaded mode
 */
bool Sound_IsThreaded(void)
{
	return audio_thread.initialized && audio_thread.running;
}

/**
 * Get current audio command queue size
 */
int Sound_GetQueueSize(void)
{
	if (!audio_thread.initialized) {
		return 0;
	}
	
	SDL_LockMutex(audio_thread.command_mutex);
	int size = audio_thread.queue_size;
	SDL_UnlockMutex(audio_thread.command_mutex);
	
	return size;
}

/**
 * Get audio system statistics - simplified for turn-based games
 */
void Sound_GetAudioStats(int *queue_size, int *processed_commands, float *avg_latency)
{
	if (queue_size) {
		*queue_size = Sound_GetQueueSize();
	}
	if (processed_commands) {
		*processed_commands = audio_stats.total_commands_processed;
	}
	if (avg_latency) {
		*avg_latency = 0.0f; /* Not tracking latency for turn-based games */
	}
}

/**
 * Test function to verify threaded audio system
 */
void Sound_TestThreadedSystem(void)
{
	if (!Sound_IsThreaded()) {
		LOG_ERROR("Audio system is not running in threaded mode");
		return;
	}
	
	LOG_INFO("Testing threaded audio system...");
	
	/* Test sound effects */
	Sound_PlayEffect(SOUND_MENU);
	SDL_Delay(100);
	Sound_PlayEffect(SOUND_TURN);
	SDL_Delay(100);
	Sound_PlayEffect(SOUND_WIN);
	SDL_Delay(100);
	Sound_PlayEffect(SOUND_LOOSE);
	SDL_Delay(100);
	
	/* Test volume controls */
	Sound_IncreaseVolume();
	SDL_Delay(50);
	Sound_DecreaseVolume();
	SDL_Delay(50);
	
	/* Check queue status */
	int queue_size = Sound_GetQueueSize();
	LOG_INFO("Audio test completed. Current queue size: %d", queue_size);
	
	/* Wait for commands to be processed */
	while (Sound_GetQueueSize() > 0) {
		SDL_Delay(10);
	}
	
	LOG_INFO("All audio commands processed successfully");
}

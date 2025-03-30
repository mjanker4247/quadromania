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

	#include "datatypes.h"
	#include "sysconfig.h"

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

	/**************
	 * PROTOTYPES *
	 **************/
	void Sound_Init(void);
	void Sound_PlayEffect(SoundEffect snd);
	void Sound_Exit(void);
	void Sound_SetVolume(Uint8 volume);
	void Sound_IncreaseVolume(void);
	void Sound_DecreaseVolume(void);

#endif /* __SOUND_H */

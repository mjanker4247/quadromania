/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: main.h - declarations for the main module
 * last Modified: 05.03.2010 : 18:02
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

#ifndef __MAIN_H
	#define __MAIN_H

    #include "version.h"
	#include "sysconfig.h"
	#include "datatypes.h"
	#include "boolean.h"

	/**************************
	 * DATA TYPE DECLARATIONS *
	 **************************/
	enum GAMESTATE
	{
		UNINITIALIZED, NONE, TITLE, INSTRUCTIONS, SETUPCHANGED, GAME, WON, GAMEOVER, HIGHSCORE_ENTRY, SHOW_HIGHSCORES, QUIT
	};

	/**************
	 * PROTOTYPES *
	 **************/

	BOOLEAN InitGameEngine(BOOLEAN activate_fullscreen);
	void MainHandler(void);

#endif

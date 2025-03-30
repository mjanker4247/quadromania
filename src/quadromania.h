/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: quadromania.h - header file for the game logic module
 * last Modified: 06.03.2010 : 11:28
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

#ifndef __QUADROMANIA_H
	#define __QUADROMANIA_H
	#include "boolean.h"

	/**************
	 * PROTOTYPES *
	 **************/

	void Quadromania_ClearPlayfield(void);
	void Quadromania_InitPlayfield(Uint16, Uint8);
	void Quadromania_Rotate(Uint32, Uint32);
	void Quadromania_DrawPlayfield(void);
	BOOLEAN Quadromania_IsGameWon(void);
	BOOLEAN Quadromania_IsTurnLimithit(void);
	Uint32 Quadromania_GetPercentOfSolution(void);
	Uint16 Quadromania_GetRotationsPerLevel(Uint8);
#endif

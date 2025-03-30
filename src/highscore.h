/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: highscore.h - header file for the highscore API
 * last Modified: 12.06.2010 : 18:12
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

#ifndef __HIGHSCORE_H
#define __HIGHSCORE_H

	#include "datatypes.h"

	/************
	 * DEFINES  *
	 ************/

	#define HIGHSCORE_MAX_LEN_OF_NAME         20
	#define HIGHSCORE_NR_OF_TABLES            10
	#define HIGHSCORE_NR_OF_ENTRIES_PER_TABLE 8

	/**********
	 * MACROS *
	 **********/

	#define HIGHSCORE_NO_ENTRY                HIGHSCORE_NR_OF_ENTRIES_PER_TABLE
    #define HIGHSCORE_FILENAME                "quadromania.scores"

	/**************************
	 * DATA TYPE DECLARATIONS *
	 **************************/

	typedef struct
	{
		Uint32 score;
		char name[HIGHSCORE_MAX_LEN_OF_NAME];
	} HighscoreEntry; /* a single highscore entry */

	typedef struct
	{
		HighscoreEntry Entry[HIGHSCORE_NR_OF_TABLES][HIGHSCORE_NR_OF_ENTRIES_PER_TABLE];
	} HighscoreFile; /* the highscore data as a binary file to be laoded and saved to disk */

	/**************
	 * PROTOTYPES *
	 **************/

	void Highscore_LoadTable(void);
	void Highscore_SaveTable(void);
	Uint16 Highscore_GetPosition(Uint16 table, Uint32 score);
	void Highscore_EnterScore(Uint16 table, Uint32 score, char *name, Uint16 position);
	HighscoreEntry* Highscore_GetEntry(Uint16 table, Uint16 rank);
	char* Highscore_GetNameFromTimestamp(void);

#endif /* __HIGHSCORE_H */

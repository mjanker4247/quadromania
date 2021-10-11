/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: datatypes.h - header file for defining platform independent data types
 * last Modified: 18.11.2010 : 19:05
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

#ifndef __DATATYPES_H
#define __DATATYPES_H

	#include "sysconfig.h"

#if(HAVE_SDL_DATATYPES == 1)
	#include <SDL.h>
#else
	/* we have to typedef the datatypes with SDL naming conventions if we don't have them */
	/* FIXME: if stdint.h is available point to those instead */

	typedef unsigned char Uint8;
	typedef unsigned short Uint16;
	typedef unsigned long Uint32;
	typedef signed char Sint8;
	typedef signed short Sint16;
	typedef signed long Sint32;

#endif

#endif /* __DATATYPES_H */

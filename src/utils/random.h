/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: random.h - header files for the random number generator
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
#ifndef __RANDOM_H
#define __RANDOM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the random number generator with a seed based on current time.
 * Uses high-resolution clock for better entropy.
 */
void Random_InitSeed(void);

/**
 * Get a high-quality pseudo random number using Mersenne Twister.
 * Returns a 32-bit unsigned integer in the range [0, UINT32_MAX].
 */
uint32_t Random_GetRandom(void);

#ifdef __cplusplus
}
#endif

#endif /* __RANDOM_H */

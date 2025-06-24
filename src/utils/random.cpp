/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: random.cpp - implements a modern random number generator using C++ standard library
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

#include <random>
#include <chrono>
#include "utils/random.h"

// Global random number generator using Mersenne Twister
static std::mt19937 rng;
static std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

/**
 * This function initializes the random number generator with a seed based on current time.
 * Uses high-resolution clock for better entropy.
 */
extern "C" void Random_InitSeed()
{
    auto now = std::chrono::steady_clock::now();
    auto seed = now.time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));
}

/**
 * This function provides a high-quality pseudo random number using Mersenne Twister.
 * Returns a 32-bit unsigned integer in the range [0, UINT32_MAX].
 */
extern "C" uint32_t Random_GetRandom()
{
    return dist(rng);
} 
# Game Purpose and Concept

Quadromania is a puzzle board game where the player's objective is to restore a grid of colored tiles back to their original state (all red) by strategically rotating 3x3 sections of the board. It's essentially a color-matching puzzle game with turn-based mechanics.

## Core Game Mechanics

1. Game Board Structure

- The game uses an 18×13 grid of colored tiles
- Each tile can have colors ranging from 0 (red/base color) to a maximum number of colors (1-4, configurable)
- The goal is to get all tiles back to color 0 (red)

1. Rotation System

- Players click on the center of any 3×3 section of tiles
- This rotates/cycles the colors of all 9 tiles in that section
- Each rotation increments the color value of each tile by 1
- When a tile reaches the maximum color value, it cycles back to 0 (red)

1. Game Setup

- The computer randomly rotates various 3×3 sections to scramble the board
- The number of initial rotations depends on the selected level
- Players have a turn limit equal to the number of initial rotations × maximum colors

1. Scoring System

- Players score based on how efficiently they solve the puzzle
- Score = ((limit - turns) * 10000) / turns
- Higher scores are achieved by using fewer turns
- If you exceed the turn limit, you score 0

## Technical Implementation

### Core Components

- Game Logic (src/core/game.c)
- Manages the 18×13 playfield array
- Handles rotation mechanics via Quadromania_Rotate()
- Tracks turns and win/loss conditions
- Implements turn-based rendering optimization
- State Management (src/core/state_manager.c)
- Manages game flow through different states:
- Title screen
- Instructions
- Active gameplay
- Win/loss screens
- Highscores
- Handles menu navigation and game configuration
- Input System (src/input/events.c)
- Processes mouse clicks and touch input
- Converts screen coordinates to grid positions
- Handles quit requests
- Graphics System (src/graphics/)
- Renders the colored tiles using GameKit
- Displays UI elements and text
- Implements turn-based rendering optimization

## Key Algorithms

- Rotation Algorithm (Quadromania_Rotate()):

```code

void Quadromania_Rotate(Uint32 x, Uint32 y) {
 for (i = x - 1; i < (x + 2); ++i) {
   for (j = y - 1; j < (y + 2); ++j) {
   playfield[i][j]++;
   if (playfield[i][j] > rotations)
    playfield[i][j] = Quadromania_Basecolor;
  }
 }
}
```

- Win Condition Check (Quadromania_IsGameWon()):
- Scans the entire playfield to verify all tiles are color 0
- Turn Limit Check (Quadromania_IsTurnLimithit()):
- Compares current turns against the calculated limit

## Game Flow

- Title Screen: Players configure difficulty (colors: 1-4, level: 1-10)
- Game Initialization: Computer scrambles the board with random rotations
- Gameplay: Players click 3×3 sections to rotate colors
- Win/Loss Check: Game ends when board is solved or turn limit exceeded
- Scoring: High scores are saved and displayed

## Technical Features

- Cross-platform: Uses GameKit for graphics, audio, and input
- Optimized Rendering: Only redraws when the playfield changes
- Persistent Highscores: Saves scores to disk between sessions
- Audio Feedback: Sound effects for menu navigation and gameplay
- Configurable Difficulty: Adjustable color count and level complexity

This is essentially a mathematical puzzle game that challenges players to think strategically about which rotations will most efficiently restore the board to its original state, with the constraint of a limited number of moves.

# Quadromania

A macOS puzzle game. Restore an 18×13 grid of colored tiles to their base color by strategically rotating 3×3 sections — before you run out of turns.

## Requirements

- macOS 13 or later
- Xcode 15 or later (to build from source)

## Installation

### Build from source

1. Clone the repository:
   ```bash
   git clone https://github.com/mjanker4247/quadromania.git
   cd quadromania
   ```

2. Open `Quadromania.xcodeproj` in Xcode and press **Cmd+R**, or build from the terminal:
   ```bash
   xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
   ```

3. The built app is placed in `Build/Products/Debug/Quadromania.app`. Double-click to run, or launch it directly from Xcode.

### Run the tests

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test
```

There are 19 unit tests covering the core game logic (`GameModel` and `PuzzleGenerator`).

## How to play

The board starts scrambled. Your goal is to return every tile to color 0 — the base color — before the turn limit runs out.

**Rotating** — click any interior tile to rotate the 3×3 block centered on it. Every tile in that block advances one step along the color cycle. Tiles at the maximum color wrap back to 0.

**Turn limit** — displayed in the HUD. Each click costs one turn. The limit is set at the start of each game based on the difficulty you chose; it is always solvable within that limit.

**Winning** — solve the board in fewer turns for a higher score. The score formula is `((limit − turns) × 10000) / turns`.

**Losing** — if you exhaust the turn limit without solving the board, the game ends. Click anywhere on the overlay to start a new game.

## Game menu

| Menu | Option | Description |
|------|--------|-------------|
| **Game** | New Game `⌘N` | Start a new game at any time |
| | Colors | Set how many colors tiles cycle through: 2, 3, 4, or 5 |
| | Transition | Choose the tile rotation animation: Ring Sweep, Sequential, or Radial Pulse |
| | Difficulty | Beginner, Intermediate, or Expert — controls how scrambled the starting board is |
| | Instructions | Open the instructions screen |
| **Sound** | Stop / Start Music `⇧⌘M` | Toggle background music |
| **Palette** | Spring / Ocean / Sunset / Forest | Switch the color theme |
| | Custom | Apply your custom palette |
| | Edit Custom Colors… | Open the color picker to define your own palette |
| | Color Symbols | Overlay shape symbols on tiles for colorblind accessibility |

Settings are saved automatically and restored on the next launch.

## Difficulty levels

| Level | Scramble rotations | Notes |
|-------|--------------------|-------|
| Beginner | 186 | Easiest — longest path to solution, most turns allowed |
| Intermediate | 126 | Balanced |
| Expert | 69 | Fewest rotations, tightest turn limit |

The turn limit equals `initial rotations × (colors − 1)`. Choosing more colors increases both difficulty and the number of available turns.

## Tutorial

Open **Game → Instructions**, then click **Start Tutorial** for a guided walkthrough on a small 5×4 grid.

## Project structure

| Target | Contents |
|--------|----------|
| `Quadromania` | macOS app — all scenes, menus, visuals, and audio |
| `QuadroCore` | Static library — pure game logic (`GameModel`, `PuzzleGenerator`), no SpriteKit dependency |
| `QuadroTests` | XCTest suite for `QuadroCore` |

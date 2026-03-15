# Quadromania

A macOS puzzle game. Restore an 18×13 grid of colored tiles to their lightest color before you run out of turns.

## Build

Open `Quadromania.xcodeproj` in Xcode and press **Cmd+R**, or from the terminal:

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
```

Requires macOS 13+ and Xcode 15+.

## How to play

1. **Goal** — Return every tile to its lightest color (color 0) before you run out of turns.
2. **Click** any tile to rotate the 3×3 block centered on it. Every tile in that block steps forward by one color. Tiles at the maximum color wrap back to 0.
3. **Turn limit** — shown at the top. Each click costs one turn. Plan carefully.
4. **Scoring** — solve the board in fewer turns to score higher.

## Options

- **Select colors** — sets how many color steps tiles cycle through (1–4 extra colors).
- **Select level** — controls how scrambled the board starts (1 = easiest, 10 = hardest).
- **Palette** menu — choose from Spring 🌸, Ocean 🌊, Sunset 🌅, or Forest 🌿.
- **Color Symbols** — adds shape symbols to tiles for colorblind accessibility.

## Tutorial

Open **Instructions** from the main menu, then click **Start Tutorial** for a guided walkthrough on a small grid.

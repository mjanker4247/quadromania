# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

Open `Quadromania.xcodeproj` in Xcode and use **Cmd+R** to build and run, or from the terminal:

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
```

Run the unit tests (game logic only, no UI):

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test
```

There are 19 unit tests in the `QuadroTests` XCTest target covering `GameModel` and `PuzzleGenerator`.

## Game Concept

Quadromania is a puzzle game where players restore an **18×13 grid** of colored tiles back to color 0 by clicking the center of any 3×3 section, which increments every tile in that section by 1 (cycling back to 0 at the max color). The computer scrambles the board at the start; players have a fixed turn limit to solve it.

- **Grid**: 18 columns × 13 rows = 234 tiles
- **Colors**: 2 up to 5 (configurable per game via "Select colors")
- **Color palettes**: 4 themes (Spring, Ocean, Sunset, Forest); selectable in title screen and menu bar
- **Accessibility**: "Color Symbols" toggle in Palette menu adds shape symbols to tiles
- **Turn limit**: `initialRotations × maxColors`
- **Score**: `((limit - turns) × 10000) / turns`, or 0 if limit exceeded
- **Level rotations**: `56 + (11 − level) × 13` initial scramble rotations (level 1 = 186 rotations = easiest; level 10 = 69 rotations = hardest)

## Architecture

Quadromania is a **macOS SpriteKit game** written in Swift. The entrypoint is `GameViewController`, which creates an `SKView` at **1280×960** and presents `TitleScene`.

### Targets

| Target | Type | Contents |
|--------|------|----------|
| `Quadromania` | macOS App | All UI/scene code, `TilePalette` |
| `QuadroCore` | Static Library | `GameModel`, `PuzzleGenerator` (pure logic, no SpriteKit) |
| `QuadroTests` | XCTest | 19 tests for `QuadroCore` |

`QuadroCore` exists so game logic can be tested without instantiating any SpriteKit scene.

### Scene Flow

```
AppDelegate
  └─ GameViewController (SKView 1280×960)
       └─ TitleScene          ← main menu
            ├─ GamePlayScene  ← active gameplay
            │    └─ TitleScene (on game end)
            ├─ InstructionsScene
            │    └─ TutorialScene  ← scripted 3-step tutorial
            ├─ HighscoreScene
            └─ TitleScene (quit via NSApp.terminate)
```

### Source Files

| File | Responsibility |
|------|----------------|
| `AppDelegate.swift` | App lifecycle; Sound menu (music toggle); Palette menu (palette + symbol overlay) |
| `GameViewController.swift` | Hosts `SKView`; presents `TitleScene` on launch |
| `SoundManager.swift` | AVFoundation singleton; background music + 4 WAV effects |
| `TilePalette.swift` | `TilePalette` enum — 4 color themes (Spring/Ocean/Sunset/Forest), 5 `SKColor`s each |
| `QuadroCore/GameModel.swift` | Pure game logic: 18×13 playfield, rotate, win/loss, scoring |
| `QuadroCore/PuzzleGenerator.swift` | Generates solvable puzzles via backwards construction |
| `HighscoreManager.swift` | Persistent highscore tables (10 levels × 10 entries, `UserDefaults`) |
| `TitleScene.swift` | Main menu scene; 2×2 palette swatch grid; menu bar sync |
| `GamePlayScene.swift` | Active gameplay scene; tile grid; win/loss overlays; menu bar sync |
| `InstructionsScene.swift` | Instructions screen with "Start Tutorial" button |
| `TutorialScene.swift` | Scripted tutorial; standalone 5×4 grid; 3-step walkthrough |
| `HighscoreScene.swift` | Highscore display scene |
| `TileGridNode.swift` | 18×13 grid of `SKSpriteNode` tiles; palette swap; symbol overlay |

### Menu Bar

`AppDelegate` injects two custom menus at launch:

| Menu | Items |
|------|-------|
| Sound | Start/Stop Music |
| Palette | 🌸 Spring / 🌊 Ocean / 🌅 Sunset / 🌿 Forest / — / Color Symbols |

Selecting a palette posts `Notification.Name.paletteDidChange`; toggling symbols posts `.symbolOverlayDidChange`. Both `TitleScene` and `GamePlayScene` observe these.

### Existing Working Components

**`SoundManager`** (`SoundManager.shared`) wraps AVFoundation:
- Background music — `sounds/music.m4a`, loops forever.
- Four WAV effects in `sounds/`: `menu.wav`, `turn.wav`, `win.wav`, `loose.wav`.

## Key Conventions

- **No SDL, no C/C++ code** — all logic is Swift. The `old_src/` directory is reference-only.
- `GameViewController` creates its view programmatically — canonical window size is **1280×960**.
- Game logic lives in `GameModel` (in `QuadroCore`), completely decoupled from SpriteKit.
- Tile colors are `Int` values 0…`maxColors`. Color 0 is always the goal state.
- Rotation algorithm: every tile in the 3×3 block centered at (x,y) increments by 1; if `value > maxColors`, reset to 0.
- Coordinate system: SpriteKit uses bottom-left origin; mouse clicks are converted to grid (col, row) via `TileGridNode.gridCoordinates(for:)`.
- Highscores are persisted via `UserDefaults`. 10 difficulty levels, 10 entries per level.

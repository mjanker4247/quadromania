# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

Open `Quadromania.xcodeproj` in Xcode and use **Cmd+R** to build and run, or from the terminal:

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
```

There are no unit tests at this time.

## Game Concept

Quadromania is a puzzle game where players restore an **18×13 grid** of colored tiles back to color 0 (red) by clicking the center of any 3×3 section, which increments every tile in that section by 1 (cycling back to 0 at the max color). The computer scrambles the board at the start; players have a fixed turn limit to solve it.

- **Grid**: 18 columns × 13 rows = 234 tiles
- **Colors**: 0 (red/base) up to 4 (configurable per game)
- **Turn limit**: `initialRotations × maxColors`
- **Score**: `((limit - turns) × 10000) / turns`, or 0 if limit exceeded
- **Level rotations**: `56 + (11 − level) × 13` initial scramble rotations (level 1 = 186 rotations = most turns = easiest; level 10 = 69 rotations = fewest turns = hardest)

## Architecture

Quadromania is a **macOS SpriteKit game** written in Swift, targeting macOS via Cocoa. The intended entrypoint is `GameViewController`, which programmatically creates an `SKView` at **1280×960** and presents `TitleScene`.

### Scene Flow

```
AppDelegate
  └─ GameViewController (SKView 1280×960)
       └─ TitleScene          ← main menu (colors, level, start, instructions, highscores, quit)
            └─ GamePlayScene  ← active gameplay (tile grid, turn counter, win/loss check)
                 └─ TitleScene (on game end, after highscore display)
```

Each game state from the original C code maps to a SpriteKit scene or overlay:

| Old C state (`state_manager.c`)  | Swift / SpriteKit equivalent      |
|----------------------------------|-----------------------------------|
| `GAME_STATE_TITLE`               | `TitleScene`                      |
| `GAME_STATE_INSTRUCTIONS`        | `InstructionsScene` (or overlay)  |
| `GAME_STATE_GAME`                | `GamePlayScene`                   |
| `GAME_STATE_WON` / `GAMEOVER`    | Overlay node in `GamePlayScene`   |
| `GAME_STATE_HIGHSCORES`          | `HighscoreScene`                  |

### Source Files

| File | Responsibility |
|------|----------------|
| `AppDelegate.swift` | App lifecycle; starts music; injects Sound menu (Cmd+Shift+M) |
| `GameViewController.swift` | Hosts `SKView`; presents `TitleScene` on launch |
| `SoundManager.swift` | AVFoundation singleton; background music + 4 WAV effects |
| `Models/GameModel.swift` | *(to be created)* Pure game logic: playfield array, rotate, win/loss checks |
| `Models/HighscoreManager.swift` | *(to be created)* Persistent highscore tables (10 levels × 10 entries) |
| `Scenes/TitleScene.swift` | *(to be created)* Main menu scene |
| `Scenes/GamePlayScene.swift` | *(to be created)* Active gameplay scene with tile grid |
| `Scenes/InstructionsScene.swift` | *(to be created)* Instructions screen |
| `Scenes/HighscoreScene.swift` | *(to be created)* Highscore display scene |
| `Nodes/TileGridNode.swift` | *(to be created)* 18×13 grid of `SKSpriteNode` tiles |

### Legacy Files (Xcode template — to be removed)

- `ViewController.swift` — storyboard path, loads `GameScene.sks` via `GKScene`. Superseded by `GameViewController`.
- `GameScene.swift` — Xcode template scene with spinny-node demo code. Will be replaced by `GamePlayScene`.
- `GameScene.sks` — Scene file for the template `GameScene`. Not used by the active entrypoint.

### Existing Working Components

**`SoundManager`** (`SoundManager.shared`) wraps AVFoundation:
- Background music — `sounds/music.m4a`, loops forever. Falls back to `.mp3` then `.ogg`.
- Four WAV effects in `sounds/`: `menu.wav`, `turn.wav`, `win.wav`, `loose.wav` — played via `playEffect(_:)`.

**`AppDelegate`** starts music on launch and injects a Sound menu:
- **Cmd+Shift+M** — toggle music on/off

## Key Conventions

- **No SDL, no C/C++ code** — all logic is Swift. The `old_src/` directory is reference-only.
- Sound effects are cases of the `SoundEffect` enum in `SoundManager.swift`. Add new cases there and a corresponding `AVAudioPlayer` property.
- All audio files live under `Quadromania/sounds/`. Music is `.m4a`; effects are `.wav`.
- `GameViewController` creates its view programmatically — canonical window size is **1280×960**.
- Game logic lives in `GameModel` (a plain Swift class/struct), completely decoupled from SpriteKit. Scenes read from `GameModel` and call its mutating methods.
- Tile colors are `Int` values 0…`maxColors`. Color 0 is always the goal (red). The rotation algorithm: every tile in the 3×3 block centered at (x,y) increments by 1, wrapping to 0 at `maxColors + 1`.
- Coordinate system: SpriteKit uses bottom-left origin; mouse clicks must be converted from SpriteKit scene coordinates to grid column/row indices.
- Highscores are persisted via `UserDefaults` (or a file in the app's Application Support folder). 10 difficulty levels, 10 entries per level.
- Background art index is chosen randomly at game start (0–9), matching the 10 texture variants in the sprite sheet.

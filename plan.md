# Quadromania — Migration Plan: C/SDL → Swift/SpriteKit

## Goal

Port all game logic and rendering from the old C/SDL source (`old_src/`) into the existing Swift/SpriteKit Xcode project. The result must be a fully playable macOS app with no C code, no SDL dependency, and no legacy files.

---

## Reference: Old C → Swift Mapping

| Old C file | Swift replacement | Notes |
|---|---|---|
| `core/game.c` | `Models/GameModel.swift` | Pure value-type game logic |
| `core/state_manager.c` | Scene transitions + `GameModel` state | States become SpriteKit scenes |
| `graphics/renderer.c` | SpriteKit nodes (per-scene) | SDL textures → `SKSpriteNode` |
| `graphics/ui.c` | `Scenes/TitleScene.swift` | Menu as `SKLabelNode` items |
| `data/highscore.c` | `Models/HighscoreManager.swift` | File → `UserDefaults` or JSON |
| `audio/sound.c` | `SoundManager.swift` | Already ported — no changes needed |
| `input/events.c` | `mouseDown(with:)` in each scene | SpriteKit event overrides |
| `utils/random.cpp` | `Int.random(in:)` | Swift stdlib |
| `utils/logger.cpp` | `os.Logger` | Already used in `SoundManager` |

---

## Phase 1 — Game Model (`GameModel.swift`)

**New file**: `Quadromania/Models/GameModel.swift`

Port `core/game.c` as a plain Swift class with no SpriteKit imports.

```
Constants (from game.c):
  baseRotations = 56
  modifierPerLevel = 13
  gridWidth = 18, gridHeight = 13
  baseColor = 0

State:
  playfield: [[Int]]  (18×13, all 0 initially)
  rotations: Int      (max color value, 1–4)
  turns: Int
  limit: Int          (initialRotations × maxRotations)
  backgroundArtIndex: Int  (0–9, random at init)

Methods (direct ports):
  init(level: Int, maxColors: Int)
    - backgroundArtIndex = Int.random(in: 0...9)
    - clearPlayfield()
    - scramble with initialRotations random rotate() calls (x: 1...16, y: 1...11)
    - limit = initialRotations * maxColors
  rotate(x: Int, y: Int)
    - increments all 9 tiles in (x-1...x+1) × (y-1...y+1), wrapping at rotations
    - turns++
  isGameWon() -> Bool     (all tiles == 0)
  isTurnLimitHit() -> Bool  (turns > limit)
  score() -> Int            (((limit-turns)*10000)/turns, or 0 if limit hit)
  rotationsForLevel(_ level: Int) -> Int  (baseRotations + level*modifierPerLevel)
```

No rendering, no SpriteKit. This is pure logic.

---

## Phase 2 — Highscore Manager (`HighscoreManager.swift`)

**New file**: `Quadromania/Models/HighscoreManager.swift`

Port `data/highscore.c`. Replace raw file I/O with `UserDefaults` (or JSON in Application Support).

```
Constants:
  numberOfTables = 10   (levels 1–10)
  entriesPerTable = 10

Types:
  struct HighscoreEntry: Codable { var score: Int; var name: String }

State:
  tables: [[HighscoreEntry]]  (10 tables × 10 entries)

Methods:
  load()           — decode from UserDefaults
  save()           — encode to UserDefaults
  position(table:, score:) -> Int?   — first slot where score > existing
  enterScore(table:, score:, name:, at:)
  entries(for table: Int) -> [HighscoreEntry]
  nameFromTimestamp() -> String  — "YYYY-MM-DD HH:mm"
```

---

## Phase 3 — Tile Grid Node (`Nodes/TileGridNode.swift`)

**New file**: `Quadromania/Nodes/TileGridNode.swift`

An `SKNode` subclass that holds the 18×13 grid of colored tile sprites.

```
State:
  tiles: [[SKSpriteNode]]  (18×13)
  colorTextures: [SKTexture]  (one per color, 0…maxColors)

Init:
  - load "dots" texture atlas or sprite sheet
  - create 18×13 SKSpriteNode grid, positioned within the scene
  - tiles are sized to fit inside the 1280×960 window with the frame border

updateTile(x:, y:, color:)
  - set texture on tiles[x][y] to colorTextures[color]

updateAll(from playfield: [[Int]])
  - batch update all 234 tiles
```

Tile sizing:
- Frame border consumes ~1 tile width/height on each edge
- Usable area: approximately 1216×896 for the 18×13 grid
- Tile size: ~67×69 px (exact values from frame border measurements)

---

## Phase 4 — Title Scene (`Scenes/TitleScene.swift`)

**New file**: `Quadromania/Scenes/TitleScene.swift`

Port `graphics/ui.c` / `handle_title_state()`. An `SKScene` showing the main menu.

```
State:
  selectedColors: Int = 1    (1–4)
  selectedLevel: Int = 1     (1–10)

Visual elements (SKNode children):
  - Background texture (tiled SKSpriteNode, backgroundArtIndex=9 for menu)
  - Frame overlay (SKSpriteNode, "frame" asset)
  - Title logo (SKSpriteNode, "title" asset)
  - Copyright image (SKSpriteNode, "copyright" asset)
  - Menu labels (SKLabelNode array):
      "Start the game"
      "Select colors"  + dot preview sprites
      "Select initial turns" + rotation count label
      "Instructions"
      "Highscores"
      "Quit"

Interaction:
  mouseMoved: highlight label under cursor (yellow vs white)
  mouseDown: act on clicked label entry
    - Start → create GameModel(level:, maxColors:), transition to GamePlayScene
    - Colors → cycle selectedColors 1→2→3→4→1
    - Turns → cycle selectedLevel 1→10→1
    - Instructions → transition to InstructionsScene
    - Highscores → transition to HighscoreScene
    - Quit → NSApp.terminate
  All menu clicks → SoundManager.shared.playEffect(.menu)
```

---

## Phase 5 — Gameplay Scene (`Scenes/GamePlayScene.swift`)

**New file**: `Quadromania/Scenes/GamePlayScene.swift`

Port `handle_game_state()` and `Quadromania_DrawPlayfield()`. The main gameplay scene.

```
State:
  model: GameModel          (passed in from TitleScene)
  tileGrid: TileGridNode    (child node)
  turnsLabel: SKLabelNode
  limitLabel: SKLabelNode

Init(model: GameModel):
  - Add background sprite (model.backgroundArtIndex)
  - Add frame overlay
  - Add tileGrid, populate from model.playfield
  - Add HUD labels (turns, limit, version string)

mouseDown(with event:):
  - Convert event.location(in: self) to grid coords (xraster, yraster)
  - Validate: xraster in 1...16, yraster in 1...11
  - model.rotate(x: xraster, y: yraster)
  - tileGrid.updateAll(from: model.playfield)
  - Update HUD labels
  - SoundManager.shared.playEffect(.turn)
  - Check model.isTurnLimitHit() → showGameOver()
  - Check model.isGameWon() → showWin()

showWin():
  - Overlay message box (blue background, "Congratulations! You've won!")
  - SoundManager.shared.playEffect(.win)
  - On next click: check highscore, transition to HighscoreScene

showGameOver():
  - Overlay message box (red background, "GAME OVER! You hit the turn limit!")
  - SoundManager.shared.playEffect(.loose)
  - On next click: transition to HighscoreScene
```

Coordinate conversion (SpriteKit bottom-left → grid):
- SpriteKit Y is flipped relative to the original SDL coordinate system
- `xraster = Int((location.x - tileWidth) / tileWidth)`
- `yraster = Int((sceneHeight - location.y - tileHeight) / tileHeight)`

---

## Phase 6 — Instructions Scene (`Scenes/InstructionsScene.swift`)

**New file**: `Quadromania/Scenes/InstructionsScene.swift`

Simple scene showing game instructions. Port of `Graphics_DrawInstructions()`.

```
Visual:
  - Background + frame + title logo
  - Instructions text (either an image asset or programmatic SKLabelNodes)
  - "Click here to continue!" label at bottom-right

mouseDown: transition back to TitleScene
```

---

## Phase 7 — Highscore Scene (`Scenes/HighscoreScene.swift`)

**New file**: `Quadromania/Scenes/HighscoreScene.swift`

Port of `Graphics_ListHighscores()`.

```
Init(table: Int, manager: HighscoreManager):
  - Background + frame + title logo
  - "High scores for Level N" label
  - 10 rows: name (left) + score (right)

mouseDown: transition back to TitleScene
```

---

## Phase 8 — Graphics Assets

The old C code uses SDL sprite sheets loaded from `data/`. These need to exist as named assets in `Assets.xcassets` or as files in the bundle.

Required assets (from `renderer.c`):
| Asset | Description | Usage |
|---|---|---|
| `dots` | Horizontal strip of 5 colored circle sprites | One per color (0–4) |
| `frame` | Outer wooden border frame | Full-screen overlay |
| `texture` | 10 tiled background textures in a strip | Background fill |
| `title` | Title logo image | Menu + screens |
| `copyright` | Copyright/credits image | Below title on menu |
| `instructions` | Instructions diagram image | Instructions screen |

**Action needed**: Locate or recreate these image assets. The old `data/` directory (not in `old_src/`) likely contains the original SDL PNGs. They can be imported directly into `Assets.xcassets` as image sets; the sprite sheets (dots, textures) should be sliced into individual named images or loaded via `SKTextureAtlas`.

---

## Phase 9 — Cleanup

Remove or repurpose legacy files once all scenes are implemented:

- Delete `GameScene.swift` (replace with `GamePlayScene.swift`)
- Delete `GameScene.sks`
- Delete `ViewController.swift` (storyboard path) — or keep but wire to `GameViewController`
- Update `Main.storyboard` to point to `GameViewController` as the window's content view controller
- Delete `old_src/` from the repository (keep as archive branch if desired)

---

## Implementation Order

1. `GameModel.swift` — no dependencies, testable in isolation
2. `HighscoreManager.swift` — no dependencies
3. `TileGridNode.swift` — depends on assets being imported
4. `TitleScene.swift` — depends on assets + `GameModel`
5. `GamePlayScene.swift` — depends on `GameModel` + `TileGridNode`
6. `InstructionsScene.swift` — depends on assets
7. `HighscoreScene.swift` — depends on `HighscoreManager`
8. Wire `TitleScene` → `GamePlayScene` → `HighscoreScene` transitions
9. Cleanup legacy files

---

## What Already Works (Do Not Change)

- `AppDelegate.swift` — Sound menu, music start
- `SoundManager.swift` — All audio playback, `SoundEffect` enum
- `GameViewController.swift` — SKView creation, scene presentation
- `sounds/` — All audio assets in place

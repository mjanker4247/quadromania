# Title Screen Redesign — Design Spec

## Overview

Remove all interactive UI from `TitleScene`, move game settings into the Game menu bar, and replace the title screen with an animated decorative playfield.

---

## Section 1 — TitleScene

### Layout

- **"QUADROMANIA"** — large title text, centered horizontally, positioned in the upper third of the canvas (~y=700 in 1280×960 coordinate space).
- **"Puzzle Game"** — small subtitle text, centered below the title.
- All existing interactive elements are removed: New Game / Instructions / Highscores / Quit buttons, the 2×2 palette swatch grid (`paletteContainerNodes`/`paletteBorderNodes`), the color dot row, and the level selector stepper.

### Decorative grid

- **Size**: 16 columns × 8 rows of frosted gem circles.
- **Tile style**: `SKShapeNode(circleOfRadius: 22)` — same frosted highlight child as `TileGridNode`. Cell pitch: 50 px (diameter 44 + 6 px gap). Total grid: ~800×400 px.
- **Position**: center the grid at approximately (640, 300) in scene coordinates (scene is 1280×960, origin bottom-left), so the grid occupies roughly y=100 to y=500.
- **No click handling** — purely decorative.

### Diagonal colour wave animation

- `TitleScene` maintains a `wavePhase: Double` advancing at **0.8 units/second** in `update(_ currentTime:)`.
- Each tile at `(col, row)` tracks its current displayed `colorIndex`. On each `update` tick, compute the target index:

  ```swift
  let n = palette.colors.count  // always 5
  let raw = Int(wavePhase) - (col + row)
  let targetIndex = ((raw % n) + n) % n  // always non-negative
  ```

  `Int(wavePhase)` truncates monotonically from 0, so it is always non-negative and safe. The wave advances in discrete integer steps; `wavePhase` increments by `delta * 0.8` per frame, so each step lasts ~1.25 seconds. When `targetIndex != colorIndex`, the tile runs a `colorTransition` action (same `SKAction.customAction` + `lerpColor` approach as `TileGridNode`) with key `"colorTransition"`, replacing any in-flight transition.

- Observes `paletteDidChange` and `customPaletteDidChange`; forces a full color refresh by resetting all `colorIndex` values to -1 on the next `update` tick so every tile re-animates.

---

## Section 2 — Game Menu Restructure

### Full menu bar (unchanged menus preserved)

| Menu | Items |
|------|-------|
| **Sound** | Start/Stop Music (unchanged) |
| **Game** | See structure below |
| **Palette** | 🌸 Spring / 🌊 Ocean / 🌅 Sunset / 🌿 Forest / 🎨 Custom / Edit Custom… / — / Color Symbols (unchanged) |

### Game menu structure (separators only, no section labels)

```
New Game          ⌘N
─────────────────
Colors             ▶
Transition         ▶
─────────────────
Difficulty         ▶
─────────────────
Instructions
```

### Colors submenu

Radio group: **2 Colors / 3 Colors / 4 Colors / 5 Colors**. Checkmark on the active value.

The stored value is the **user-facing count** (2–5). Before constructing `GameModel`, subtract 1 to get the `maxColors` parameter (range 1–4), matching the existing `GameModel(level:maxColors:)` convention.

### Transition submenu

Radio group: **Ring Sweep / Sequential / Radial Pulse** (existing, unchanged).

### Difficulty submenu

Radio group: **Beginner / Intermediate / Expert**. The exact `selectedLevel` stored per tier:

| Label | selectedLevel |
|-------|--------------|
| Beginner | 1 |
| Intermediate | 5 |
| Expert | 10 |

These match the values already used in the existing `TitleScene` difficulty picker.

### Settings ownership

`AppDelegate` is the single source of truth. Add a shared accessor at the top of `AppDelegate`:

```swift
static var shared: AppDelegate!
```

Set it in `applicationDidFinishLaunching`:

```swift
AppDelegate.shared = self
```

Use implicitly unwrapped optional (`!`) rather than `weak` — `AppDelegate` is owned by the Cocoa application object and lives for the full app lifetime, so `weak` would leave the reference dangling after ARC does its bookkeeping. Force-unwrapping a `weak` optional is unsafe; an IUO strong reference is the correct pattern here.

Then add two `UserDefaults`-backed properties with the following keys:

- `var selectedColors: Int` — `UserDefaults` key `"selectedColors"`, default `2` (user-facing count). **No migration needed**: `selectedColors` was previously a local instance variable in `TitleScene`, not persisted to `UserDefaults`, so there is no existing stored data to conflict with.
- `var selectedLevel: Int` — `UserDefaults` key `"selectedLevel"`, default `5` (Intermediate). Same: was local, not persisted.

All setting changes are saved to `UserDefaults` immediately when selected, regardless of whether a new game is started.

### Notification names

Add the following to the `extension Notification.Name` block in `AppDelegate.swift`:

```swift
static let newGameRequested = Notification.Name("newGameRequested")
static let colorsDidChange = Notification.Name("colorsDidChange")
static let difficultyDidChange = Notification.Name("difficultyDidChange")
```

The existing names (`.paletteDidChange`, `.symbolOverlayDidChange`, `.showInstructions`, `.transitionStyleDidChange`, `.customPaletteDidChange`) are unchanged.

### Notifications posted

| Action | Notification |
|--------|-------------|
| New Game selected | `.newGameRequested` |
| Colors item selected | `.colorsDidChange` |
| Difficulty item selected | `.difficultyDidChange` |
| Instructions selected | `.showInstructions` (existing) |
| Transition item selected | `.transitionStyleDidChange` (existing) |

---

## Section 3 — New Game Flow & Mid-Game Dialog

### GamePlayScene initialiser — unchanged

`GamePlayScene.init(model:palette:size:)` keeps its existing signature. This preserves the `InstructionsScene` → `GamePlayScene` return path, which passes the live in-progress `GameModel` back into a new `GamePlayScene` instance.

`TitleScene` is responsible for reading settings from `AppDelegate.shared` and constructing the `GameModel` before calling the init:

```swift
let model = GameModel(level: AppDelegate.shared.selectedLevel,
                      maxColors: AppDelegate.shared.selectedColors - 1)
let scene = GamePlayScene(model: model, palette: currentPalette, size: size)
view?.presentScene(scene, transition: ...)
```

`GamePlayScene` itself does not read from `AppDelegate` at init time. It only reads from `AppDelegate.shared` inside `startNewGame()` (see below).

### TitleScene observers

`TitleScene` observes `.newGameRequested`. On receipt it reads settings from `AppDelegate.shared`, constructs a `GameModel`, and transitions to `GamePlayScene` as shown above.

`TitleScene` also continues to observe `.paletteDidChange` and `.customPaletteDidChange` (existing behaviour, palette sync for decorative grid).

### From GamePlayScene (game in progress)

"Game in progress" means the game is active and has not yet reached a win or loss state (`waitingForClick == false`). When the game is already over (win/loss overlay visible, `waitingForClick == true`), all setting-change notifications call `startNewGame()` immediately without any dialog.

| Notification | Active game behaviour | Game-over behaviour |
|-------------|----------------------|---------------------|
| `.newGameRequested` | Show alert | Call `startNewGame()` immediately |
| `.colorsDidChange` | Show alert | Call `startNewGame()` immediately |
| `.difficultyDidChange` | Show alert | Call `startNewGame()` immediately |
| `.transitionStyleDidChange` | No dialog | No dialog |

### NSAlert

Use `beginSheetModal(for:completionHandler:)` (non-blocking, correct for SpriteKit):

```swift
let alert = NSAlert()
alert.messageText = "Start a new game?"
alert.informativeText = "…" // describes what changed
alert.addButton(withTitle: "New Game")
alert.addButton(withTitle: "Continue")
alert.beginSheetModal(for: view!.window!) { response in
    if response == .alertFirstButtonReturn {
        self.startNewGame()
    }
}
```

**"New Game"** calls `startNewGame()`. **"Continue"** does nothing — the setting is already saved.

### `mouseDown` win/loss click behaviour

The current `mouseDown` handler in `GamePlayScene` calls `returnToTitle()` when `waitingForClick == true`. This branch must be changed: **clicking after win/loss now calls `startNewGame()` instead of returning to `TitleScene`**. The title screen no longer receives returning players via this path — game over always restarts in place.

### `startNewGame()` contract

`startNewGame()` is an in-place reset within the existing `GamePlayScene` instance (no scene transition):

1. Read updated settings from `AppDelegate.shared`.
2. Construct a new `GameModel`.
3. Rebuild `TileGridNode` (or call a reset method on it) with the new model.
4. Reset HUD labels (turn count, score display) to their initial state.
5. Remove any win/loss overlay nodes.
6. Set `waitingForClick = false`.

### InstructionsScene context preserved

`GamePlayScene` continues to pass its live `GameModel` and palette to `InstructionsScene` when `.showInstructions` is received. `InstructionsScene` uses these to reconstruct `GamePlayScene(model:palette:size:)` on "Back" — this path is unaffected because `GamePlayScene.init` retains the `model:` parameter. No change to `InstructionsScene.init(size:sourceGame:sourcePalette:)` — note `size:` is the first parameter label.

---

## Out of Scope

- **Highscore scene**: Will not be used any longer. Remove reference and code completely.
- **Tutorial**: remains accessible via Instructions → Start Tutorial (preserved).
- Any changes to `GamePlayScene` gameplay logic, `TileGridNode`, scoring, or the Palette/Sound menus.

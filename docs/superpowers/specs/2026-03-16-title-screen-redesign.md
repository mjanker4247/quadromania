# Title Screen Redesign — Design Spec

## Overview

Remove all interactive UI from `TitleScene`, move game settings into the Game menu bar, and replace the title screen with an animated decorative playfield.

---

## Section 1 — TitleScene

### Layout

- **"QUADROMANIA"** — large title text, centered horizontally, positioned in the upper third of the canvas (~y=700 in 1280×960 coordinate space).
- **"Puzzle Game"** — small subtitle text, centered below the title.
- All existing buttons (New Game, Instructions, Highscores, Quit), color dot grid, and level selector are removed.

### Decorative grid

- **Size**: 16 columns × 8 rows of frosted gem circles (same `SKShapeNode(circleOfRadius:)` style as `TileGridNode`).
- **Cell size**: ~50 px per tile with a small gap; total grid approximately 830×420 px, centered in the lower two-thirds of the canvas.
- **No click handling** — purely decorative.

### Diagonal colour wave animation

- `TitleScene` maintains a `wavePhase: Double` that advances at **0.8 units/second** in `update(_ currentTime:)`.
- Each tile at `(col, row)` displays `palette.colors[Int((wavePhase - Double(col + row)).truncatingRemainder ... ) % palette.count]` — diagonal stripes of colour that ripple from top-left to bottom-right.
- Colour transitions use the same `lerpColor(from:to:t:)` approach as `TileGridNode` for smooth blending between wave steps.
- The grid re-reads palette colours on every `update` tick — no rebuild needed when the palette changes.
- Observes `paletteDidChange` and `customPaletteDidChange` to pick up the new palette immediately.

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

### Transition submenu

Radio group: **Ring Sweep / Sequential / Radial Pulse** (existing behaviour, unchanged).

### Difficulty submenu

Radio group: **Beginner / Intermediate / Expert**, mapping to level ranges:
- Beginner → levels 1–3
- Intermediate → levels 4–7
- Expert → levels 8–10

The actual level used is the midpoint of the selected range (or a fixed representative level per tier).

### Settings ownership

`AppDelegate` is the single source of truth:

- `var selectedColors: Int` — `UserDefaults`-backed, default 2.
- `var selectedLevel: Int` — `UserDefaults`-backed, default 5 (Intermediate).

All setting changes are saved to `UserDefaults` immediately when selected, regardless of whether a new game is started.

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

### From TitleScene (no active game)

`.newGameRequested` → `TitleScene` transitions directly to `GamePlayScene`, reading `selectedColors` and `selectedLevel` from `AppDelegate`. No dialog shown.

### From GamePlayScene (game in progress)

| Notification | Behaviour |
|-------------|-----------|
| `.newGameRequested` | NSAlert: "Start a new game?" — buttons: **New Game** / **Continue** |
| `.colorsDidChange` | NSAlert: "Start a new game?" with message explaining color count changed — same buttons |
| `.difficultyDidChange` | NSAlert: "Start a new game?" with message explaining difficulty changed — same buttons |
| `.transitionStyleDidChange` | No dialog — transition style applies silently (animation only, no effect on puzzle state) |

**NSAlert behaviour**: choosing **New Game** restarts immediately with the updated settings. Choosing **Continue** keeps the current game running. In both cases the setting change is already persisted.

### GamePlayScene reads settings from AppDelegate

`GamePlayScene` no longer owns `selectedColors` or `selectedLevel`. It reads from `AppDelegate` at game start.

---

## Out of Scope

- Highscore scene entry point (will be added via a separate menu or retained via keyboard shortcut in a future iteration).
- Tutorial entry point (remains accessible via Instructions → Start Tutorial, which is preserved).
- Any changes to `GamePlayScene` gameplay logic, `TileGridNode`, or scoring.

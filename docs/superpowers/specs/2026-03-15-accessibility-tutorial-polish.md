# Accessibility, Tutorial & Polish Design

**Goal:** Fix the palette chooser bug, add palette selection to the macOS menu bar, add a symbol overlay for colorblind accessibility, rewrite the instructions text, update docs, and add a scripted tutorial mode accessible from the Instructions screen.

---

## 1. Palette Chooser Bug Fix

`paletteHit(at:)` in `TitleScene` uses `SKNode.frame.contains(point)` on bare `SKNode` containers. `SKNode.frame` is the accumulated bounding box of children in parent-local coordinates, which is unreliable for hit-testing in scene coordinates when the container has a non-zero position.

**Fix:** Replace with an explicit hit rect computed from `container.position` and known layout constants:

```swift
private func paletteHit(at point: CGPoint) -> TilePalette? {
    let halfW = stripWidth / 2 + 4
    let halfH = swatchSize / 2 + 4
    for (palette, node) in paletteContainerNodes {
        let rect = CGRect(
            x: node.position.x - halfW, y: node.position.y - halfH,
            width: halfW * 2,           height: halfH * 2
        )
        if rect.contains(point) { return palette }
    }
    return nil
}
```

The constants `stripWidth = 106` and `swatchSize = 18` are already local to `addPaletteGrid()`; they must be promoted to `private let` properties on `TitleScene` so `paletteHit` can reference them.

---

## 2. Menu Bar Palette Selection

**`AppDelegate`** gains a `"Palette"` `NSMenu` inserted after the `"Sound"` menu. It contains four `NSMenuItem`s, one per `TilePalette` case, each with:
- Title: palette `displayName` (e.g., "🌸 Spring")
- `state`: `.on` for the active palette, `.off` for others (radio-button behaviour managed manually)
- Action: `selectPalette(_:)` on `AppDelegate`

`AppDelegate` stores:
```swift
var activePalette: TilePalette = .spring
```

On selection, `AppDelegate`:
1. Updates `activePalette`
2. Refreshes all menu item states
3. Posts `Notification.Name("paletteDidChange")` with `userInfo: ["palette": activePalette.rawValue]`

**Observers:**
- `TitleScene.didMove(to:)` registers for `paletteDidChange`; on receipt calls `selectPalette(_:)` with the new value and deregisters in `deinit`.
- `GamePlayScene` registers and calls a new `applyPalette(_:)` method on `TileGridNode` that recolors all tiles immediately (without the entrance animation).

`TileGridNode` gains:
```swift
func applyPalette(_ palette: TilePalette) {
    self.palette = palette
    updateAll(animated: false)
}
```

---

## 3. Symbol Overlay (Colorblind Accessibility)

**`TileGridNode`** gains:
```swift
var symbolOverlayEnabled: Bool = false {
    didSet { updateSymbols() }
}
```

Five Unicode symbols map to tile color indices 0–4:

| Index | Symbol |
|-------|--------|
| 0     | ●      |
| 1     | ■      |
| 2     | ▲      |
| 3     | ◆      |
| 4     | ★      |

`buildGrid` adds a child `SKLabelNode` (name `"symbol"`) to every tile sprite. Properties: `fontName = "Helvetica-Bold"`, `fontSize = 14`, `fontColor = SKColor(white: 0, alpha: 0.6)`, `verticalAlignmentMode = .center`, `horizontalAlignmentMode = .center`. Initially hidden.

`updateAll(animated:)` updates each symbol label's text to match the current color index.

`updateSymbols()` iterates all tile sprites and sets `label.isHidden = !symbolOverlayEnabled`.

**Menu bar** — `AppDelegate` appends a separator and a `"Color Symbols"` `NSMenuItem` to the Palette menu. It has a checkmark toggle. Toggling posts `Notification.Name("symbolOverlayDidChange")` with `userInfo: ["enabled": Bool]`. `GamePlayScene` observes and flips `tileGrid.symbolOverlayEnabled`.

`TitleScene` color dots are not affected (decorative, non-interactive).

---

## 4. Tutorial Mode

### Entry point

`InstructionsScene` gains a **"Start Tutorial"** `SKLabelNode` button at the bottom of the scene. Clicking it presents `TutorialScene(size: size)` with a `.fade(withDuration: 0.3)` transition.

### TutorialScene

A new scene in `Quadromania/Scenes/TutorialScene.swift`.

**Grid:** 5 columns × 4 rows, `maxColors = 1` (only colors 0 and 1). Uses a hardcoded initial playfield and a matching hardcoded step sequence — no `PuzzleGenerator`.

**Data structures:**
```swift
private struct TutorialStep {
    let message: String
    let targetCol: Int
    let targetRow: Int
}
```

**Layout:**
- Title label: "Tutorial" at top, y ≈ 880
- `TileGridNode` (small grid, ~60px tiles): centered horizontally, y ≈ 580–780
- Message panel: rounded `SKShapeNode` below grid, y ≈ 460, contains an `SKLabelNode` (multiline via `numberOfLines = 0`, `preferredMaxLayoutWidth`)
- Pulse ring: `SKShapeNode` circle drawn around the target tile, runs `SKAction.sequence([.scale(to: 1.15, duration: 0.4), .scale(to: 1.0, duration: 0.4)]).repeatForever()`
- "Skip Tutorial" button: top-right corner, y ≈ 900

**Interaction:**
- `mouseDown`: convert point to grid col/row. If it matches current step's `targetCol/targetRow`: advance step (remove ring, apply rotation to `GameModel`, update `TileGridNode`, show next message, place ring on next target). If wrong tile: flash message panel red briefly (`SKAction.sequence([colorize red, colorize clear])`).
- On final step completion: remove ring, show "You're ready to play! Good luck." in message panel, show "Back to Menu" button.
- "Skip Tutorial" and "Back to Menu" both present a fresh `TitleScene`.

**Hardcoded content (example — exact moves determined during implementation):**

| Step | Message |
|------|---------|
| 1 | "Welcome! Your goal: turn all tiles to the lightest color. Click the highlighted tile to rotate the 3×3 block around it." |
| 2 | "Every tile in that block just incremented by one color. Click the next highlighted tile." |
| 3 | "Notice how tiles at the maximum color wrap back to 0. Keep going!" |
| 4 | "Last move — solve the board!" |

---

## 5. Instructions Text

Replace current `InstructionsScene` body text with:

**Goal**
Return every tile to its lightest color (color 0) before you run out of turns.

**How to play**
Click any tile to rotate the 3×3 block centered on it. Every tile in that block steps forward by one color. Tiles at the maximum color wrap back to 0.

**Turn limit**
Your total turns are shown at the top of the screen. Each click costs one turn — plan carefully.

**Scoring**
Solve the board faster to score higher. If you exceed the turn limit, the game is over with no score recorded.

**Tip**
Start by clicking tiles in the middle of large same-color regions to make progress efficiently.

---

## 6. CLAUDE.md & README

**CLAUDE.md** changes:
- Source Files table: mark all files as created, add `TilePalette.swift`, add `TutorialScene.swift`
- Add `QuadroCore` static library section (target name, what it contains, why it exists)
- Update "no unit tests" note to reflect the `QuadroTests` XCTest target
- Remove all "to be created" notes

**README.md** (new file):
- Project name and one-sentence description
- Build instructions (Xcode and `xcodebuild` command from CLAUDE.md)
- How to play (same 4 points as instructions text above)
- Palette and accessibility options

---

## Files

| Action | File |
|--------|------|
| Modify | `Quadromania/TitleScene.swift` — promote layout constants, fix `paletteHit` |
| Modify | `Quadromania/TileGridNode.swift` — add `symbolOverlayEnabled`, symbol labels, `applyPalette` |
| Modify | `Quadromania/GamePlayScene.swift` — observe `paletteDidChange` + `symbolOverlayDidChange` |
| Modify | `Quadromania/AppDelegate.swift` — add Palette menu + Color Symbols toggle |
| Modify | `Quadromania/Scenes/InstructionsScene.swift` — new text, "Start Tutorial" button |
| Create | `Quadromania/Scenes/TutorialScene.swift` — new tutorial scene |
| Modify | `CLAUDE.md` — update to reflect current state |
| Create | `README.md` — new project readme |

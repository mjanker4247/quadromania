# Accessibility, Tutorial & Polish Design

**Goal:** Fix the palette chooser bug, add palette selection to the macOS menu bar, add a symbol overlay for colorblind accessibility, rewrite the instructions text, update docs, and add a scripted tutorial mode accessible from the Instructions screen.

---

## 1. Palette Chooser Bug Fix

`paletteHit(at:)` in `TitleScene` uses `SKNode.frame.contains(point)` on bare `SKNode` containers. Although these containers are direct children of the scene (so `.frame` is in scene coordinates), the accumulated bounding box of their children can be fragile — floating-point accumulation across multiple `SKSpriteNode` children with sub-pixel positions means the reported frame may not align perfectly with the intended hit area. Replacing with an explicit rect computed from known constants is more robust.

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
   — `activePalette.rawValue` is an `Int` (the `TilePalette` enum is `RawRepresentable` with `Int` raw values)

**Observers:**

Use the selector-based `NotificationCenter.default.addObserver(_:selector:name:object:)` API for both scenes. Deregister with `NotificationCenter.default.removeObserver(self)` in `willMove(from:)` for both `TitleScene` and `GamePlayScene` (consistent SpriteKit lifecycle — `deinit` timing is unreliable for scene objects).

`TitleScene.didMove(to:)` registers for `paletteDidChange`. On receipt, guard-unwrap the palette:
```swift
guard let raw = notification.userInfo?["palette"] as? Int,
      let palette = TilePalette(rawValue: raw) else { return }
selectPalette(palette)
```

`GamePlayScene.didMove(to:)` registers for both `paletteDidChange` and `symbolOverlayDidChange`. On `paletteDidChange`, same guard pattern then call `tileGrid.applyPalette(palette)`. On `symbolOverlayDidChange`, guard-unwrap `userInfo["enabled"] as? Bool` then set `tileGrid.symbolOverlayEnabled`.

On `paletteDidChange`, `GamePlayScene` calls `tileGrid.applyPalette(_:)` (see below).
On `symbolOverlayDidChange`, `GamePlayScene` reads `userInfo["enabled"] as! Bool` and sets `tileGrid.symbolOverlayEnabled`.

**`TileGridNode` changes required for `applyPalette`:**

1. Change `private let palette: TilePalette` → `private var palette: TilePalette` (must be `var` for mutation).
2. Add `private var currentPlayfield: [[Int]] = []`. Set this in `buildGrid(playfield:)` at the start: `currentPlayfield = playfield`. Also update it at the top of `updateAll(from:)`: `currentPlayfield = playfield`.
3. Add:
```swift
func applyPalette(_ newPalette: TilePalette) {
    palette = newPalette
    updateAll(from: currentPlayfield)   // recolor all tiles using stored playfield
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

`buildGrid` adds a child `SKLabelNode` (name `"symbol"`) to every tile sprite. Properties: `fontName = "Helvetica-Bold"`, `fontSize = 14`, `fontColor = SKColor(white: 0, alpha: 0.6)`, `verticalAlignmentMode = .center`, `horizontalAlignmentMode = .center`. Initially `isHidden = true`.

`updateAll(from:)` (after updating tile colors) iterates tiles and updates each symbol label's text to `symbols[colorIndex]` where `symbols = ["●","■","▲","◆","★"]`.

`updateSymbols()` iterates all tile sprites and sets the child `SKLabelNode` (name `"symbol"`) `isHidden = !symbolOverlayEnabled`.

**Menu bar** — `AppDelegate` appends a separator and a `"Color Symbols"` `NSMenuItem` to the Palette menu. It has a checkmark toggle (`state = .on`/`.off`). Toggling posts `Notification.Name("symbolOverlayDidChange")` with `userInfo: ["enabled": Bool]`. `GamePlayScene` observes and flips `tileGrid.symbolOverlayEnabled`.

`TitleScene` color dots are not affected (decorative, non-interactive).

`TileGridNode.symbolOverlayEnabled` initialises to `false`. When `GamePlayScene` creates a `TileGridNode`, it should read the initial value from `AppDelegate`'s stored state. Add `var symbolOverlayEnabled: Bool = false` to `AppDelegate`, toggled by the menu item. `GamePlayScene.buildUI()` sets `tileGrid.symbolOverlayEnabled = (NSApp.delegate as? AppDelegate)?.symbolOverlayEnabled ?? false` after creating the grid.

---

## 4. Tutorial Mode

### Entry point

`InstructionsScene` gains a **"Start Tutorial"** `SKLabelNode` button at the bottom of the scene. The existing `mouseDown` handler unconditionally returns to `TitleScene` on any click — this must be replaced with hit-tested routing:

```swift
override func mouseDown(with event: NSEvent) {
    let point = event.location(in: self)
    if let label = tutorialButtonLabel, label.frame.contains(point) {
        let scene = TutorialScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    } else {
        let scene = TitleScene()
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }
}
```

`tutorialButtonLabel` is a `private var SKLabelNode?` property set when the button is created.

### TutorialScene

A new scene in `Quadromania/Scenes/TutorialScene.swift`.

**State:** `TutorialScene` manages its own 5×4 playfield as a plain `[[Int]]` value — it does **not** use `GameModel` (which is hardcoded for 18×13) or `PuzzleGenerator`. It directly mutates a `private var tutorialPlayfield: [[Int]]` using the same rotation algorithm as `GameModel.rotate(col:row:)`.

Declare `private let tutorialMaxColors = 1` (only colors 0 and 1). The rotation algorithm — identical to `GameModel` — increments every cell `(x, y)` where `abs(x - col) <= 1 && abs(y - row) <= 1`, clamped to grid bounds, then applies `value = value > tutorialMaxColors ? 0 : value` (same `if value > maxColors { value = 0 }` branch as `GameModel.applyRotate`). Do not use `% (maxColors + 1)` — replicate the exact `>` comparison to avoid off-by-one.

**Grid rendering:** `TutorialScene` does **not** use `TileGridNode` (which is hardcoded for 18×13 at 64px tiles). Instead it manages its own 5×4 grid of `SKSpriteNode` tiles directly, using a `tileSize` of `60`. The grid is built in `buildTutorialGrid()` and updated by iterating the sprites in a `private var tileSprites: [[SKSpriteNode]]` (indexed `[col][row]`). Palette used: `.spring` (hardcoded — the tutorial is a standalone demonstration).

**Data structures:**
```swift
private struct TutorialStep {
    let message: String
    let targetCol: Int   // 0-based; valid range 1–3 (ensures 3×3 block fits within 5-wide grid)
    let targetRow: Int   // 0-based; valid range 1–2 (ensures 3×3 block fits within 4-tall grid)
}
```

**Layout:**
- Title label: "Tutorial" at top, y = 880
- Tile grid (5×4 at 60px): centered horizontally at x = 640; bottom edge at y = 580, top edge at y = 820 (60px × 4 rows + gaps)
- Message panel: rounded `SKShapeNode` rect below grid, centred at y = 460, width = 700, height = 120; contains a multiline `SKLabelNode` (`numberOfLines = 0`, `preferredMaxLayoutWidth = 660`)
- Pulse ring: `SKShapeNode` circle (radius ≈ 36) positioned at the target tile's scene position, running `SKAction.sequence([.scale(to: 1.15, duration: 0.4), .scale(to: 1.0, duration: 0.4)]).repeatForever()`
- "Skip Tutorial" button: top-right, position ≈ (1180, 920)

**Wrong-tile flash:** The message panel `SKShapeNode` has `fillColor = .clear` normally. On wrong click, run a custom action via `SKAction.customAction(withDuration: 0.3) { node, t in (node as! SKShapeNode).fillColor = t < 0.15 ? SKColor.red.withAlphaComponent(0.3) : .clear }`.

**Interaction:**
- `mouseDown`: compute which col/row was clicked from the tile grid's origin and `tileSize`. If `(col, row)` matches current step's `(targetCol, targetRow)`: apply rotation to `tutorialPlayfield`, refresh `tileSprites` colors, advance to next step. If wrong tile: flash panel. If beyond last step: unreachable by construction.
- On final step completion: remove ring, update message to "You're ready to play! Good luck.", show "Back to Menu" button.
- "Skip Tutorial" and "Back to Menu" both present a fresh `TitleScene`.
- "Back to Menu" button: centered horizontally at x = 640, y = 80 (bottom of scene). Same style as "Skip Tutorial" (`SKLabelNode`, `Helvetica-Bold`, `fontSize = 28`, `normalColor`).

**Hardcoded content (example — exact moves determined during implementation):**

| Step | Message |
|------|---------|
| 1 | "Welcome! Your goal: turn all tiles to the lightest color. Click the highlighted tile to rotate the 3×3 block around it." |
| 2 | "Every tile in that block just incremented by one color. Click the next highlighted tile." |
| 3 | "Notice how tiles at the maximum color wrap back to 0. Keep going!" |
| 4 | "Last move — solve the board!" |

The hardcoded `tutorialPlayfield` and step sequence must be designed so that executing each step in order produces all-zero tiles after the final step. Determine the exact playfield + moves during implementation by constructing a solved board and applying the inverse of each planned move in reverse.

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
- Source Files table: mark all files as created, mark `TilePalette.swift` as created (it already exists), add `TutorialScene.swift`
- Add `QuadroCore` static library section (target name, what it contains, why it exists)
- Update "no unit tests" note to reflect the `QuadroTests` XCTest target with 19 tests
- Remove all "to be created" notes

**README.md** (new file, explicitly requested by user):
- Project name and one-sentence description
- Build instructions (Xcode and `xcodebuild` command from CLAUDE.md)
- How to play (same 4 points as instructions text above)
- Palette and accessibility options

---

## Files

| Action | File |
|--------|------|
| Modify | `Quadromania/TitleScene.swift` — promote layout constants to `private let`, fix `paletteHit`, observe `paletteDidChange` |
| Modify | `Quadromania/TileGridNode.swift` — `palette` → `var`, cache `currentPlayfield`, add `applyPalette`, add symbol labels, add `symbolOverlayEnabled` |
| Modify | `Quadromania/GamePlayScene.swift` — observe `paletteDidChange` + `symbolOverlayDidChange`, deregister in `willMove(from:)` |
| Modify | `Quadromania/AppDelegate.swift` — add Palette menu + Color Symbols toggle |
| Modify | `Quadromania/Scenes/InstructionsScene.swift` — new text, "Start Tutorial" button, hit-tested `mouseDown` |
| Create | `Quadromania/Scenes/TutorialScene.swift` — standalone tutorial scene with own 5×4 grid state |
| Modify | `CLAUDE.md` — update to reflect current state |
| Create | `README.md` — new project readme (explicitly requested) |

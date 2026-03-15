# Gameplay UX Polish — Design Spec
**Date:** 2026-03-15
**Status:** Draft

## Overview

Five focused improvements to Quadromania's gameplay UX:
1. Colour cycle strip above the game field
2. Difficulty renamed to Beginner / Intermediate / Expert
3. Instructions accessible from macOS menu bar with context-aware back navigation
4. (Already implemented) Palette change propagates to running game
5. Remove highscores completely

---

## 1. Colour Cycle Strip

### What
A horizontal row of coloured squares rendered above the tile grid in `GamePlayScene`, showing the full rotation cycle for the current game. The strip shows all `model.maxColors + 1` colours (indices 0 through `model.maxColors`) left-to-right with `→` arrows between them and a `↩` loop arrow at the right end.

### Layout
- Grid dimensions: `tileSize = 64`, `gridPixelWidth = 18 × 64 = 1152`, `gridPixelHeight = 13 × 64 = 832`
- Grid bottom: `gridY = 50` → grid top: `y = 882`
- Strip centre Y: `gridY + TileGridNode.gridPixelHeight + 10 + 14 = 906` (10 px gap, 14 = half of 28 px swatch height)
- Swatches: 28 × 28 px `SKSpriteNode`, one per colour index
- Arrows: `SKLabelNode` with text `"→"`, fontSize 16, between consecutive swatches
- Loop arrow: `SKLabelNode` with text `"↩"` after the last swatch
- Entire strip centred horizontally over the grid: `x = gridX + TileGridNode.gridPixelWidth / 2`

### State
```swift
private var colorSwatchNodes: [SKSpriteNode] = []
```
Built in `buildColorStrip()` called from `buildUI()` after placing the tile grid.

### Palette updates
`handlePaletteDidChange(_:)` already calls `tileGrid.applyPalette(palette)`. It additionally iterates `colorSwatchNodes` and updates each sprite's `color` to `newPalette.colors[index]`.

---

## 2. Difficulty Levels — Beginner / Intermediate / Expert

### Mapping
| Name         | Level | Initial rotations |
|--------------|-------|-------------------|
| Beginner     | 1     | 186               |
| Intermediate | 5     | 126               |
| Expert       | 10    | 69                |

### TitleScene changes
- `selectedLevel` still an `Int`, valid values restricted to `[1, 5, 10]`
- Two private constants:
  ```swift
  private let difficultyLevels = [1, 5, 10]
  private let difficultyNames: [Int: String] = [1: "Beginner", 5: "Intermediate", 10: "Expert"]
  ```
- Menu item label: `"Select difficulty:"` (was `"Select level:"`)
- `updateDynamicLabels()` displays `difficultyNames[selectedLevel]!` (e.g. `"Beginner"`) instead of `"Level N (X rotations)"`
- Cycling on click: find current index in `difficultyLevels`, advance modulo 3
  ```swift
  let idx = (difficultyLevels.firstIndex(of: selectedLevel) ?? 0 + 1) % 3
  selectedLevel = difficultyLevels[idx]
  ```
- Default: `selectedLevel = 1` (Beginner) — no change to initialisation

---

## 3. Instructions from macOS Menu Bar + Context-Aware Back

### Menu bar addition
`AppDelegate.applicationDidFinishLaunching` calls `buildGameMenu()` (new, called before `buildPaletteMenu()`).

`buildGameMenu()` creates an `NSMenu` titled `"Game"` with one item:
- Title: `"Instructions"`, action: `#selector(showInstructionsMenuAction(_:))`, key equivalent: `""`

```swift
@objc private func showInstructionsMenuAction(_ sender: NSMenuItem) {
    NotificationCenter.default.post(name: .showInstructions, object: nil)
}
```

New notification name (added to the existing `Notification.Name` extension in `AppDelegate.swift`):
```swift
static let showInstructions = Notification.Name("showInstructions")
```

### InstructionsScene — new initialiser
```swift
init(size: CGSize, sourceGame: GameModel? = nil, sourcePalette: TilePalette = .spring) {
    self.sourceGame    = sourceGame
    self.sourcePalette = sourcePalette
    super.init(size: size)
}
private let sourceGame:    GameModel?
private let sourcePalette: TilePalette
```
`required init?(coder:)` is unchanged (fatal error).

### Back navigation
```swift
override func mouseDown(with event: NSEvent) {
    let point = event.location(in: self)
    if let btn = tutorialButtonLabel, btn.frame.contains(point) {
        // existing tutorial path
    } else if let model = sourceGame {
        let scene = GamePlayScene(model: model, palette: sourcePalette, size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    } else {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }
}
```

### Footer hint text
Adapts based on context:
- `sourceGame != nil` → `"Click anywhere to return to game"`
- `sourceGame == nil` → `"Click anywhere to return"`

### TitleScene — existing click path
The existing `.instructions` switch case in `TitleScene.mouseDown` is updated to use the new initialiser:
```swift
case .instructions:
    let scene = InstructionsScene(size: size)   // sourceGame: nil (default)
    scene.scaleMode = scaleMode
    view?.presentScene(scene, transition: .fade(withDuration: 0.2))
```
No change required here (default parameter `sourceGame: nil` covers it).

### TitleScene and GamePlayScene — notification observers
Both scenes add an observer for `.showInstructions` in `didMove(to:)` and remove it in `willMove(from:)`.

**TitleScene handler:**
```swift
@objc private func handleShowInstructions(_ notification: Notification) {
    let scene = InstructionsScene(size: size)
    scene.scaleMode = scaleMode
    view?.presentScene(scene, transition: .fade(withDuration: 0.2))
}
```

**GamePlayScene handler:**
```swift
@objc private func handleShowInstructions(_ notification: Notification) {
    let palette = (NSApp.delegate as? AppDelegate)?.activePalette ?? .spring
    let scene = InstructionsScene(size: size, sourceGame: model, sourcePalette: palette)
    scene.scaleMode = scaleMode
    view?.presentScene(scene, transition: .fade(withDuration: 0.2))
}
```

---

## 4. Palette Change During Running Game (Already Implemented)

`GamePlayScene` already observes `.paletteDidChange` and calls `tileGrid.applyPalette(_:)`. No new work required.

---

## 5. Remove Highscores

### Files to delete
- `Quadromania/HighscoreScene.swift`
- `Quadromania/HighscoreManager.swift`

### TitleScene
- Remove `.highscores` from `MenuItem` enum
- Remove `.highscores` from `itemY` dictionary
- Remove `.highscores` entry from `items` array in `addMenuLabels()`
- Remove `case .highscores:` from `mouseDown` switch
- Update `.quit` y-position: `menuStartY - 6 * lineSpacing` (was `- 7 *`)

### GamePlayScene
- Remove `recordHighscoreIfQualifies()` call from `showWin()`
- Remove `recordHighscoreIfQualifies()` function body
- Win overlay `subtext` keeps `"Score: \(model.score)"` — score feedback is still shown, just not persisted

---

## Files Affected

| File | Change |
|------|--------|
| `Quadromania/GamePlayScene.swift` | Add colour strip, observe `.showInstructions`, remove highscore recording |
| `Quadromania/TitleScene.swift` | Difficulty naming, observe `.showInstructions`, remove `.highscores` item |
| `Quadromania/InstructionsScene.swift` | New init with game context, context-aware back nav, footer text |
| `Quadromania/AppDelegate.swift` | Add `buildGameMenu()`, add `.showInstructions` notification name |
| `Quadromania/HighscoreScene.swift` | **Delete** |
| `Quadromania/HighscoreManager.swift` | **Delete** |

No changes to `TileGridNode.swift`, `QuadroCore/`, or test targets.

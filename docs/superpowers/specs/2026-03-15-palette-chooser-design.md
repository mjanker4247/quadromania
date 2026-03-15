# Palette Chooser Implementation Design

> **For agentic workers:** Use superpowers:executing-plans or superpowers:subagent-driven-development to implement this plan.

**Goal:** Add a 2×2 grid of clickable palette swatches to the TitleScene menu, letting the player pick one of four named color themes before starting a game.

**Architecture:** A new `TilePalette` enum in the app target owns all color definitions. `TitleScene` holds the selection and passes it down to `GamePlayScene`, which passes it to `TileGridNode`. No game logic changes — palettes are a pure display concern.

---

## Palettes

Four palettes, each with 5 `SKColor` values (index 0 = goal/win state):

| Key | Name | 0 goal | 1 | 2 | 3 | 4 |
|-----|------|--------|---|---|---|---|
| `spring` | 🌸 Spring | `#FACBD7` cherry blossom | `#85E1AB` mint | `#C3A9EF` lilac | `#FDEA6B` buttercup | `#80C8F8` sky blue |
| `ocean`  | 🌊 Ocean  | `#B8EDE4` seafoam | `#3A8FC7` ocean blue | `#1D7A68` deep teal | `#F47B6C` coral | `#F5C561` sandy gold |
| `sunset` | 🌅 Sunset | `#FDE68A` golden | `#FB923C` coral orange | `#F472B6` rose | `#A78BFA` violet | `#EF4444` warm red |
| `forest` | 🌿 Forest | `#D1E8C4` moss | `#2D7D3A` forest green | `#C4A882` mushroom | `#D4721E` autumn orange | `#8B5E3C` bark brown |

---

## Files

| Action | File | Change |
|--------|------|--------|
| Create | `Quadromania/TilePalette.swift` | New `enum TilePalette` with 4 cases, `colors: [SKColor]`, `displayName: String` |
| Modify | `Quadromania/TileGridNode.swift` | Remove `static let tileColors`; add `private let palette: TilePalette`; update `init` and `updateAll` |
| Modify | `Quadromania/TitleScene.swift` | Add `selectedPalette`; add 2×2 swatch grid; update `addMenuLabels` layout; update `addColorDots`; update `mouseDown`; pass palette to `GamePlayScene` |
| Modify | `Quadromania/GamePlayScene.swift` | Add `palette: TilePalette` parameter to `init`; pass to `TileGridNode` |

**Implementation order:** Create `TilePalette.swift` first. Then update `TileGridNode`, `TitleScene`, and `GamePlayScene` together before building — these three files form a compile dependency triangle: `TileGridNode.tileColors` is removed (breaking `TitleScene.addColorDots`), `TileGridNode.init` gains `palette:` (breaking `GamePlayScene.buildUI`), and `GamePlayScene.init` gains `palette:` (breaking `TitleScene.mouseDown`). All four files must be in a buildable state before running the compiler.

---

## TilePalette

```swift
// Quadromania/TilePalette.swift
import SpriteKit

enum TilePalette: Int, CaseIterable {
    case spring, ocean, sunset, forest

    var displayName: String {
        switch self {
        case .spring: return "🌸 Spring"
        case .ocean:  return "🌊 Ocean"
        case .sunset: return "🌅 Sunset"
        case .forest: return "🌿 Forest"
        }
    }

    var colors: [SKColor] {
        switch self {
        case .spring: return [
            SKColor(red: 0.98, green: 0.80, blue: 0.84, alpha: 1),  // 0 cherry blossom
            SKColor(red: 0.52, green: 0.88, blue: 0.67, alpha: 1),  // 1 mint
            SKColor(red: 0.76, green: 0.66, blue: 0.94, alpha: 1),  // 2 lilac
            SKColor(red: 0.99, green: 0.92, blue: 0.42, alpha: 1),  // 3 buttercup
            SKColor(red: 0.50, green: 0.78, blue: 0.97, alpha: 1),  // 4 sky blue
        ]
        case .ocean: return [
            SKColor(red: 0.72, green: 0.93, blue: 0.89, alpha: 1),  // 0 seafoam
            SKColor(red: 0.23, green: 0.56, blue: 0.78, alpha: 1),  // 1 ocean blue
            SKColor(red: 0.11, green: 0.48, blue: 0.41, alpha: 1),  // 2 deep teal
            SKColor(red: 0.96, green: 0.48, blue: 0.42, alpha: 1),  // 3 coral
            SKColor(red: 0.96, green: 0.77, blue: 0.38, alpha: 1),  // 4 sandy gold
        ]
        case .sunset: return [
            SKColor(red: 0.99, green: 0.91, blue: 0.54, alpha: 1),  // 0 golden
            SKColor(red: 0.98, green: 0.57, blue: 0.24, alpha: 1),  // 1 coral orange
            SKColor(red: 0.96, green: 0.45, blue: 0.71, alpha: 1),  // 2 rose
            SKColor(red: 0.65, green: 0.55, blue: 0.98, alpha: 1),  // 3 violet
            SKColor(red: 0.94, green: 0.27, blue: 0.27, alpha: 1),  // 4 warm red
        ]
        case .forest: return [
            SKColor(red: 0.82, green: 0.91, blue: 0.77, alpha: 1),  // 0 moss
            SKColor(red: 0.18, green: 0.49, blue: 0.23, alpha: 1),  // 1 forest green
            SKColor(red: 0.77, green: 0.66, blue: 0.51, alpha: 1),  // 2 mushroom
            SKColor(red: 0.83, green: 0.45, blue: 0.12, alpha: 1),  // 3 autumn orange
            SKColor(red: 0.55, green: 0.37, blue: 0.24, alpha: 1),  // 4 bark brown
        ]
        }
    }
}
```

The Spring values intentionally differ slightly from the old `TileGridNode.tileColors` (minor green-channel tweaks). The `TilePalette` values are authoritative — `tileColors` is removed entirely.

---

## TileGridNode

Remove `static let tileColors` entirely. Update the `required init?(coder:)` fatalError message to `"Use init(playfield:palette:)"`. Replace the designated init with:

```swift
private let palette: TilePalette

init(playfield: [[Int]], palette: TilePalette) {
    self.palette = palette
    super.init()
    buildGrid(playfield: playfield)
}
```

Every reference to `TileGridNode.tileColors[...]` becomes `palette.colors[...]`.

---

## TitleScene

### New properties

```swift
private var selectedPalette: TilePalette = .spring
private var paletteContainerNodes: [TilePalette: SKNode] = [:]
private var paletteBorderNodes:    [TilePalette: SKShapeNode] = [:]
```

### Menu label layout change

The existing `addMenuLabels()` uses `index * lineSpacing` for all items. The palette grid sits between "Select level:" and "Instructions" and needs ~84 px of extra vertical room. Replace the formula-based Y with explicit per-item Y values:

```swift
let itemY: [MenuItem: CGFloat] = [
    .startGame:    menuStartY,
    .selectColors: menuStartY - lineSpacing,
    .selectTurns:  menuStartY - 2 * lineSpacing,
    .instructions: menuStartY - 5 * lineSpacing,
    .highscores:   menuStartY - 6 * lineSpacing,
    .quit:         menuStartY - 7 * lineSpacing,
]
```

Use `itemY[item]!` instead of `CGFloat(index) * lineSpacing` when positioning each label. The hit-testing loop in `menuItem(at:)` is unchanged — it reads positions from `menuLabels` at runtime.

### Palette grid layout constants

```swift
let swatchSize:  CGFloat = 18
let swatchGap:   CGFloat = 4
let stripWidth:  CGFloat = 5 * swatchSize + 4 * swatchGap   // 106
let columnGap:   CGFloat = 12
let rowGap:      CGFloat = 8

// Grid anchor: vertically centred in the gap between "Select level:" and "Instructions"
let gridAnchorY: CGFloat = menuStartY - 3 * lineSpacing - 26  // = 498
let gridX:       CGFloat = menuX + 240
```

Strip centres (`gridX + stripWidth + columnGap = 350 + 106 + 12 = 468`):
- Top-left (spring):  `(gridX,                         gridAnchorY + rowGap/2 + swatchSize/2)` = `(350, 511)`
- Top-right (ocean):  `(gridX + stripWidth + columnGap, gridAnchorY + rowGap/2 + swatchSize/2)` = `(468, 511)`
- Bottom-left (sunset): `(gridX,                        gridAnchorY - rowGap/2 - swatchSize/2)` = `(350, 485)`
- Bottom-right (forest):`(gridX + stripWidth + columnGap,gridAnchorY - rowGap/2 - swatchSize/2)` = `(468, 485)`

### Building the swatch grid — addPaletteGrid()

```swift
private func addPaletteGrid() {
    let positions: [(TilePalette, CGPoint)] = [
        (.spring, CGPoint(x: gridX,                          y: gridAnchorY + rowGap/2 + swatchSize/2)),
        (.ocean,  CGPoint(x: gridX + stripWidth + columnGap, y: gridAnchorY + rowGap/2 + swatchSize/2)),
        (.sunset, CGPoint(x: gridX,                          y: gridAnchorY - rowGap/2 - swatchSize/2)),
        (.forest, CGPoint(x: gridX + stripWidth + columnGap, y: gridAnchorY - rowGap/2 - swatchSize/2)),
    ]

    for (palette, centre) in positions {
        let container = SKNode()
        container.position = centre   // ← must set position so frame-based hit-testing works

        // Five colour squares
        for i in 0..<5 {
            let sq = SKSpriteNode(
                color: palette.colors[i],
                size: CGSize(width: swatchSize, height: swatchSize)
            )
            sq.position = CGPoint(
                x: CGFloat(i) * (swatchSize + swatchGap) - (stripWidth / 2 - swatchSize / 2),
                y: 0
            )
            container.addChild(sq)
        }

        // Border (shown only for selected palette)
        let borderRect = CGRect(
            x: -(stripWidth / 2) - 2,
            y: -(swatchSize / 2) - 2,
            width: stripWidth + 4,
            height: swatchSize + 4
        )
        let border = SKShapeNode(rect: borderRect, cornerRadius: 4)
        border.strokeColor = SKColor(white: 1, alpha: 0.7)
        border.lineWidth   = 1.5
        border.fillColor   = .clear
        border.isHidden    = (palette != selectedPalette)
        container.addChild(border)

        addChild(container)
        paletteContainerNodes[palette] = container
        paletteBorderNodes[palette]    = border
    }
}
```

Call `addPaletteGrid()` from `buildUI()` after `addMenuLabels()`.

### selectPalette(_:)

```swift
private func selectPalette(_ palette: TilePalette) {
    selectedPalette = palette
    for (p, border) in paletteBorderNodes {
        border.isHidden = (p != palette)
    }
    addColorDots()   // refresh colour dots to match new palette
}
```

### paletteHit(at:)

```swift
private func paletteHit(at point: CGPoint) -> TilePalette? {
    for (palette, node) in paletteContainerNodes {
        if node.frame.contains(point) { return palette }
    }
    return nil
}
```

`node.frame` is valid because each container has `position` set and its children accumulate into the frame.

### mouseDown — palette check before the guard

The existing `mouseDown` has a `guard let item = menuItem(at:) else { return }` at the top. Palette clicks must be checked **before** this guard, otherwise `menuItem(at:)` returns `nil` for palette positions and the guard exits early.

```swift
override func mouseDown(with event: NSEvent) {
    let point = event.location(in: self)

    // Check palette grid first — it is outside the MenuItem system
    if let palette = paletteHit(at: point) {
        SoundManager.shared.playEffect(.menu)
        selectPalette(palette)
        return
    }

    guard let item = menuItem(at: point) else { return }
    SoundManager.shared.playEffect(.menu)

    switch item {
    case .startGame:
        let model = GameModel(level: selectedLevel, maxColors: selectedColors)
        let scene = GamePlayScene(model: model, palette: selectedPalette, size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    // … rest of cases unchanged …
    }
}
```

### addColorDots update

Change the one reference inside `addColorDots()`:

```swift
// Before:
color: TileGridNode.tileColors[i]
// After:
color: selectedPalette.colors[i]
```

The `startX = menuX + 240` constant inside `addColorDots()` is intentionally the same value as `gridX` in `addPaletteGrid()`. Do not extract a shared constant — the two methods are independently positioned and happen to share the same offset.

The `y` position of the dots (`menuStartY - lineSpacing + dotSize/2 - 8`) is unchanged. After the `itemY` dict change, `.selectColors` maps to `menuStartY - lineSpacing` — the same value as before — so dot Y alignment is preserved.

---

## GamePlayScene

```swift
private let palette: TilePalette

init(model: GameModel, palette: TilePalette, size: CGSize) {
    self.palette = palette
    super.init(size: size)
    // …
}
```

Pass `palette` to `TileGridNode`:

```swift
tileGrid = TileGridNode(playfield: model.playfield, palette: palette)
```

---

## Palette reset on return

`returnToTitle()` creates a fresh `TitleScene(size: size)`, so `selectedPalette` resets to `.spring` each session. This is a known limitation — level and color count reset for the same reason. If persistence is added to `UserDefaults` for level/colors in future, palette should be included at the same time.

## Hover state

Palette swatch strips have no hover highlighting. The existing `mouseMoved` / `setHighlight` system operates only on `menuLabels` and is unchanged. Mouse-over a swatch produces no visual feedback — this is intentional.

---

## Testing

No unit tests required. Manual verification:
- All 4 palette strips are clickable; selected strip shows white border
- "Select colors:" dots update immediately when palette changes
- Correct tile colors appear in-game for each palette
- Spring is the default on launch
- Returning to title resets to Spring

# Visual Customization — Design Spec
**Date:** 2026-03-15
**Status:** Draft

## Overview

Three interrelated visual and customization improvements:
1. Frosted gem tile appearance (circles with soft highlight)
2. Three selectable rotation animation transitions
3. Custom colour palette (one user-defined slot, persisted locally)

---

## 1. Frosted Gem Tiles

### Tile shape change

Each tile changes from a flat-coloured square (`SKSpriteNode`) to a circle (`SKShapeNode`). The circle radius is `(tileSize - gap) / 2` where `tileSize = 64` and `gap = 2`, giving radius `31`.

```swift
SKShapeNode(circleOfRadius: (TileGridNode.tileSize - 2) / 2)
```

`fillColor` = palette colour for this tile. `strokeColor = .clear`.

### Frosted highlight overlay

Each tile has a static child `SKShapeNode` (an ellipse) representing the highlight:
- Shape: `SKShapeNode(ellipseIn: CGRect(x: -18, y: 2, width: 22, height: 18))`
  (positioned top-left relative to tile centre, using the tile's local coordinate space)
- `fillColor = SKColor(white: 1, alpha: 0.42)`
- `strokeColor = .clear`
- Never receives colour actions — stays white/translucent always

### TileGridNode type change

`private var tiles: [[SKSpriteNode]]` → `private var tiles: [[SKShapeNode]]`

All methods that reference `tiles[col][row]` update accordingly.

### Colour transition

`SKAction.colorize` is not available on `SKShapeNode`. Replace with `SKAction.customAction(withDuration: 0.18)` that linearly interpolates `fillColor` from old to new colour:

```swift
private func colorTransition(from oldColor: SKColor, to newColor: SKColor) -> SKAction {
    SKAction.customAction(withDuration: 0.18) { node, elapsed in
        guard let shape = node as? SKShapeNode else { return }
        let t = min(elapsed / 0.18, 1.0)
        shape.fillColor = SKColor.lerp(from: oldColor, to: newColor, t: t)
    }
}
```

A `SKColor.lerp` helper (file-level private function in `TileGridNode.swift`):
```swift
private func lerpColor(from: SKColor, to: SKColor, t: CGFloat) -> SKColor {
    var r1: CGFloat = 0, g1: CGFloat = 0, b1: CGFloat = 0, a1: CGFloat = 0
    var r2: CGFloat = 0, g2: CGFloat = 0, b2: CGFloat = 0, a2: CGFloat = 0
    from.getRed(&r1, green: &g1, blue: &b1, alpha: &a1)
    to.getRed(&r2, green: &g2, blue: &b2, alpha: &a2)
    return SKColor(red: r1 + (r2-r1)*t, green: g1 + (g2-g1)*t,
                   blue: b1 + (b2-b1)*t, alpha: 1)
}
```

### applyPalette update

`applyPalette` currently uses `SKAction.colorize`. Replace with direct `fillColor` assignment (instant, no animation):

```swift
func applyPalette(_ newPalette: TilePalette) {
    palette = newPalette
    let colors = newPalette.colors
    for col in 0..<tiles.count {
        for row in 0..<tiles[col].count {
            tiles[col][row].fillColor = colors[colorIndices[col][row]]
        }
    }
}
```

### Colour swatch strip (GamePlayScene)

`buildColorStrip` currently creates `SKSpriteNode` swatches. Change to `SKShapeNode(circleOfRadius: swatchSize/2)` with `fillColor` set directly. No highlight overlay (swatches are 28px — too small for the effect). Update `handlePaletteDidChange` to set `swatch.fillColor` instead of `swatch.color`.

`colorSwatchNodes` type changes from `[SKSpriteNode]` to `[SKShapeNode]`. Declared in `GamePlayScene`.

### TutorialScene tiles

`TutorialScene` has its own inline tile drawing (independent of `TileGridNode`). Update its tile creation to `SKShapeNode` circles with the same frosted highlight overlay for visual consistency.

---

## 2. Three Selectable Animation Transitions

### TransitionStyle enum

New file `Quadromania/TransitionStyle.swift`:

```swift
enum TransitionStyle: Int, CaseIterable {
    case ringSweep   = 0
    case sequential  = 1
    case radialPulse = 2

    var displayName: String {
        switch self {
        case .ringSweep:   return "Ring Sweep"
        case .sequential:  return "Sequential"
        case .radialPulse: return "Radial Pulse"
        }
    }
}
```

### AppDelegate additions

```swift
var transitionStyle: TransitionStyle = .ringSweep
private var transitionMenuItems: [TransitionStyle: NSMenuItem] = [:]
```

`buildGameMenu()` adds a "Transition" `NSMenu` submenu with three radio-style items. On launch, the stored `UserDefaults.standard.integer(forKey: "transitionStyle")` restores the last selection (default 0 = ringSweep).

```swift
@objc private func selectTransitionStyle(_ sender: NSMenuItem) {
    guard let style = TransitionStyle(rawValue: sender.tag) else { return }
    transitionStyle = style
    UserDefaults.standard.set(style.rawValue, forKey: "transitionStyle")
    transitionMenuItems.values.forEach { $0.state = .off }
    sender.state = .on
    NotificationCenter.default.post(name: .transitionStyleDidChange,
                                    object: nil,
                                    userInfo: ["style": style.rawValue])
}
```

New notification name in `AppDelegate.swift` extension:
```swift
static let transitionStyleDidChange = Notification.Name("transitionStyleDidChange")
```

### TileGridNode changes

`updateAll(from:)` gains a `rotationCenter` parameter:
```swift
func updateAll(from playfield: [[Int]], rotationCenter: (col: Int, row: Int)? = nil)
```

`TileGridNode` gains `var transitionStyle: TransitionStyle = .ringSweep`.

When `rotationCenter` is non-nil, use the current `transitionStyle`'s animation for tiles within the 3×3 block; tiles outside the 3×3 block always use the plain 0.18 s colour transition (no special animation).

#### Ring Sweep (style = .ringSweep)

1. Compute the centre of the 3×3 block in TileGridNode's local coordinates.
2. Create a temporary `SKShapeNode` arc path starting at angle −π/2 (top), sweeping clockwise.
3. Animate with `SKAction.customAction(withDuration: 0.30)` that at each frame redraws the arc from 0 to `elapsed/0.30 * 2π`.
4. `strokeColor` = white, `lineWidth = 4`, `fillColor = .clear`.
5. After the arc completes, fade it out (`fadeOut(withDuration: 0.10)` then `removeFromParent`).
6. Simultaneously, after `0.20` s delay, run the colour transitions on tiles.

```swift
// Arc radius = distance from block centre to block corner
let blockRadius = TileGridNode.tileSize * CGFloat(sqrt(2.0) * 1.5)
```

#### Sequential (style = .sequential)

Clockwise ordering of the 8 outer tiles of the 3×3 (col offset, row offset from centre):
```
(-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0)
```
(Row offsets use SpriteKit's flipped Y — row -1 = visually above.)

Each outer tile gets its colour transition delayed by `tileIndex * 0.035` s. Centre tile delayed by `8 * 0.035 = 0.280` s.

All tiles outside the 3×3 that also changed (not possible in current game rules, but guard anyway) use immediate transitions.

#### Radial Pulse (style = .radialPulse)

All 9 tiles in the 3×3 change colour simultaneously (0 delay), but each also runs a scale animation:
```swift
let distance = abs(colOffset) + abs(rowOffset)  // Manhattan distance from centre (0, 1, or 2)
let delay = CGFloat(distance) * 0.040
let scaleAction = SKAction.sequence([
    SKAction.wait(forDuration: delay),
    SKAction.scale(to: 1.12, duration: 0.08),
    SKAction.scale(to: 1.00, duration: 0.10)
])
```
Colour transition runs concurrently (no delay, 0.18 s).

### GamePlayScene changes

In `mouseDown`, pass the clicked `(col, row)` to `updateAll`:
```swift
model.rotate(x: col, y: row)
tileGrid.updateAll(from: model.playfield, rotationCenter: (col, row))
```

Observe `.transitionStyleDidChange` in `didMove(to:)` / deregister in `willMove(from:)`:
```swift
@objc private func handleTransitionStyleDidChange(_ notification: Notification) {
    guard let raw = notification.userInfo?["style"] as? Int,
          let style = TransitionStyle(rawValue: raw) else { return }
    tileGrid.transitionStyle = style
}
```

On scene load, sync from AppDelegate:
```swift
if let appDelegate = NSApp.delegate as? AppDelegate {
    tileGrid.transitionStyle = appDelegate.transitionStyle
}
```

---

## 3. Custom Colour Palette

### CustomPaletteStore

New file `Quadromania/CustomPaletteStore.swift`:

```swift
class CustomPaletteStore {
    static let shared = CustomPaletteStore()
    private static let defaultsKey = "customPaletteColors"

    private init() {}

    /// Five SKColors for the custom palette.
    var colors: [SKColor] {
        get {
            guard let data = UserDefaults.standard.array(forKey: Self.defaultsKey)
                    as? [[CGFloat]], data.count == 5 else {
                return defaultColors
            }
            return data.map { SKColor(red: $0[0], green: $0[1], blue: $0[2], alpha: 1) }
        }
        set {
            let data = newValue.map { c -> [CGFloat] in
                var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
                c.getRed(&r, green: &g, blue: &b, alpha: &a)
                return [r, g, b]
            }
            UserDefaults.standard.set(data, forKey: Self.defaultsKey)
        }
    }

    func save(_ colors: [SKColor]) {
        self.colors = colors
        NotificationCenter.default.post(name: .customPaletteDidChange, object: nil)
    }

    // Default: copy of Spring palette
    private let defaultColors: [SKColor] = [
        SKColor(red: 0.98, green: 0.80, blue: 0.84, alpha: 1),
        SKColor(red: 0.52, green: 0.88, blue: 0.67, alpha: 1),
        SKColor(red: 0.76, green: 0.66, blue: 0.94, alpha: 1),
        SKColor(red: 0.99, green: 0.92, blue: 0.42, alpha: 1),
        SKColor(red: 0.50, green: 0.78, blue: 0.97, alpha: 1),
    ]
}
```

New notification name:
```swift
static let customPaletteDidChange = Notification.Name("customPaletteDidChange")
```

### TilePalette.custom

Add `case custom = 4` to `TilePalette`:
```swift
enum TilePalette: Int, CaseIterable {
    case spring, ocean, sunset, forest, custom
}
```

`displayName` for `.custom`: `"🎨 Custom"`

`colors` for `.custom`: `CustomPaletteStore.shared.colors`

### CustomPalettePanel

New file `Quadromania/CustomPalettePanel.swift`. A floating `NSPanel` with:
- Title: `"Edit Custom Colors"`
- 5 `NSColorWell` objects in a horizontal `NSStackView`, labelled `"0"` through `"4"` below each well
- Well size: 44 × 44 pt
- OK and Cancel buttons
- On `init`, populate wells from `CustomPaletteStore.shared.colors`
- On OK: call `CustomPaletteStore.shared.save(colors)` then close
- On Cancel: close without saving

```swift
class CustomPalettePanel: NSPanel {
    private var colorWells: [NSColorWell] = []

    convenience init() {
        self.init(contentRect: CGRect(x: 0, y: 0, width: 360, height: 140), ...)
        // build layout
    }
}
```

`AppDelegate` holds `private var customPalettePanel: CustomPalettePanel?` (retained while open).

### AppDelegate Palette menu additions

`buildPaletteMenu()` adds:
- `"🎨 Custom"` as a fifth palette radio item (tag = 4, same `selectPaletteItem` action)
- A separator
- `"Edit Custom Colors…"` item targeting `openCustomPaletteEditor(_:)`

```swift
@objc private func openCustomPaletteEditor(_ sender: NSMenuItem) {
    if customPalettePanel == nil {
        customPalettePanel = CustomPalettePanel()
    }
    customPalettePanel?.makeKeyAndOrderFront(nil)
}
```

### Scene refresh on custom palette change

`TitleScene` and `GamePlayScene` already observe `.paletteDidChange`. They additionally observe `.customPaletteDidChange`:

```swift
@objc private func handleCustomPaletteDidChange(_ notification: Notification) {
    guard palette == .custom else { return }   // only refresh if custom is active
    // TitleScene: refresh colour dots
    // GamePlayScene: tileGrid.applyPalette(.custom)
}
```

In `TitleScene`, `handleCustomPaletteDidChange` calls `addColorDots()` to refresh the dot strip (same as palette change).
In `GamePlayScene`, it calls `tileGrid.applyPalette(.custom)` and refreshes `colorSwatchNodes`.

Both register in `didMove(to:)` and deregister via `removeObserver(self)` in `willMove(from:)`.

---

## Files Affected

| File | Change |
|------|--------|
| `Quadromania/TileGridNode.swift` | Tile type → `SKShapeNode`, highlight overlay, `lerpColor`, updated `updateAll(from:rotationCenter:)` with 3 animation modes |
| `Quadromania/GamePlayScene.swift` | Pass `rotationCenter` to `updateAll`, observe `.transitionStyleDidChange`, observe `.customPaletteDidChange`, `colorSwatchNodes` → `[SKShapeNode]` |
| `Quadromania/TitleScene.swift` | Observe `.customPaletteDidChange` |
| `Quadromania/TutorialScene.swift` | Tile drawing → `SKShapeNode` circles with highlight |
| `Quadromania/TilePalette.swift` | Add `case custom = 4` |
| `Quadromania/AppDelegate.swift` | Add Transition submenu to Game menu, `transitionStyle` state, `openCustomPaletteEditor`, add Custom palette item + Edit item to Palette menu, `.transitionStyleDidChange` + `.customPaletteDidChange` notification names |
| `Quadromania/TransitionStyle.swift` | **New** — `TransitionStyle` enum |
| `Quadromania/CustomPaletteStore.swift` | **New** — UserDefaults persistence for custom colours |
| `Quadromania/CustomPalettePanel.swift` | **New** — NSPanel with 5 NSColorWell objects |

No changes to `QuadroCore/` or `QuadroTests`.

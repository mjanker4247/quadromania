# Visual Customization Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace square tiles with frosted gem circles, add three selectable rotation animation transitions, and add a custom editable colour palette stored in UserDefaults.

**Architecture:** Three independent feature layers: (1) visual migration of all tile nodes from `SKSpriteNode` to `SKShapeNode`, (2) pluggable animation strategy on `TileGridNode` controlled via `TransitionStyle` enum + AppDelegate Game menu, (3) custom palette slot backed by `CustomPaletteStore` singleton + `NSPanel` editor. All wired through `NotificationCenter` (existing pattern).

**Tech Stack:** Swift, SpriteKit (`SKShapeNode`, `SKAction.customAction`), AppKit (`NSMenu`, `NSPanel`, `NSColorWell`, `NSStackView`), `UserDefaults`.

---

## Chunk 1: Frosted Gem Tiles (visual migration)

### Task 1: TileGridNode — SKShapeNode tiles + lerpColor + applyPalette

**Files:**
- Modify: `Quadromania/TileGridNode.swift` (full rewrite of buildGrid, applyPalette, updateAll)

**Context:**
- Current: `private var tiles: [[SKSpriteNode]]`, built with `SKSpriteNode(color:size:)`, animated with `SKAction.colorize`
- `SKAction.colorize` is NOT available on `SKShapeNode` — must use `customAction`
- Symbol overlay (`SKLabelNode` named `"symbol"`) can be a child of `SKShapeNode` unchanged
- `colorIndices: [[Int]]` field stays unchanged
- `tileSize = 64`, gap = 2, so circle radius = `(64 - 2) / 2 = 31`
- Highlight child ellipse: `SKShapeNode(ellipseIn: CGRect(x: -18, y: 2, width: 22, height: 18))`, fillColor = white alpha 0.42, strokeColor .clear

- [ ] **Step 1: Add `lerpColor` free function and `colorTransition` method**

Add above the `TileGridNode` class declaration (file-level private):

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

Add inside the `TileGridNode` class (private method):

```swift
private func colorTransition(from oldColor: SKColor, to newColor: SKColor) -> SKAction {
    SKAction.customAction(withDuration: 0.18) { node, elapsed in
        guard let shape = node as? SKShapeNode else { return }
        let t = min(elapsed / CGFloat(0.18), 1.0)
        shape.fillColor = lerpColor(from: oldColor, to: newColor, t: t)
    }
}
```

- [ ] **Step 2: Change `tiles` type declaration**

Change line 24:
```swift
// Before:
private var tiles: [[SKSpriteNode]] = []
// After:
private var tiles: [[SKShapeNode]] = []
```

- [ ] **Step 3: Rewrite `buildGrid` to create SKShapeNode circles**

Replace the entire `buildGrid` method:

```swift
private func buildGrid(playfield: [[Int]]) {
    let s = TileGridNode.tileSize
    let radius = (s - 2) / 2

    tiles = Array(repeating: [], count: GameModel.gridWidth)
    colorIndices = Array(repeating: Array(repeating: 0, count: GameModel.gridHeight),
                         count: GameModel.gridWidth)

    let paletteColors = palette.colors
    for col in 0..<GameModel.gridWidth {
        tiles[col] = []
        for row in 0..<GameModel.gridHeight {
            let colorIndex = playfield[col][row]

            let circle = SKShapeNode(circleOfRadius: radius)
            circle.fillColor   = paletteColors[colorIndex]
            circle.strokeColor = .clear
            circle.position = CGPoint(
                x: CGFloat(col) * s + s / 2,
                y: CGFloat(GameModel.gridHeight - 1 - row) * s + s / 2
            )

            // Frosted highlight overlay (static, never coloured)
            let highlight = SKShapeNode(ellipseIn: CGRect(x: -18, y: 2, width: 22, height: 18))
            highlight.fillColor   = SKColor(white: 1, alpha: 0.42)
            highlight.strokeColor = .clear
            circle.addChild(highlight)

            // Symbol overlay label (same as before — SKLabelNode parented to SKShapeNode works fine)
            let symLabel = SKLabelNode(text: tileSymbols[colorIndex])
            symLabel.name = "symbol"
            symLabel.fontName = "Helvetica-Bold"
            symLabel.fontSize = 14
            symLabel.fontColor = SKColor(white: 0, alpha: 0.6)
            symLabel.verticalAlignmentMode   = .center
            symLabel.horizontalAlignmentMode = .center
            symLabel.position = .zero
            symLabel.isHidden = true
            circle.addChild(symLabel)

            addChild(circle)
            tiles[col].append(circle)
            colorIndices[col][row] = colorIndex
        }
    }
}
```

- [ ] **Step 4: Rewrite `applyPalette` to use fillColor directly**

Replace `applyPalette`:

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

- [ ] **Step 5: Rewrite `updateAll` to use colorTransition**

Replace `updateAll`:

```swift
func updateAll(from playfield: [[Int]]) {
    for col in 0..<GameModel.gridWidth {
        for row in 0..<GameModel.gridHeight {
            let newIndex = playfield[col][row]
            guard newIndex != colorIndices[col][row] else { continue }
            let oldColor = palette.colors[colorIndices[col][row]]
            let newColor = palette.colors[newIndex]
            colorIndices[col][row] = newIndex
            tiles[col][row].run(colorTransition(from: oldColor, to: newColor))
            if let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode {
                sym.text = tileSymbols[newIndex]
            }
        }
    }
}
```

- [ ] **Step 6: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 7: Commit**

```bash
git add Quadromania/TileGridNode.swift
git commit -m "feat: migrate TileGridNode tiles from SKSpriteNode to SKShapeNode circles with frosted highlight"
```

---

### Task 2: GamePlayScene — color swatch strip migration

**Files:**
- Modify: `Quadromania/GamePlayScene.swift` (lines 15, 110–114, 246–248)

**Context:**
- `colorSwatchNodes: [SKSpriteNode]` → `[SKShapeNode]`
- `buildColorStrip` creates `SKSpriteNode(color:size:)` at line 110; change to `SKShapeNode(circleOfRadius: swatchSize/2)` with `fillColor`
- `handlePaletteDidChange` sets `swatch.color` at line 247; change to `swatch.fillColor`
- No highlight overlay on swatches (28px — too small)

- [ ] **Step 1: Change `colorSwatchNodes` type and update `buildColorStrip`**

Change line 15:
```swift
// Before:
private var colorSwatchNodes: [SKSpriteNode] = []
// After:
private var colorSwatchNodes: [SKShapeNode] = []
```

In `buildColorStrip`, replace the swatch creation block (around lines 107–114):
```swift
// Before:
let swatch = SKSpriteNode(color: palette.colors[i],
                          size: CGSize(width: swatchSize, height: swatchSize))
swatch.position = CGPoint(x: x, y: centerY)
addChild(swatch)
colorSwatchNodes.append(swatch)

// After:
let swatch = SKShapeNode(circleOfRadius: swatchSize / 2)
swatch.fillColor   = palette.colors[i]
swatch.strokeColor = .clear
swatch.position = CGPoint(x: x, y: centerY)
addChild(swatch)
colorSwatchNodes.append(swatch)
```

- [ ] **Step 2: Update `handlePaletteDidChange` to use fillColor**

Change lines 246–248 in `handlePaletteDidChange`:
```swift
// Before:
for (i, swatch) in colorSwatchNodes.enumerated() {
    swatch.color = newPalette.colors[i]
}

// After:
for (i, swatch) in colorSwatchNodes.enumerated() {
    swatch.fillColor = newPalette.colors[i]
}
```

- [ ] **Step 3: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 4: Commit**

```bash
git add Quadromania/GamePlayScene.swift
git commit -m "feat: migrate GamePlayScene color swatches from SKSpriteNode to SKShapeNode circles"
```

---

### Task 3: TutorialScene — SKShapeNode tile migration

**Files:**
- Modify: `Quadromania/TutorialScene.swift` (lines 54, 136–149, 217–228)

**Context:**
- `tileSprites: [[SKSpriteNode]]` → `[[SKShapeNode]]`
- `buildTutorialGrid` creates `SKSpriteNode` — replace with `SKShapeNode(circleOfRadius:)` with frosted highlight
- `refreshTileColors` uses `SKAction.colorize` (unavailable on `SKShapeNode`) — replace with direct `fillColor`
- `tileSize = 60` (not 64); radius = `(60 - 2) / 2 = 29`

- [ ] **Step 1: Change `tileSprites` type**

Change line 54:
```swift
// Before:
private var tileSprites: [[SKSpriteNode]] = []
// After:
private var tileSprites: [[SKShapeNode]] = []
```

- [ ] **Step 2: Rewrite `buildTutorialGrid`**

Replace the entire `buildTutorialGrid` method:

```swift
private func buildTutorialGrid() {
    let colors = palette.colors
    let radius = (tileSize - 2) / 2
    tileSprites = Array(repeating: [], count: cols)
    for col in 0..<cols {
        tileSprites[col] = []
        for row in 0..<rows {
            let circle = SKShapeNode(circleOfRadius: radius)
            circle.fillColor   = colors[tutorialPlayfield[col][row]]
            circle.strokeColor = .clear
            circle.position = tilePosition(col: col, row: row)

            // Frosted highlight overlay
            let highlight = SKShapeNode(ellipseIn: CGRect(x: -18, y: 2, width: 22, height: 18))
            highlight.fillColor   = SKColor(white: 1, alpha: 0.42)
            highlight.strokeColor = .clear
            circle.addChild(highlight)

            addChild(circle)
            tileSprites[col].append(circle)
        }
    }
}
```

- [ ] **Step 3: Rewrite `refreshTileColors`**

Replace `refreshTileColors`:

```swift
private func refreshTileColors() {
    let colors = palette.colors
    for col in 0..<cols {
        for row in 0..<rows {
            tileSprites[col][row].fillColor = colors[tutorialPlayfield[col][row]]
        }
    }
}
```

- [ ] **Step 4: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 5: Commit**

```bash
git add Quadromania/TutorialScene.swift
git commit -m "feat: migrate TutorialScene tiles from SKSpriteNode to SKShapeNode circles with frosted highlight"
```

---

### Task 4: TitleScene — colorDotNodes migration

**Files:**
- Modify: `Quadromania/TitleScene.swift` (lines 24, 127–141)

**Context:**
- `colorDotNodes: [SKSpriteNode]` → `[SKShapeNode]`
- `addColorDots` creates `SKSpriteNode(color:size:)` — change to `SKShapeNode(circleOfRadius: dotSize/2)` with `fillColor`
- `dotSize = 24`; radius = 12

- [ ] **Step 1: Change `colorDotNodes` type**

Change line 24:
```swift
// Before:
private var colorDotNodes: [SKSpriteNode] = []
// After:
private var colorDotNodes: [SKShapeNode] = []
```

- [ ] **Step 2: Update `addColorDots` to create SKShapeNode circles**

In `addColorDots`, replace the dot creation (around lines 133–140):
```swift
// Before:
let dot = SKSpriteNode(
    color: selectedPalette.colors[i],
    size: CGSize(width: dotSize, height: dotSize)
)
dot.position = CGPoint(x: startX + CGFloat(i) * dotSpacing, y: y)
addChild(dot)
colorDotNodes.append(dot)

// After:
let dot = SKShapeNode(circleOfRadius: dotSize / 2)
dot.fillColor   = selectedPalette.colors[i]
dot.strokeColor = .clear
dot.position = CGPoint(x: startX + CGFloat(i) * dotSpacing, y: y)
addChild(dot)
colorDotNodes.append(dot)
```

- [ ] **Step 3: Build and run unit tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "error:|Test Suite.*passed|FAILED"
```
Expected: `Build succeeded`, all 19 tests pass.

**Note:** `TitleScene.addPaletteGrid()` (lines 157-208) has five `SKSpriteNode` squares per palette entry. These are intentionally NOT migrated — migrating them is out of spec scope. The palette selector grid stays as-is; only `colorDotNodes` changes.

- [ ] **Step 4: Commit**

```bash
git add Quadromania/TitleScene.swift
git commit -m "feat: migrate TitleScene color dots from SKSpriteNode to SKShapeNode circles"
```

---

## Chunk 2: Selectable Animation Transitions

### Task 5: TransitionStyle enum (new file)

**Files:**
- Create: `Quadromania/TransitionStyle.swift`

- [ ] **Step 1: Create `TransitionStyle.swift`**

```swift
// TransitionStyle.swift
// Three selectable rotation animation modes for the 3×3 block.

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

- [ ] **Step 2: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 3: Commit**

```bash
git add Quadromania/TransitionStyle.swift
git commit -m "feat: add TransitionStyle enum with ringSweep, sequential, radialPulse cases"
```

---

### Task 6: AppDelegate — Transition submenu in Game menu

**Files:**
- Modify: `Quadromania/AppDelegate.swift`

**Context:**
- Add `var transitionStyle: TransitionStyle = .ringSweep` and `private var transitionMenuItems: [TransitionStyle: NSMenuItem] = [:]`
- `buildGameMenu()` currently adds only an Instructions item. Extend it with a "Transition" submenu.
- On launch, restore from `UserDefaults.standard.integer(forKey: "transitionStyle")`
- Add two new notification names to the `Notification.Name` extension at the bottom

- [ ] **Step 1: Add transitionStyle properties to AppDelegate**

After `private var symbolMenuItem: NSMenuItem?` (line 19), add:
```swift
var transitionStyle: TransitionStyle = .ringSweep
private var transitionMenuItems: [TransitionStyle: NSMenuItem] = [:]
```

- [ ] **Step 2: Update `applicationDidFinishLaunching` to restore transitionStyle from UserDefaults**

After `buildPaletteMenu()` call (or after `buildGameMenu()`), add:
```swift
let savedStyle = UserDefaults.standard.integer(forKey: "transitionStyle")
if let style = TransitionStyle(rawValue: savedStyle) {
    transitionStyle = style
    transitionMenuItems[style]?.state = .on
}
```

Note: this must run AFTER `buildGameMenu()` since `transitionMenuItems` is populated there.

- [ ] **Step 3: Add Transition submenu to `buildGameMenu()`**

Update `buildGameMenu()` to add a Transition submenu after the Instructions item:

```swift
private func buildGameMenu() {
    let menu = NSMenu(title: "Game")

    // Instructions item (existing)
    let instrItem = NSMenuItem(
        title: "Instructions",
        action: #selector(showInstructionsMenuAction(_:)),
        keyEquivalent: ""
    )
    instrItem.target = self
    menu.addItem(instrItem)

    // Transition submenu
    menu.addItem(.separator())
    let transitionMenu = NSMenu(title: "Transition")
    for style in TransitionStyle.allCases {
        let item = NSMenuItem(
            title: style.displayName,
            action: #selector(selectTransitionStyle(_:)),
            keyEquivalent: ""
        )
        item.tag    = style.rawValue
        item.state  = .off   // Step 6 restore block sets the correct initial selection
        item.target = self
        transitionMenu.addItem(item)
        transitionMenuItems[style] = item
    }
    let transitionItem = NSMenuItem(title: "Transition", action: nil, keyEquivalent: "")
    transitionItem.submenu = transitionMenu
    menu.addItem(transitionItem)

    let menuItem = NSMenuItem(title: "Game", action: nil, keyEquivalent: "")
    menuItem.submenu = menu
    NSApp.mainMenu?.addItem(menuItem)
}
```

- [ ] **Step 4: Add `selectTransitionStyle` action**

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

- [ ] **Step 5: Add new notification names to the extension at the bottom of the file**

```swift
extension Notification.Name {
    static let paletteDidChange         = Notification.Name("paletteDidChange")
    static let symbolOverlayDidChange   = Notification.Name("symbolOverlayDidChange")
    static let showInstructions         = Notification.Name("showInstructions")
    static let transitionStyleDidChange = Notification.Name("transitionStyleDidChange")
    static let customPaletteDidChange   = Notification.Name("customPaletteDidChange")
}
```

- [ ] **Step 6: Fix the UserDefaults restore ordering**

The restore block in `applicationDidFinishLaunching` must run AFTER `buildGameMenu()`. Verify the order:
```swift
buildGameMenu()    // populates transitionMenuItems
buildPaletteMenu()
// Restore transition style selection
let savedStyle = UserDefaults.standard.integer(forKey: "transitionStyle")
if let style = TransitionStyle(rawValue: savedStyle) {
    transitionStyle = style
    transitionMenuItems.values.forEach { $0.state = .off }
    transitionMenuItems[style]?.state = .on
}
```

- [ ] **Step 7: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 8: Commit**

```bash
git add Quadromania/AppDelegate.swift
git commit -m "feat: add Transition submenu to Game menu with UserDefaults persistence"
```

---

### Task 7: TileGridNode — animation transitions (ring sweep, sequential, radial pulse)

**Requires: Chunk 1 (Tasks 1–4) must be complete.** This task calls `colorTransition(from:to:)` and types `tiles` as `[[SKShapeNode]]` — both come from Task 1. Applying this task before Chunk 1 is done will produce compile errors.

**Files:**
- Modify: `Quadromania/TileGridNode.swift`

**Context:**
- Add `var transitionStyle: TransitionStyle = .ringSweep` property
- Change `updateAll(from:)` signature to `updateAll(from:rotationCenter:)` with default nil
- When `rotationCenter` is nil: same as today (plain `colorTransition` on all changed tiles)
- When `rotationCenter` is non-nil: apply the selected animation to 9 tiles in the 3×3 block; plain transition on tiles outside the block
- Ring sweep: arc node at zPosition 5 on `self`, wait 0.20s then all 9 tiles transition simultaneously
- Sequential: 8 outer tiles clockwise (offsets below), delay = `Double(index) * 0.035`, centre last at 0.280s
- Radial pulse: all 9 tiles with `SKAction.group([scaleAction, colourAction])`, delay by Manhattan distance

Sequential clockwise offsets (colOffset, rowOffset):
```
(-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0)
```
Row offset -1 in model coords = lower row index = visually higher on screen.

- [ ] **Step 1: Add `transitionStyle` property**

After `private var colorIndices: [[Int]] = []`, add:
```swift
var transitionStyle: TransitionStyle = .ringSweep
```

- [ ] **Step 2: Add `updateAll(from:rotationCenter:)` signature and route by style**

Replace the existing `updateAll(from:)` with:

```swift
func updateAll(from playfield: [[Int]], rotationCenter: (col: Int, row: Int)? = nil) {
    if let center = rotationCenter {
        animateBlock(from: playfield, center: center)
    } else {
        for col in 0..<GameModel.gridWidth {
            for row in 0..<GameModel.gridHeight {
                let newIndex = playfield[col][row]
                guard newIndex != colorIndices[col][row] else { continue }
                let oldColor = palette.colors[colorIndices[col][row]]
                let newColor = palette.colors[newIndex]
                colorIndices[col][row] = newIndex
                tiles[col][row].run(colorTransition(from: oldColor, to: newColor))
                if let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode {
                    sym.text = tileSymbols[newIndex]
                }
            }
        }
    }
}
```

- [ ] **Step 3: Add `animateBlock` dispatcher**

```swift
private func animateBlock(from playfield: [[Int]], center: (col: Int, row: Int)) {
    // Collect which tiles changed and their new colours
    // First update colorIndices and symbol labels; animation is purely visual
    let blockCols = (center.col - 1)...(center.col + 1)
    let blockRows = (center.row - 1)...(center.row + 1)

    // Apply transitions for tiles OUTSIDE the 3x3 block first (plain transition)
    for col in 0..<GameModel.gridWidth {
        for row in 0..<GameModel.gridHeight {
            let newIndex = playfield[col][row]
            guard newIndex != colorIndices[col][row] else { continue }
            if blockCols.contains(col) && blockRows.contains(row) { continue }
            let oldColor = palette.colors[colorIndices[col][row]]
            let newColor = palette.colors[newIndex]
            colorIndices[col][row] = newIndex
            tiles[col][row].run(colorTransition(from: oldColor, to: newColor))
            if let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode {
                sym.text = tileSymbols[newIndex]
            }
        }
    }

    // Gather 3x3 block data (only tiles whose colour actually changes)
    var blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)] = []
    for dc in -1...1 {
        for dr in -1...1 {
            let col = center.col + dc
            let row = center.row + dr
            guard col >= 0, col < GameModel.gridWidth,
                  row >= 0, row < GameModel.gridHeight else { continue }
            let newIndex = playfield[col][row]
            let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode
            let oldColor = palette.colors[colorIndices[col][row]]
            let newColor = palette.colors[newIndex]
            colorIndices[col][row] = newIndex
            sym?.text = tileSymbols[newIndex]
            // Only add to animation list if colour actually changed; skip scale/colour animations
            // on tiles that stayed the same (avoids spurious radial pulse on unchanged tiles)
            guard oldColor != newColor else { continue }
            blockTiles.append((
                tile: tiles[col][row],
                oldColor: oldColor,
                newColor: newColor,
                colOffset: dc,
                rowOffset: dr,
                sym: sym,
                newSymText: tileSymbols[newIndex]
            ))
        }
    }

    switch transitionStyle {
    case .ringSweep:   animateRingSweep(blockTiles: blockTiles, center: center)
    case .sequential:  animateSequential(blockTiles: blockTiles)
    case .radialPulse: animateRadialPulse(blockTiles: blockTiles)
    }
}
```

- [ ] **Step 4: Add ring sweep animation**

```swift
private func animateRingSweep(
    blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                  colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)],
    center: (col: Int, row: Int)
) {
    let s = TileGridNode.tileSize
    let centerX = CGFloat(center.col) * s + s / 2
    let centerY = CGFloat(GameModel.gridHeight - 1 - center.row) * s + s / 2
    let blockRadius = s * CGFloat(sqrt(2.0) * 1.5)

    // Arc sweep node
    let arc = SKShapeNode()
    arc.strokeColor = .white
    arc.lineWidth   = 4
    arc.fillColor   = .clear
    arc.position    = CGPoint(x: centerX, y: centerY)
    arc.zPosition   = 5
    addChild(arc)

    arc.run(SKAction.sequence([
        SKAction.customAction(withDuration: 0.30) { node, elapsed in
            guard let shape = node as? SKShapeNode else { return }
            let progress = min(elapsed / 0.30, 1.0)
            let endAngle = CGFloat(progress) * 2 * .pi
            let path = CGMutablePath()
            // Start at top (-π/2), sweep clockwise (negative direction in SpriteKit's flipped Y)
            path.addArc(center: .zero, radius: blockRadius,
                        startAngle: -.pi / 2, endAngle: -.pi / 2 - endAngle,
                        clockwise: true)
            shape.path = path
        },
        SKAction.fadeOut(withDuration: 0.10),
        SKAction.removeFromParent()
    ]))

    // All 9 tiles transition simultaneously after 0.20 s delay
    for entry in blockTiles {
        let action = SKAction.sequence([
            SKAction.wait(forDuration: 0.20),
            colorTransition(from: entry.oldColor, to: entry.newColor)
        ])
        entry.tile.run(action)
    }
}
```

- [ ] **Step 5: Add sequential animation**

```swift
private func animateSequential(
    blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                  colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)]
) {
    // Clockwise ordering of the 8 outer tiles (colOffset, rowOffset from centre)
    let clockwiseOffsets: [(Int, Int)] = [
        (-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0)
    ]
    // Map from offset to tile entry
    var offsetMap: [String: (tile: SKShapeNode, oldColor: SKColor, newColor: SKColor)] = [:]
    var centerEntry: (tile: SKShapeNode, oldColor: SKColor, newColor: SKColor)?
    for entry in blockTiles {
        let key = "\(entry.colOffset),\(entry.rowOffset)"
        if entry.colOffset == 0 && entry.rowOffset == 0 {
            centerEntry = (entry.tile, entry.oldColor, entry.newColor)
        } else {
            offsetMap[key] = (entry.tile, entry.oldColor, entry.newColor)
        }
    }

    // Outer tiles in clockwise order
    for (index, (dc, dr)) in clockwiseOffsets.enumerated() {
        let key = "\(dc),\(dr)"
        guard let entry = offsetMap[key] else { continue }
        let delay = Double(index) * 0.035
        let action = SKAction.sequence([
            SKAction.wait(forDuration: delay),
            colorTransition(from: entry.oldColor, to: entry.newColor)
        ])
        entry.tile.run(action)
    }

    // Centre tile last
    if let c = centerEntry {
        let action = SKAction.sequence([
            SKAction.wait(forDuration: Double(8) * 0.035),
            colorTransition(from: c.oldColor, to: c.newColor)
        ])
        c.tile.run(action)
    }
}
```

- [ ] **Step 6: Add radial pulse animation**

```swift
private func animateRadialPulse(
    blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                  colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)]
) {
    for entry in blockTiles {
        let distance = abs(entry.colOffset) + abs(entry.rowOffset) // 0, 1, or 2
        let delay = TimeInterval(distance) * 0.040
        let scaleAction = SKAction.sequence([
            SKAction.wait(forDuration: delay),
            SKAction.scale(to: 1.12, duration: 0.08),
            SKAction.scale(to: 1.00, duration: 0.10)
        ])
        let colourAction = colorTransition(from: entry.oldColor, to: entry.newColor)
        entry.tile.run(SKAction.group([scaleAction, colourAction]))
    }
}
```

- [ ] **Step 7: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 8: Commit**

```bash
git add Quadromania/TileGridNode.swift
git commit -m "feat: add transitionStyle property and ring sweep, sequential, radial pulse animations to TileGridNode"
```

---

### Task 8: GamePlayScene — wire transition style

**Files:**
- Modify: `Quadromania/GamePlayScene.swift`

**Context:**
- `mouseDown` currently calls `tileGrid.updateAll(from: model.playfield)` — change to pass `rotationCenter: (col, row)`
- Add `.transitionStyleDidChange` observer in `didMove(to:)` (already deregistered in `willMove(from:)` via `removeObserver(self)`)
- On scene load, sync `tileGrid.transitionStyle` from `AppDelegate`

- [ ] **Step 1: Update `mouseDown` to pass rotationCenter**

In `mouseDown`, change the `updateAll` call (around line 161):
```swift
// Before:
tileGrid.updateAll(from: model.playfield)
// After:
tileGrid.updateAll(from: model.playfield, rotationCenter: (col, row))
```

- [ ] **Step 2: Add `.transitionStyleDidChange` observer in `didMove(to:)`**

Add after the existing `addObserver` calls (before the closing `}`):
```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleTransitionStyleDidChange(_:)),
    name: .transitionStyleDidChange,
    object: nil
)
```

- [ ] **Step 3: Add the handler method**

```swift
@objc private func handleTransitionStyleDidChange(_ notification: Notification) {
    guard let raw = notification.userInfo?["style"] as? Int,
          let style = TransitionStyle(rawValue: raw) else { return }
    tileGrid.transitionStyle = style
}
```

- [ ] **Step 4: Sync transitionStyle on scene load in `buildUI`**

In `buildUI()`, after `tileGrid.symbolOverlayEnabled` sync (around line 73), add:
```swift
if let appDelegate = NSApp.delegate as? AppDelegate {
    tileGrid.transitionStyle = appDelegate.transitionStyle
}
```

- [ ] **Step 5: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 6: Commit**

```bash
git add Quadromania/GamePlayScene.swift
git commit -m "feat: wire transitionStyle into GamePlayScene — pass rotationCenter to updateAll, observe transitionStyleDidChange"
```

---

## Chunk 3: Custom Colour Palette

**Requires: Chunk 2 (Tasks 5–8) must be complete.** Chunk 2 Task 6 Step 5 adds `.customPaletteDidChange` to the `Notification.Name` extension in `AppDelegate.swift`. Tasks 9 and 13 in this chunk reference that name — applying them before Task 6 is done will produce compile errors.

### Task 9: CustomPaletteStore (new file)

**Files:**
- Create: `Quadromania/CustomPaletteStore.swift`

**Context:**
- Singleton: `CustomPaletteStore.shared`
- UserDefaults key: `"customPaletteColors"`, stored as `[[Double]]` (NOT `[[CGFloat]]` — UserDefaults bridges via NSNumber to Double)
- 5 colours; default is a copy of Spring palette
- `save(_:)` persists and posts `.customPaletteDidChange` (declared in AppDelegate.swift extension)

- [ ] **Step 1: Create `CustomPaletteStore.swift`**

```swift
// CustomPaletteStore.swift
// Persistent storage for the user's custom colour palette.

import SpriteKit

class CustomPaletteStore {
    static let shared = CustomPaletteStore()
    private static let defaultsKey = "customPaletteColors"

    private init() {}

    /// Five SKColors for the custom palette.
    var colors: [SKColor] {
        get {
            // UserDefaults round-trips numeric arrays as [[Double]] via NSNumber bridging;
            // casting as [[CGFloat]] always returns nil even though CGFloat == Double on 64-bit.
            guard let data = UserDefaults.standard.array(forKey: Self.defaultsKey)
                    as? [[Double]], data.count == 5 else {
                return defaultColors
            }
            return data.map { SKColor(red: CGFloat($0[0]), green: CGFloat($0[1]),
                                      blue: CGFloat($0[2]), alpha: 1) }
        }
        set {
            // Store as [[Double]] — consistent with the [[Double]] read-back in the getter
            let data = newValue.map { c -> [Double] in
                var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
                c.getRed(&r, green: &g, blue: &b, alpha: &a)
                return [Double(r), Double(g), Double(b)]
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

- [ ] **Step 2: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 3: Commit**

```bash
git add Quadromania/CustomPaletteStore.swift
git commit -m "feat: add CustomPaletteStore singleton for custom palette UserDefaults persistence"
```

---

### Task 10: TilePalette — add `.custom` case

**Files:**
- Modify: `Quadromania/TilePalette.swift`

**Context:**
- Add `case custom = 4` to the enum
- Add `.custom` to both `switch self` statements (displayName + colors)
- `TilePalette` must stay in `Quadromania` target (not QuadroCore) due to `CustomPaletteStore` dependency

- [ ] **Step 1: Add `case custom = 4`**

Change line 8:
```swift
// Before:
enum TilePalette: Int, CaseIterable {
    case spring, ocean, sunset, forest
// After:
enum TilePalette: Int, CaseIterable {
    case spring, ocean, sunset, forest, custom
```

- [ ] **Step 2: Add `.custom` to `displayName` switch**

Add before the closing `}` of the `displayName` switch:
```swift
case .custom: return "🎨 Custom"
```

- [ ] **Step 3: Add `.custom` to `colors` switch**

Add before the closing `}` of the `colors` switch:
```swift
case .custom: return CustomPaletteStore.shared.colors
```

- [ ] **Step 4: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 5: Commit**

```bash
git add Quadromania/TilePalette.swift
git commit -m "feat: add TilePalette.custom case backed by CustomPaletteStore"
```

---

### Task 11: CustomPalettePanel (new file)

**Files:**
- Create: `Quadromania/CustomPalettePanel.swift`

**Context:**
- `NSPanel` subclass (utility window, non-releasing)
- 5 `NSColorWell` objects in horizontal stack, each 44×44pt, with label below
- OK/Cancel buttons right-aligned
- On OK: `CustomPaletteStore.shared.save(colorWells.map { $0.color })` then `close()`
- On Cancel / window close: `close()` without saving
- `SKColor` is a typealias for `NSColor` on macOS — no cast needed
- `center()` will be called from AppDelegate before showing

- [ ] **Step 1: Create `CustomPalettePanel.swift`**

```swift
// CustomPalettePanel.swift
// Floating NSPanel with 5 NSColorWell editors for the custom palette.

import Cocoa
import SpriteKit

class CustomPalettePanel: NSPanel, NSWindowDelegate {
    private var colorWells: [NSColorWell] = []

    convenience init() {
        self.init(contentRect: CGRect(x: 0, y: 0, width: 360, height: 160),
                  styleMask: [.titled, .closable, .utilityWindow],
                  backing: .buffered,
                  defer: false)
        title = "Edit Custom Colors"
        isReleasedWhenClosed = false
        delegate = self
        buildLayout()
    }

    private func buildLayout() {
        guard let contentView = contentView else { return }

        // Five well+label vertical stacks in a horizontal stack
        var wellStacks: [NSView] = []
        let storedColors = CustomPaletteStore.shared.colors
        for i in 0..<5 {
            let well = NSColorWell()
            well.color = storedColors[i]
            well.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                well.widthAnchor.constraint(equalToConstant: 44),
                well.heightAnchor.constraint(equalToConstant: 44)
            ])
            colorWells.append(well)

            let label = NSTextField(labelWithString: "\(i)")
            label.font = NSFont.systemFont(ofSize: 11)
            label.alignment = .center

            let stack = NSStackView(views: [well, label])
            stack.orientation = .vertical
            stack.alignment = .centerX
            stack.spacing = 4
            wellStacks.append(stack)
        }

        let wellsRow = NSStackView(views: wellStacks)
        wellsRow.orientation = .horizontal
        wellsRow.spacing = 12
        wellsRow.translatesAutoresizingMaskIntoConstraints = false

        // OK / Cancel buttons
        let okButton = NSButton(title: "OK", target: self, action: #selector(okClicked(_:)))
        okButton.keyEquivalent = "\r"
        let cancelButton = NSButton(title: "Cancel", target: self, action: #selector(cancelClicked(_:)))
        cancelButton.keyEquivalent = "\u{1b}"

        let buttonsRow = NSStackView(views: [cancelButton, okButton])
        buttonsRow.orientation = .horizontal
        buttonsRow.spacing = 8
        buttonsRow.translatesAutoresizingMaskIntoConstraints = false

        contentView.addSubview(wellsRow)
        contentView.addSubview(buttonsRow)

        NSLayoutConstraint.activate([
            wellsRow.centerXAnchor.constraint(equalTo: contentView.centerXAnchor),
            wellsRow.topAnchor.constraint(equalTo: contentView.topAnchor, constant: 16),

            buttonsRow.trailingAnchor.constraint(equalTo: contentView.trailingAnchor, constant: -12),
            buttonsRow.bottomAnchor.constraint(equalTo: contentView.bottomAnchor, constant: -12)
        ])
    }

    @objc private func okClicked(_ sender: NSButton) {
        // SKColor is a typealias for NSColor on macOS — no cast needed
        CustomPaletteStore.shared.save(colorWells.map { $0.color })
        close()
    }

    @objc private func cancelClicked(_ sender: NSButton) {
        close()
    }

    // Window close button also cancels
    func windowShouldClose(_ sender: NSWindow) -> Bool {
        return true
    }
}
```

- [ ] **Step 2: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 3: Commit**

```bash
git add Quadromania/CustomPalettePanel.swift
git commit -m "feat: add CustomPalettePanel NSPanel with 5 NSColorWell editors"
```

---

### Task 12: AppDelegate — Custom palette menu additions

**Files:**
- Modify: `Quadromania/AppDelegate.swift`

**Context:**
- `buildPaletteMenu()` currently iterates `TilePalette.allCases` unconditionally — now `.custom` is in `allCases`, must skip it in the loop
- Add Custom radio item manually (stored in `paletteMenuItems[.custom]` so toggle-off works)
- Add separator + "Edit Custom Colors…" item
- Add `customPalettePanel: CustomPalettePanel?` property
- Add `openCustomPaletteEditor(_:)` action

- [ ] **Step 1: Add `customPalettePanel` property**

Add after `private var symbolMenuItem: NSMenuItem?`:
```swift
private var customPalettePanel: CustomPalettePanel?
```

- [ ] **Step 2: Update `buildPaletteMenu()` to skip `.custom` in loop**

Change the `for palette in TilePalette.allCases` loop (line 60):
```swift
// Before:
for palette in TilePalette.allCases {

// After:
for palette in TilePalette.allCases where palette != .custom {
```

- [ ] **Step 3: Add Custom item + Edit item after the loop, before the symbol separator**

The existing structure after the loop is:
```swift
menu.addItem(.separator())  // before Color Symbols
```

Add before that separator:
```swift
menu.addItem(.separator())
let customItem = NSMenuItem(
    title: TilePalette.custom.displayName,
    action: #selector(selectPaletteItem(_:)),
    keyEquivalent: ""
)
customItem.tag    = TilePalette.custom.rawValue  // 4
customItem.state  = (activePalette == .custom) ? .on : .off
customItem.target = self
paletteMenuItems[.custom] = customItem  // must be in paletteMenuItems for toggle-off
menu.addItem(customItem)

menu.addItem(.separator())
let editItem = NSMenuItem(
    title: "Edit Custom Colors…",
    action: #selector(openCustomPaletteEditor(_:)),
    keyEquivalent: ""
)
editItem.target = self
menu.addItem(editItem)
```

- [ ] **Step 4: Add `openCustomPaletteEditor` action**

```swift
@objc private func openCustomPaletteEditor(_ sender: NSMenuItem) {
    if customPalettePanel == nil {
        customPalettePanel = CustomPalettePanel()
    }
    customPalettePanel?.center()
    customPalettePanel?.makeKeyAndOrderFront(nil)
}
```

- [ ] **Step 5: Build to verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
```
Expected: `Build succeeded`

- [ ] **Step 6: Commit**

```bash
git add Quadromania/AppDelegate.swift
git commit -m "feat: add Custom palette item and Edit Custom Colors menu item to Palette menu"
```

---

### Task 13: TitleScene + GamePlayScene — custom palette observers

**Files:**
- Modify: `Quadromania/TitleScene.swift`
- Modify: `Quadromania/GamePlayScene.swift`

**Context:**
- Both scenes must observe `.customPaletteDidChange` and refresh only when their current palette is `.custom`
- `TitleScene`: refresh by calling `addColorDots()` (palette border does NOT change — no Custom swatch in grid)
- `GamePlayScene`: refresh by calling `tileGrid.applyPalette(.custom)` and updating `colorSwatchNodes`
- Both already deregister all observers via `NotificationCenter.default.removeObserver(self)` in `willMove(from:)`

- [ ] **Step 1: Add `.customPaletteDidChange` observer to `TitleScene.didMove(to:)`**

Add after the `.showInstructions` observer:
```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleCustomPaletteDidChange(_:)),
    name: .customPaletteDidChange,
    object: nil
)
```

- [ ] **Step 2: Add handler to TitleScene**

```swift
@objc private func handleCustomPaletteDidChange(_ notification: Notification) {
    guard selectedPalette == .custom else { return }
    addColorDots()
}
```

- [ ] **Step 3: Add `.customPaletteDidChange` observer to `GamePlayScene.didMove(to:)`**

Add after the `.showInstructions` observer:
```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleCustomPaletteDidChange(_:)),
    name: .customPaletteDidChange,
    object: nil
)
```

- [ ] **Step 4: Add handler to GamePlayScene**

```swift
@objc private func handleCustomPaletteDidChange(_ notification: Notification) {
    guard palette == .custom else { return }
    tileGrid.applyPalette(.custom)
    for (i, swatch) in colorSwatchNodes.enumerated() {
        swatch.fillColor = TilePalette.custom.colors[i]
    }
}
```

- [ ] **Step 5: Build and run all tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|Build succeeded"
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "error:|Test Suite.*passed|FAILED"
```
Expected: `Build succeeded`, all 19 tests pass.

- [ ] **Step 6: Commit**

```bash
git add Quadromania/TitleScene.swift Quadromania/GamePlayScene.swift
git commit -m "feat: observe customPaletteDidChange in TitleScene and GamePlayScene for live custom palette refresh"
```

---

## Manual Verification Checklist

After all tasks are complete, run the app (`Cmd+R` in Xcode or `xcodebuild … build` + run the `.app`) and verify:

1. **Frosted gem tiles:** All tiles are circles with a translucent white highlight in the top-left. No square tiles remain in game, tutorial, or title screen.
2. **Color dots (title):** The dots next to "Select colors:" are circles matching the selected palette.
3. **Color swatches (gameplay):** The swatch strip above the grid shows circles.
4. **Ring Sweep:** Select Game → Transition → Ring Sweep. Click a tile during gameplay. A white arc should sweep around the 3×3 block, then tiles change colour simultaneously after 0.2s.
5. **Sequential:** Select Sequential. Tiles should change in clockwise order around the 3×3 with a stagger.
6. **Radial Pulse:** Select Radial Pulse. Tiles should scale up then back with colour changing; centre tile first, corners last.
7. **Transition persistence:** Quit and relaunch. The selected transition is restored from the menu.
8. **Custom palette:** Select Palette → 🎨 Custom. Dots and tiles should use the default Spring-like colours. Open "Edit Custom Colors…", change a well, click OK. The tile colours update immediately.
9. **Custom palette persistence:** Quit and relaunch. Custom palette colours are restored.
10. **Symbol overlay:** Enable Color Symbols (Palette menu). Symbols appear on circles correctly.
11. **Tutorial:** Open Instructions → Start Tutorial. Tutorial tiles are circles with highlight. Completing all steps works.

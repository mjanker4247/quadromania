# Accessibility, Tutorial & Polish Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the palette chooser bug, add palette/accessibility options to the macOS menu bar, add a symbol overlay for colorblind users, rewrite the instructions text, create a scripted tutorial scene, and update all documentation.

**Architecture:** `TileGridNode` gains symbol labels and palette-swap support; `AppDelegate` gains a Palette menu that broadcasts `NSNotification`s observed by `TitleScene` and `GamePlayScene`; `TutorialScene` is a self-contained 5×4 scene with its own grid state. No changes to `QuadroCore`.

**Tech Stack:** Swift, SpriteKit, Cocoa (`NSMenu`/`NSMenuItem`), `NotificationCenter`

---

## Chunk 1: Core infrastructure

### Task 1: Fix palette chooser bug in TitleScene

**Files:**
- Modify: `Quadromania/TitleScene.swift`

The bug: `paletteHit(at:)` calls `node.frame.contains(point)` on bare `SKNode` containers. Replace with an explicit rect computed from the container's `position` and known constants. Also promote `stripWidth` and `swatchSize` from local `let`s inside `addPaletteGrid()` to `private let` properties so `paletteHit` can read them.

- [ ] **Step 1: Promote layout constants to properties**

In `TitleScene.swift`, find the `// MARK: - Layout constants` section. After `private let lineSpacing`, add:

```swift
private let stripWidth:  CGFloat = 106   // 5 * 18 + 4 * 4
private let swatchSize:  CGFloat = 18
```

Then inside `addPaletteGrid()`, delete the two local `let` declarations for `swatchSize` and `stripWidth` (keep `swatchGap`, `columnGap`, `rowGap` as locals — they are not needed by `paletteHit`).

- [ ] **Step 2: Replace paletteHit with explicit geometry**

Replace the entire `paletteHit(at:)` method body:

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

- [ ] **Step 3: Build and verify**

```bash
cd /Users/marcojanker/development/quadromania
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 4: Commit**

```bash
git add Quadromania/TitleScene.swift
git commit -m "fix: palette chooser hit-testing using explicit geometry"
```

---

### Task 2: Enhance TileGridNode — palette swap + symbol overlay

**Files:**
- Modify: `Quadromania/TileGridNode.swift`

Three independent additions: (a) make `palette` mutable and add `applyPalette`; (b) add symbol labels to tiles; (c) add `symbolOverlayEnabled` toggle.

Note: `applyPalette` iterates `colorIndices` directly (rather than calling `updateAll`) because `updateAll` has an early-exit guard that skips tiles whose index hasn't changed — on a palette swap every tile must be recolored regardless.

Define the symbol array once as a file-level constant (above the class declaration) so both `buildGrid` and `updateAll` can share it without duplication:

```swift
private let tileSymbols = ["●", "■", "▲", "◆", "★"]
```

- [ ] **Step 1: Make palette mutable and add applyPalette**

In `TileGridNode.swift`:

1. Add `private let tileSymbols = ["●", "■", "▲", "◆", "★"]` as a file-level constant above the `class TileGridNode` line.
2. Change `private let palette: TilePalette` → `private var palette: TilePalette`
3. After `updateAll`, add the new method:

```swift
/// Recolor all tiles for a new palette without changing playfield state.
/// Uses colorIndices directly (not updateAll) to recolor every tile unconditionally.
func applyPalette(_ newPalette: TilePalette) {
    palette = newPalette
    let colors = newPalette.colors
    for col in 0..<tiles.count {
        for row in 0..<tiles[col].count {
            let idx = colorIndices[col][row]
            tiles[col][row].run(SKAction.colorize(with: colors[idx],
                                                   colorBlendFactor: 1.0,
                                                   duration: 0.18))
        }
    }
}
```

- [ ] **Step 2: Add symbol labels in buildGrid**

Inside `buildGrid`, after `addChild(sprite)` and `tiles[col].append(sprite)`, add (uses the file-level `tileSymbols` constant):

```swift
let symLabel = SKLabelNode(text: tileSymbols[colorIndex])
symLabel.name = "symbol"
symLabel.fontName = "Helvetica-Bold"
symLabel.fontSize = 14
symLabel.fontColor = SKColor(white: 0, alpha: 0.6)
symLabel.verticalAlignmentMode   = .center
symLabel.horizontalAlignmentMode = .center
symLabel.position = .zero
symLabel.isHidden = true
sprite.addChild(symLabel)
```

- [ ] **Step 3: Update symbol text in updateAll**

Inside `updateAll(from:)`, after `tiles[col][row].run(action)` (this runs only when `newIndex != colorIndices[col][row]`, which is correct — same index means same symbol), add (uses the file-level `tileSymbols` constant):

```swift
if let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode {
    sym.text = tileSymbols[newIndex]
}
```

- [ ] **Step 4: Add symbolOverlayEnabled property and updateSymbols()**

After the `applyPalette` method, add:

```swift
var symbolOverlayEnabled: Bool = false {
    didSet { updateSymbols() }
}

private func updateSymbols() {
    for col in tiles {
        for sprite in col {
            sprite.childNode(withName: "symbol")?.isHidden = !symbolOverlayEnabled
        }
    }
}
```

- [ ] **Step 5: Build and verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 6: Commit**

```bash
git add Quadromania/TileGridNode.swift
git commit -m "feat: add applyPalette, symbol overlay, and currentPlayfield cache to TileGridNode"
```

---

### Task 3: Add Palette menu and symbol toggle to AppDelegate

**Files:**
- Modify: `Quadromania/AppDelegate.swift`

Pattern follows the existing Sound menu. The Palette menu is built programmatically and inserted into `NSApp.mainMenu`. Selecting a palette posts `paletteDidChange`; toggling symbols posts `symbolOverlayDidChange`.

- [ ] **Step 1: Add stored state and notification names**

At the top of `AppDelegate` class body, before `func applicationDidFinishLaunching`, add:

```swift
// MARK: - Palette & accessibility state

var activePalette: TilePalette = .spring
var symbolOverlayEnabled: Bool = false

private var paletteMenuItems: [TilePalette: NSMenuItem] = [:]
private var symbolMenuItem: NSMenuItem?
```

- [ ] **Step 2: Build the Palette menu**

Add a new method:

```swift
private func buildPaletteMenu() {
    let menu = NSMenu(title: "Palette")

    for palette in TilePalette.allCases {
        let item = NSMenuItem(
            title: palette.displayName,
            action: #selector(selectPaletteItem(_:)),
            keyEquivalent: ""
        )
        item.tag = palette.rawValue
        item.state = (palette == activePalette) ? .on : .off
        item.target = self
        menu.addItem(item)
        paletteMenuItems[palette] = item
    }

    menu.addItem(.separator())

    let symItem = NSMenuItem(
        title: "Color Symbols",
        action: #selector(toggleSymbolOverlay(_:)),
        keyEquivalent: ""
    )
    symItem.state = symbolOverlayEnabled ? .on : .off
    symItem.target = self
    menu.addItem(symItem)
    symbolMenuItem = symItem

    let paletteMenuItem = NSMenuItem(title: "Palette", action: nil, keyEquivalent: "")
    paletteMenuItem.submenu = menu
    NSApp.mainMenu?.addItem(paletteMenuItem)
}
```

- [ ] **Step 3: Add action methods**

```swift
@objc private func selectPaletteItem(_ sender: NSMenuItem) {
    guard let palette = TilePalette(rawValue: sender.tag) else { return }
    activePalette = palette
    paletteMenuItems.values.forEach { $0.state = .off }
    sender.state = .on
    NotificationCenter.default.post(
        name: .paletteDidChange,
        object: nil,
        userInfo: ["palette": palette.rawValue]
    )
}

@objc private func toggleSymbolOverlay(_ sender: NSMenuItem) {
    symbolOverlayEnabled.toggle()
    sender.state = symbolOverlayEnabled ? .on : .off
    NotificationCenter.default.post(
        name: .symbolOverlayDidChange,
        object: nil,
        userInfo: ["enabled": symbolOverlayEnabled]
    )
}
```

- [ ] **Step 4: Add Notification.Name extensions**

After the closing `}` of `AppDelegate`, add:

```swift
extension Notification.Name {
    static let paletteDidChange    = Notification.Name("paletteDidChange")
    static let symbolOverlayDidChange = Notification.Name("symbolOverlayDidChange")
}
```

- [ ] **Step 5: Call buildPaletteMenu() on launch**

In `applicationDidFinishLaunching`, after `updateMusicMenuItem()`, add:
```swift
buildPaletteMenu()
```

- [ ] **Step 6: Build and verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 7: Commit**

```bash
git add Quadromania/AppDelegate.swift
git commit -m "feat: add Palette menu and Color Symbols toggle to menu bar"
```

---

## Chunk 2: Scene notification observers

### Task 4: TitleScene — observe paletteDidChange

**Files:**
- Modify: `Quadromania/TitleScene.swift`

`TitleScene` already has `selectPalette(_:)`. We just need to register/deregister and handle the notification.

- [ ] **Step 1: Register in didMove(to:)**

In `TitleScene.swift`, find `override func didMove(to view: SKView)`. After `buildUI()`, add:

```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handlePaletteDidChange(_:)),
    name: .paletteDidChange,
    object: nil
)
// Sync with whatever the menu bar already has selected
if let appDelegate = NSApp.delegate as? AppDelegate {
    selectPalette(appDelegate.activePalette)
}
```

- [ ] **Step 2: Add handler and deregister**

After the closing `}` of `mouseDown`, add:

```swift
@objc private func handlePaletteDidChange(_ notification: Notification) {
    guard let raw = notification.userInfo?["palette"] as? Int,
          let palette = TilePalette(rawValue: raw) else { return }
    selectPalette(palette)
}

override func willMove(from view: SKView) {
    NotificationCenter.default.removeObserver(self)
}
```

- [ ] **Step 3: Build and verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 4: Commit**

```bash
git add Quadromania/TitleScene.swift
git commit -m "feat: TitleScene observes paletteDidChange notification"
```

---

### Task 5: GamePlayScene — observe palette and symbol notifications

**Files:**
- Modify: `Quadromania/GamePlayScene.swift`

- [ ] **Step 1: Add Cocoa import**

At the top of `GamePlayScene.swift`, add `import Cocoa` after `import SpriteKit` (needed for `NSApp`).

- [ ] **Step 2: Register observers in didMove(to:)**

In `didMove(to:)`, after `buildUI()`, add:

```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handlePaletteDidChange(_:)),
    name: .paletteDidChange,
    object: nil
)
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleSymbolOverlayDidChange(_:)),
    name: .symbolOverlayDidChange,
    object: nil
)
```

- [ ] **Step 3: Sync initial state after buildUI**

In `buildUI()`, after `addChild(tileGrid)`, add:

```swift
if let appDelegate = NSApp.delegate as? AppDelegate {
    tileGrid.symbolOverlayEnabled = appDelegate.symbolOverlayEnabled
}
```

- [ ] **Step 4: Add handlers and deregister**

After `returnToTitle()`, add:

```swift
@objc private func handlePaletteDidChange(_ notification: Notification) {
    guard let raw = notification.userInfo?["palette"] as? Int,
          let palette = TilePalette(rawValue: raw) else { return }
    tileGrid.applyPalette(palette)
}

@objc private func handleSymbolOverlayDidChange(_ notification: Notification) {
    guard let enabled = notification.userInfo?["enabled"] as? Bool else { return }
    tileGrid.symbolOverlayEnabled = enabled
}

override func willMove(from view: SKView) {
    NotificationCenter.default.removeObserver(self)
}
```

- [ ] **Step 5: Build and verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 6: Commit**

```bash
git add Quadromania/GamePlayScene.swift
git commit -m "feat: GamePlayScene observes palette and symbol overlay notifications"
```

---

## Chunk 3: Instructions and tutorial

### Task 6: Update InstructionsScene

**Files:**
- Modify: `Quadromania/InstructionsScene.swift`

New body text, a "Start Tutorial" button, and hit-tested `mouseDown`.

- [ ] **Step 1: Replace body text and add tutorial button**

Add `private var tutorialButtonLabel: SKLabelNode?` as a property in the class body, above `didMove(to:)`.

Then replace the entire `buildUI()` method with:

```swift
private func buildUI() {
    backgroundColor = SKColor(red: 0.06, green: 0.08, blue: 0.14, alpha: 1)

    // Title
    let title = SKLabelNode(text: "HOW TO PLAY")
    title.fontName  = "Helvetica-Bold"
    title.fontSize  = 52
    title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
    title.horizontalAlignmentMode = .center
    title.position = CGPoint(x: size.width / 2, y: 860)
    addChild(title)

    let lines: [(String, Bool)] = [
        ("Goal",                       true),
        ("Return every tile to its lightest color (color 0) before you run out of turns.", false),
        ("",                           false),
        ("How to play",                true),
        ("Click any tile to rotate the 3×3 block centered on it.", false),
        ("Every tile in that block steps forward by one color.", false),
        ("Tiles at the maximum color wrap back to 0.", false),
        ("",                           false),
        ("Turn limit",                 true),
        ("Your total turns are shown at the top of the screen.", false),
        ("Each click costs one turn — plan carefully.", false),
        ("",                           false),
        ("Scoring",                    true),
        ("Solve the board faster to score higher.", false),
        ("Exceeding the turn limit ends the game with no score recorded.", false),
        ("",                           false),
        ("Tip",                        true),
        ("Click tiles in the middle of large same-color regions to make progress efficiently.", false),
    ]

    var y: CGFloat = 790
    for (text, isHeader) in lines {
        guard !text.isEmpty else { y -= 16; continue }
        let label = SKLabelNode(text: text)
        label.fontName  = isHeader ? "Helvetica-Bold" : "Helvetica"
        label.fontSize  = isHeader ? 24 : 20
        label.fontColor = isHeader
            ? SKColor(red: 0.60, green: 0.85, blue: 1.0, alpha: 1)
            : SKColor(white: 0.88, alpha: 1)
        label.horizontalAlignmentMode = .left
        label.position = CGPoint(x: 100, y: y)
        addChild(label)
        y -= isHeader ? 34 : 28
    }

    // Tutorial button
    let tutBtn = SKLabelNode(text: "▶  Start Tutorial")
    tutBtn.fontName  = "Helvetica-Bold"
    tutBtn.fontSize  = 26
    tutBtn.fontColor = SKColor(red: 0.60, green: 0.85, blue: 1.0, alpha: 1)
    tutBtn.horizontalAlignmentMode = .center
    tutBtn.position = CGPoint(x: size.width / 2, y: 120)
    addChild(tutBtn)
    tutorialButtonLabel = tutBtn

    // Footer hint
    let hint = SKLabelNode(text: "Click anywhere else to return")
    hint.fontName  = "Helvetica"
    hint.fontSize  = 18
    hint.fontColor = SKColor(white: 0.45, alpha: 1)
    hint.horizontalAlignmentMode = .center
    hint.position = CGPoint(x: size.width / 2, y: 60)
    hint.run(.repeatForever(.sequence([
        .fadeAlpha(to: 0.2, duration: 0.9),
        .fadeAlpha(to: 1.0, duration: 0.9)
    ])))
    addChild(hint)
}
```


- [ ] **Step 2: Replace mouseDown with hit-tested routing**

Replace the existing `mouseDown` method:

```swift
override func mouseDown(with event: NSEvent) {
    let point = event.location(in: self)
    if let btn = tutorialButtonLabel, btn.frame.contains(point) {
        SoundManager.shared.playEffect(.menu)
        let scene = TutorialScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    } else {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }
}
```

- [ ] **Step 3: Build and verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 4: Commit**

```bash
git add Quadromania/InstructionsScene.swift
git commit -m "feat: update instructions text and add Start Tutorial button"
```

---

### Task 7: Create TutorialScene

**Files:**
- Create: `Quadromania/TutorialScene.swift`

Note: all scene files live directly in `Quadromania/` (no `Scenes/` subdirectory). The project uses Xcode 16+ file-system synchronization so the new file is auto-included in the build target when placed in this directory.

Standalone 5×4 tutorial scene. Does not use `TileGridNode` or `GameModel` — manages its own playfield and tile sprites.

**Pre-computed tutorial data:** The scrambled board and 3 steps were verified by hand:

Scrambled `tutorialPlayfield[col][row]`:
- col 0: `[1, 1, 1, 0]`
- col 1: `[1, 0, 0, 1]`
- col 2: `[0, 1, 1, 1]`
- col 3: `[1, 0, 0, 1]`
- col 4: `[1, 1, 1, 0]`

Steps: (col=1,row=1), (col=3,row=1), (col=2,row=2) — applying all three produces all zeros.

- [ ] **Step 1: Create TutorialScene.swift**

```swift
// TutorialScene.swift
// Scripted tutorial — 5×4 grid with 3 guided steps.

import SpriteKit
import Cocoa

class TutorialScene: SKScene {

    // MARK: - Types

    private struct TutorialStep {
        let message: String
        let targetCol: Int   // valid range 1–3 for 5-wide grid
        let targetRow: Int   // valid range 1–2 for 4-tall grid
    }

    // MARK: - Constants

    private let cols = 5
    private let rows = 4
    private let tileSize: CGFloat = 60
    private let tutorialMaxColors = 1
    private let palette = TilePalette.spring

    // MARK: - Tutorial data
    // Scrambled board that is solved by executing the steps in order.
    // Verified: applying steps (1,1), (3,1), (2,2) to this board produces all zeros.
    private let initialPlayfield: [[Int]] = [
        [1, 1, 1, 0],  // col 0
        [1, 0, 0, 1],  // col 1
        [0, 1, 1, 1],  // col 2
        [1, 0, 0, 1],  // col 3
        [1, 1, 1, 0],  // col 4
    ]

    private let steps: [TutorialStep] = [
        TutorialStep(
            message: "Welcome! Your goal: turn all tiles to the lightest color.\nClick the highlighted tile to rotate the 3×3 block around it.",
            targetCol: 1, targetRow: 1
        ),
        TutorialStep(
            message: "Every tile in that block incremented by one color.\nNow click the next highlighted tile.",
            targetCol: 3, targetRow: 1
        ),
        TutorialStep(
            message: "Notice how tiles at the maximum color wrap back to 0.\nLast move — solve the board!",
            targetCol: 2, targetRow: 2
        ),
    ]

    // MARK: - State

    private var tutorialPlayfield: [[Int]] = []
    private var tileSprites: [[SKSpriteNode]] = []
    private var currentStepIndex = 0
    private var pulseRing: SKShapeNode?
    private var messageLabel: SKLabelNode!
    private var messagePanel: SKShapeNode!
    private var backButton: SKLabelNode?
    private var skipButton: SKLabelNode!

    // MARK: - Layout helpers

    private var gridOriginX: CGFloat {
        let gridW = CGFloat(cols) * tileSize
        return (size.width - gridW) / 2
    }
    private let gridBottomY: CGFloat = 580

    private func tilePosition(col: Int, row: Int) -> CGPoint {
        // row 0 at the top; flip Y so row 0 renders highest
        let x = gridOriginX + CGFloat(col) * tileSize + tileSize / 2
        let y = gridBottomY + CGFloat(rows - 1 - row) * tileSize + tileSize / 2
        return CGPoint(x: x, y: y)
    }

    // MARK: - Lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.10, green: 0.08, blue: 0.15, alpha: 1)
        tutorialPlayfield = initialPlayfield.map { $0 }
        buildUI()
        showStep(0)
    }

    // MARK: - Build UI

    private func buildUI() {
        // Title
        let title = SKLabelNode(text: "Tutorial")
        title.fontName  = "Helvetica-Bold"
        title.fontSize  = 48
        title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
        title.horizontalAlignmentMode = .center
        title.position = CGPoint(x: size.width / 2, y: 880)
        addChild(title)

        // Tile grid
        buildTutorialGrid()

        // Message panel
        let panelRect = CGRect(x: (size.width - 700) / 2, y: 400,
                               width: 700, height: 120)
        let panel = SKShapeNode(rect: panelRect, cornerRadius: 12)
        panel.fillColor   = SKColor(white: 0.12, alpha: 0.95)
        panel.strokeColor = SKColor(white: 0.4, alpha: 0.5)
        panel.lineWidth   = 1.5
        addChild(panel)
        messagePanel = panel

        let msg = SKLabelNode()
        msg.fontName               = "Helvetica"
        msg.fontSize               = 20
        msg.fontColor              = SKColor(white: 0.92, alpha: 1)
        msg.horizontalAlignmentMode = .center
        msg.verticalAlignmentMode  = .center
        msg.numberOfLines          = 0
        msg.preferredMaxLayoutWidth = 660
        msg.position = CGPoint(x: size.width / 2, y: 462)
        addChild(msg)
        messageLabel = msg

        // Skip button
        let skip = SKLabelNode(text: "Skip Tutorial")
        skip.fontName  = "Helvetica"
        skip.fontSize  = 22
        skip.fontColor = SKColor(white: 0.50, alpha: 1)
        skip.horizontalAlignmentMode = .right
        skip.position = CGPoint(x: 1200, y: 920)
        addChild(skip)
        skipButton = skip
    }

    private func buildTutorialGrid() {
        let colors = palette.colors
        let drawSize = CGSize(width: tileSize - 2, height: tileSize - 2)
        tileSprites = Array(repeating: [], count: cols)
        for col in 0..<cols {
            tileSprites[col] = []
            for row in 0..<rows {
                let sprite = SKSpriteNode(
                    color: colors[tutorialPlayfield[col][row]],
                    size: drawSize
                )
                sprite.position = tilePosition(col: col, row: row)
                addChild(sprite)
                tileSprites[col].append(sprite)
            }
        }
    }

    // MARK: - Step management

    private func showStep(_ index: Int) {
        guard index < steps.count else { return }
        let step = steps[index]
        messageLabel.text = step.message
        placePulseRing(col: step.targetCol, row: step.targetRow)
    }

    private func placePulseRing(col: Int, row: Int) {
        pulseRing?.removeFromParent()
        let ring = SKShapeNode(circleOfRadius: tileSize * 0.52)
        ring.strokeColor = SKColor(red: 1.0, green: 0.9, blue: 0.0, alpha: 0.9)
        ring.lineWidth   = 3
        ring.fillColor   = .clear
        ring.position    = tilePosition(col: col, row: row)
        ring.zPosition   = 5
        ring.run(.repeatForever(.sequence([
            .scale(to: 1.15, duration: 0.4),
            .scale(to: 1.00, duration: 0.4)
        ])))
        addChild(ring)
        pulseRing = ring
    }

    private func advanceStep() {
        currentStepIndex += 1
        if currentStepIndex < steps.count {
            showStep(currentStepIndex)
        } else {
            // All steps complete
            pulseRing?.removeFromParent()
            pulseRing = nil
            messageLabel.text = "You're ready to play! Good luck."
            showBackButton()
        }
    }

    private func showBackButton() {
        let btn = SKLabelNode(text: "Back to Menu")
        btn.fontName  = "Helvetica-Bold"
        btn.fontSize  = 28
        btn.fontColor = SKColor(white: 0.90, alpha: 1)
        btn.horizontalAlignmentMode = .center
        btn.position = CGPoint(x: size.width / 2, y: 80)
        addChild(btn)
        backButton = btn
    }

    // MARK: - Rotation

    private func applyRotation(col: Int, row: Int) {
        for dc in -1...1 {
            for dr in -1...1 {
                let c = col + dc, r = row + dr
                guard c >= 0, c < cols, r >= 0, r < rows else { continue }
                tutorialPlayfield[c][r] += 1
                if tutorialPlayfield[c][r] > tutorialMaxColors {
                    tutorialPlayfield[c][r] = 0
                }
            }
        }
        refreshTileColors()
    }

    private func refreshTileColors() {
        let colors = palette.colors
        for col in 0..<cols {
            for row in 0..<rows {
                let idx = tutorialPlayfield[col][row]
                tileSprites[col][row].run(
                    SKAction.colorize(with: colors[idx],
                                      colorBlendFactor: 1.0,
                                      duration: 0.18)
                )
            }
        }
    }

    // MARK: - Wrong-tile flash

    private func flashWrongTile() {
        messagePanel.run(SKAction.customAction(withDuration: 0.35) { node, t in
            guard let panel = node as? SKShapeNode else { return }
            panel.fillColor = t < 0.17
                ? SKColor.red.withAlphaComponent(0.35)
                : SKColor(white: 0.12, alpha: 0.95)
        })
    }

    // MARK: - Input

    override func mouseDown(with event: NSEvent) {
        let point = event.location(in: self)

        // Skip button
        if skipButton.frame.contains(point) {
            returnToTitle()
            return
        }

        // Back to Menu button (shown after completion)
        if let btn = backButton, btn.frame.contains(point) {
            returnToTitle()
            return
        }

        // Ignore clicks after completion
        guard currentStepIndex < steps.count else { return }

        // Convert point to grid col/row
        let col = Int((point.x - gridOriginX) / tileSize)
        let screenRow = Int((point.y - gridBottomY) / tileSize)
        let row = (rows - 1) - screenRow

        guard col >= 0, col < cols, row >= 0, row < rows else { return }

        let step = steps[currentStepIndex]
        if col == step.targetCol && row == step.targetRow {
            SoundManager.shared.playEffect(.turn)
            applyRotation(col: col, row: row)
            advanceStep()
        } else {
            flashWrongTile()
        }
    }

    // MARK: - Navigation

    private func returnToTitle() {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    }
}
```

- [ ] **Step 2: Build and verify**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
```

Expected: `Build succeeded`

- [ ] **Step 3: Commit**

```bash
git add Quadromania/TutorialScene.swift
git commit -m "feat: add TutorialScene with scripted 3-step walkthrough"
```

---

## Chunk 4: Documentation

### Task 8: Update CLAUDE.md and create README

**Files:**
- Modify: `CLAUDE.md`
- Create: `README.md`

- [ ] **Step 1: Update CLAUDE.md**

Replace the entire contents of `CLAUDE.md` with:

```markdown
# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

Open `Quadromania.xcodeproj` in Xcode and use **Cmd+R** to build and run, or from the terminal:

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
```

Run the unit tests (game logic only, no UI):

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test
```

There are 19 unit tests in the `QuadroTests` XCTest target covering `GameModel` and `PuzzleGenerator`.

## Game Concept

Quadromania is a puzzle game where players restore an **18×13 grid** of colored tiles back to color 0 by clicking the center of any 3×3 section, which increments every tile in that section by 1 (cycling back to 0 at the max color). The computer scrambles the board at the start; players have a fixed turn limit to solve it.

- **Grid**: 18 columns × 13 rows = 234 tiles
- **Colors**: 2 up to 5 (configurable per game via "Select colors")
- **Color palettes**: 4 themes (Spring, Ocean, Sunset, Forest); selectable in title screen and menu bar
- **Accessibility**: "Color Symbols" toggle in Palette menu adds shape symbols to tiles
- **Turn limit**: `initialRotations × maxColors`
- **Score**: `((limit - turns) × 10000) / turns`, or 0 if limit exceeded
- **Level rotations**: `56 + (11 − level) × 13` initial scramble rotations (level 1 = 186 rotations = easiest; level 10 = 69 rotations = hardest)

## Architecture

Quadromania is a **macOS SpriteKit game** written in Swift. The entrypoint is `GameViewController`, which creates an `SKView` at **1280×960** and presents `TitleScene`.

### Targets

| Target | Type | Contents |
|--------|------|----------|
| `Quadromania` | macOS App | All UI/scene code, `TilePalette` |
| `QuadroCore` | Static Library | `GameModel`, `PuzzleGenerator` (pure logic, no SpriteKit) |
| `QuadroTests` | XCTest | 19 tests for `QuadroCore` |

`QuadroCore` exists so game logic can be tested without instantiating any SpriteKit scene.

### Scene Flow

```
AppDelegate
  └─ GameViewController (SKView 1280×960)
       └─ TitleScene          ← main menu
            ├─ GamePlayScene  ← active gameplay
            │    └─ TitleScene (on game end)
            ├─ InstructionsScene
            │    └─ TutorialScene  ← scripted 3-step tutorial
            ├─ HighscoreScene
            └─ TitleScene (quit via NSApp.terminate)
```

### Source Files

| File | Responsibility |
|------|----------------|
| `AppDelegate.swift` | App lifecycle; Sound menu (music toggle); Palette menu (palette + symbol overlay) |
| `GameViewController.swift` | Hosts `SKView`; presents `TitleScene` on launch |
| `SoundManager.swift` | AVFoundation singleton; background music + 4 WAV effects |
| `TilePalette.swift` | `TilePalette` enum — 4 color themes (Spring/Ocean/Sunset/Forest), 5 `SKColor`s each |
| `QuadroCore/GameModel.swift` | Pure game logic: 18×13 playfield, rotate, win/loss, scoring |
| `QuadroCore/PuzzleGenerator.swift` | Generates solvable puzzles via backwards construction |
| `Models/HighscoreManager.swift` | Persistent highscore tables (10 levels × 10 entries, `UserDefaults`) |
| `Scenes/TitleScene.swift` | Main menu scene; 2×2 palette swatch grid; menu bar sync |
| `Scenes/GamePlayScene.swift` | Active gameplay scene; tile grid; win/loss overlays; menu bar sync |
| `Scenes/InstructionsScene.swift` | Instructions screen with "Start Tutorial" button |
| `Scenes/TutorialScene.swift` | Scripted tutorial; standalone 5×4 grid; 3-step walkthrough |
| `Scenes/HighscoreScene.swift` | Highscore display scene |
| `Nodes/TileGridNode.swift` | 18×13 grid of `SKSpriteNode` tiles; palette swap; symbol overlay |

### Menu Bar

`AppDelegate` injects two custom menus at launch:

| Menu | Items |
|------|-------|
| Sound | Start/Stop Music |
| Palette | 🌸 Spring / 🌊 Ocean / 🌅 Sunset / 🌿 Forest / — / Color Symbols |

Selecting a palette posts `Notification.Name.paletteDidChange`; toggling symbols posts `.symbolOverlayDidChange`. Both `TitleScene` and `GamePlayScene` observe these.

### Existing Working Components

**`SoundManager`** (`SoundManager.shared`) wraps AVFoundation:
- Background music — `sounds/music.m4a`, loops forever.
- Four WAV effects in `sounds/`: `menu.wav`, `turn.wav`, `win.wav`, `loose.wav`.

## Key Conventions

- **No SDL, no C/C++ code** — all logic is Swift. The `old_src/` directory is reference-only.
- `GameViewController` creates its view programmatically — canonical window size is **1280×960**.
- Game logic lives in `GameModel` (in `QuadroCore`), completely decoupled from SpriteKit.
- Tile colors are `Int` values 0…`maxColors`. Color 0 is always the goal state.
- Rotation algorithm: every tile in the 3×3 block centered at (x,y) increments by 1; if `value > maxColors`, reset to 0.
- Coordinate system: SpriteKit uses bottom-left origin; mouse clicks are converted to grid (col, row) via `TileGridNode.gridCoordinates(for:)`.
- Highscores are persisted via `UserDefaults`. 10 difficulty levels, 10 entries per level.
```

- [ ] **Step 2: Create README.md**

Create `/Users/marcojanker/development/quadromania/README.md`:

```markdown
# Quadromania

A macOS puzzle game. Restore an 18×13 grid of colored tiles to their lightest color before you run out of turns.

## Build

Open `Quadromania.xcodeproj` in Xcode and press **Cmd+R**, or from the terminal:

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
```

Requires macOS 13+ and Xcode 15+.

## How to play

1. **Goal** — Return every tile to its lightest color (color 0) before you run out of turns.
2. **Click** any tile to rotate the 3×3 block centered on it. Every tile in that block steps forward by one color. Tiles at the maximum color wrap back to 0.
3. **Turn limit** — shown at the top. Each click costs one turn. Plan carefully.
4. **Scoring** — solve the board in fewer turns to score higher.

## Options

- **Select colors** — sets how many color steps tiles cycle through (1–4 extra colors).
- **Select level** — controls how scrambled the board starts (1 = easiest, 10 = hardest).
- **Palette** menu — choose from Spring 🌸, Ocean 🌊, Sunset 🌅, or Forest 🌿.
- **Color Symbols** — adds shape symbols to tiles for colorblind accessibility.

## Tutorial

Open **Instructions** from the main menu, then click **Start Tutorial** for a guided walkthrough on a small grid.
```

- [ ] **Step 3: Build and run tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "(error:|Build succeeded|Build failed)"
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "(error:|Test Suite|passed|failed)"
```

Expected: build succeeded, all 19 tests pass.

- [ ] **Step 4: Commit**

```bash
git add CLAUDE.md README.md
git commit -m "docs: update CLAUDE.md and add README"
```

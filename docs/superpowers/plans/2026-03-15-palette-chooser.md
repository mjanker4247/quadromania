# Palette Chooser Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a 2×2 clickable palette swatch grid to the TitleScene menu, letting players choose one of four color themes (Spring, Ocean, Sunset, Forest) before starting a game.

**Architecture:** A new `TilePalette` enum owns all color data. It flows from `TitleScene` → `GamePlayScene` → `TileGridNode`. Three existing files have a compile dependency triangle and must be updated together before building.

**Tech Stack:** Swift, SpriteKit, macOS. No new frameworks.

---

## Chunk 1: TilePalette enum

### Task 1: Create TilePalette.swift

**Files:**
- Create: `Quadromania/TilePalette.swift`

This file has no dependencies on any other file being changed — create and commit it alone.

- [ ] **Step 1: Create `Quadromania/TilePalette.swift`** with the full content below.

```swift
// TilePalette.swift
// Four named colour palettes for the tile grid.
// Index 0 is always the goal/win colour.

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

    /// Five colours: index 0 = goal state, 1–4 = scramble colours.
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

- [ ] **Step 2: Build to confirm the new file compiles in isolation**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

> Note: `TileGridNode` still references the old `static let tileColors` — that's fine, we haven't changed it yet. The build should pass because we've only *added* a new file.

- [ ] **Step 3: Commit**

```bash
git add Quadromania/TilePalette.swift
git commit -m "feat: add TilePalette enum with 4 spring/ocean/sunset/forest palettes"
```

---

## Chunk 2: Wire palette through TileGridNode, GamePlayScene, TitleScene

### Task 2: Update the three dependent files atomically

**Files:**
- Modify: `Quadromania/TileGridNode.swift`
- Modify: `Quadromania/GamePlayScene.swift`
- Modify: `Quadromania/TitleScene.swift`

⚠️ **Important:** These three files form a compile dependency triangle. Do NOT build between individual file edits — edit all three files first, then build once.

- `TileGridNode.tileColors` is removed → breaks `TitleScene.addColorDots()`
- `TileGridNode.init` gains `palette:` → breaks `GamePlayScene.buildUI()`
- `GamePlayScene.init` gains `palette:` → breaks `TitleScene.mouseDown(.startGame)`

#### Step 1: Replace TileGridNode.swift

- [ ] **Replace the full content of `Quadromania/TileGridNode.swift`** with:

```swift
// TileGridNode.swift
// 18×13 grid of colored tile sprites — port of graphics/renderer.c dot drawing.
// Tiles are drawn programmatically; no image assets required.

import SpriteKit
import QuadroCore

class TileGridNode: SKNode {

    // MARK: - Layout

    static let tileSize: CGFloat = 64

    /// Total pixel width of the grid.
    static var gridPixelWidth:  CGFloat { CGFloat(GameModel.gridWidth)  * tileSize }
    /// Total pixel height of the grid.
    static var gridPixelHeight: CGFloat { CGFloat(GameModel.gridHeight) * tileSize }

    // MARK: - State

    private let palette: TilePalette
    private var tiles: [[SKSpriteNode]] = []
    private var colorIndices: [[Int]] = []

    // MARK: - Init

    init(playfield: [[Int]], palette: TilePalette) {
        self.palette = palette
        super.init()
        buildGrid(playfield: playfield)
    }

    required init?(coder: NSCoder) { fatalError("Use init(playfield:palette:)") }

    // MARK: - Build

    private func buildGrid(playfield: [[Int]]) {
        let s = TileGridNode.tileSize
        let gap: CGFloat = 2          // 2 px gap between tiles
        let tileDrawSize = CGSize(width: s - gap, height: s - gap)

        tiles = Array(repeating: [], count: GameModel.gridWidth)
        colorIndices = Array(repeating: Array(repeating: 0, count: GameModel.gridHeight),
                             count: GameModel.gridWidth)

        for col in 0..<GameModel.gridWidth {
            tiles[col] = []
            for row in 0..<GameModel.gridHeight {
                let colorIndex = playfield[col][row]
                let sprite = SKSpriteNode(
                    color: palette.colors[colorIndex],
                    size: tileDrawSize
                )
                // Row 0 renders at the top (flip Y so row 0 is highest on screen)
                sprite.position = CGPoint(
                    x: CGFloat(col) * s + s / 2,
                    y: CGFloat(GameModel.gridHeight - 1 - row) * s + s / 2
                )
                addChild(sprite)
                tiles[col].append(sprite)
                colorIndices[col][row] = colorIndex
            }
        }
    }

    // MARK: - Update

    /// Refresh tile colors from the current playfield state, animating any changes.
    func updateAll(from playfield: [[Int]]) {
        for col in 0..<GameModel.gridWidth {
            for row in 0..<GameModel.gridHeight {
                let newIndex = playfield[col][row]
                guard newIndex != colorIndices[col][row] else { continue }
                colorIndices[col][row] = newIndex
                let action = SKAction.colorize(
                    with: palette.colors[newIndex],
                    colorBlendFactor: 1.0,
                    duration: 0.18
                )
                tiles[col][row].run(action)
            }
        }
    }

    // MARK: - Hit testing

    /// Convert a point in this node's local coordinate space to grid (col, row).
    /// Returns nil if the point is outside the grid bounds.
    func gridCoordinates(for localPoint: CGPoint) -> (col: Int, row: Int)? {
        let s = TileGridNode.tileSize
        let col = Int(localPoint.x / s)
        let screenRow = Int(localPoint.y / s)
        let row = (GameModel.gridHeight - 1) - screenRow

        guard col  >= 0, col  < GameModel.gridWidth,
              row  >= 0, row  < GameModel.gridHeight else { return nil }
        return (col, row)
    }
}
```

#### Step 2: Replace GamePlayScene.swift

- [ ] **Replace the full content of `Quadromania/GamePlayScene.swift`** with:

```swift
// GamePlayScene.swift
// Active gameplay scene — port of state_manager handle_game_state() + Quadromania_DrawPlayfield().

import SpriteKit
import QuadroCore

class GamePlayScene: SKScene {

    // MARK: - State

    private let model: GameModel
    private let palette: TilePalette
    private var tileGrid: TileGridNode!
    private var turnsLabel: SKLabelNode!
    private var limitLabel: SKLabelNode!
    private var waitingForClick = false   // true after win/loss, before transition

    // MARK: - Init

    init(model: GameModel, palette: TilePalette, size: CGSize) {
        self.model   = model
        self.palette = palette
        super.init(size: size)
    }

    required init?(coder: NSCoder) { fatalError("Use init(model:palette:size:)") }

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(
            red: CGFloat(model.backgroundArtIndex) * 0.05 + 0.05,
            green: 0.08,
            blue: CGFloat(model.backgroundArtIndex) * 0.03 + 0.06,
            alpha: 1
        )
        buildUI()
    }

    // MARK: - UI

    private func buildUI() {
        // --- Grid ---
        tileGrid = TileGridNode(playfield: model.playfield, palette: palette)

        // Centre the grid horizontally; leave 50 px at the bottom for HUD.
        let gridX = (size.width  - TileGridNode.gridPixelWidth)  / 2
        let gridY: CGFloat = 50
        tileGrid.position = CGPoint(x: gridX, y: gridY)
        addChild(tileGrid)

        // --- HUD labels ---
        let labelY: CGFloat = 18

        turnsLabel = makeHudLabel(text: turnsText)
        turnsLabel.horizontalAlignmentMode = .left
        turnsLabel.position = CGPoint(x: gridX, y: labelY)
        addChild(turnsLabel)

        limitLabel = makeHudLabel(text: limitText)
        limitLabel.horizontalAlignmentMode = .right
        limitLabel.position = CGPoint(x: gridX + TileGridNode.gridPixelWidth, y: labelY)
        addChild(limitLabel)

        // Version string (bottom-left of screen, like the original)
        let versionLabel = makeHudLabel(text: "v0.0.1")
        versionLabel.horizontalAlignmentMode = .left
        versionLabel.position = CGPoint(x: 8, y: labelY)
        versionLabel.fontColor = SKColor(white: 0.4, alpha: 1)
        addChild(versionLabel)
    }

    private func makeHudLabel(text: String) -> SKLabelNode {
        let label = SKLabelNode(text: text)
        label.fontName  = "Helvetica"
        label.fontSize  = 20
        label.fontColor = SKColor(white: 0.85, alpha: 1)
        return label
    }

    // MARK: - HUD helpers

    private var turnsText: String { "Turns: \(model.turns)" }
    private var limitText: String { "Limit: \(model.limit)" }

    private func updateHUD() {
        turnsLabel.text = turnsText
        limitLabel.text = limitText
    }

    // MARK: - Input

    override func mouseDown(with event: NSEvent) {
        if waitingForClick {
            returnToTitle()
            return
        }

        let scenePoint = event.location(in: self)
        let localPoint = tileGrid.convert(scenePoint, from: self)
        guard let (col, row) = tileGrid.gridCoordinates(for: localPoint) else { return }

        // Valid interior click: same bounds as original (col 1–16, row 1–11)
        guard col >= 1, col <= 16, row >= 1, row <= 11 else { return }

        model.rotate(x: col, y: row)
        tileGrid.updateAll(from: model.playfield)
        updateHUD()
        SoundManager.shared.playEffect(.turn)

        if model.isTurnLimitHit {
            showGameOver()
        } else if model.isGameWon {
            showWin()
        }
    }

    // MARK: - Win / Loss overlays

    private func showWin() {
        SoundManager.shared.playEffect(.win)
        showOverlay(
            message: "You solved it!",
            subtext: "Score: \(model.score)",
            color: SKColor(red: 0.0, green: 0.15, blue: 0.55, alpha: 0.88)
        )
        recordHighscoreIfQualifies()
        waitingForClick = true
    }

    private func showGameOver() {
        SoundManager.shared.playEffect(.loose)
        showOverlay(
            message: "Turn limit hit!",
            subtext: "Better luck next time",
            color: SKColor(red: 0.55, green: 0.05, blue: 0.05, alpha: 0.88)
        )
        waitingForClick = true
    }

    private func showOverlay(message: String, subtext: String, color: SKColor) {
        let pad: CGFloat = 24
        let boxW = size.width * 0.6
        let boxH: CGFloat = 120
        let boxX = (size.width  - boxW) / 2
        let boxY = (size.height - boxH) / 2

        let box = SKShapeNode(rect: CGRect(x: boxX, y: boxY, width: boxW, height: boxH), cornerRadius: 12)
        box.fillColor   = color
        box.strokeColor = SKColor(white: 1, alpha: 0.4)
        box.lineWidth   = 2
        box.zPosition   = 10
        addChild(box)

        let msgLabel = SKLabelNode(text: message)
        msgLabel.fontName  = "Helvetica-Bold"
        msgLabel.fontSize  = 36
        msgLabel.fontColor = .white
        msgLabel.position  = CGPoint(x: boxX + boxW / 2, y: boxY + boxH - pad - 36)
        msgLabel.zPosition = 11
        addChild(msgLabel)

        let subLabel = SKLabelNode(text: subtext)
        subLabel.fontName  = "Helvetica"
        subLabel.fontSize  = 22
        subLabel.fontColor = SKColor(white: 0.9, alpha: 1)
        subLabel.position  = CGPoint(x: boxX + boxW / 2, y: boxY + pad)
        subLabel.zPosition = 11
        addChild(subLabel)

        let hint = SKLabelNode(text: "Click to continue")
        hint.fontName  = "Helvetica"
        hint.fontSize  = 16
        hint.fontColor = SKColor(white: 0.6, alpha: 1)
        hint.position  = CGPoint(x: size.width / 2, y: boxY - 30)
        hint.zPosition = 11
        addChild(hint)
    }

    // MARK: - Highscore

    private func recordHighscoreIfQualifies() {
        let tableIndex = model.level - 1   // level 1–10 → table 0–9
        let hs = HighscoreManager.shared
        if let pos = hs.position(forTable: tableIndex, score: model.score) {
            hs.enterScore(model.score,
                          name: hs.nameFromTimestamp,
                          table: tableIndex,
                          at: pos)
        }
    }

    // MARK: - Navigation

    private func returnToTitle() {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.4))
    }
}
```

#### Step 3: Replace TitleScene.swift

- [ ] **Replace the full content of `Quadromania/TitleScene.swift`** with:

```swift
// TitleScene.swift
// Main menu scene — port of gui.c GUI_DrawMainmenu() + handle_title_state().

import SpriteKit
import QuadroCore

class TitleScene: SKScene {

    // MARK: - Menu item enum
    private enum MenuItem: CaseIterable {
        case startGame, selectColors, selectTurns, instructions, highscores, quit
    }

    // MARK: - Game configuration
    private var selectedColors  = 1              // 1–4
    private var selectedLevel   = 1              // 1–10
    private var selectedPalette: TilePalette = .spring

    // MARK: - UI nodes
    private var menuLabels:    [MenuItem: SKLabelNode] = [:]
    private var colorDotNodes: [SKSpriteNode] = []
    private var rotationsValueLabel: SKLabelNode!
    private var highlightedItem: MenuItem? = nil
    private var paletteContainerNodes: [TilePalette: SKNode]      = [:]
    private var paletteBorderNodes:    [TilePalette: SKShapeNode] = [:]

    // MARK: - Layout constants
    private let menuX:       CGFloat = 110
    private let menuStartY:  CGFloat = 680
    private let lineSpacing: CGFloat = 52

    // MARK: - Colors
    private let normalColor:    SKColor = .white
    private let highlightColor: SKColor = SKColor(red: 1.0, green: 0.9, blue: 0.0, alpha: 1)

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.10, green: 0.08, blue: 0.15, alpha: 1)
        buildUI()
    }

    // MARK: - Build UI

    private func buildUI() {
        addTitleLabel()
        addMenuLabels()
        addColorDots()
        updateDynamicLabels()
        addPaletteGrid()
    }

    private func addTitleLabel() {
        let title = SKLabelNode(text: "QUADROMANIA")
        title.fontName  = "Helvetica-Bold"
        title.fontSize  = 72
        title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
        title.horizontalAlignmentMode = .center
        title.position  = CGPoint(x: size.width / 2, y: 840)
        addChild(title)

        let sub = SKLabelNode(text: "Puzzle Game")
        sub.fontName  = "Helvetica"
        sub.fontSize  = 22
        sub.fontColor = SKColor(white: 0.55, alpha: 1)
        sub.horizontalAlignmentMode = .center
        sub.position  = CGPoint(x: size.width / 2, y: 800)
        addChild(sub)
    }

    private func addMenuLabels() {
        // Items 0–2 keep uniform spacing.
        // Items 3–5 are shifted down by 2 extra lineSpacings to make room for the palette grid.
        let itemY: [MenuItem: CGFloat] = [
            .startGame:    menuStartY,
            .selectColors: menuStartY - lineSpacing,
            .selectTurns:  menuStartY - 2 * lineSpacing,
            .instructions: menuStartY - 5 * lineSpacing,
            .highscores:   menuStartY - 6 * lineSpacing,
            .quit:         menuStartY - 7 * lineSpacing,
        ]

        let items: [(MenuItem, String)] = [
            (.startGame,    "Start the game"),
            (.selectColors, "Select colors:"),
            (.selectTurns,  "Select level:"),
            (.instructions, "Instructions"),
            (.highscores,   "Highscores"),
            (.quit,         "Quit"),
        ]

        for (item, text) in items {
            let label = SKLabelNode(text: text)
            label.fontName  = "Helvetica-Bold"
            label.fontSize  = 32
            label.fontColor = normalColor
            label.horizontalAlignmentMode = .left
            label.position = CGPoint(x: menuX, y: itemY[item]!)
            addChild(label)
            menuLabels[item] = label
        }
    }

    private func addColorDots() {
        // Small colored squares displayed next to "Select colors:"
        colorDotNodes.forEach { $0.removeFromParent() }
        colorDotNodes = []

        let dotSize: CGFloat = 24
        let dotSpacing: CGFloat = 30
        let startX = menuX + 240
        let y = menuStartY - lineSpacing + dotSize / 2 - 8   // align with selectColors row

        for i in 0...selectedColors {
            let dot = SKSpriteNode(
                color: selectedPalette.colors[i],
                size: CGSize(width: dotSize, height: dotSize)
            )
            dot.position = CGPoint(x: startX + CGFloat(i) * dotSpacing, y: y)
            addChild(dot)
            colorDotNodes.append(dot)
        }
    }

    private func updateDynamicLabels() {
        // Rotations count next to "Select level:"
        rotationsValueLabel?.removeFromParent()
        let rotations = GameModel.rotations(forLevel: selectedLevel)
        let label = SKLabelNode(text: "Level \(selectedLevel)  (\(rotations) rotations)")
        label.fontName  = "Helvetica"
        label.fontSize  = 24
        label.fontColor = SKColor(white: 0.75, alpha: 1)
        label.horizontalAlignmentMode = .left
        label.position = CGPoint(x: menuX + 240, y: menuStartY - 2 * lineSpacing + 4)
        addChild(label)
        rotationsValueLabel = label
    }

    private func addPaletteGrid() {
        // Layout constants (local to this method — not shared with addColorDots)
        let swatchSize:  CGFloat = 18
        let swatchGap:   CGFloat = 4
        let stripWidth:  CGFloat = 5 * swatchSize + 4 * swatchGap   // = 106
        let columnGap:   CGFloat = 12
        let rowGap:      CGFloat = 8

        // Vertically centred in the gap between "Select level:" (y=576) and "Instructions" (y=420)
        let gridAnchorY: CGFloat = menuStartY - 3 * lineSpacing - 26  // = 498
        let gridX:       CGFloat = menuX + 240                        // = 350

        let positions: [(TilePalette, CGPoint)] = [
            (.spring, CGPoint(x: gridX,                          y: gridAnchorY + rowGap / 2 + swatchSize / 2)),
            (.ocean,  CGPoint(x: gridX + stripWidth + columnGap, y: gridAnchorY + rowGap / 2 + swatchSize / 2)),
            (.sunset, CGPoint(x: gridX,                          y: gridAnchorY - rowGap / 2 - swatchSize / 2)),
            (.forest, CGPoint(x: gridX + stripWidth + columnGap, y: gridAnchorY - rowGap / 2 - swatchSize / 2)),
        ]

        for (palette, centre) in positions {
            let container = SKNode()
            container.position = centre  // position in scene coords; frame used for hit-testing

            // Five colour squares side by side
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

            // Selection border (SKShapeNode; SKSpriteNode has no strokeColor)
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

    // MARK: - Palette selection

    private func selectPalette(_ palette: TilePalette) {
        selectedPalette = palette
        for (p, border) in paletteBorderNodes {
            border.isHidden = (p != palette)
        }
        addColorDots()   // refresh colour dots to reflect new palette
    }

    private func paletteHit(at point: CGPoint) -> TilePalette? {
        // node.frame is in scene coordinates (containers are direct children of the scene)
        for (palette, node) in paletteContainerNodes {
            if node.frame.contains(point) { return palette }
        }
        return nil
    }

    // MARK: - Hit testing

    /// Returns the menu item whose label row the point falls within.
    private func menuItem(at point: CGPoint) -> MenuItem? {
        let hitHeight: CGFloat = lineSpacing * 0.85
        for (item, label) in menuLabels {
            let labelY = label.position.y
            if point.x >= menuX - 10 && point.x <= size.width - 60 &&
               point.y >= labelY - hitHeight / 2 &&
               point.y <= labelY + hitHeight / 2 {
                return item
            }
        }
        return nil
    }

    // MARK: - Hover

    private func setHighlight(_ item: MenuItem?) {
        guard item != highlightedItem else { return }
        highlightedItem = item
        for (menuItem, label) in menuLabels {
            label.fontColor = (menuItem == item) ? highlightColor : normalColor
        }
    }

    // MARK: - Input

    override func mouseMoved(with event: NSEvent) {
        setHighlight(menuItem(at: event.location(in: self)))
    }

    override func mouseDown(with event: NSEvent) {
        let point = event.location(in: self)

        // Palette grid is outside the MenuItem system — check it first.
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

        case .selectColors:
            selectedColors = (selectedColors % 4) + 1   // cycles 1→2→3→4→1
            addColorDots()

        case .selectTurns:
            selectedLevel = (selectedLevel % 10) + 1    // cycles 1→2→…→10→1
            updateDynamicLabels()

        case .instructions:
            let scene = InstructionsScene(size: size)
            scene.scaleMode = scaleMode
            view?.presentScene(scene, transition: .fade(withDuration: 0.2))

        case .highscores:
            let scene = HighscoreScene(level: selectedLevel, size: size)
            scene.scaleMode = scaleMode
            view?.presentScene(scene, transition: .fade(withDuration: 0.2))

        case .quit:
            NSApp.terminate(nil)
        }
    }
}
```

#### Step 4: Build

- [ ] **Build to verify all three files compile together**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

If you see errors, check:
- `TilePalette` referenced in TileGridNode/TitleScene but `TilePalette.swift` not added to the Xcode target → verify the file was saved to `Quadromania/TilePalette.swift` (Xcode uses `PBXFileSystemSynchronizedRootGroup` for auto-sync, so files in the folder are included automatically)
- Any lingering reference to `TileGridNode.tileColors` → search all `.swift` files and remove

- [ ] **Run the tests**

```bash
xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania -destination 'platform=macOS' 2>&1 | grep -E "(TEST SUCCEEDED|TEST FAILED|passed|failed)" | tail -25
```

Expected: `** TEST SUCCEEDED **` — all 19 tests pass (palette is UI-only; no logic tests need updating).

- [ ] **Step 5: Commit**

```bash
git add Quadromania/TileGridNode.swift Quadromania/GamePlayScene.swift Quadromania/TitleScene.swift
git commit -m "feat: add 2x2 palette chooser to title screen with spring/ocean/sunset/forest themes"
```

---

## Manual verification checklist

After building and running (`Cmd+R` in Xcode or `open Build/Products/Debug/Quadromania.app`):

- [ ] Spring palette is selected (white border) by default on launch
- [ ] Clicking each of the 4 strips selects it (border moves to clicked strip)
- [ ] "Select colors:" dots update immediately when a palette is clicked
- [ ] Starting the game shows tile colors matching the selected palette
- [ ] Rotating tiles animates to the new palette color (not back to spring)
- [ ] Returning to title resets selection to Spring

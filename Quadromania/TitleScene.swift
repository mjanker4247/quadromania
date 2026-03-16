// TitleScene.swift
// Main menu scene — port of gui.c GUI_DrawMainmenu() + handle_title_state().

import SpriteKit
import QuadroCore
import Cocoa

class TitleScene: SKScene {

    // MARK: - Menu item enum
    private enum MenuItem: CaseIterable {
        case startGame, selectColors, selectTurns, instructions, quit
    }

    // MARK: - Game configuration

    /// Number of extra tile colours in play (1 = 2 colours total including colour 0).
    private var selectedColors  = 1              // 1–4
    /// Index into `difficultyLevels`; controls the number of scramble rotations.
    private var selectedLevel   = 1              // restricted to difficultyLevels
    /// The three supported difficulty tiers, each mapping to a distinct scramble-rotation count.
    private let difficultyLevels = [1, 5, 10]
    private let difficultyNames: [Int: String] = [1: "Beginner", 5: "Intermediate", 10: "Expert"]
    /// The colour palette currently shown in the title-screen swatch grid and passed to GamePlayScene.
    private var selectedPalette: TilePalette = .spring

    // MARK: - UI nodes
    private var menuLabels:    [MenuItem: SKLabelNode] = [:]
    /// Small circles next to "Select colors:" showing the active colour values for the current palette.
    private var colorDotNodes: [SKShapeNode] = []
    /// Inline label showing the difficulty name next to "Select difficulty:"; replaced on each change.
    private var rotationsValueLabel: SKLabelNode!
    private var highlightedItem: MenuItem? = nil
    /// Root node for each 5-swatch palette strip; used for hit-testing clicks.
    private var paletteContainerNodes: [TilePalette: SKNode]      = [:]
    /// Selection border drawn around the currently active palette strip.
    private var paletteBorderNodes:    [TilePalette: SKShapeNode] = [:]

    // MARK: - Layout constants
    private let menuX:       CGFloat = 110
    private let menuStartY:  CGFloat = 680
    private let lineSpacing: CGFloat = 52
    /// Total pixel width of one 5-swatch palette strip: 5 × 18 px tiles + 4 × 4 px gaps.
    private let stripWidth:  CGFloat = 106   // 5 * 18 + 4 * 4
    private let swatchSize:  CGFloat = 18

    // MARK: - Colors
    private let normalColor:    SKColor = .white
    private let highlightColor: SKColor = SKColor(red: 1.0, green: 0.9, blue: 0.0, alpha: 1)

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.10, green: 0.08, blue: 0.15, alpha: 1)
        buildUI()
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handlePaletteDidChange(_:)),
            name: .paletteDidChange,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleShowInstructions(_:)),
            name: .showInstructions,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleCustomPaletteDidChange(_:)),
            name: .customPaletteDidChange,
            object: nil
        )
        // Sync with whatever palette the menu bar currently has selected
        if let appDelegate = NSApp.delegate as? AppDelegate {
            selectPalette(appDelegate.activePalette)
        }
    }

    // MARK: - Build UI

    /// Constructs all static and dynamic UI elements for the title screen.
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

    /// Creates the five clickable menu labels and positions them on screen.
    /// Items 0–2 keep uniform spacing; items 3–4 are shifted down by 2 extra line spacings
    /// to leave room for the 2×2 palette-swatch grid inserted between them.
    private func addMenuLabels() {
        // Items 0–2 keep uniform spacing.
        // Items 3–5 are shifted down by 2 extra lineSpacings to make room for the palette grid.
        let itemY: [MenuItem: CGFloat] = [
            .startGame:    menuStartY,
            .selectColors: menuStartY - lineSpacing,
            .selectTurns:  menuStartY - 2 * lineSpacing,
            .instructions: menuStartY - 5 * lineSpacing,
            .quit:         menuStartY - 6 * lineSpacing,
        ]

        let items: [(MenuItem, String)] = [
            (.startGame,    "Start the game"),
            (.selectColors, "Select colors:"),
            (.selectTurns,  "Select difficulty:"),
            (.instructions, "Instructions"),
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

    /// Redraws the colour-dot row next to "Select colors:".
    /// Called on first build and whenever `selectedColors` or `selectedPalette` changes.
    private func addColorDots() {
        // Remove any dots from a previous call before recreating them
        colorDotNodes.forEach { $0.removeFromParent() }
        colorDotNodes = []

        let dotSize: CGFloat = 24
        let dotSpacing: CGFloat = 30
        let startX = menuX + 240
        let y = menuStartY - lineSpacing + dotSize / 2 - 8   // align with selectColors row

        // Draw one dot per active colour level (0 through selectedColors inclusive)
        for i in 0...selectedColors {
            let dot = SKShapeNode(circleOfRadius: dotSize / 2)
            dot.fillColor   = selectedPalette.colors[i]
            dot.strokeColor = .clear
            dot.position = CGPoint(x: startX + CGFloat(i) * dotSpacing, y: y)
            addChild(dot)
            colorDotNodes.append(dot)
        }
    }

    private func updateDynamicLabels() {
        rotationsValueLabel?.removeFromParent()
        let name = difficultyNames[selectedLevel] ?? "Beginner"
        let label = SKLabelNode(text: name)
        label.fontName  = "Helvetica"
        label.fontSize  = 24
        label.fontColor = SKColor(white: 0.75, alpha: 1)
        label.horizontalAlignmentMode = .left
        label.position = CGPoint(x: menuX + 240, y: menuStartY - 2 * lineSpacing + 4)
        addChild(label)
        rotationsValueLabel = label
    }

    /// Builds the 2×2 grid of palette strips used to visually select a colour theme.
    /// Each strip shows all 5 colours of its palette as small squares with a border
    /// that highlights when that palette is currently active.
    private func addPaletteGrid() {
        let swatchGap:   CGFloat = 4    // gap between squares within one strip
        let columnGap:   CGFloat = 12   // horizontal gap between the two strip columns
        let rowGap:      CGFloat = 8    // vertical gap between the two strip rows

        // Vertically centred in the gap between "Select level:" (y=576) and "Instructions" (y=420)
        let gridAnchorY: CGFloat = menuStartY - 3 * lineSpacing - 26  // = 498
        let gridX:       CGFloat = menuX + 240                        // = 350

        // 2×2 layout: spring/ocean on top, sunset/forest on bottom
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
                // Offset each square relative to the container's centre so the strip is centred
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
            // Only show the border on the palette that is currently selected
            border.isHidden    = (palette != selectedPalette)
            container.addChild(border)

            addChild(container)
            paletteContainerNodes[palette] = container
            paletteBorderNodes[palette]    = border
        }
    }

    // MARK: - Palette selection

    /// Updates the active palette, repositions the selection border, and refreshes the colour dots.
    private func selectPalette(_ palette: TilePalette) {
        selectedPalette = palette
        for (p, border) in paletteBorderNodes {
            border.isHidden = (p != palette)
        }
        addColorDots()   // refresh colour dots to reflect new palette
    }

    /// Returns the palette whose swatch strip contains `point`, or `nil` if none.
    private func paletteHit(at point: CGPoint) -> TilePalette? {
        // Expand the hit area slightly beyond the visual strip bounds for easier clicking
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
            // Cycle through 1–4 extra colours; wraps from 4 back to 1
            selectedColors = (selectedColors % 4) + 1
            addColorDots()

        case .selectTurns:
            // Advance to the next difficulty tier, wrapping from Expert back to Beginner
            let idx = ((difficultyLevels.firstIndex(of: selectedLevel) ?? 0) + 1) % 3
            selectedLevel = difficultyLevels[idx]
            updateDynamicLabels()

        case .instructions:
            let scene = InstructionsScene(size: size)
            scene.scaleMode = scaleMode
            view?.presentScene(scene, transition: .fade(withDuration: 0.2))

        case .quit:
            NSApp.terminate(nil)
        }
    }

    // MARK: - Notification handlers

    /// Syncs the on-screen palette selection when the user changes the palette via the menu bar.
    @objc private func handlePaletteDidChange(_ notification: Notification) {
        guard let raw = notification.userInfo?["palette"] as? Int,
              let palette = TilePalette(rawValue: raw) else { return }
        selectPalette(palette)
    }

    /// Refreshes colour dots when the custom palette colours change while `.custom` is active.
    @objc private func handleCustomPaletteDidChange(_ notification: Notification) {
        guard selectedPalette == .custom else { return }
        addColorDots()
    }

    @objc private func handleShowInstructions(_ notification: Notification) {
        let scene = InstructionsScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }

    override func willMove(from view: SKView) {
        NotificationCenter.default.removeObserver(self)
    }
}

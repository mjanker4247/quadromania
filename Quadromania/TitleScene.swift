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
    private var selectedColors = 1   // 1–4
    private var selectedLevel  = 1   // 1–10

    // MARK: - UI nodes
    private var menuLabels: [MenuItem: SKLabelNode] = [:]
    private var colorDotNodes: [SKSpriteNode] = []
    private var rotationsValueLabel: SKLabelNode!
    private var highlightedItem: MenuItem? = nil

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
        let items: [(MenuItem, String)] = [
            (.startGame,    "Start the game"),
            (.selectColors, "Select colors:"),
            (.selectTurns,  "Select level:"),
            (.instructions, "Instructions"),
            (.highscores,   "Highscores"),
            (.quit,         "Quit"),
        ]

        for (index, (item, text)) in items.enumerated() {
            let label = SKLabelNode(text: text)
            label.fontName  = "Helvetica-Bold"
            label.fontSize  = 32
            label.fontColor = normalColor
            label.horizontalAlignmentMode = .left
            label.position = CGPoint(x: menuX, y: menuStartY - CGFloat(index) * lineSpacing)
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
                color: TileGridNode.tileColors[i],
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
        guard let item = menuItem(at: event.location(in: self)) else { return }
        SoundManager.shared.playEffect(.menu)

        switch item {
        case .startGame:
            let model = GameModel(level: selectedLevel, maxColors: selectedColors)
            let scene = GamePlayScene(model: model, size: size)
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

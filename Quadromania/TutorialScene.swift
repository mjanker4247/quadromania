// TutorialScene.swift
// Scripted tutorial — 5×4 grid with 3 guided steps.

import SpriteKit
import Cocoa

class TutorialScene: SKScene {

    // MARK: - Types

    /// One scripted move in the tutorial walkthrough.
    private struct TutorialStep {
        let message: String
        let targetCol: Int   // valid range 1–3 for 5-wide grid
        let targetRow: Int   // valid range 1–2 for 4-tall grid
    }

    // MARK: - Constants

    private let cols = 5
    private let rows = 4
    private let tileSize: CGFloat = 60
    /// Tutorial uses only 2 colour values (0 and 1) to keep the walkthrough simple.
    private let tutorialMaxColors = 1
    /// Always use the Spring palette for a bright, welcoming tutorial appearance.
    private let palette = TilePalette.spring

    // MARK: - Tutorial data

    // Scrambled board that is solved by executing the steps in order.
    // Verified: applying steps (1,1), (3,1), (2,2) to this board produces all zeros.
    // Layout: tutorialPlayfield[col][row], with row 0 at the top.
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

    /// Live copy of the board state; updated after each tutorial step rotation.
    private var tutorialPlayfield: [[Int]] = []
    /// Sprite references indexed as tileSprites[col][row] for fast colour updates.
    private var tileSprites: [[SKShapeNode]] = []
    /// Index of the step the player must complete next (0-based).
    private var currentStepIndex = 0
    /// Animated ring shown over the target tile; replaced on each step advance.
    private var pulseRing: SKShapeNode?
    private var messageLabel: SKLabelNode!
    private var messagePanel: SKShapeNode!
    /// Nil until all steps are complete; click navigates back to TitleScene.
    private var backButton: SKLabelNode?
    private var skipButton: SKLabelNode!

    // MARK: - Layout helpers

    /// X origin of the leftmost tile column, computed to centre the grid horizontally.
    private var gridOriginX: CGFloat {
        let gridW = CGFloat(cols) * tileSize
        return (size.width - gridW) / 2
    }
    /// Y coordinate of the bottom edge of the lowest tile row in scene space.
    private let gridBottomY: CGFloat = 580

    /// Returns the scene-space centre point of the tile at the given grid position.
    private func tilePosition(col: Int, row: Int) -> CGPoint {
        // row 0 at the top; flip Y so row 0 renders highest
        let x = gridOriginX + CGFloat(col) * tileSize + tileSize / 2
        let y = gridBottomY + CGFloat(rows - 1 - row) * tileSize + tileSize / 2
        return CGPoint(x: x, y: y)
    }

    // MARK: - Lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.10, green: 0.08, blue: 0.15, alpha: 1)
        // Deep-copy the initial playfield so edits don't mutate the constant
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

    /// Creates the 5×4 grid of circular tile sprites and populates `tileSprites`.
    /// Each sprite's fill colour is set from the palette using the initial playfield values.
    private func buildTutorialGrid() {
        let colors = palette.colors
        let radius = (tileSize - 2) / 2   // 1 px inset so adjacent circles don't visually merge
        tileSprites = Array(repeating: [], count: cols)
        for col in 0..<cols {
            tileSprites[col] = []
            for row in 0..<rows {
                let circle = SKShapeNode(circleOfRadius: radius)
                circle.fillColor   = colors[tutorialPlayfield[col][row]]
                circle.strokeColor = .clear
                circle.position = tilePosition(col: col, row: row)

                // Frosted highlight overlay gives tiles a subtle three-dimensional sheen
                let highlight = SKShapeNode(ellipseIn: CGRect(x: -18, y: 2, width: 22, height: 18))
                highlight.fillColor   = SKColor(white: 1, alpha: 0.42)
                highlight.strokeColor = .clear
                circle.addChild(highlight)

                addChild(circle)
                tileSprites[col].append(circle)
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

    /// Replaces the pulse ring with a new one centred on the target tile for the given step.
    private func placePulseRing(col: Int, row: Int) {
        pulseRing?.removeFromParent()
        let ring = SKShapeNode(circleOfRadius: tileSize * 0.52)
        ring.strokeColor = SKColor(red: 1.0, green: 0.9, blue: 0.0, alpha: 0.9)
        ring.lineWidth   = 3
        ring.fillColor   = .clear
        ring.position    = tilePosition(col: col, row: row)
        ring.zPosition   = 5   // above tiles, below any overlay
        // Repeating scale oscillation draws the player's eye to the target tile
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
            // All steps complete — remove the guide ring and show a completion message
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

    /// Increments every tile in the 3×3 neighbourhood centred on (col, row), wrapping at `tutorialMaxColors`.
    private func applyRotation(col: Int, row: Int) {
        for dc in -1...1 {
            for dr in -1...1 {
                let c = col + dc, r = row + dr
                // Clamp to grid bounds — edge tiles have smaller neighbourhoods
                guard c >= 0, c < cols, r >= 0, r < rows else { continue }
                tutorialPlayfield[c][r] += 1
                if tutorialPlayfield[c][r] > tutorialMaxColors {
                    tutorialPlayfield[c][r] = 0
                }
            }
        }
        refreshTileColors()
    }

    /// Repaints every tile sprite to match the current `tutorialPlayfield` state.
    private func refreshTileColors() {
        let colors = palette.colors
        for col in 0..<cols {
            for row in 0..<rows {
                tileSprites[col][row].fillColor = colors[tutorialPlayfield[col][row]]
            }
        }
    }

    // MARK: - Wrong-tile flash

    /// Briefly tints the message panel red to signal that the player clicked the wrong tile.
    private func flashWrongTile() {
        messagePanel.run(SKAction.customAction(withDuration: 0.35) { node, t in
            guard let panel = node as? SKShapeNode else { return }
            // First half of the duration: red tint; second half: fade back to normal
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

        // Convert scene point to grid indices
        let col = Int((point.x - gridOriginX) / tileSize)
        // screenRow 0 = bottom row in scene space; invert to get row 0 at top
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

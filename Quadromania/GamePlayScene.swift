// GamePlayScene.swift
// Active gameplay scene — port of state_manager handle_game_state() + Quadromania_DrawPlayfield().

import SpriteKit
import Cocoa

class GamePlayScene: SKScene {

    // MARK: - State

    private var model: GameModel
    /// The active colour palette; updated in-place when the user changes it via the Palette menu.
    private var palette: TilePalette
    private var tileGrid: TileGridNode!
    /// One circular swatch per colour level (0…maxColors), shown in the strip above the grid.
    private var colorSwatchNodes: [SKShapeNode] = []
    private var turnsLabel: SKLabelNode!
    private var limitLabel: SKLabelNode!
    /// Set to `true` after win/loss overlay is shown; the next click calls `startNewGame()`.
    private var waitingForClick = false

    // MARK: - Init

    init(model: GameModel, palette: TilePalette, size: CGSize) {
        self.model   = model
        self.palette = palette
        super.init(size: size)
    }

    required init?(coder: NSCoder) { fatalError("Use init(model:palette:size:)") }

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        // Vary the background colour slightly per game so consecutive games look distinct.
        backgroundColor = SKColor(
            red: CGFloat(model.backgroundArtIndex) * 0.05 + 0.05,
            green: 0.08,
            blue: CGFloat(model.backgroundArtIndex) * 0.03 + 0.06,
            alpha: 1
        )
        buildUI()
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
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleShowInstructions(_:)),
            name: .showInstructions,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleTransitionStyleDidChange(_:)),
            name: .transitionStyleDidChange,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleCustomPaletteDidChange(_:)),
            name: .customPaletteDidChange,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleNewGameRequested(_:)),
            name: .newGameRequested,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleColorsDidChange(_:)),
            name: .colorsDidChange,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleDifficultyDidChange(_:)),
            name: .difficultyDidChange,
            object: nil
        )
    }

    // MARK: - UI

    /// Constructs all visual elements: the tile grid, the colour-cycle strip, and the HUD labels.
    private func buildUI() {
        // --- Grid ---
        tileGrid = TileGridNode(playfield: model.playfield, palette: palette)

        // Centre the grid horizontally; leave 50 px at the bottom for HUD.
        let gridX = (size.width  - TileGridNode.gridPixelWidth)  / 2
        let gridY: CGFloat = 50
        tileGrid.position = CGPoint(x: gridX, y: gridY)
        addChild(tileGrid)
        buildColorStrip(gridX: gridX)
        // Sync symbol overlay and transition style from AppDelegate's persisted state
        if let appDelegate = NSApp.delegate as? AppDelegate {
            tileGrid.symbolOverlayEnabled = appDelegate.symbolOverlayEnabled
            tileGrid.transitionStyle = appDelegate.transitionStyle
        }

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

    /// Builds the colour-cycle indicator strip above the tile grid.
    /// Shows one circle per colour level with arrows between them, ending with a wrap-around ↩ arrow
    /// to illustrate that the last colour cycles back to colour 0.
    private func buildColorStrip(gridX: CGFloat) {
        let swatchSize: CGFloat = 28
        let elementSpacing: CGFloat = 46   // 28 swatch + 18 gap
        // +1 because colour values range 0…maxColors (inclusive)
        let count = model.maxColors + 1
        let centerX = gridX + TileGridNode.gridPixelWidth / 2
        // Position the strip 10 px above the top edge of the tile grid
        let centerY = 50 + TileGridNode.gridPixelHeight + 10 + swatchSize / 2
        // Centre the swatch midpoints over the grid centre; the trailing ↩ arrow extends slightly right
        let startX  = centerX - CGFloat(count - 1) * elementSpacing / 2

        colorSwatchNodes = []
        for i in 0..<count {
            let x = startX + CGFloat(i) * elementSpacing

            let swatch = SKShapeNode(circleOfRadius: swatchSize / 2)
            swatch.fillColor = palette.colors[i]
            swatch.strokeColor = .clear
            swatch.position = CGPoint(x: x, y: centerY)
            addChild(swatch)
            colorSwatchNodes.append(swatch)

            // → between colours; ↩ after the last colour to show the wrap-around
            let arrowText = (i < count - 1) ? "→" : "↩"
            let arrow = SKLabelNode(text: arrowText)
            arrow.fontName  = "Helvetica"
            arrow.fontSize  = 16
            arrow.fontColor = SKColor(white: 0.55, alpha: 1)
            arrow.verticalAlignmentMode   = .center
            arrow.position = CGPoint(x: x + swatchSize / 2 + 9, y: centerY)
            addChild(arrow)
        }
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
        // Any click after the overlay appears restarts the game in-place
        if waitingForClick {
            startNewGame()
            return
        }

        let scenePoint = event.location(in: self)
        // Convert from scene coords to TileGridNode's local coordinate space for hit-testing
        let localPoint = tileGrid.convert(scenePoint, from: self)
        guard let (col, row) = tileGrid.gridCoordinates(for: localPoint) else { return }

        // Valid interior click: same bounds as original (col 1–16, row 1–11)
        // The outer ring of tiles cannot be a rotation center because the 3×3 block would extend
        // outside the 18×13 grid boundary.
        guard col >= 1, col <= 16, row >= 1, row <= 11 else { return }

        model.rotate(x: col, y: row)
        // Pass the rotation center so TileGridNode can stagger the tile animations outward from it
        tileGrid.updateAll(from: model.playfield, rotationCenter: (col, row))
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

        let hint = SKLabelNode(text: "Click to play again")
        hint.fontName  = "Helvetica"
        hint.fontSize  = 16
        hint.fontColor = SKColor(white: 0.6, alpha: 1)
        hint.position  = CGPoint(x: size.width / 2, y: boxY - 30)
        hint.zPosition = 11
        addChild(hint)
    }

    // MARK: - Navigation

    /// Resets the game in-place with fresh settings from AppDelegate.
    /// Called when the player clicks after win/loss, or confirms an NSAlert.
    private func startNewGame() {
        // Rebuild model from current AppDelegate settings.
        // AppDelegate.shared is a strong IUO set before any scene is presented; safe to force-access here.
        model = GameModel(
            level:     AppDelegate.shared.selectedLevel,
            maxColors: AppDelegate.shared.selectedColors - 1
        )
        // Sync palette in case it changed while the old game was running
        palette = AppDelegate.shared.activePalette
        // Vary background colour per new game for visual distinction
        backgroundColor = SKColor(
            red:   CGFloat(model.backgroundArtIndex) * 0.05 + 0.05,
            green: 0.08,
            blue:  CGFloat(model.backgroundArtIndex) * 0.03 + 0.06,
            alpha: 1
        )
        waitingForClick  = false
        colorSwatchNodes = []
        removeAllChildren()
        buildUI()
        // buildUI() already applies symbolOverlayEnabled and transitionStyle from NSApp.delegate
    }

    /// Presents a non-blocking sheet asking whether to start a new game or continue.
    /// Calls `startNewGame()` only if the user confirms.
    private func showNewGameAlert(informative: String) {
        let alert = NSAlert()
        alert.messageText     = "Start a new game?"
        alert.informativeText = informative
        alert.addButton(withTitle: "New Game")
        alert.addButton(withTitle: "Continue")
        guard let window = view?.window else { return }
        alert.beginSheetModal(for: window) { [weak self] response in
            if response == .alertFirstButtonReturn {
                self?.startNewGame()
            }
        }
    }

    // MARK: - Notification handlers

    /// Applies the new palette to the tile grid and refreshes the colour-strip swatches.
    @objc private func handlePaletteDidChange(_ notification: Notification) {
        guard let raw = notification.userInfo?["palette"] as? Int,
              let newPalette = TilePalette(rawValue: raw) else { return }
        palette = newPalette
        tileGrid.applyPalette(newPalette)
        // Update each swatch circle to the corresponding colour in the new palette
        for (i, swatch) in colorSwatchNodes.enumerated() {
            swatch.fillColor = newPalette.colors[i]
        }
    }

    /// Forwards the symbol-overlay toggle to the tile grid.
    @objc private func handleSymbolOverlayDidChange(_ notification: Notification) {
        guard let enabled = notification.userInfo?["enabled"] as? Bool else { return }
        tileGrid.symbolOverlayEnabled = enabled
    }

    /// Forwards the new transition animation style to the tile grid.
    @objc private func handleTransitionStyleDidChange(_ notification: Notification) {
        guard let raw = notification.userInfo?["style"] as? Int,
              let style = TransitionStyle(rawValue: raw) else { return }
        tileGrid.transitionStyle = style
    }

    /// Re-applies the custom palette when its colours are edited via the Custom Palette panel.
    /// Only acts when `.custom` is the currently active palette to avoid unnecessary redraws.
    @objc private func handleCustomPaletteDidChange(_ notification: Notification) {
        guard palette == .custom else { return }
        tileGrid.applyPalette(.custom)
        // Refresh swatch colours to match the newly saved custom values
        for (i, swatch) in colorSwatchNodes.enumerated() {
            swatch.fillColor = TilePalette.custom.colors[i]
        }
    }

    @objc private func handleShowInstructions(_ notification: Notification) {
        // Ignore the menu action while a win/loss overlay is waiting for dismissal
        guard !waitingForClick else { return }
        let scene = InstructionsScene(size: size, sourceGame: model, sourcePalette: palette)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }

    /// Starts a new game immediately if the old one is over; shows an alert if a game is in progress.
    @objc private func handleNewGameRequested(_ notification: Notification) {
        if waitingForClick {
            startNewGame()
        } else {
            showNewGameAlert(informative: "Your current game will be lost.")
        }
    }

    /// Responds to a colour-count change: silent restart if game is over, alert if game is active.
    @objc private func handleColorsDidChange(_ notification: Notification) {
        if waitingForClick {
            startNewGame()
        } else {
            showNewGameAlert(informative: "You changed the number of colors.")
        }
    }

    /// Responds to a difficulty change: silent restart if game is over, alert if game is active.
    @objc private func handleDifficultyDidChange(_ notification: Notification) {
        if waitingForClick {
            startNewGame()
        } else {
            showNewGameAlert(informative: "You changed the difficulty.")
        }
    }

    override func willMove(from view: SKView) {
        NotificationCenter.default.removeObserver(self)
    }
}

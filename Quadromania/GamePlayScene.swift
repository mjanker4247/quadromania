// GamePlayScene.swift
// Active gameplay scene — port of state_manager handle_game_state() + Quadromania_DrawPlayfield().

import SpriteKit
import QuadroCore
import Cocoa

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
        if let appDelegate = NSApp.delegate as? AppDelegate {
            tileGrid.symbolOverlayEnabled = appDelegate.symbolOverlayEnabled
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

    // MARK: - Navigation

    private func returnToTitle() {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.4))
    }

    @objc private func handlePaletteDidChange(_ notification: Notification) {
        guard let raw = notification.userInfo?["palette"] as? Int,
              let palette = TilePalette(rawValue: raw) else { return }
        tileGrid.applyPalette(palette)
    }

    @objc private func handleSymbolOverlayDidChange(_ notification: Notification) {
        guard let enabled = notification.userInfo?["enabled"] as? Bool else { return }
        tileGrid.symbolOverlayEnabled = enabled
    }

    @objc private func handleShowInstructions(_ notification: Notification) {
        let newPalette = (NSApp.delegate as? AppDelegate)?.activePalette ?? .spring
        let scene = InstructionsScene(size: size, sourceGame: model, sourcePalette: newPalette)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }

    override func willMove(from view: SKView) {
        NotificationCenter.default.removeObserver(self)
    }
}

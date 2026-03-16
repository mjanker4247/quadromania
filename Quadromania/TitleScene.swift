// TitleScene.swift
// Animated title screen — displays "QUADROMANIA" above a decorative colour-wave grid.
// All game controls live in the Game menu (AppDelegate). This scene is purely visual.

import SpriteKit
import QuadroCore
import Cocoa

class TitleScene: SKScene {

    // MARK: - Decorative grid dimensions

    /// Number of tile columns in the decorative background grid.
    private let gridCols = 16
    /// Number of tile rows in the decorative background grid.
    private let gridRows = 8
    /// Center-to-center distance between adjacent tiles (px).
    private let cellPitch: CGFloat = 50
    /// Radius of each frosted gem circle (px).
    private let tileRadius: CGFloat = 22

    // MARK: - Grid state

    /// All SKShapeNode tiles indexed as [row][col].
    private var gridTiles: [[SKShapeNode]] = []
    /// Current displayed colour index per tile; -1 means unset (force redraw on first tick).
    private var colorIndices: [[Int]] = []

    // MARK: - Wave animation state

    /// Monotonically increasing phase counter (units/sec × time = phase units).
    /// `Int(wavePhase) - (col + row)` gives each tile's target palette index.
    private var wavePhase: Double = 0
    /// Timestamp of the previous `update` call; 0 before the first frame.
    private var lastUpdateTime: TimeInterval = 0

    // MARK: - Palette

    /// Active colour palette — kept in sync with AppDelegate and the Palette menu.
    private var currentPalette: TilePalette = .spring

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.10, green: 0.08, blue: 0.15, alpha: 1)
        // Sync palette before building the grid so tiles start with the correct colours
        currentPalette = AppDelegate.shared.activePalette
        buildTitleText()
        buildDecorativeGrid()
        registerObservers()
    }

    // MARK: - Build UI

    /// Adds the "QUADROMANIA" heading and "Puzzle Game" subtitle near the top of the scene.
    private func buildTitleText() {
        let title = SKLabelNode(text: "QUADROMANIA")
        title.fontName  = "Helvetica-Bold"
        title.fontSize  = 72
        title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
        title.horizontalAlignmentMode = .center
        title.position  = CGPoint(x: size.width / 2, y: 700)
        addChild(title)

        let sub = SKLabelNode(text: "Puzzle Game")
        sub.fontName  = "Helvetica"
        sub.fontSize  = 22
        sub.fontColor = SKColor(white: 0.55, alpha: 1)
        sub.horizontalAlignmentMode = .center
        sub.position  = CGPoint(x: size.width / 2, y: 650)
        addChild(sub)
    }

    /// Creates a 16×8 grid of frosted gem circles centred at (640, 300).
    /// Tiles start at colour 0; the wave animation fills them in on the first update tick.
    private func buildDecorativeGrid() {
        // Total span from first to last tile center
        let gridWidth  = CGFloat(gridCols - 1) * cellPitch  // 750 px
        let gridHeight = CGFloat(gridRows - 1) * cellPitch  // 350 px
        // Bottom-left origin so the grid is centred at (640, 300)
        let originX = size.width  / 2 - gridWidth  / 2  // = 265
        let originY = 300         - gridHeight / 2       // = 125

        // Initialise tracking arrays (-1 = unset, forces colour assignment on first tick)
        colorIndices = Array(repeating: Array(repeating: -1, count: gridCols), count: gridRows)
        gridTiles    = []

        for row in 0..<gridRows {
            var rowTiles: [SKShapeNode] = []
            for col in 0..<gridCols {
                let tile = SKShapeNode(circleOfRadius: tileRadius)
                tile.strokeColor = .clear
                tile.fillColor   = currentPalette.colors[0]
                tile.position    = CGPoint(
                    x: originX + CGFloat(col) * cellPitch,
                    y: originY + CGFloat(row) * cellPitch
                )

                // Frosted highlight: small white ellipse in the upper-left of the circle,
                // matching the style used in TileGridNode.
                let hl = SKShapeNode(ellipseOf: CGSize(
                    width:  tileRadius * 0.84,
                    height: tileRadius * 0.70
                ))
                hl.fillColor   = SKColor(white: 1, alpha: 0.42)
                hl.strokeColor = .clear
                hl.position    = CGPoint(x: -tileRadius * 0.2, y: tileRadius * 0.25)
                tile.addChild(hl)

                addChild(tile)
                rowTiles.append(tile)
            }
            gridTiles.append(rowTiles)
        }
    }

    // MARK: - Wave animation

    /// Advances the wave and recolours any tile whose target index has changed.
    override func update(_ currentTime: TimeInterval) {
        // Compute delta; clamp to 0.1 s to avoid a large jump on the very first frame.
        let delta = lastUpdateTime == 0 ? 0.0 : min(currentTime - lastUpdateTime, 0.1)
        lastUpdateTime = currentTime
        wavePhase += delta * 0.8  // 0.8 phase units per second

        let n = currentPalette.colors.count  // always 5

        for row in 0..<gridRows {
            for col in 0..<gridCols {
                // Diagonal wave: (col + row) shifts each tile's phase along the diagonal.
                // Double-modulo ensures the result is always non-negative even when
                // Int(wavePhase) < (col + row).
                let raw         = Int(wavePhase) - (col + row)
                let targetIndex = ((raw % n) + n) % n

                guard targetIndex != colorIndices[row][col] else { continue }
                colorIndices[row][col] = targetIndex

                let tile = gridTiles[row][col]
                let from = tile.fillColor
                let to   = currentPalette.colors[targetIndex]
                // Cancel any in-flight transition before starting a new one
                tile.removeAction(forKey: "colorTransition")
                tile.run(colorTransition(from: from, to: to), withKey: "colorTransition")
            }
        }
    }

    // MARK: - Colour transition helpers

    /// Returns an SKAction that smoothly blends a tile's fillColor from `from` to `to`.
    private func colorTransition(from: SKColor, to: SKColor) -> SKAction {
        let duration: TimeInterval = 0.35
        return SKAction.customAction(withDuration: duration) { node, elapsed in
            guard let tile = node as? SKShapeNode else { return }
            let t = CGFloat(elapsed) / CGFloat(duration)
            tile.fillColor = TitleScene.lerpColor(from: from, to: to, t: t)
        }
    }

    /// Linearly interpolates between two SKColors component-wise (alpha always 1).
    /// Matches the TileGridNode.lerpColor pattern.
    private static func lerpColor(from: SKColor, to: SKColor, t: CGFloat) -> SKColor {
        var r1: CGFloat = 0, g1: CGFloat = 0, b1: CGFloat = 0, a1: CGFloat = 0
        var r2: CGFloat = 0, g2: CGFloat = 0, b2: CGFloat = 0, a2: CGFloat = 0
        from.getRed(&r1, green: &g1, blue: &b1, alpha: &a1)
        to.getRed  (&r2, green: &g2, blue: &b2, alpha: &a2)
        return SKColor(
            red:   r1 + (r2 - r1) * t,
            green: g1 + (g2 - g1) * t,
            blue:  b1 + (b2 - b1) * t,
            alpha: 1
        )
    }

    // MARK: - Notification observers

    private func registerObservers() {
        let nc = NotificationCenter.default
        // New game requested from menu bar — transition to GamePlayScene
        nc.addObserver(self, selector: #selector(handleNewGameRequested(_:)),
                       name: .newGameRequested, object: nil)
        // Palette changed — refresh grid on next update tick
        nc.addObserver(self, selector: #selector(handlePaletteDidChange(_:)),
                       name: .paletteDidChange, object: nil)
        // Custom palette colours edited — refresh grid if .custom is active
        nc.addObserver(self, selector: #selector(handleCustomPaletteDidChange(_:)),
                       name: .customPaletteDidChange, object: nil)
        // Instructions menu item — navigate to InstructionsScene
        nc.addObserver(self, selector: #selector(handleShowInstructions(_:)),
                       name: .showInstructions, object: nil)
    }

    /// Constructs a GameModel from AppDelegate settings and transitions to GamePlayScene.
    @objc private func handleNewGameRequested(_ notification: Notification) {
        let model = GameModel(
            level:     AppDelegate.shared.selectedLevel,
            maxColors: AppDelegate.shared.selectedColors - 1
        )
        let scene = GamePlayScene(model: model, palette: currentPalette, size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    }

    /// Applies a new palette and resets `colorIndices` so all tiles re-animate on the next tick.
    @objc private func handlePaletteDidChange(_ notification: Notification) {
        guard let raw = notification.userInfo?["palette"] as? Int,
              let palette = TilePalette(rawValue: raw) else { return }
        currentPalette = palette
        // -1 forces every tile to recompute its colour on the next update tick
        colorIndices = Array(repeating: Array(repeating: -1, count: gridCols), count: gridRows)
    }

    /// Resets `colorIndices` when the custom palette colours are edited while `.custom` is active.
    @objc private func handleCustomPaletteDidChange(_ notification: Notification) {
        guard currentPalette == .custom else { return }
        colorIndices = Array(repeating: Array(repeating: -1, count: gridCols), count: gridRows)
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

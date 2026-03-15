// TileGridNode.swift
// 18×13 grid of colored tile sprites — port of graphics/renderer.c dot drawing.
// Tiles are drawn programmatically; no image assets required.

import SpriteKit
import QuadroCore

private let tileSymbols = ["●", "■", "▲", "◆", "★"]

private func lerpColor(from: SKColor, to: SKColor, t: CGFloat) -> SKColor {
    var r1: CGFloat = 0, g1: CGFloat = 0, b1: CGFloat = 0, a1: CGFloat = 0
    var r2: CGFloat = 0, g2: CGFloat = 0, b2: CGFloat = 0, a2: CGFloat = 0
    from.getRed(&r1, green: &g1, blue: &b1, alpha: &a1)
    to.getRed(&r2, green: &g2, blue: &b2, alpha: &a2)
    return SKColor(red: r1 + (r2-r1)*t, green: g1 + (g2-g1)*t,
                   blue: b1 + (b2-b1)*t, alpha: 1)
}

class TileGridNode: SKNode {

    // MARK: - Layout

    static let tileSize: CGFloat = 64

    /// Total pixel width of the grid.
    static var gridPixelWidth:  CGFloat { CGFloat(GameModel.gridWidth)  * tileSize }
    /// Total pixel height of the grid.
    static var gridPixelHeight: CGFloat { CGFloat(GameModel.gridHeight) * tileSize }

    // MARK: - State

    private var palette: TilePalette
    private var tiles: [[SKShapeNode]] = []
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

                // Symbol overlay label
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

    // MARK: - Animations

    private func colorTransition(from oldColor: SKColor, to newColor: SKColor) -> SKAction {
        SKAction.customAction(withDuration: 0.18) { node, elapsed in
            guard let shape = node as? SKShapeNode else { return }
            let t = min(elapsed / CGFloat(0.18), 1.0)
            shape.fillColor = lerpColor(from: oldColor, to: newColor, t: t)
        }
    }

    // MARK: - Update

    /// Refresh tile colors from the current playfield state, animating any changes.
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

    /// Recolor all tiles for a new palette without changing playfield state.
    /// Uses colorIndices directly (not updateAll) to recolor every tile unconditionally.
    func applyPalette(_ newPalette: TilePalette) {
        palette = newPalette
        let colors = newPalette.colors
        for col in 0..<tiles.count {
            for row in 0..<tiles[col].count {
                tiles[col][row].fillColor = colors[colorIndices[col][row]]
            }
        }
    }

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

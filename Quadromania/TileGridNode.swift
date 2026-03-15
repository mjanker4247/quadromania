// TileGridNode.swift
// 18×13 grid of colored tile sprites — port of graphics/renderer.c dot drawing.
// Tiles are drawn programmatically; no image assets required.

import SpriteKit
import QuadroCore

private let tileSymbols = ["●", "■", "▲", "◆", "★"]

class TileGridNode: SKNode {

    // MARK: - Layout

    static let tileSize: CGFloat = 64

    /// Total pixel width of the grid.
    static var gridPixelWidth:  CGFloat { CGFloat(GameModel.gridWidth)  * tileSize }
    /// Total pixel height of the grid.
    static var gridPixelHeight: CGFloat { CGFloat(GameModel.gridHeight) * tileSize }

    // MARK: - State

    private var palette: TilePalette
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

        let paletteColors = palette.colors
        for col in 0..<GameModel.gridWidth {
            tiles[col] = []
            for row in 0..<GameModel.gridHeight {
                let colorIndex = playfield[col][row]
                let sprite = SKSpriteNode(
                    color: paletteColors[colorIndex],
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
                let idx = colorIndices[col][row]
                tiles[col][row].run(SKAction.colorize(with: colors[idx],
                                                       colorBlendFactor: 1.0,
                                                       duration: 0.18))
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

// TileGridNode.swift
// 18×13 grid of colored tile sprites — port of graphics/renderer.c dot drawing.
// Tiles are drawn programmatically; no image assets required.

import SpriteKit

class TileGridNode: SKNode {

    // MARK: - Layout

    static let tileSize: CGFloat = 64

    /// Total pixel width of the grid.
    static var gridPixelWidth:  CGFloat { CGFloat(GameModel.gridWidth)  * tileSize }
    /// Total pixel height of the grid.
    static var gridPixelHeight: CGFloat { CGFloat(GameModel.gridHeight) * tileSize }

    // MARK: - Colors (index 0 = goal red, 1–4 = scramble colors)

    static let tileColors: [SKColor] = [
        SKColor(red: 0.85, green: 0.10, blue: 0.10, alpha: 1),  // 0 — red   (goal)
        SKColor(red: 0.10, green: 0.75, blue: 0.15, alpha: 1),  // 1 — green
        SKColor(red: 0.15, green: 0.40, blue: 0.90, alpha: 1),  // 2 — blue
        SKColor(red: 0.92, green: 0.78, blue: 0.00, alpha: 1),  // 3 — yellow
        SKColor(red: 0.68, green: 0.10, blue: 0.82, alpha: 1),  // 4 — purple
    ]

    // MARK: - State

    private var tiles: [[SKSpriteNode]] = []

    // MARK: - Init

    init(playfield: [[Int]]) {
        super.init()
        buildGrid(playfield: playfield)
    }

    required init?(coder: NSCoder) { fatalError("Use init(playfield:)") }

    // MARK: - Build

    private func buildGrid(playfield: [[Int]]) {
        let s = TileGridNode.tileSize
        let gap: CGFloat = 2          // 2 px gap between tiles
        let tileDrawSize = CGSize(width: s - gap, height: s - gap)

        tiles = Array(repeating: [], count: GameModel.gridWidth)

        for col in 0..<GameModel.gridWidth {
            tiles[col] = []
            for row in 0..<GameModel.gridHeight {
                let colorIndex = playfield[col][row]
                let sprite = SKSpriteNode(
                    color: TileGridNode.tileColors[colorIndex],
                    size: tileDrawSize
                )
                // Row 0 renders at the top (flip Y so row 0 is highest on screen)
                sprite.position = CGPoint(
                    x: CGFloat(col) * s + s / 2,
                    y: CGFloat(GameModel.gridHeight - 1 - row) * s + s / 2
                )
                addChild(sprite)
                tiles[col].append(sprite)
            }
        }
    }

    // MARK: - Update

    /// Refresh all tile colors from the current playfield state.
    func updateAll(from playfield: [[Int]]) {
        for col in 0..<GameModel.gridWidth {
            for row in 0..<GameModel.gridHeight {
                tiles[col][row].color = TileGridNode.tileColors[playfield[col][row]]
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

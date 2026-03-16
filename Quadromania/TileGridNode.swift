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
    var transitionStyle: TransitionStyle = .ringSweep

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
                let symLabel = SKLabelNode(text: colorIndex < tileSymbols.count ? tileSymbols[colorIndex] : "")
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

    private static let colorTransitionDuration: TimeInterval = 0.18

    private func colorTransition(from oldColor: SKColor, to newColor: SKColor) -> SKAction {
        let d = CGFloat(Self.colorTransitionDuration)
        return SKAction.customAction(withDuration: Self.colorTransitionDuration) { node, elapsed in
            guard let shape = node as? SKShapeNode else { return }
            shape.fillColor = lerpColor(from: oldColor, to: newColor, t: min(elapsed / d, 1.0))
        }
    }

    // MARK: - Update

    /// Refresh tile colors from the current playfield state, animating any changes.
    func updateAll(from playfield: [[Int]], rotationCenter: (col: Int, row: Int)? = nil) {
        if let center = rotationCenter {
            animateBlock(from: playfield, center: center)
        } else {
            for col in 0..<GameModel.gridWidth {
                for row in 0..<GameModel.gridHeight {
                    let newIndex = playfield[col][row]
                    guard newIndex != colorIndices[col][row] else { continue }
                    let oldColor = palette.colors[colorIndices[col][row]]
                    let newColor = palette.colors[newIndex]
                    colorIndices[col][row] = newIndex
                    let key = "colorTransition"
                    tiles[col][row].removeAction(forKey: key)
                    tiles[col][row].run(colorTransition(from: oldColor, to: newColor), withKey: key)
                    if let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode {
                        sym.text = newIndex < tileSymbols.count ? tileSymbols[newIndex] : ""
                    }
                }
            }
        }
    }

    private func animateBlock(from playfield: [[Int]], center: (col: Int, row: Int)) {
        let blockCols = (center.col - 1)...(center.col + 1)
        let blockRows = (center.row - 1)...(center.row + 1)

        // Plain transition for tiles OUTSIDE the 3×3 block
        for col in 0..<GameModel.gridWidth {
            for row in 0..<GameModel.gridHeight {
                let newIndex = playfield[col][row]
                guard newIndex != colorIndices[col][row] else { continue }
                if blockCols.contains(col) && blockRows.contains(row) { continue }
                let oldColor = palette.colors[colorIndices[col][row]]
                let newColor = palette.colors[newIndex]
                colorIndices[col][row] = newIndex
                let key = "colorTransition"
                tiles[col][row].removeAction(forKey: key)
                tiles[col][row].run(colorTransition(from: oldColor, to: newColor), withKey: key)
                if let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode {
                    sym.text = newIndex < tileSymbols.count ? tileSymbols[newIndex] : ""
                }
            }
        }

        // Gather 3×3 block data (only tiles whose colour actually changes)
        var blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                          colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)] = []
        for dc in -1...1 {
            for dr in -1...1 {
                let col = center.col + dc
                let row = center.row + dr
                guard col >= 0, col < GameModel.gridWidth,
                      row >= 0, row < GameModel.gridHeight else { continue }
                let newIndex = playfield[col][row]
                let sym = tiles[col][row].childNode(withName: "symbol") as? SKLabelNode
                let oldColor = palette.colors[colorIndices[col][row]]
                let newColor = palette.colors[newIndex]
                colorIndices[col][row] = newIndex
                sym?.text = newIndex < tileSymbols.count ? tileSymbols[newIndex] : ""
                guard oldColor != newColor else { continue }
                blockTiles.append((
                    tile: tiles[col][row],
                    oldColor: oldColor,
                    newColor: newColor,
                    colOffset: dc,
                    rowOffset: dr,
                    sym: sym,
                    newSymText: newIndex < tileSymbols.count ? tileSymbols[newIndex] : ""
                ))
            }
        }

        switch transitionStyle {
        case .ringSweep:   animateRingSweep(blockTiles: blockTiles, center: center)
        case .sequential:  animateSequential(blockTiles: blockTiles)
        case .radialPulse: animateRadialPulse(blockTiles: blockTiles)
        }
    }

    private func animateRingSweep(
        blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)],
        center: (col: Int, row: Int)
    ) {
        let s = TileGridNode.tileSize
        let centerX = CGFloat(center.col) * s + s / 2
        let centerY = CGFloat(GameModel.gridHeight - 1 - center.row) * s + s / 2
        let blockRadius = s * CGFloat(sqrt(2.0) * 1.5)

        let arc = SKShapeNode()
        arc.strokeColor = .white
        arc.lineWidth   = 4
        arc.fillColor   = .clear
        arc.position    = CGPoint(x: centerX, y: centerY)
        arc.zPosition   = 5
        addChild(arc)

        arc.run(SKAction.sequence([
            SKAction.customAction(withDuration: 0.30) { node, elapsed in
                guard let shape = node as? SKShapeNode else { return }
                let progress = min(elapsed / 0.30, 1.0)
                let endAngle = CGFloat(progress) * 2 * .pi
                let path = CGMutablePath()
                path.addArc(center: .zero, radius: blockRadius,
                            startAngle: -.pi / 2, endAngle: -.pi / 2 - endAngle,
                            clockwise: true)
                shape.path = path
            },
            SKAction.fadeOut(withDuration: 0.10),
            SKAction.removeFromParent()
        ]))

        for entry in blockTiles {
            let action = SKAction.sequence([
                SKAction.wait(forDuration: 0.20),
                colorTransition(from: entry.oldColor, to: entry.newColor)
            ])
            entry.tile.run(action)
        }
    }

    private func animateSequential(
        blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)]
    ) {
        let clockwiseOffsets: [(Int, Int)] = [
            (-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0)
        ]
        var offsetMap: [String: (tile: SKShapeNode, oldColor: SKColor, newColor: SKColor)] = [:]
        var centerEntry: (tile: SKShapeNode, oldColor: SKColor, newColor: SKColor)?
        for entry in blockTiles {
            if entry.colOffset == 0 && entry.rowOffset == 0 {
                centerEntry = (entry.tile, entry.oldColor, entry.newColor)
            } else {
                let key = "\(entry.colOffset),\(entry.rowOffset)"
                offsetMap[key] = (entry.tile, entry.oldColor, entry.newColor)
            }
        }

        for (index, (dc, dr)) in clockwiseOffsets.enumerated() {
            let key = "\(dc),\(dr)"
            guard let entry = offsetMap[key] else { continue }
            let delay = Double(index) * 0.035
            let action = SKAction.sequence([
                SKAction.wait(forDuration: delay),
                colorTransition(from: entry.oldColor, to: entry.newColor)
            ])
            entry.tile.run(action)
        }

        if let c = centerEntry {
            let action = SKAction.sequence([
                SKAction.wait(forDuration: Double(8) * 0.035),
                colorTransition(from: c.oldColor, to: c.newColor)
            ])
            c.tile.run(action)
        }
    }

    private func animateRadialPulse(
        blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int, sym: SKLabelNode?, newSymText: String)]
    ) {
        for entry in blockTiles {
            let distance = abs(entry.colOffset) + abs(entry.rowOffset)
            let delay = TimeInterval(distance) * 0.040
            let scaleAction = SKAction.sequence([
                SKAction.wait(forDuration: delay),
                SKAction.scale(to: 1.12, duration: 0.08),
                SKAction.scale(to: 1.00, duration: 0.10)
            ])
            let colourAction = colorTransition(from: entry.oldColor, to: entry.newColor)
            entry.tile.run(SKAction.group([scaleAction, colourAction]))
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

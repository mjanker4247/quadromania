// TileGridNode.swift
// 18×13 grid of colored tile sprites — port of graphics/renderer.c dot drawing.
// Tiles are drawn programmatically; no image assets required.

import SpriteKit

/// One symbol per color index (0–4), shown when the accessibility symbol overlay is enabled.
private let tileSymbols = ["●", "■", "▲", "◆", "★"]

/// Linearly interpolates between two SKColors by blending each RGB channel separately.
/// `t` should be in 0…1; values outside that range are not clamped here.
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
    private var tiles: [[SKShapeNode]] = []       // [col][row] — SKShapeNode circles
    private var colorIndices: [[Int]] = []        // mirrors playfield color values for change detection
    /// Which rotation animation plays when the player clicks a 3×3 block.
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

    /// Duration shared by every color-fade animation (seconds).
    private static let colorTransitionDuration: TimeInterval = 0.18

    /// Returns an SKAction that smoothly fades a tile's fillColor from `oldColor` to `newColor`
    /// over `colorTransitionDuration` seconds using per-frame RGB interpolation via `lerpColor`.
    /// SKAction.colorize is unavailable on SKShapeNode, so customAction is used instead.
    private func colorTransition(from oldColor: SKColor, to newColor: SKColor) -> SKAction {
        let d = CGFloat(Self.colorTransitionDuration)
        return SKAction.customAction(withDuration: Self.colorTransitionDuration) { node, elapsed in
            guard let shape = node as? SKShapeNode else { return }
            // Clamp t to 1.0 — the final callback can fire slightly past duration due to frame timing
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

    /// Dispatches the selected rotation animation for the 3×3 block at `center`.
    /// Tiles outside the block receive a plain color transition immediately.
    /// Tiles inside the block are handled by the style-specific animation method.
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
                          colOffset: Int, rowOffset: Int)] = []
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
                    rowOffset: dr
                ))
            }
        }

        switch transitionStyle {
        case .ringSweep:   animateRingSweep(blockTiles: blockTiles, center: center)
        case .sequential:  animateSequential(blockTiles: blockTiles)
        case .radialPulse: animateRadialPulse(blockTiles: blockTiles)
        }
    }

    /// Total time for the ring sweep wave to travel around all 8 ring positions (seconds).
    private static let ringSweepDuration: TimeInterval = 0.30

    /// Ring Sweep animation: tiles flash and color-transition in clockwise circular order.
    /// Each tile scales up briefly when it fires, creating a visible wave around the block.
    /// No overlay node is used — the effect lives entirely on the tiles themselves.
    ///
    /// Ordering: the sweep starts at angle −π/2 (bottom tile in SpriteKit Y-up coords) and
    /// advances clockwise. A tile at offset (dc, dr) has SpriteKit angle atan2(−dr, dc);
    /// its delay = (angular distance from −π/2 clockwise to that angle) / 2π × sweepDuration.
    /// The centre tile (no angular position) fires at the midpoint of the sweep.
    private func animateRingSweep(
        blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int)],
        center: (col: Int, row: Int)
    ) {
        let sweepDuration = Self.ringSweepDuration
        let twoPi = CGFloat.pi * 2

        for entry in blockTiles {
            // Compute when this tile's position is reached by the clockwise sweep
            let delay: TimeInterval
            if entry.colOffset == 0 && entry.rowOffset == 0 {
                delay = sweepDuration * 0.5
            } else {
                let tileAngle = atan2(CGFloat(-entry.rowOffset), CGFloat(entry.colOffset))
                // Normalize angular distance from sweep start (−π/2) clockwise to this tile → [0, 2π)
                var angDist = (-.pi / 2) - tileAngle
                angDist = ((angDist.truncatingRemainder(dividingBy: twoPi)) + twoPi)
                              .truncatingRemainder(dividingBy: twoPi)
                delay = TimeInterval(angDist / twoPi) * sweepDuration
            }

            // Each tile: wait until the sweep reaches it, then flash (scale up + back) while
            // simultaneously transitioning to the new colour
            let scaleAction = SKAction.sequence([
                SKAction.scale(to: 1.18, duration: 0.07),
                SKAction.scale(to: 1.00, duration: 0.09)
            ])
            let colourAction = colorTransition(from: entry.oldColor, to: entry.newColor)
            entry.tile.run(SKAction.sequence([
                SKAction.wait(forDuration: delay),
                SKAction.group([scaleAction, colourAction])
            ]))
        }
    }

    /// Sequential animation: the 8 outer tiles color-transition one by one in clockwise order
    /// (35 ms apart), with the center tile firing last after all ring tiles have started.
    private func animateSequential(
        blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int)]
    ) {
        // Clockwise ring order starting top-left: top row L→R, right col T→B, bottom row R→L, left col B→T
        let clockwiseOffsets: [(Int, Int)] = [
            (-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0)
        ]
        // Index changed tiles by their (colOffset, rowOffset) string key for O(1) lookup
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

    /// Radial Pulse animation: tiles scale outward and color-transition in rings expanding from
    /// the center. Delay is proportional to Manhattan distance so the center fires first,
    /// the 4 edge-adjacent tiles next, and the 4 corner tiles last.
    private func animateRadialPulse(
        blockTiles: [(tile: SKShapeNode, oldColor: SKColor, newColor: SKColor,
                      colOffset: Int, rowOffset: Int)]
    ) {
        for entry in blockTiles {
            // Manhattan distance: 0 = center, 1 = edge-adjacent, 2 = corner
            let distance = abs(entry.colOffset) + abs(entry.rowOffset)
            let delay = TimeInterval(distance) * 0.040
            let scaleAction = SKAction.sequence([
                SKAction.wait(forDuration: delay),
                SKAction.scale(to: 1.12, duration: 0.08),
                SKAction.scale(to: 1.00, duration: 0.10)
            ])
            let colourAction = SKAction.sequence([
                SKAction.wait(forDuration: delay),
                colorTransition(from: entry.oldColor, to: entry.newColor)
            ])
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

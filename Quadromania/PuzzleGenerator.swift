// PuzzleGenerator.swift
// Generates guaranteed-solvable Quadromania puzzles using backwards construction.
// See docs/superpowers/specs/2026-03-13-puzzle-solver-design.md for the proof.
// No SpriteKit or UIKit imports.

import Foundation

struct PuzzleGenerator {

    struct Result {
        /// Scrambled playfield in [column][row] order (18 columns × 13 rows).
        /// Values are in 0...maxColors.
        let playfield: [[Int]]

        /// Total player presses in the known solution. Always ≥ 1 and ≤ limit.
        let knownSolutionMoveCount: Int

        /// Press counts per rotation center, indexed [column][row] over 18×13.
        /// Only positions in x ∈ 1...16, y ∈ 1...11 can be non-zero.
        /// f[x][y] presses at (x,y) will solve the board.
        let solutionMap: [[Int]]
    }

    // Valid rotation-center ranges: the 1-tile border cannot be a 3×3 center.
    private static let validX = 1...(GameModel.gridWidth  - 2)  // 1...16
    private static let validY = 1...(GameModel.gridHeight - 2)  // 1...11

    /// Generate a solvable puzzle for the given level and color count.
    ///
    /// - Parameters:
    ///   - level: Difficulty level. Clamped to 1...10.
    ///   - maxColors: Maximum color index. Clamped to 1...4.
    /// - Returns: A scrambled playfield, the known solution move count, and the solution map.
    static func generate(level: Int, maxColors: Int) -> Result {

        let level = max(1, min(10, level))
        let maxColors = max(1, min(4, maxColors))
        let modulus = maxColors + 1

        let initialRotations = 56 + (11 - level) * 13
        let limit = initialRotations * maxColors
        let margin = 0.60 + Double(level - 1) * (0.30 / 9.0)

        // Cap at grid capacity to guarantee buildSolutionMap terminates.
        let gridCapacity = validX.count * validY.count * (modulus - 1)
        let targetMoves  = max(1, min(Int(Double(limit) * margin), gridCapacity))

        let f = buildSolutionMap(targetMoves: targetMoves, modulus: modulus)

        var board = Array(
            repeating: Array(repeating: 0, count: GameModel.gridHeight),
            count: GameModel.gridWidth
        )

        for i in 0..<GameModel.gridWidth {
            for j in 0..<GameModel.gridHeight {

                var cover = 0

                for x in max(validX.lowerBound, i - 1)...min(validX.upperBound, i + 1) {
                    for y in max(validY.lowerBound, j - 1)...min(validY.upperBound, j + 1) {
                        cover += f[x][y]
                    }
                }

                board[i][j] = (modulus - cover % modulus) % modulus
            }
        }

        return Result(
            playfield: board,
            knownSolutionMoveCount: targetMoves,
            solutionMap: f
        )
    }

    static func buildSolutionMap(targetMoves: Int, modulus: Int) -> [[Int]] {

        var f = Array(
            repeating: Array(repeating: 0, count: GameModel.gridHeight),
            count: GameModel.gridWidth
        )

        var remaining = targetMoves

        let positions = validX.flatMap { x in validY.map { y in (x, y) } }.shuffled()

        for (x, y) in positions {

            if remaining == 0 { break }

            let maxAdd = min(modulus - 1, remaining)
            let add = Int.random(in: 1...maxAdd)

            f[x][y] = add
            remaining -= add
        }

        // Distribute any remainder one-by-one into cells that still have capacity.
        while remaining > 0 {
            let x = Int.random(in: validX)
            let y = Int.random(in: validY)

            if f[x][y] < modulus - 1 {
                f[x][y] += 1
                remaining -= 1
            }
        }

        return f
    }
}

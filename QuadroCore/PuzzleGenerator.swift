// PuzzleGenerator.swift
// Generates guaranteed-solvable Quadromania puzzles using backwards construction.
// See docs/superpowers/specs/2026-03-13-puzzle-solver-design.md for the proof.
// No SpriteKit or UIKit imports.

import Foundation

public struct PuzzleGenerator {

    public struct Result {
        /// Scrambled playfield in [column][row] order (18 columns × 13 rows).
        /// Values are in 0...maxColors.
        public let playfield: [[Int]]

        /// Total player presses in the known solution. Always ≥ 1 and ≤ limit.
        public let knownSolutionMoveCount: Int

        /// Press counts per rotation center, indexed [column][row] over 18×13.
        /// Only positions in x ∈ 1...16, y ∈ 1...11 can be non-zero.
        /// f[x][y] presses at (x,y) will solve the board.
        public let solutionMap: [[Int]]
    }

    /// Generate a solvable puzzle for the given level and color count.
    ///
    /// - Parameters:
    ///   - level: Difficulty level. Clamped to 1...10.
    ///   - maxColors: Maximum color index. Clamped to 1...4.
    /// - Returns: A scrambled playfield, the known solution move count, and the solution map.
    public static func generate(level: Int, maxColors: Int) -> Result {
        let level     = max(1, min(10, level))
        let maxColors = max(1, min(4, maxColors))
        let modulus   = maxColors + 1

        let initialRotations = 56 + (11 - level) * 13
        let limit            = initialRotations * maxColors
        let margin           = 0.60 + Double(level - 1) * (0.30 / 9.0)
        let S                = max(1, Int(Double(limit) * margin))

        // Step 1: Sample S positions, accumulate counts mod modulus.
        var f = Array(repeating: Array(repeating: 0, count: 13), count: 18)

        // Sample solution presses
        for _ in 0..<S {
            let x = Int.random(in: 1...16)
            let y = Int.random(in: 1...11)
            f[x][y] = (f[x][y] + 1) % modulus
        }

        // Ensure non-zero solution
        var moveCount = 0
        for x in 1...16 {
            for y in 1...11 {
                moveCount += f[x][y]
            }
        }

        if moveCount == 0 {
            f[8][6] = 1
            moveCount = 1
        }

        // Build board analytically
        var board = Array(repeating: Array(repeating: 0, count: 13), count: 18)

        for i in 0..<18 {
            for j in 0..<13 {

                var cover = 0

                for x in max(1, i-1)...min(16, i+1) {
                    for y in max(1, j-1)...min(11, j+1) {
                        cover += f[x][y]
                    }
                }

                board[i][j] = (modulus - cover % modulus) % modulus
            }
        }

        return Result(
            playfield: board,
            knownSolutionMoveCount: moveCount,
            solutionMap: f
        )
    }
}

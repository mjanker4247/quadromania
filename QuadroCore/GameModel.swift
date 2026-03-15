// GameModel.swift
// Pure game logic — port of old_src/core/game.c
// No SpriteKit or UIKit imports.

import Foundation

/// All mutable game state. A new instance represents one started game.
public class GameModel {

    // MARK: - Constants (from game.c)

    public static let gridWidth  = 18
    public static let gridHeight = 13
    public static let baseColor  = 0

    /// Starting scramble count before level multiplier: 56 + level × 13
    private static let baseRotations: Int     = 56
    private static let modifierPerLevel: Int  = 13

    // MARK: - State

    /// 2-D array [column][row], values 0…maxColors
    public private(set) var playfield: [[Int]]

    /// Difficulty level (1–10).
    public let level: Int

    /// Maximum color index (1–4). A tile wraps back to 0 after this value.
    public let maxColors: Int

    /// Number of scramble rotations used to set up the board.
    public let initialRotations: Int

    /// Maximum allowed player turns.
    public let limit: Int

    /// Number of player turns taken so far.
    public private(set) var turns: Int = 0

    /// Random background art index (0–9) chosen at init.
    public let backgroundArtIndex: Int

    /// Total player presses in the known solution for this puzzle. Always ≤ limit.
    public let knownSolutionMoveCount: Int

    // MARK: - Init

    /// Create and scramble a new game.
    /// - Parameters:
    ///   - level: Difficulty level 1–10.
    ///   - maxColors: Number of colors 1–4.
    public init(level: Int, maxColors: Int) {
        self.level            = max(1, min(10, level))
        self.maxColors        = max(1, min(4, maxColors))
        self.initialRotations = GameModel.rotations(forLevel: level)
        self.limit            = self.initialRotations * self.maxColors
        self.backgroundArtIndex = Int.random(in: 0...9)

        // Generate a guaranteed-solvable puzzle via backwards construction.
        let generated = PuzzleGenerator.generate(level: self.level, maxColors: self.maxColors)
        playfield = generated.playfield
        knownSolutionMoveCount = generated.knownSolutionMoveCount
    }

    // MARK: - Public API

    /// Rotate the 3×3 block centered at (x, y).
    /// Caller must ensure x is in 1...16 and y in 1...11.
    public func rotate(x: Int, y: Int) {
        applyRotate(x: x, y: y)
        turns += 1
    }

    /// True when all tiles are back to baseColor (0).
    public var isGameWon: Bool {
        for col in playfield {
            for cell in col where cell != GameModel.baseColor { return false }
        }
        return true
    }

    /// True when the player has exceeded the turn limit.
    public var isTurnLimitHit: Bool { turns > limit }

    /// Score = ((limit - turns) × 10000) / turns, or 0 if limit exceeded.
    public var score: Int {
        guard !isTurnLimitHit, turns > 0 else { return 0 }
        return ((limit - turns) * 10_000) / turns
    }

    // MARK: - Static helpers

    /// Number of scramble rotations for a given level (1–10).
    /// Level 1 gives the most rotations (easiest — most turns to solve);
    /// level 10 gives the fewest (hardest — fewest turns to solve).
    public static func rotations(forLevel level: Int) -> Int {
        baseRotations + (11 - level) * modifierPerLevel
    }

    // MARK: - Private

    private func applyRotate(x: Int, y: Int) {
        for i in (x - 1)...(x + 1) {
            for j in (y - 1)...(y + 1) {
                playfield[i][j] += 1
                if playfield[i][j] > maxColors {
                    playfield[i][j] = GameModel.baseColor
                }
            }
        }
    }
}

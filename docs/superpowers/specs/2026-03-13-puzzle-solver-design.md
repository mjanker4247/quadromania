# Puzzle Generator — Design Spec

**Date:** 2026-03-13
**Feature branch:** `feature/puzzle-solver`
**Status:** Approved

---

## Overview

Quadromania currently scrambles the board with a fully random sequence of rotations, providing no guarantee that the resulting puzzle is solvable within the player's turn limit. This feature replaces that scramble with a **backwards-construction puzzle generator** that always produces a board with a known solution fitting within a level-scaled margin of the turn limit.

The solver is **internal and invisible to the player** — there is no hint system. Its only role is to guarantee every puzzle is fair.

---

## Mathematical Foundation

All rotations in Quadromania commute because they are additions over Z_{maxColors+1} (modular arithmetic). This means:

- The order of moves does not matter.
- Every rotation is invertible: applying it `(maxColors + 1 - k) % (maxColors + 1)` more times undoes `k` prior applications.
- Given any known set of player presses `f[x][y]`, the board that those presses solve can be computed analytically — no simulation needed.

**Solvability guarantee (proof):**

Minimum `limit` over all valid inputs: `(56 + 1×13) × 1 = 69`, so `limit ≥ 69 > 0` always.
Let `P` denote the set of all valid rotation centers `{(x,y) | x ∈ 1...16, y ∈ 1...11}`.
`S = max(1, Int(Double(limit) × margin)) ≤ limit` since `margin ≤ 0.90 < 1.0`.
Each sample increments exactly one `c[x][y]`, so `Σ_{P} c[x][y] = S`.
Since `f[x][y] = c[x][y] % (maxColors+1) ≤ c[x][y]` (modulo only reduces non-negative integers),
`knownSolutionMoveCount = Σ_{P} f[x][y] ≤ Σ_{P} c[x][y] = S ≤ limit`. ∎

Each turn in-game costs 1 regardless of which cell is pressed, so `knownSolutionMoveCount` counts total individual presses, matching the game's turn counter.

**Coverage note:** Every cell `(i,j)` in the 18×13 grid is covered by at least one valid rotation center (e.g., corner `(0,0)` is covered by `(1,1)`). Cells where `coverSum % modulus == 0` will have `board[i][j] = 0` — correct and intentional; those cells start at the goal value and the solution presses leave them at 0 net.

---

## Margin Formula

The generator ensures the known solution fits within a level-dependent fraction of the turn limit:

```
margin(level) = 0.60 + (level − 1) × (0.30 / 9)
```

| Level | Margin | Buffer |
|-------|--------|--------|
| 1     | 60%    | 40%    |
| 5     | ~73%   | ~27%   |
| 10    | 90%    | 10%    |

`S = max(1, Int(Double(limit) × margin(level)))` — clamped to at least 1 to ensure the board is never trivially solved.
`limit = initialRotations × maxColors`, where `initialRotations = 56 + level × 13`.
Inputs are clamped to `level ∈ 1...10`, `maxColors ∈ 1...4` before any computation (matching `GameModel`'s existing clamps).

---

## Architecture

A new file `PuzzleGenerator.swift` is added alongside `GameModel.swift`. It has no dependencies on SpriteKit, UIKit, or any other game subsystem.

```
GameModel.init(level:maxColors:)
    └── PuzzleGenerator.generate(level:maxColors:) → Result
             ├── playfield: [[Int]]   // [column][row], 18 columns × 13 rows
             └── knownSolutionMoveCount: Int
```

No other files are modified. Scenes, scoring, highscores, and sound are unaffected.

---

## `PuzzleGenerator` API

```swift
struct PuzzleGenerator {

    /// The result of puzzle generation.
    struct Result {
        /// Scrambled playfield in [column][row] order (18 columns × 13 rows).
        /// Values are in 0...maxColors. Index [0][0] is the top-left tile
        /// in game coordinates (column 0, row 0).
        let playfield: [[Int]]

        /// Total player presses in the known solution. Always ≥ 1 and ≤ limit.
        let knownSolutionMoveCount: Int

        /// Press counts per rotation center, indexed [column][row] over the full 18×13 grid.
        /// Only positions in x ∈ 1...16, y ∈ 1...11 can be non-zero.
        /// Applying `solutionMap[x][y]` presses at each (x,y) solves the board.
        /// Exposed to enable correctness testing and future hint system support.
        let solutionMap: [[Int]]
    }

    /// Generate a solvable puzzle.
    ///
    /// - Parameters:
    ///   - level: Difficulty level. Clamped to 1...10.
    ///   - maxColors: Maximum color index. Clamped to 1...4.
    /// - Returns: A scrambled playfield, the known solution move count, and the solution map.
    static func generate(level: Int, maxColors: Int) -> Result
}
```

---

## Coordinate System

The playfield array is `[column][row]`, matching `GameModel.playfield`:
- Columns: 0...17 (left to right)
- Rows: 0...12 (top to bottom in game coordinates)

Valid rotation centers (positions where the player can click):
- `x ∈ 1...16` (column index)
- `y ∈ 1...11` (row index)

This guarantees the full 3×3 block `(x−1...x+1) × (y−1...y+1)` stays within the 18×13 grid.

**Covering relation:** Position `(x, y)` covers cell `(i, j)` if and only if `|x − i| ≤ 1` AND `|y − j| ≤ 1`.

---

## Generation Algorithm

All steps are O(S + gridWidth × gridHeight).

1. **Clamp and compute parameters**
   ```
   level           = clamp(level, 1, 10)
   maxColors       = clamp(maxColors, 1, 4)
   modulus         = maxColors + 1
   initialRotations = 56 + level × 13
   limit           = initialRotations × maxColors
   margin          = 0.60 + Double(level − 1) × (0.30 / 9.0)
   S               = max(1, Int(Double(limit) × margin))
   ```

2. **Sample S individual presses**
   Draw `S` random positions, each uniformly from `x ∈ 1...16`, `y ∈ 1...11`.
   Accumulate: `c[x][y] += 1` for each drawn position.
   Uses `SystemRandomNumberGenerator` — no reproducibility requirement.

3. **Compute required player presses per position**
   `f[x][y] = c[x][y] % modulus`
   Positions where `f[x][y] == 0` require no player action (their contribution cancels out).

4. **Build the scrambled board analytically**
   For each cell `(i, j)` where `i ∈ 0...17`, `j ∈ 0...12`:
   ```
   coverSum = Σ_{(x,y) ∈ P : |x−i|≤1 ∧ |y−j|≤1} f[x][y]
   board[i][j] = (modulus − coverSum % modulus) % modulus
   ```
   Where `P = {(x,y) | x ∈ 1...16, y ∈ 1...11}` (valid rotation centers only).
   This produces values in `0...maxColors`.

5. **Compute and return**
   ```
   knownSolutionMoveCount = Σ_{(x,y) ∈ P} f[x][y]   // sum over all 16×11 rotation centers
   ```
   Return `Result(playfield: board, knownSolutionMoveCount: knownSolutionMoveCount)`.

**Correctness:** When the player applies `f[x][y]` presses at every position, each cell `(i,j)` accumulates `coverSum` additional increments. Adding to the board value:
`board[i][j] + coverSum ≡ (modulus − coverSum) + coverSum ≡ modulus ≡ 0 (mod modulus)` ✓

**Note on the trivially-solved edge case:** If all `f[x][y] == 0` (all samples cancelled out mod `modulus`), the board is all zeros — already solved. The `max(1, ...)` clamp on S makes this astronomically unlikely but not impossible; the implementation may optionally retry once if `knownSolutionMoveCount == 0`.

---

## Changes to `GameModel`

### Removed
The scramble loop in `init`:
```swift
for _ in 1..<initialRotations {
    let rx = Int.random(in: 1...16)
    let ry = Int.random(in: 1...11)
    applyRotate(x: rx, y: ry)
}
turns = 0
```

### Added

New stored property:
```swift
/// Total player presses in the known solution. Always ≤ limit. Informational only.
let knownSolutionMoveCount: Int
```

Replacement in `init`:
```swift
let generated = PuzzleGenerator.generate(level: level, maxColors: maxColors)
self.playfield = generated.playfield
self.knownSolutionMoveCount = generated.knownSolutionMoveCount
```

All other `GameModel` properties and methods (`rotate`, `isGameWon`, `isTurnLimitHit`, `score`, `limit`, `initialRotations`) are unchanged. The `applyRotate` private method remains, used at runtime by `rotate(x:y:)`.

---

## Out of Scope

- Hint system (showing the player the next move)
- Optimal (minimum-move) solution computation
- Seeded / reproducible random generation
- Changes to scenes, UI, scoring, highscores, or sound

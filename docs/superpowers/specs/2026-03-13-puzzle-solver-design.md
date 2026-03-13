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

The target solution size `S = Int(Double(limit) × margin(level))`, where `limit = initialRotations × maxColors`.

---

## Architecture

A new file `PuzzleGenerator.swift` is added alongside `GameModel.swift`. It has no dependencies on SpriteKit, UIKit, or any other game subsystem.

```
GameModel.init(level:maxColors:)
    └── PuzzleGenerator.generate(level:maxColors:) → Result
             ├── playfield: [[Int]]
             └── knownSolutionMoveCount: Int
```

No other files are modified.

---

## `PuzzleGenerator` API

```swift
struct PuzzleGenerator {

    struct Result {
        /// The scrambled playfield, ready to assign to GameModel.
        let playfield: [[Int]]
        /// Total player moves in the known solution. Always ≤ limit × margin(level).
        let knownSolutionMoveCount: Int
    }

    /// Generate a solvable puzzle for the given level and color count.
    static func generate(level: Int, maxColors: Int) -> Result
}
```

---

## Generation Algorithm

All steps are O(S + gridWidth × gridHeight).

1. **Compute parameters**
   - `initialRotations = 56 + level × 13`
   - `limit = initialRotations × maxColors`
   - `margin = 0.60 + Double(level − 1) × (0.30 / 9.0)`
   - `S = Int(Double(limit) × margin)`

2. **Sample positions**
   Pick `S` random positions from the valid interior: `x ∈ 1...16`, `y ∈ 1...11`.
   Accumulate counts: `c[x][y] += 1`.

3. **Compute required player presses**
   `f[x][y] = c[x][y] % (maxColors + 1)`
   Positions where `f[x][y] == 0` require no player action and are skipped.

4. **Build the scrambled board analytically**
   For each cell `(i, j)` in `0...17 × 0...12`:
   ```
   coverSum = Σ f[x][y]  for all (x,y) such that |x−i| ≤ 1 AND |y−j| ≤ 1
                          AND x ∈ 1...16, y ∈ 1...11
   board[i][j] = (maxColors + 1 − coverSum % (maxColors + 1)) % (maxColors + 1)
   ```

5. **Compute solution move count**
   `knownSolutionMoveCount = Σ f[x][y]` over all interior positions.
   This is always `≤ S ≤ limit × margin ≤ limit`.

**Correctness proof:**
When the player applies `f[x][y]` presses at every position, cell `(i, j)` accumulates `coverSum`. Adding this to the board value:
`board[i][j] + coverSum ≡ (maxColors+1 − coverSum) + coverSum ≡ maxColors+1 ≡ 0 (mod maxColors+1)` ✓

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
/// Minimum known solution length for this puzzle. Always ≤ limit. Informational only.
let knownSolutionMoveCount: Int
```

Replacement in `init`:
```swift
let generated = PuzzleGenerator.generate(level: level, maxColors: maxColors)
self.playfield = generated.playfield
self.knownSolutionMoveCount = generated.knownSolutionMoveCount
```

All other `GameModel` properties and methods (`rotate`, `isGameWon`, `isTurnLimitHit`, `score`, `limit`, `initialRotations`) are unchanged.

---

## Out of Scope

- Hint system (showing the player the next move)
- Optimal (minimum-move) solution computation
- Changes to scenes, UI, scoring, highscores, or sound

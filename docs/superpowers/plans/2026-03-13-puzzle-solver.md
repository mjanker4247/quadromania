# Puzzle Generator Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the random scramble in `GameModel` with a backwards-construction generator that guarantees every puzzle is solvable within a level-scaled fraction of the turn limit.

**Architecture:** `PuzzleGenerator.generate(level:maxColors:)` samples `S` random rotation positions, accumulates press counts modulo `(maxColors+1)`, then analytically computes the scrambled board from those counts. `GameModel.init` calls it instead of the old scramble loop. The mathematical proof (spec §2) guarantees `knownSolutionMoveCount ≤ S ≤ limit`.

**Tech Stack:** Swift 5, Foundation, XCTest (new test target), Xcode 16, macOS target.

---

## File Structure

| Path | Action | Responsibility |
|------|--------|----------------|
| `Quadromania/PuzzleGenerator.swift` | **Create** | Pure generator — margin formula, sampling, board construction |
| `Quadromania/GameModel.swift` | **Modify** (lines 49–71) | Remove scramble loop; add `knownSolutionMoveCount` property |
| `QuadroTests/PuzzleGeneratorTests.swift` | **Create** | Unit tests for PuzzleGenerator (shape, values, correctness) |
| `QuadroTests/GameModelIntegrationTests.swift` | **Create** | Integration tests for GameModel (board validity, limit, turns) |

---

## Chunk 1: Test Infrastructure + PuzzleGenerator

### Task 0: Commit this plan to the feature branch

**Files:**
- `docs/superpowers/plans/2026-03-13-puzzle-solver.md`

- [ ] **Step 1: Commit the plan file**

  ```bash
  git add docs/superpowers/plans/2026-03-13-puzzle-solver.md
  git commit -m "docs: add puzzle generator implementation plan"
  ```

---

### Task 1: Add XCTest target to Xcode

**Files:**
- Xcode project UI — creates `QuadroTests/` folder and updates `project.pbxproj`

- [ ] **Step 1: Open Xcode, add Unit Testing Bundle target**

  In Xcode:
  1. File → New → Target
  2. Search for "Unit Testing Bundle" → Next
  3. Product Name: `QuadroTests`
  4. Team: same as Quadromania target
  5. Target to be Tested: `Quadromania`
  6. Click Finish

  Xcode creates `QuadroTests/QuadroTests.swift` (sample file) and `QuadroTests/Info.plist`.

- [ ] **Step 2: Delete the generated sample file**

  Delete `QuadroTests/QuadroTests.swift` (the boilerplate Xcode generates — we will replace it with real tests in Task 2).

- [ ] **Step 3: Verify the test target compiles**

  ```bash
  xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania \
    -destination 'platform=macOS,arch=arm64' 2>&1 | tail -5
  ```

  Expected output contains: `** TEST SUCCEEDED **` (no test cases run yet — that is correct).

- [ ] **Step 4: Commit the empty test target**

  ```bash
  git add Quadromania.xcodeproj/project.pbxproj QuadroTests/
  git commit -m "test: add QuadroTests XCTest target"
  ```

---

### Task 2: Implement PuzzleGenerator (TDD)

**Files:**
- Create: `QuadroTests/PuzzleGeneratorTests.swift`
- Create: `Quadromania/PuzzleGenerator.swift`

- [ ] **Step 1: Write the failing tests**

  Create `QuadroTests/PuzzleGeneratorTests.swift`:

  ```swift
  import XCTest
  @testable import Quadromania

  final class PuzzleGeneratorTests: XCTestCase {

      // MARK: - Playfield shape

      func testPlayfieldHas18Columns() {
          let result = PuzzleGenerator.generate(level: 5, maxColors: 3)
          XCTAssertEqual(result.playfield.count, 18)
      }

      func testPlayfieldHas13Rows() {
          let result = PuzzleGenerator.generate(level: 5, maxColors: 3)
          XCTAssertTrue(result.playfield.allSatisfy { $0.count == 13 })
      }

      // MARK: - Cell values in range

      func testAllCellValuesWithinRange() {
          for maxColors in 1...4 {
              let result = PuzzleGenerator.generate(level: 5, maxColors: maxColors)
              for col in result.playfield {
                  for cell in col {
                      XCTAssertGreaterThanOrEqual(cell, 0,
                          "maxColors=\(maxColors): cell below 0")
                      XCTAssertLessThanOrEqual(cell, maxColors,
                          "maxColors=\(maxColors): cell \(cell) exceeds maxColors")
                  }
              }
          }
      }

      // MARK: - Solution move count

      func testKnownSolutionMoveCountAtLeastOne() {
          for level in [1, 5, 10] {
              for maxColors in [1, 2, 4] {
                  let result = PuzzleGenerator.generate(level: level, maxColors: maxColors)
                  XCTAssertGreaterThanOrEqual(result.knownSolutionMoveCount, 1,
                      "level=\(level) maxColors=\(maxColors)")
              }
          }
      }

      func testKnownSolutionMoveCountWithinLimit() {
          for level in 1...10 {
              for maxColors in 1...4 {
                  let initialRotations = 56 + level * 13
                  let limit = initialRotations * maxColors
                  let result = PuzzleGenerator.generate(level: level, maxColors: maxColors)
                  XCTAssertLessThanOrEqual(result.knownSolutionMoveCount, limit,
                      "level=\(level) maxColors=\(maxColors): moveCount=\(result.knownSolutionMoveCount) limit=\(limit)")
              }
          }
      }

      // MARK: - Correctness: applying the solution solves the board

      func testApplyingSolutionSolvesBoard() {
          // Apply result.solutionMap presses to result.playfield and verify all cells → 0.
          for maxColors in 1...4 {
              let modulus = maxColors + 1
              let result = PuzzleGenerator.generate(level: 5, maxColors: maxColors)
              var board = result.playfield

              for x in 1...16 {
                  for y in 1...11 {
                      for _ in 0..<result.solutionMap[x][y] {
                          for i in (x - 1)...(x + 1) {
                              for j in (y - 1)...(y + 1) {
                                  board[i][j] = (board[i][j] + 1) % modulus
                              }
                          }
                      }
                  }
              }

              let solved = board.allSatisfy { col in col.allSatisfy { $0 == 0 } }
              XCTAssertTrue(solved,
                  "Board not solved after applying solutionMap: maxColors=\(maxColors)")
          }
      }

      // MARK: - SolutionMap shape and constraints

      func testSolutionMapIs18x13() {
          let result = PuzzleGenerator.generate(level: 5, maxColors: 3)
          XCTAssertEqual(result.solutionMap.count, 18)
          XCTAssertTrue(result.solutionMap.allSatisfy { $0.count == 13 })
      }

      func testSolutionMapBorderCellsAreZero() {
          // Non-interior positions (x=0, x=17, y=0, y=12) must be 0.
          let result = PuzzleGenerator.generate(level: 5, maxColors: 3)
          for j in 0...12 {
              XCTAssertEqual(result.solutionMap[0][j], 0, "solutionMap[0][\(j)] should be 0")
              XCTAssertEqual(result.solutionMap[17][j], 0, "solutionMap[17][\(j)] should be 0")
          }
          for i in 0...17 {
              XCTAssertEqual(result.solutionMap[i][0], 0, "solutionMap[\(i)][0] should be 0")
              XCTAssertEqual(result.solutionMap[i][12], 0, "solutionMap[\(i)][12] should be 0")
          }
      }

      func testSolutionMapValuesAtMostMaxColors() {
          // f[x][y] = c[x][y] % (maxColors+1) ≤ maxColors
          for maxColors in 1...4 {
              let result = PuzzleGenerator.generate(level: 5, maxColors: maxColors)
              for col in result.solutionMap {
                  for val in col {
                      XCTAssertLessThanOrEqual(val, maxColors,
                          "solutionMap value \(val) exceeds maxColors \(maxColors)")
                      XCTAssertGreaterThanOrEqual(val, 0)
                  }
              }
          }
      }

      // MARK: - Input clamping

      func testLevelBelowMinIsClamped() {
          // level=0 should behave like level=1
          let result0 = PuzzleGenerator.generate(level: 0, maxColors: 2)
          let limit1 = (56 + 1 * 13) * 2
          XCTAssertLessThanOrEqual(result0.knownSolutionMoveCount, limit1)
      }

      func testMaxColorsAboveMaxIsClamped() {
          // maxColors=5 should behave like maxColors=4
          let result = PuzzleGenerator.generate(level: 5, maxColors: 5)
          for col in result.playfield {
              for cell in col {
                  XCTAssertLessThanOrEqual(cell, 4)
              }
          }
      }
  }
  ```

- [ ] **Step 2: Run tests to verify they all fail (PuzzleGenerator not yet defined)**

  ```bash
  xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania \
    -destination 'platform=macOS,arch=arm64' 2>&1 | grep -E "(error:|FAILED|passed|failed)"
  ```

  Expected: compile errors about `PuzzleGenerator` not being found. If tests somehow pass, stop and investigate — the type must already exist somewhere.

- [ ] **Step 3: Implement PuzzleGenerator.swift**

  Create `Quadromania/PuzzleGenerator.swift`:

  ```swift
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

      /// Generate a solvable puzzle for the given level and color count.
      ///
      /// - Parameters:
      ///   - level: Difficulty level. Clamped to 1...10.
      ///   - maxColors: Maximum color index. Clamped to 1...4.
      /// - Returns: A scrambled playfield, the known solution move count, and the solution map.
      static func generate(level: Int, maxColors: Int) -> Result {
          let level     = max(1, min(10, level))
          let maxColors = max(1, min(4, maxColors))
          let modulus   = maxColors + 1

          let initialRotations = 56 + level * 13
          let limit            = initialRotations * maxColors
          let margin           = 0.60 + Double(level - 1) * (0.30 / 9.0)
          let S                = max(1, Int(Double(limit) * margin))

          // Step 1: Sample S positions, accumulate counts mod modulus.
          // c[x][y] = c[x][y] accumulated; f[x][y] = c[x][y] % modulus (done inline).
          var f = Array(repeating: Array(repeating: 0, count: 13), count: 18)
          for _ in 0..<S {
              let x = Int.random(in: 1...16)
              let y = Int.random(in: 1...11)
              f[x][y] = (f[x][y] + 1) % modulus
          }

          // Step 2: Guard against the astronomically rare all-zero edge case
          // (all samples cancel to 0 mod modulus). The spec allows an optional retry;
          // we use a deterministic fallback instead — simpler and avoids non-termination risk.
          let total = (1...16).flatMap { x in (1...11).map { y in f[x][y] } }.reduce(0, +)
          if total == 0 {
              f[8][6] = 1   // force a single press at the grid center
          }

          // Step 3: Compute knownSolutionMoveCount = Σ_{P} f[x][y].
          let knownSolutionMoveCount = (1...16)
              .flatMap { x in (1...11).map { y in f[x][y] } }
              .reduce(0, +)

          // Step 4: Build the scrambled board analytically.
          // board[i][j] = (modulus − coverSum % modulus) % modulus
          // where coverSum = Σ_{(x,y)∈P : |x−i|≤1 ∧ |y−j|≤1} f[x][y].
          var board = Array(repeating: Array(repeating: 0, count: 13), count: 18)
          for i in 0...17 {
              for j in 0...12 {
                  var coverSum = 0
                  for x in max(1, i - 1)...min(16, i + 1) {
                      for y in max(1, j - 1)...min(11, j + 1) {
                          coverSum += f[x][y]
                      }
                  }
                  board[i][j] = (modulus - coverSum % modulus) % modulus
              }
          }

          return Result(
              playfield: board,
              knownSolutionMoveCount: knownSolutionMoveCount,
              solutionMap: f
          )
      }
  }
  ```

- [ ] **Step 4: Run tests to verify they all pass**

  ```bash
  xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania \
    -destination 'platform=macOS,arch=arm64' 2>&1 | grep -E "(Test Suite|passed|failed|error:)"
  ```

  Expected:
  ```
  Test Suite 'PuzzleGeneratorTests' passed
  Executed 10 tests, with 0 failures
  ```

  If any test fails:
  - `testAllCellValuesWithinRange` fails → check the `board[i][j]` formula, especially the trailing `% modulus` ensuring values stay in 0...maxColors
  - `testApplyingSolutionSolvesBoard` fails → verify the coverSum loop bounds exactly match the board construction (both use `max(1, i-1)...min(16, i+1)`)
  - `testKnownSolutionMoveCountWithinLimit` fails → verify `knownSolutionMoveCount` is computed as `Σ f[x][y]` (not `Σ c[x][y]`), and that `S = max(1, Int(Double(limit) * margin))` uses the clamped `level` and `maxColors`

- [ ] **Step 5: Commit**

  ```bash
  git add Quadromania/PuzzleGenerator.swift QuadroTests/PuzzleGeneratorTests.swift
  git commit -m "feat: add PuzzleGenerator with backwards-construction algorithm"
  ```

---

## Chunk 2: GameModel Integration

> **Prerequisites:** Chunk 1 must be complete — the XCTest target (`QuadroTests`) and `PuzzleGenerator.swift` must exist before starting this chunk.
>
> **Note on file discovery:** The Xcode project uses `PBXFileSystemSynchronizedRootGroup` for both the main `Quadromania/` folder and the `QuadroTests/` test target (created via Xcode UI). This means any `.swift` file added to either directory on disk is automatically compiled — no manual `project.pbxproj` editing is needed.

### Task 3: Integrate PuzzleGenerator into GameModel (TDD)

**Files:**
- Create: `QuadroTests/GameModelIntegrationTests.swift`
- Modify: `Quadromania/GameModel.swift`

- [ ] **Step 1: Write the failing integration tests**

  Create `QuadroTests/GameModelIntegrationTests.swift`:

  ```swift
  import XCTest
  @testable import Quadromania

  final class GameModelIntegrationTests: XCTestCase {

      // MARK: - Board validity after init

      func testBoardHasCorrectDimensions() {
          let model = GameModel(level: 5, maxColors: 3)
          XCTAssertEqual(model.playfield.count, 18)
          XCTAssertTrue(model.playfield.allSatisfy { $0.count == 13 })
      }

      func testAllBoardCellValuesInRange() {
          for maxColors in 1...4 {
              let model = GameModel(level: 5, maxColors: maxColors)
              for col in model.playfield {
                  for cell in col {
                      XCTAssertGreaterThanOrEqual(cell, 0)
                      XCTAssertLessThanOrEqual(cell, maxColors,
                          "maxColors=\(maxColors): cell \(cell) out of range")
                  }
              }
          }
      }

      func testBoardIsNotTriviallySolved() {
          // knownSolutionMoveCount ≥ 1 guarantees at least one cell is non-zero.
          for level in [1, 5, 10] {
              let model = GameModel(level: level, maxColors: 3)
              XCTAssertFalse(model.isGameWon,
                  "Board should not start solved: level=\(level)")
          }
      }

      // MARK: - knownSolutionMoveCount

      func testKnownSolutionMoveCountExistsAndWithinLimit() {
          for level in 1...10 {
              for maxColors in 1...4 {
                  let model = GameModel(level: level, maxColors: maxColors)
                  XCTAssertGreaterThanOrEqual(model.knownSolutionMoveCount, 1,
                      "level=\(level) maxColors=\(maxColors)")
                  XCTAssertLessThanOrEqual(model.knownSolutionMoveCount, model.limit,
                      "level=\(level) maxColors=\(maxColors): moveCount=\(model.knownSolutionMoveCount) limit=\(model.limit)")
              }
          }
      }

      // MARK: - Turn counter not affected by generation

      func testTurnsIsZeroAfterInit() {
          let model = GameModel(level: 5, maxColors: 3)
          XCTAssertEqual(model.turns, 0)
      }

      func testRotateIncrementsPlayerTurns() {
          let model = GameModel(level: 5, maxColors: 3)
          model.rotate(x: 5, y: 5)
          XCTAssertEqual(model.turns, 1)
      }

      func testTurnLimitNotHitAtStart() {
          let model = GameModel(level: 5, maxColors: 3)
          XCTAssertFalse(model.isTurnLimitHit)
      }

      // MARK: - Limit formula unchanged

      func testLimitEqualsInitialRotationsTimesMaxColors() {
          for level in [1, 5, 10] {
              for maxColors in [1, 4] {
                  let model = GameModel(level: level, maxColors: maxColors)
                  let expected = (56 + level * 13) * maxColors
                  XCTAssertEqual(model.limit, expected,
                      "level=\(level) maxColors=\(maxColors)")
              }
          }
      }
  }
  ```

- [ ] **Step 2: Run tests to verify knownSolutionMoveCount tests fail**

  ```bash
  xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania \
    -destination 'platform=macOS,arch=arm64' 2>&1 | grep -E "(error:|failed|passed)"
  ```

  Expected: compile error — `GameModel` has no member `knownSolutionMoveCount`. The test target will not build and 0 tests will run. This is correct — proceed to Step 3.

- [ ] **Step 3: Modify GameModel.swift — add property and replace scramble**

  In `Quadromania/GameModel.swift`, make the following targeted changes.

  **3a — Add `knownSolutionMoveCount` property** (after the existing `backgroundArtIndex` property, around line 42):

  Replace:
  ```swift
      /// Random background art index (0–9) chosen at init.
      let backgroundArtIndex: Int
  ```
  With:
  ```swift
      /// Random background art index (0–9) chosen at init.
      let backgroundArtIndex: Int

      /// Total player presses in the known solution for this puzzle. Always ≤ limit.
      let knownSolutionMoveCount: Int
  ```

  **3b — Replace the scramble block in `init`** (lines 56–70, the playfield init + scramble loop + turns reset):

  Replace this block:
  ```swift
          // Start with a cleared playfield.
          playfield = Array(
              repeating: Array(repeating: GameModel.baseColor, count: GameModel.gridHeight),
              count: GameModel.gridWidth
          )

          // Scramble: same logic as Quadromania_InitPlayfield.
          // x in 1...16, y in 1...11 (interior cells that allow a full 3×3 block)
          for _ in 1..<initialRotations {
              let rx = Int.random(in: 1...16)
              let ry = Int.random(in: 1...11)
              applyRotate(x: rx, y: ry)
          }
          // turns is reset to 0 after scramble — scramble moves don't count
          turns = 0
  ```
  With:
  ```swift
          // Generate a guaranteed-solvable puzzle via backwards construction.
          let generated = PuzzleGenerator.generate(level: self.level, maxColors: self.maxColors)
          playfield = generated.playfield
          knownSolutionMoveCount = generated.knownSolutionMoveCount
  ```

- [ ] **Step 4: Run all tests to verify they pass**

  ```bash
  xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania \
    -destination 'platform=macOS,arch=arm64' 2>&1 | grep -E "(Test Suite|passed|failed|error:)"
  ```

  Expected:
  ```
  Test Suite 'PuzzleGeneratorTests' passed
  Executed 11 tests, with 0 failures
  Test Suite 'GameModelIntegrationTests' passed
  Executed 8 tests, with 0 failures
  ```

  If `testBoardIsNotTriviallySolved` fails → the guard `if total == 0 { f[8][6] = 1 }` in `PuzzleGenerator` may not be reached; verify the accumulation loop in `generate`. After integrating PuzzleGenerator, this test is deterministically non-flaky (knownSolutionMoveCount ≥ 1 is guaranteed).

  If `testLimitEqualsInitialRotationsTimesMaxColors` fails → `GameModel.init` uses raw `level` for `initialRotations` (before clamping). This is pre-existing behaviour; the test must use the same raw level. Adjust the test's `expected` to use `(56 + level * 13) * maxColors` directly (as written above) — this matches what `GameModel.rotations(forLevel:)` produces.

- [ ] **Step 5: Build the app target to verify no regressions**

  ```bash
  xcodebuild -project Quadromania.xcodeproj -scheme Quadromania \
    -configuration Debug build 2>&1 | tail -3
  ```

  Expected: `** BUILD SUCCEEDED **`

- [ ] **Step 6: Commit**

  ```bash
  git add Quadromania/GameModel.swift QuadroTests/GameModelIntegrationTests.swift
  git commit -m "feat: integrate PuzzleGenerator into GameModel, guarantee solvable puzzles"
  ```

---

### Task 4: Final verification

**Files:** None — read-only checks.

- [ ] **Step 1: Run full test suite one last time**

  ```bash
  xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania \
    -destination 'platform=macOS,arch=arm64' 2>&1 | grep -E "(Test Suite 'All|passed|failed)"
  ```

  Expected:
  ```
  Test Suite 'All tests' passed
  Executed 19 tests, with 0 failures (0 unexpected)
  ```

  (11 from `PuzzleGeneratorTests` + 8 from `GameModelIntegrationTests`)

- [ ] **Step 2: Confirm feature branch is ready for PR**

  ```bash
  git log main..HEAD --oneline
  ```

  Expected (4 commits added on this branch):
  ```
  feat: integrate PuzzleGenerator into GameModel, guarantee solvable puzzles
  feat: add PuzzleGenerator with backwards-construction algorithm
  test: add QuadroTests XCTest target
  docs: add puzzle generator implementation plan
  ```

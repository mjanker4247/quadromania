import XCTest
@testable import QuadroCore

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
                let initialRotations = 56 + (11 - level) * 13
                let limit = initialRotations * maxColors
                let result = PuzzleGenerator.generate(level: level, maxColors: maxColors)
                XCTAssertLessThanOrEqual(result.knownSolutionMoveCount, limit,
                    "level=\(level) maxColors=\(maxColors): moveCount=\(result.knownSolutionMoveCount) limit=\(limit)")
            }
        }
    }

    // MARK: - Correctness: applying the solution solves the board

    func testApplyingSolutionSolvesBoard() {
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
        let result0 = PuzzleGenerator.generate(level: 0, maxColors: 2)
        let limit1 = (56 + (11 - 1) * 13) * 2   // level 1 → 186 rotations
        XCTAssertLessThanOrEqual(result0.knownSolutionMoveCount, limit1)
    }

    func testMaxColorsAboveMaxIsClamped() {
        let result = PuzzleGenerator.generate(level: 5, maxColors: 5)
        for col in result.playfield {
            for cell in col {
                XCTAssertLessThanOrEqual(cell, 4)
            }
        }
    }
}

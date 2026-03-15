import XCTest
@testable import QuadroCore

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
                let expected = (56 + (11 - level) * 13) * maxColors
                XCTAssertEqual(model.limit, expected,
                    "level=\(level) maxColors=\(maxColors)")
            }
        }
    }
}

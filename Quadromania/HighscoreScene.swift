// HighscoreScene.swift
// Highscore display — port of Graphics_ListHighscores().

import SpriteKit

class HighscoreScene: SKScene {

    private let level: Int   // 1-based

    init(level: Int, size: CGSize) {
        self.level = level
        super.init(size: size)
    }

    required init?(coder: NSCoder) { fatalError("Use init(level:size:)") }

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.08, green: 0.10, blue: 0.18, alpha: 1)
        buildUI()
    }

    // MARK: - Build UI

    private func buildUI() {
        // Title
        let title = SKLabelNode(text: "QUADROMANIA")
        title.fontName  = "Helvetica-Bold"
        title.fontSize  = 52
        title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
        title.horizontalAlignmentMode = .center
        title.position = CGPoint(x: size.width / 2, y: 870)
        addChild(title)

        let heading = SKLabelNode(text: "High Scores — Level \(level)")
        heading.fontName  = "Helvetica-Bold"
        heading.fontSize  = 30
        heading.fontColor = .white
        heading.horizontalAlignmentMode = .center
        heading.position = CGPoint(x: size.width / 2, y: 810)
        addChild(heading)

        // Column headers
        addRowLabel(rank: "#", name: "Name", score: "Score", y: 760, isHeader: true)

        // Entries
        let entries = HighscoreManager.shared.entries(forTable: level - 1)
        for (i, entry) in entries.enumerated() {
            let y = 720 - CGFloat(i) * 44
            let scoreText = entry.score > 0 ? "\(entry.score)" : "—"
            addRowLabel(rank: "\(i + 1)", name: entry.name, score: scoreText, y: y, isHeader: false)
        }

        // Footer
        let hint = SKLabelNode(text: "Click anywhere to return")
        hint.fontName  = "Helvetica"
        hint.fontSize  = 20
        hint.fontColor = SKColor(white: 0.45, alpha: 1)
        hint.horizontalAlignmentMode = .center
        hint.position = CGPoint(x: size.width / 2, y: 30)
        hint.run(.repeatForever(.sequence([
            .fadeAlpha(to: 0.2, duration: 0.9),
            .fadeAlpha(to: 1.0, duration: 0.9)
        ])))
        addChild(hint)
    }

    private func addRowLabel(rank: String, name: String, score: String, y: CGFloat, isHeader: Bool) {
        let font   = isHeader ? "Helvetica-Bold" : "Helvetica"
        let size_: CGFloat = isHeader ? 22 : 20
        let color  = isHeader ? SKColor(white: 0.65, alpha: 1) : SKColor.white

        let rankLabel = SKLabelNode(text: rank)
        rankLabel.fontName  = font
        rankLabel.fontSize  = size_
        rankLabel.fontColor = color
        rankLabel.horizontalAlignmentMode = .center
        rankLabel.position = CGPoint(x: 160, y: y)
        addChild(rankLabel)

        let nameLabel = SKLabelNode(text: name)
        nameLabel.fontName  = font
        nameLabel.fontSize  = size_
        nameLabel.fontColor = color
        nameLabel.horizontalAlignmentMode = .left
        nameLabel.position = CGPoint(x: 220, y: y)
        addChild(nameLabel)

        let scoreLabel = SKLabelNode(text: score)
        scoreLabel.fontName  = font
        scoreLabel.fontSize  = size_
        scoreLabel.fontColor = color
        scoreLabel.horizontalAlignmentMode = .right
        scoreLabel.position = CGPoint(x: 1100, y: y)
        addChild(scoreLabel)
    }

    // MARK: - Input

    override func mouseDown(with event: NSEvent) {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    }
}

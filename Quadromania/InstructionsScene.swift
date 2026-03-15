// InstructionsScene.swift
// How-to-play screen — port of Graphics_DrawInstructions().

import SpriteKit
import QuadroCore

class InstructionsScene: SKScene {

    // MARK: - Context (set nil when launched from title screen)
    private let sourceGame:    GameModel?
    private let sourcePalette: TilePalette

    private var tutorialButtonLabel: SKLabelNode?

    init(size: CGSize, sourceGame: GameModel? = nil, sourcePalette: TilePalette = .spring) {
        self.sourceGame    = sourceGame
        self.sourcePalette = sourcePalette
        super.init(size: size)
    }

    required init?(coder: NSCoder) { fatalError("Use init(size:)") }

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.06, green: 0.08, blue: 0.14, alpha: 1)
        buildUI()
    }

    private func buildUI() {
        // Title
        let title = SKLabelNode(text: "HOW TO PLAY")
        title.fontName  = "Helvetica-Bold"
        title.fontSize  = 52
        title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
        title.horizontalAlignmentMode = .center
        title.position = CGPoint(x: size.width / 2, y: 860)
        addChild(title)

        let lines: [(String, Bool)] = [
            ("Goal",                       true),
            ("Return every tile to its lightest color (color 0) before you run out of turns.", false),
            ("",                           false),
            ("How to play",                true),
            ("Click any tile to rotate the 3×3 block centered on it.", false),
            ("Every tile in that block steps forward by one color.", false),
            ("Tiles at the maximum color wrap back to 0.", false),
            ("",                           false),
            ("Turn limit",                 true),
            ("Your total turns are shown at the top of the screen.", false),
            ("Each click costs one turn — plan carefully.", false),
            ("",                           false),
            ("Scoring",                    true),
            ("Solve the board faster to score higher.", false),
            ("Exceeding the turn limit ends the game with no score recorded.", false),
            ("",                           false),
            ("Tip",                        true),
            ("Click tiles in the middle of large same-color regions to make progress efficiently.", false),
        ]

        var y: CGFloat = 790
        for (text, isHeader) in lines {
            guard !text.isEmpty else { y -= 16; continue }
            let label = SKLabelNode(text: text)
            label.fontName  = isHeader ? "Helvetica-Bold" : "Helvetica"
            label.fontSize  = isHeader ? 24 : 20
            label.fontColor = isHeader
                ? SKColor(red: 0.60, green: 0.85, blue: 1.0, alpha: 1)
                : SKColor(white: 0.88, alpha: 1)
            label.horizontalAlignmentMode = .left
            label.position = CGPoint(x: 100, y: y)
            addChild(label)
            y -= isHeader ? 34 : 28
        }

        // Tutorial button
        let tutBtn = SKLabelNode(text: "▶  Start Tutorial")
        tutBtn.fontName  = "Helvetica-Bold"
        tutBtn.fontSize  = 26
        tutBtn.fontColor = SKColor(red: 0.60, green: 0.85, blue: 1.0, alpha: 1)
        tutBtn.horizontalAlignmentMode = .center
        tutBtn.position = CGPoint(x: size.width / 2, y: 120)
        addChild(tutBtn)
        tutorialButtonLabel = tutBtn

        // Footer hint
        let hintText = (sourceGame != nil) ? "Click anywhere to return to game" : "Click anywhere to return"
        let hint = SKLabelNode(text: hintText)
        hint.fontName  = "Helvetica"
        hint.fontSize  = 18
        hint.fontColor = SKColor(white: 0.45, alpha: 1)
        hint.horizontalAlignmentMode = .center
        hint.position = CGPoint(x: size.width / 2, y: 60)
        hint.run(.repeatForever(.sequence([
            .fadeAlpha(to: 0.2, duration: 0.9),
            .fadeAlpha(to: 1.0, duration: 0.9)
        ])))
        addChild(hint)
    }

    // MARK: - Input

    override func mouseDown(with event: NSEvent) {
        let point = event.location(in: self)
        if let btn = tutorialButtonLabel, btn.frame.contains(point) {
            SoundManager.shared.playEffect(.menu)
            let scene = TutorialScene(size: size)
            scene.scaleMode = scaleMode
            view?.presentScene(scene, transition: .fade(withDuration: 0.3))
        } else if let model = sourceGame {
            let scene = GamePlayScene(model: model, palette: sourcePalette, size: size)
            scene.scaleMode = scaleMode
            view?.presentScene(scene, transition: .fade(withDuration: 0.2))
        } else {
            let scene = TitleScene(size: size)
            scene.scaleMode = scaleMode
            view?.presentScene(scene, transition: .fade(withDuration: 0.2))
        }
    }
}

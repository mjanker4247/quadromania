// InstructionsScene.swift
// How-to-play screen — port of Graphics_DrawInstructions().

import SpriteKit

class InstructionsScene: SKScene {

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
            ("The Goal",                   true),
            ("Return all 234 tiles (18×13 grid) to RED.", false),
            ("",                           false),
            ("How to rotate",              true),
            ("Click any tile to rotate the 3×3 block around it.", false),
            ("Each rotation increments every tile's colour by 1.", false),
            ("Tiles cycle back to red after reaching the maximum colour.", false),
            ("",                           false),
            ("Colours & Level",            true),
            ("'Select colors' sets how many colour steps tiles cycle through (1–4).", false),
            ("'Select level' controls how scrambled the board starts (1–10).", false),
            ("",                           false),
            ("Turn Limit",                 true),
            ("You have  initial_rotations × colours  turns to solve the board.", false),
            ("Exceeding the limit scores 0.", false),
            ("",                           false),
            ("Scoring",                    true),
            ("Score = ((limit − turns) × 10 000) ÷ turns", false),
            ("Fewer turns = higher score.", false),
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

        // Footer hint
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

    // MARK: - Input

    override func mouseDown(with event: NSEvent) {
        let scene = TitleScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }
}

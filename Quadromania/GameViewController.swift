import SpriteKit

/// NSViewController that hosts an SKView and presents the initial TitleScene.
class GameViewController: NSViewController {

    override func loadView() {
        let skView = SKView(frame: NSRect(x: 0, y: 0, width: 1280, height: 960))
        skView.ignoresSiblingOrder = true
        skView.showsFPS = false
        skView.showsNodeCount = false
        self.view = skView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        let scene = TitleScene(size: CGSize(width: 1280, height: 960))
        scene.scaleMode = .aspectFit
        (view as! SKView).presentScene(scene)
    }

    override func viewDidAppear() {
        super.viewDidAppear()
        // Required for mouseMoved events to reach the SpriteKit scene.
        view.window?.acceptsMouseMovedEvents = true
    }

    // MARK: - Game menu actions

    @objc func newGame(_ sender: Any?) {
        guard let skView = view as? SKView else { return }
        SoundManager.shared.playEffect(.menu)
        let scene = TitleScene(size: CGSize(width: 1280, height: 960))
        scene.scaleMode = .aspectFit
        skView.presentScene(scene, transition: .fade(withDuration: 0.3))
    }

    @objc func showInstructions(_ sender: Any?) {
        guard let skView = view as? SKView else { return }
        SoundManager.shared.playEffect(.menu)
        let scene = InstructionsScene(size: CGSize(width: 1280, height: 960))
        scene.scaleMode = .aspectFit
        skView.presentScene(scene, transition: .fade(withDuration: 0.2))
    }

    @objc func showHighscores(_ sender: Any?) {
        guard let skView = view as? SKView else { return }
        SoundManager.shared.playEffect(.menu)
        let scene = HighscoreScene(level: 1, size: CGSize(width: 1280, height: 960))
        scene.scaleMode = .aspectFit
        skView.presentScene(scene, transition: .fade(withDuration: 0.2))
    }
}

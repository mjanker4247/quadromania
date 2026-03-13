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
}

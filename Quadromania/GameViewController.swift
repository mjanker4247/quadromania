import SpriteKit

/// NSViewController that hosts an SKView and presents the initial TitleScene.
class GameViewController: NSViewController {

    override func loadView() {
        let skView = SKView()
        // Fill the window and track every resize; the scene's .aspectFit scale mode
        // keeps the 1280×960 coordinate space proportionally scaled inside the view.
        skView.autoresizingMask = [.width, .height]
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
        // Prevent the window from being shrunk below a playable size.
        view.window?.minSize = NSSize(width: 640, height: 480)
    }

}

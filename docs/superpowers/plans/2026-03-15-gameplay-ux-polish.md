# Gameplay UX Polish Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement five gameplay UX improvements: colour cycle strip, Beginner/Intermediate/Expert difficulty, Instructions from the macOS menu bar with context-aware back navigation, and removal of highscores.

**Architecture:** All changes are confined to the `Quadromania` app target. `QuadroCore` (pure game logic) and the `QuadroTests` target are untouched. No new files are created; two files (`HighscoreScene.swift`, `HighscoreManager.swift`) are deleted. The Xcode project uses `PBXFileSystemSynchronizedRootGroup` — file additions/deletions on disk are automatically reflected in the build without editing `.pbxproj`.

**Tech Stack:** Swift 5, SpriteKit, AppKit (`NSMenu`/`NSMenuItem`), `NotificationCenter` for decoupled scene ↔ menu communication.

---

## Chunk 1: Highscores removal, difficulty rename, Game menu + Instructions wiring

### Task 1: Remove highscores

**Files:**
- Delete: `Quadromania/HighscoreScene.swift`
- Delete: `Quadromania/HighscoreManager.swift`
- Modify: `Quadromania/TitleScene.swift`
- Modify: `Quadromania/GamePlayScene.swift`

- [ ] **Step 1.1 — Delete the two highscore files**

```bash
rm Quadromania/HighscoreScene.swift Quadromania/HighscoreManager.swift
```

- [ ] **Step 1.2 — Remove `.highscores` from TitleScene**

In `Quadromania/TitleScene.swift`:

Remove `.highscores` from the `MenuItem` enum (line 12):
```swift
// Before
private enum MenuItem: CaseIterable {
    case startGame, selectColors, selectTurns, instructions, highscores, quit
}
// After
private enum MenuItem: CaseIterable {
    case startGame, selectColors, selectTurns, instructions, quit
}
```

Remove the `.highscores` entry from the `itemY` dictionary and move `.quit` up one slot (around line 87–94):
```swift
let itemY: [MenuItem: CGFloat] = [
    .startGame:    menuStartY,
    .selectColors: menuStartY - lineSpacing,
    .selectTurns:  menuStartY - 2 * lineSpacing,
    .instructions: menuStartY - 5 * lineSpacing,
    .quit:         menuStartY - 6 * lineSpacing,
]
```

Remove the `.highscores` entry from the `items` array (around line 96–104):
```swift
let items: [(MenuItem, String)] = [
    (.startGame,    "Start the game"),
    (.selectColors, "Select colors:"),
    (.selectTurns,  "Select level:"),
    (.instructions, "Instructions"),
    (.quit,         "Quit"),
]
```

Remove `case .highscores:` from `mouseDown` switch (around line 293–296). The entire case block:
```swift
case .highscores:
    let scene = HighscoreScene(level: selectedLevel, size: size)
    scene.scaleMode = scaleMode
    view?.presentScene(scene, transition: .fade(withDuration: 0.2))
```

- [ ] **Step 1.3 — Remove highscore recording from GamePlayScene**

In `Quadromania/GamePlayScene.swift`, remove the call from `showWin()` (around line 143):
```swift
// Remove this line:
recordHighscoreIfQualifies()
```

Remove the entire `recordHighscoreIfQualifies()` function (lines 198–207):
```swift
// Remove entirely:
private func recordHighscoreIfQualifies() {
    let tableIndex = model.level - 1
    let hs = HighscoreManager.shared
    if let pos = hs.position(forTable: tableIndex, score: model.score) {
        hs.enterScore(model.score,
                      name: hs.nameFromTimestamp,
                      table: tableIndex,
                      at: pos)
    }
}
```

- [ ] **Step 1.4 — Build and confirm no errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|BUILD"
```
Expected: `BUILD SUCCEEDED` with zero `error:` lines.

- [ ] **Step 1.5 — Run tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "TEST SUCCEEDED|FAILED"
```
Expected: `TEST SUCCEEDED`

- [ ] **Step 1.6 — Commit**

```bash
git add -A
git commit -m "feat: remove highscores completely"
```

---

### Task 2: Rename difficulty levels to Beginner / Intermediate / Expert

**Files:**
- Modify: `Quadromania/TitleScene.swift`

- [ ] **Step 2.1 — Add difficulty constants and update selectedLevel default**

In `TitleScene.swift`, in the `// MARK: - Game configuration` section (around line 15–17), add the two constants immediately after `selectedColors`:
```swift
private var selectedColors  = 1              // 1–4
private var selectedLevel   = 1              // restricted to difficultyLevels
private let difficultyLevels = [1, 5, 10]
private let difficultyNames: [Int: String] = [1: "Beginner", 5: "Intermediate", 10: "Expert"]
```

- [ ] **Step 2.2 — Update the menu label text**

In the `items` array inside `addMenuLabels()`, change `"Select level:"` to `"Select difficulty:"`:
```swift
(.selectTurns,  "Select difficulty:"),
```

- [ ] **Step 2.3 — Update `updateDynamicLabels()` to show the difficulty name**

Replace the body of `updateDynamicLabels()` (currently around lines 138–150):
```swift
private func updateDynamicLabels() {
    rotationsValueLabel?.removeFromParent()
    let name = difficultyNames[selectedLevel] ?? "Beginner"
    let label = SKLabelNode(text: name)
    label.fontName  = "Helvetica"
    label.fontSize  = 24
    label.fontColor = SKColor(white: 0.75, alpha: 1)
    label.horizontalAlignmentMode = .left
    label.position = CGPoint(x: menuX + 240, y: menuStartY - 2 * lineSpacing + 4)
    addChild(label)
    rotationsValueLabel = label
}
```

- [ ] **Step 2.4 — Update the cycling logic in `mouseDown`**

Replace the `case .selectTurns:` block (currently `selectedLevel = (selectedLevel % 10) + 1`):
```swift
case .selectTurns:
    let idx = ((difficultyLevels.firstIndex(of: selectedLevel) ?? 0) + 1) % 3
    selectedLevel = difficultyLevels[idx]
    updateDynamicLabels()
```

- [ ] **Step 2.5 — Build and confirm no errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|BUILD"
```
Expected: `BUILD SUCCEEDED`

- [ ] **Step 2.6 — Commit**

```bash
git add Quadromania/TitleScene.swift
git commit -m "feat: replace numeric level with Beginner/Intermediate/Expert difficulty"
```

---

### Task 3: Add Game menu and `.showInstructions` notification to AppDelegate

**Files:**
- Modify: `Quadromania/AppDelegate.swift`

- [ ] **Step 3.1 — Add `.showInstructions` to the Notification.Name extension**

At the bottom of `AppDelegate.swift`, in the existing `extension Notification.Name` block (lines 120–123), add one line:
```swift
extension Notification.Name {
    static let paletteDidChange       = Notification.Name("paletteDidChange")
    static let symbolOverlayDidChange = Notification.Name("symbolOverlayDidChange")
    static let showInstructions       = Notification.Name("showInstructions")
}
```

- [ ] **Step 3.2 — Add `buildGameMenu()` method**

Add the following method to `AppDelegate`, in the `// MARK: - Palette menu` section or just before it, as a new `// MARK: - Game menu` section:
```swift
// MARK: - Game menu

private func buildGameMenu() {
    let menu = NSMenu(title: "Game")
    let item = NSMenuItem(
        title: "Instructions",
        action: #selector(showInstructionsMenuAction(_:)),
        keyEquivalent: ""
    )
    item.target = self
    menu.addItem(item)
    let menuItem = NSMenuItem(title: "Game", action: nil, keyEquivalent: "")
    menuItem.submenu = menu
    NSApp.mainMenu?.addItem(menuItem)
}

@objc private func showInstructionsMenuAction(_ sender: NSMenuItem) {
    NotificationCenter.default.post(name: .showInstructions, object: nil)
}
```

- [ ] **Step 3.3 — Call `buildGameMenu()` from `applicationDidFinishLaunching`**

In `applicationDidFinishLaunching`, call it before `buildPaletteMenu()`:
```swift
func applicationDidFinishLaunching(_ aNotification: Notification) {
    guard ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] == nil else { return }
    NSApp.activate(ignoringOtherApps: true)
    SoundManager.shared.startMusic()
    updateMusicMenuItem()
    buildGameMenu()
    buildPaletteMenu()
}
```

- [ ] **Step 3.4 — Build and confirm no errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|BUILD"
```
Expected: `BUILD SUCCEEDED`

- [ ] **Step 3.5 — Commit**

```bash
git add Quadromania/AppDelegate.swift
git commit -m "feat: add Game menu with Instructions item to menu bar"
```

---

### Task 4: Update InstructionsScene — new init and context-aware back navigation

**Files:**
- Modify: `Quadromania/InstructionsScene.swift`

- [ ] **Step 4.1 — Add stored properties and new designated initialiser**

Add `import QuadroCore` at the top (needed for `GameModel`).

Add two stored properties and the new initialiser. The full top of `InstructionsScene.swift` should become:
```swift
// InstructionsScene.swift

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
```

- [ ] **Step 4.2 — Update footer hint text to be context-aware**

In `buildUI()`, find the footer hint `SKLabelNode` creation (currently around line 74–84). Change the hardcoded text to read `self.sourceGame`:
```swift
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
```

- [ ] **Step 4.3 — Replace `mouseDown` with context-aware back navigation**

Replace the entire `mouseDown` method:
```swift
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
```

- [ ] **Step 4.4 — Build and confirm no errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|BUILD"
```
Expected: `BUILD SUCCEEDED`

- [ ] **Step 4.5 — Commit**

```bash
git add Quadromania/InstructionsScene.swift
git commit -m "feat: InstructionsScene gains game context and returns to game on back"
```

---

### Task 5: Wire `.showInstructions` observer in TitleScene and GamePlayScene

**Files:**
- Modify: `Quadromania/TitleScene.swift`
- Modify: `Quadromania/GamePlayScene.swift`

- [ ] **Step 5.1 — Register `.showInstructions` observer in TitleScene**

In `TitleScene.didMove(to:)`, add the new observer after the existing `.paletteDidChange` registration:
```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleShowInstructions(_:)),
    name: .showInstructions,
    object: nil
)
```

Add the handler method to `TitleScene` (can be placed near the existing `handlePaletteDidChange`):
```swift
@objc private func handleShowInstructions(_ notification: Notification) {
    let scene = InstructionsScene(size: size)
    scene.scaleMode = scaleMode
    view?.presentScene(scene, transition: .fade(withDuration: 0.2))
}
```

`willMove(from:)` already calls `NotificationCenter.default.removeObserver(self)` — no change needed.

- [ ] **Step 5.2 — Register `.showInstructions` observer in GamePlayScene**

In `GamePlayScene.didMove(to:)`, add the observer after the existing `.symbolOverlayDidChange` registration:
```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleShowInstructions(_:)),
    name: .showInstructions,
    object: nil
)
```

Add the handler to `GamePlayScene`:
```swift
@objc private func handleShowInstructions(_ notification: Notification) {
    let newPalette = (NSApp.delegate as? AppDelegate)?.activePalette ?? .spring
    let scene = InstructionsScene(size: size, sourceGame: model, sourcePalette: newPalette)
    scene.scaleMode = scaleMode
    view?.presentScene(scene, transition: .fade(withDuration: 0.2))
}
```

`willMove(from:)` already calls `removeObserver(self)` — no change needed.

- [ ] **Step 5.3 — Build and confirm no errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|BUILD"
```
Expected: `BUILD SUCCEEDED`

- [ ] **Step 5.4 — Run all tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "TEST SUCCEEDED|FAILED"
```
Expected: `TEST SUCCEEDED`

- [ ] **Step 5.5 — Commit**

```bash
git add Quadromania/TitleScene.swift Quadromania/GamePlayScene.swift
git commit -m "feat: TitleScene and GamePlayScene observe showInstructions notification"
```

---

### Task 6: Add colour cycle strip to GamePlayScene

**Files:**
- Modify: `Quadromania/GamePlayScene.swift`

- [ ] **Step 6.1 — Change `palette` from `let` to `var` and add `colorSwatchNodes`**

In `GamePlayScene`, in the `// MARK: - State` section (around lines 10–17):
```swift
// Change:
private let palette: TilePalette
// To:
private var palette: TilePalette

// Add after tileGrid declaration:
private var colorSwatchNodes: [SKSpriteNode] = []
```

- [ ] **Step 6.2 — Add `buildColorStrip()` method**

Add the following method to `GamePlayScene`, in the `// MARK: - UI` section, after `buildUI()`:

```swift
private func buildColorStrip(gridX: CGFloat) {
    let swatchSize: CGFloat = 28
    let elementSpacing: CGFloat = 46   // 28 swatch + 18 gap
    let count = model.maxColors + 1
    let centerX = gridX + TileGridNode.gridPixelWidth / 2
    let centerY = 50 + TileGridNode.gridPixelHeight + 10 + swatchSize / 2
    // Centre the swatch midpoints over the grid centre; the trailing ↩ arrow extends slightly right
    let startX  = centerX - CGFloat(count - 1) * elementSpacing / 2

    colorSwatchNodes = []
    for i in 0..<count {
        let x = startX + CGFloat(i) * elementSpacing

        let swatch = SKSpriteNode(color: palette.colors[i],
                                  size: CGSize(width: swatchSize, height: swatchSize))
        swatch.position = CGPoint(x: x, y: centerY)
        addChild(swatch)
        colorSwatchNodes.append(swatch)

        let arrowText = (i < count - 1) ? "→" : "↩"
        let arrow = SKLabelNode(text: arrowText)
        arrow.fontName  = "Helvetica"
        arrow.fontSize  = 16
        arrow.fontColor = SKColor(white: 0.55, alpha: 1)
        arrow.verticalAlignmentMode   = .center
        arrow.position = CGPoint(x: x + swatchSize / 2 + 9, y: centerY)
        addChild(arrow)
    }
}
```

- [ ] **Step 6.3 — Call `buildColorStrip` from `buildUI`**

In `buildUI()`, after the `addChild(tileGrid)` line (around line 63), add:
```swift
buildColorStrip(gridX: gridX)
```

- [ ] **Step 6.4 — Update `handlePaletteDidChange` to refresh swatches and store the new palette**

Replace the existing `handlePaletteDidChange` method:
```swift
@objc private func handlePaletteDidChange(_ notification: Notification) {
    guard let raw = notification.userInfo?["palette"] as? Int,
          let newPalette = TilePalette(rawValue: raw) else { return }
    palette = newPalette
    tileGrid.applyPalette(newPalette)
    for (i, swatch) in colorSwatchNodes.enumerated() {
        swatch.color = newPalette.colors[i]
    }
}
```

- [ ] **Step 6.5 — Build and confirm no errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | grep -E "error:|BUILD"
```
Expected: `BUILD SUCCEEDED`

- [ ] **Step 6.6 — Run all tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "TEST SUCCEEDED|FAILED"
```
Expected: `TEST SUCCEEDED`

- [ ] **Step 6.7 — Commit**

```bash
git add Quadromania/GamePlayScene.swift
git commit -m "feat: show colour cycle strip above game field"
```

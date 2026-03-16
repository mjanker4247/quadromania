# Title Screen Redesign Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove all UI from TitleScene and replace it with an animated colour-wave decorative grid; restructure the Game menu so all game settings live in AppDelegate; add in-place game restart to GamePlayScene.

**Architecture:** AppDelegate becomes the single source of truth for `selectedColors` and `selectedLevel` (both `UserDefaults`-backed). TitleScene is gutted to a title text + 16×8 animated `SKShapeNode` grid driven by a `wavePhase` in `update(_:)`. GamePlayScene gains a `startNewGame()` that resets in-place, replacing `returnToTitle()`.

**Tech Stack:** Swift, SpriteKit, AppKit (`NSMenu`, `NSAlert`), `UserDefaults`, `NotificationCenter`.

---

## File Structure

| File | Change |
|------|--------|
| `Quadromania/AppDelegate.swift` | Add `shared` IUO, `selectedColors`/`selectedLevel` computed props, three new notification names, restructure `buildGameMenu()` with Colors and Difficulty submenus, add `newGame(_:)` / `selectColors(_:)` / `selectDifficulty(_:)` actions |
| `Quadromania/TitleScene.swift` | Complete rewrite — remove all old UI; add decorative 16×8 wave grid + `update(_:)`; observe `.newGameRequested` |
| `Quadromania/GamePlayScene.swift` | Change `model` to `var`; add `startNewGame()`; add observers for `.newGameRequested` / `.colorsDidChange` / `.difficultyDidChange`; change win/loss click to call `startNewGame()`; update overlay hint text |

---

## Chunk 1: AppDelegate — settings ownership + Game menu restructure

### Task 1: AppDelegate — shared accessor, settings properties, notification names

**Files:**
- Modify: `Quadromania/AppDelegate.swift`

No unit-testable logic here — verification is a successful build.

- [ ] **Step 1: Add `static var shared`, `selectedColors`, `selectedLevel`, and `colorsMenuItems`/`difficultyMenuItems` lookups to AppDelegate**

At the top of the class body (after existing properties), add:

```swift
// MARK: - Shared accessor
/// Strong IUO reference set in applicationDidFinishLaunching.
/// AppDelegate is owned by the Cocoa app object and lives the full app lifetime.
static var shared: AppDelegate!

// MARK: - Game settings (UserDefaults-backed)

/// User-facing colour count (2–5). Stored in UserDefaults key "selectedColors".
/// Subtract 1 before passing to GameModel(level:maxColors:) to get the internal 1–4 range.
var selectedColors: Int {
    get {
        let saved = UserDefaults.standard.integer(forKey: "selectedColors")
        return saved >= 2 && saved <= 5 ? saved : 2
    }
    set { UserDefaults.standard.set(newValue, forKey: "selectedColors") }
}

/// Difficulty level (1 = Beginner, 5 = Intermediate, 10 = Expert).
/// Stored in UserDefaults key "selectedLevel".
var selectedLevel: Int {
    get {
        let saved = UserDefaults.standard.integer(forKey: "selectedLevel")
        return [1, 5, 10].contains(saved) ? saved : 5
    }
    set { UserDefaults.standard.set(newValue, forKey: "selectedLevel") }
}

/// Lookup table for Colors submenu checkmarks; keyed by user-facing colour count (2–5).
private var colorsMenuItems: [Int: NSMenuItem] = [:]
/// Lookup table for Difficulty submenu checkmarks; keyed by level value (1, 5, 10).
private var difficultyMenuItems: [Int: NSMenuItem] = [:]
```

- [ ] **Step 2: Set `AppDelegate.shared = self` in `applicationDidFinishLaunching`**

Add as the very first line inside the guard block in `applicationDidFinishLaunching`:

```swift
AppDelegate.shared = self
```

The guard checks `XCTestConfigurationFilePath`, so the full beginning of the method becomes:

```swift
func applicationDidFinishLaunching(_ aNotification: Notification) {
    guard ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] == nil else { return }
    AppDelegate.shared = self
    NSApp.activate(ignoringOtherApps: true)
    // ... rest unchanged
```

- [ ] **Step 3: Add three new notification names to the `extension Notification.Name` block**

```swift
/// Posted when the user selects "New Game" from the Game menu.
static let newGameRequested   = Notification.Name("newGameRequested")
/// Posted when the user changes the colour count from the Colors submenu.
static let colorsDidChange    = Notification.Name("colorsDidChange")
/// Posted when the user changes difficulty from the Difficulty submenu.
static let difficultyDidChange = Notification.Name("difficultyDidChange")
```

- [ ] **Step 4: Build to verify no compile errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

- [ ] **Step 5: Commit**

```bash
git add Quadromania/AppDelegate.swift
git commit -m "feat: add AppDelegate.shared, selectedColors/Level props, new notification names"
```

---

### Task 2: AppDelegate — restructure Game menu

**Files:**
- Modify: `Quadromania/AppDelegate.swift`

- [ ] **Step 1: Replace `buildGameMenu()` with the new implementation**

Replace the entire `buildGameMenu()` method (lines ~57–92) with:

```swift
/// Builds and appends the "Game" menu to the main menu bar.
/// Structure: New Game / — / Colors ▶ / Transition ▶ / — / Difficulty ▶ / — / Instructions
private func buildGameMenu() {
    let menu = NSMenu(title: "Game")

    // New Game (⌘N)
    let newGameItem = NSMenuItem(
        title: "New Game",
        action: #selector(newGame(_:)),
        keyEquivalent: "n"
    )
    newGameItem.target = self
    menu.addItem(newGameItem)

    menu.addItem(.separator())

    // Colors submenu — radio group 2/3/4/5 Colors
    let colorsMenu = NSMenu(title: "Colors")
    for count in 2...5 {
        let item = NSMenuItem(
            title: "\(count) Colors",
            action: #selector(selectColors(_:)),
            keyEquivalent: ""
        )
        item.tag    = count
        item.state  = (count == selectedColors) ? .on : .off
        item.target = self
        colorsMenu.addItem(item)
        colorsMenuItems[count] = item
    }
    let colorsItem = NSMenuItem(title: "Colors", action: nil, keyEquivalent: "")
    colorsItem.submenu = colorsMenu
    menu.addItem(colorsItem)

    // Transition submenu — existing, unchanged
    let transitionMenu = NSMenu(title: "Transition")
    for style in TransitionStyle.allCases {
        let item = NSMenuItem(
            title: style.displayName,
            action: #selector(selectTransitionStyle(_:)),
            keyEquivalent: ""
        )
        item.tag    = style.rawValue
        item.state  = .off
        item.target = self
        transitionMenu.addItem(item)
        transitionMenuItems[style] = item
    }
    let transitionItem = NSMenuItem(title: "Transition", action: nil, keyEquivalent: "")
    transitionItem.submenu = transitionMenu
    menu.addItem(transitionItem)

    menu.addItem(.separator())

    // Difficulty submenu — radio group Beginner / Intermediate / Expert
    let difficultyMenu = NSMenu(title: "Difficulty")
    let difficultyOptions: [(String, Int)] = [
        ("Beginner",     1),
        ("Intermediate", 5),
        ("Expert",      10)
    ]
    for (name, level) in difficultyOptions {
        let item = NSMenuItem(
            title: name,
            action: #selector(selectDifficulty(_:)),
            keyEquivalent: ""
        )
        item.tag    = level
        item.state  = (level == selectedLevel) ? .on : .off
        item.target = self
        difficultyMenu.addItem(item)
        difficultyMenuItems[level] = item
    }
    let difficultyItem = NSMenuItem(title: "Difficulty", action: nil, keyEquivalent: "")
    difficultyItem.submenu = difficultyMenu
    menu.addItem(difficultyItem)

    menu.addItem(.separator())

    // Instructions
    let instrItem = NSMenuItem(
        title: "Instructions",
        action: #selector(showInstructionsMenuAction(_:)),
        keyEquivalent: ""
    )
    instrItem.target = self
    menu.addItem(instrItem)

    let menuItem = NSMenuItem(title: "Game", action: nil, keyEquivalent: "")
    menuItem.submenu = menu
    NSApp.mainMenu?.addItem(menuItem)
}
```

Also remove the `transitionStyle` restore block from `applicationDidFinishLaunching` — the Transition submenu checkmark restore still belongs there, but the style is now set by `buildGameMenu` correctly via the `state` lines. Keep the existing restore block unchanged (it reads from UserDefaults and sets checkmarks).

- [ ] **Step 2: Add the three new action methods**

Add after `selectTransitionStyle(_:)`:

```swift
/// Posts `.newGameRequested` — handled by TitleScene (transitions) or GamePlayScene (alert).
@objc private func newGame(_ sender: NSMenuItem) {
    NotificationCenter.default.post(name: .newGameRequested, object: nil)
}

/// Persists the chosen colour count and posts `.colorsDidChange`.
@objc private func selectColors(_ sender: NSMenuItem) {
    let count = sender.tag  // 2–5
    selectedColors = count
    colorsMenuItems.values.forEach { $0.state = .off }
    sender.state = .on
    NotificationCenter.default.post(name: .colorsDidChange, object: nil)
}

/// Persists the chosen difficulty level and posts `.difficultyDidChange`.
@objc private func selectDifficulty(_ sender: NSMenuItem) {
    let level = sender.tag  // 1, 5, or 10
    selectedLevel = level
    difficultyMenuItems.values.forEach { $0.state = .off }
    sender.state = .on
    NotificationCenter.default.post(name: .difficultyDidChange, object: nil)
}
```

- [ ] **Step 3: Build to verify no compile errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

- [ ] **Step 4: Commit**

```bash
git add Quadromania/AppDelegate.swift
git commit -m "feat: restructure Game menu with New Game, Colors, Difficulty submenus"
```

---

## Chunk 2: TitleScene — animated decorative title screen

### Task 3: TitleScene — complete rewrite

**Files:**
- Modify: `Quadromania/TitleScene.swift`

The entire file is replaced. The old `MenuItem` enum, all label/dot/palette-swatch nodes, hover logic, and mouse hit-testing are removed. The new scene has only a title text pair, a 16×8 decorative `SKShapeNode` grid, and a `wavePhase` wave animation.

- [ ] **Step 1: Replace `TitleScene.swift` with the new implementation**

```swift
// TitleScene.swift
// Animated title screen — displays "QUADROMANIA" above a decorative colour-wave grid.
// All game controls live in the Game menu (AppDelegate). This scene is purely visual.

import SpriteKit
import QuadroCore
import Cocoa

class TitleScene: SKScene {

    // MARK: - Decorative grid dimensions

    /// Number of tile columns in the decorative background grid.
    private let gridCols = 16
    /// Number of tile rows in the decorative background grid.
    private let gridRows = 8
    /// Center-to-center distance between adjacent tiles (px).
    private let cellPitch: CGFloat = 50
    /// Radius of each frosted gem circle (px).
    private let tileRadius: CGFloat = 22

    // MARK: - Grid state

    /// All SKShapeNode tiles indexed as [row][col].
    private var gridTiles: [[SKShapeNode]] = []
    /// Current displayed colour index per tile; -1 means unset (force redraw on first tick).
    private var colorIndices: [[Int]] = []

    // MARK: - Wave animation state

    /// Monotonically increasing phase counter (units/sec × time = phase units).
    /// `Int(wavePhase) - (col + row)` gives each tile's target palette index.
    private var wavePhase: Double = 0
    /// Timestamp of the previous `update` call; 0 before the first frame.
    private var lastUpdateTime: TimeInterval = 0

    // MARK: - Palette

    /// Active colour palette — kept in sync with AppDelegate and the Palette menu.
    private var currentPalette: TilePalette = .spring

    // MARK: - Scene lifecycle

    override func didMove(to view: SKView) {
        backgroundColor = SKColor(red: 0.10, green: 0.08, blue: 0.15, alpha: 1)
        buildTitleText()
        buildDecorativeGrid()
        registerObservers()
        // Sync with the palette that is already active in AppDelegate
        currentPalette = AppDelegate.shared.activePalette
    }

    // MARK: - Build UI

    /// Adds the "QUADROMANIA" heading and "Puzzle Game" subtitle near the top of the scene.
    private func buildTitleText() {
        let title = SKLabelNode(text: "QUADROMANIA")
        title.fontName  = "Helvetica-Bold"
        title.fontSize  = 72
        title.fontColor = SKColor(red: 0.95, green: 0.85, blue: 0.30, alpha: 1)
        title.horizontalAlignmentMode = .center
        title.position  = CGPoint(x: size.width / 2, y: 700)
        addChild(title)

        let sub = SKLabelNode(text: "Puzzle Game")
        sub.fontName  = "Helvetica"
        sub.fontSize  = 22
        sub.fontColor = SKColor(white: 0.55, alpha: 1)
        sub.horizontalAlignmentMode = .center
        sub.position  = CGPoint(x: size.width / 2, y: 650)
        addChild(sub)
    }

    /// Creates a 16×8 grid of frosted gem circles centred at (640, 300).
    /// Tiles start at colour 0; the wave animation fills them in on the first update tick.
    private func buildDecorativeGrid() {
        // Total span from first to last tile center
        let gridWidth  = CGFloat(gridCols - 1) * cellPitch  // 750 px
        let gridHeight = CGFloat(gridRows - 1) * cellPitch  // 350 px
        // Bottom-left origin so the grid is centred at (640, 300)
        let originX = size.width  / 2 - gridWidth  / 2  // = 265
        let originY = 300         - gridHeight / 2       // = 125

        // Initialise tracking arrays (-1 = unset, forces colour assignment on first tick)
        colorIndices = Array(repeating: Array(repeating: -1, count: gridCols), count: gridRows)
        gridTiles    = []

        for row in 0..<gridRows {
            var rowTiles: [SKShapeNode] = []
            for col in 0..<gridCols {
                let tile = SKShapeNode(circleOfRadius: tileRadius)
                tile.strokeColor = .clear
                tile.fillColor   = currentPalette.colors[0]
                tile.position    = CGPoint(
                    x: originX + CGFloat(col) * cellPitch,
                    y: originY + CGFloat(row) * cellPitch
                )

                // Frosted highlight: small white ellipse in the upper-left of the circle,
                // matching the style used in TileGridNode.
                let hl = SKShapeNode(ellipseOf: CGSize(
                    width:  tileRadius * 0.84,
                    height: tileRadius * 0.70
                ))
                hl.fillColor   = SKColor(white: 1, alpha: 0.42)
                hl.strokeColor = .clear
                hl.position    = CGPoint(x: -tileRadius * 0.2, y: tileRadius * 0.25)
                tile.addChild(hl)

                addChild(tile)
                rowTiles.append(tile)
            }
            gridTiles.append(rowTiles)
        }
    }

    // MARK: - Wave animation

    /// Advances the wave and recolours any tile whose target index has changed.
    override func update(_ currentTime: TimeInterval) {
        // Compute delta; clamp to 0.1 s to avoid a large jump on the very first frame.
        let delta = lastUpdateTime == 0 ? 0.0 : min(currentTime - lastUpdateTime, 0.1)
        lastUpdateTime = currentTime
        wavePhase += delta * 0.8  // 0.8 phase units per second

        let n = currentPalette.colors.count  // always 5

        for row in 0..<gridRows {
            for col in 0..<gridCols {
                // Diagonal wave: (col + row) shifts each tile's phase along the diagonal.
                // Double-modulo ensures the result is always non-negative even when
                // Int(wavePhase) < (col + row).
                let raw         = Int(wavePhase) - (col + row)
                let targetIndex = ((raw % n) + n) % n

                guard targetIndex != colorIndices[row][col] else { continue }
                colorIndices[row][col] = targetIndex

                let tile = gridTiles[row][col]
                let from = tile.fillColor
                let to   = currentPalette.colors[targetIndex]
                // Cancel any in-flight transition before starting a new one
                tile.removeAction(forKey: "colorTransition")
                tile.run(colorTransition(from: from, to: to), withKey: "colorTransition")
            }
        }
    }

    // MARK: - Colour transition helpers

    /// Returns an SKAction that smoothly blends a tile's fillColor from `from` to `to`.
    private func colorTransition(from: SKColor, to: SKColor) -> SKAction {
        let duration: TimeInterval = 0.35
        return SKAction.customAction(withDuration: duration) { [weak self] node, elapsed in
            guard let tile = node as? SKShapeNode, let self else { return }
            let t = CGFloat(elapsed) / CGFloat(duration)
            tile.fillColor = self.lerpColor(from: from, to: to, t: t)
        }
    }

    /// Linearly interpolates between two SKColors component-wise.
    private func lerpColor(from: SKColor, to: SKColor, t: CGFloat) -> SKColor {
        var r1: CGFloat = 0, g1: CGFloat = 0, b1: CGFloat = 0, a1: CGFloat = 0
        var r2: CGFloat = 0, g2: CGFloat = 0, b2: CGFloat = 0, a2: CGFloat = 0
        from.getRed(&r1, green: &g1, blue: &b1, alpha: &a1)
        to.getRed  (&r2, green: &g2, blue: &b2, alpha: &a2)
        return SKColor(
            red:   r1 + (r2 - r1) * t,
            green: g1 + (g2 - g1) * t,
            blue:  b1 + (b2 - b1) * t,
            alpha: a1 + (a2 - a1) * t
        )
    }

    // MARK: - Notification observers

    private func registerObservers() {
        let nc = NotificationCenter.default
        // New game requested from menu bar — transition to GamePlayScene
        nc.addObserver(self, selector: #selector(handleNewGameRequested(_:)),
                       name: .newGameRequested, object: nil)
        // Palette changed — refresh grid on next update tick
        nc.addObserver(self, selector: #selector(handlePaletteDidChange(_:)),
                       name: .paletteDidChange, object: nil)
        // Custom palette colours edited — refresh grid if .custom is active
        nc.addObserver(self, selector: #selector(handleCustomPaletteDidChange(_:)),
                       name: .customPaletteDidChange, object: nil)
        // Instructions menu item — navigate to InstructionsScene
        nc.addObserver(self, selector: #selector(handleShowInstructions(_:)),
                       name: .showInstructions, object: nil)
    }

    /// Constructs a GameModel from AppDelegate settings and transitions to GamePlayScene.
    @objc private func handleNewGameRequested(_ notification: Notification) {
        let model = GameModel(
            level:     AppDelegate.shared.selectedLevel,
            maxColors: AppDelegate.shared.selectedColors - 1
        )
        let scene = GamePlayScene(model: model, palette: currentPalette, size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.3))
    }

    /// Applies a new palette and resets `colorIndices` so all tiles re-animate on the next tick.
    @objc private func handlePaletteDidChange(_ notification: Notification) {
        guard let raw = notification.userInfo?["palette"] as? Int,
              let palette = TilePalette(rawValue: raw) else { return }
        currentPalette = palette
        // -1 forces every tile to recompute its colour on the next update tick
        colorIndices = Array(repeating: Array(repeating: -1, count: gridCols), count: gridRows)
    }

    /// Resets `colorIndices` when the custom palette colours are edited while `.custom` is active.
    @objc private func handleCustomPaletteDidChange(_ notification: Notification) {
        guard currentPalette == .custom else { return }
        colorIndices = Array(repeating: Array(repeating: -1, count: gridCols), count: gridRows)
    }

    @objc private func handleShowInstructions(_ notification: Notification) {
        let scene = InstructionsScene(size: size)
        scene.scaleMode = scaleMode
        view?.presentScene(scene, transition: .fade(withDuration: 0.2))
    }

    override func willMove(from view: SKView) {
        NotificationCenter.default.removeObserver(self)
    }
}
```

- [ ] **Step 2: Build to verify no compile errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

- [ ] **Step 3: Run unit tests to verify no regressions**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "Test Suite|passed|failed"
```

Expected: 19 tests pass, 0 failures.

- [ ] **Step 4: Commit**

```bash
git add Quadromania/TitleScene.swift
git commit -m "feat: replace TitleScene with animated colour-wave decorative grid"
```

---

## Chunk 3: GamePlayScene — startNewGame + mid-game notifications

### Task 4: GamePlayScene — in-place restart and notification observers

**Files:**
- Modify: `Quadromania/GamePlayScene.swift`

- [ ] **Step 1: Change `model` from `let` to `var` and `palette` to `var`**

On line 12–14, change:

```swift
private let model: GameModel
/// The active colour palette; updated in-place when the user changes it via the Palette menu.
private var palette: TilePalette
```

to:

```swift
/// The active game model; replaced wholesale on new-game reset.
private var model: GameModel
/// The active colour palette; updated in-place when the user changes it via the Palette menu.
private var palette: TilePalette
```

(`palette` is already `var` — just verify. `model` must become `var`.)

- [ ] **Step 2: Add `startNewGame()` and `showNewGameAlert()` methods**

Add in the `// MARK: - Navigation` section, replacing `returnToTitle()`:

```swift
// MARK: - Navigation

/// Resets the game in-place with fresh settings from AppDelegate.
/// Called when the player clicks after win/loss, or confirms an NSAlert.
private func startNewGame() {
    // Rebuild model from current AppDelegate settings
    model = GameModel(
        level:     AppDelegate.shared.selectedLevel,
        maxColors: AppDelegate.shared.selectedColors - 1
    )
    // Sync palette in case it changed while the old game was running
    palette = AppDelegate.shared.activePalette
    // Vary background colour per new game for visual distinction
    backgroundColor = SKColor(
        red:   CGFloat(model.backgroundArtIndex) * 0.05 + 0.05,
        green: 0.08,
        blue:  CGFloat(model.backgroundArtIndex) * 0.03 + 0.06,
        alpha: 1
    )
    waitingForClick  = false
    colorSwatchNodes = []
    removeAllChildren()
    buildUI()
    // Re-apply persisted overlay and transition settings
    if let appDelegate = NSApp.delegate as? AppDelegate {
        tileGrid.symbolOverlayEnabled = appDelegate.symbolOverlayEnabled
        tileGrid.transitionStyle      = appDelegate.transitionStyle
    }
}

/// Presents a non-blocking sheet asking whether to start a new game or continue.
/// Calls `startNewGame()` only if the user confirms.
private func showNewGameAlert(informative: String) {
    let alert = NSAlert()
    alert.messageText     = "Start a new game?"
    alert.informativeText = informative
    alert.addButton(withTitle: "New Game")
    alert.addButton(withTitle: "Continue")
    guard let window = view?.window else { return }
    alert.beginSheetModal(for: window) { [weak self] response in
        if response == .alertFirstButtonReturn {
            self?.startNewGame()
        }
    }
}
```

- [ ] **Step 3: Update `mouseDown` to call `startNewGame()` instead of `returnToTitle()`**

Change lines 174–178 from:

```swift
if waitingForClick {
    returnToTitle()
    return
}
```

to:

```swift
if waitingForClick {
    startNewGame()
    return
}
```

- [ ] **Step 4: Update the overlay hint text from "Click to continue" to "Click to play again"**

In `showOverlay(message:subtext:color:)`, change:

```swift
let hint = SKLabelNode(text: "Click to continue")
```

to:

```swift
let hint = SKLabelNode(text: "Click to play again")
```

- [ ] **Step 5: Register observers for the three new notification names**

In `didMove(to:)`, add after the existing `addObserver` calls:

```swift
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleNewGameRequested(_:)),
    name: .newGameRequested,
    object: nil
)
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleColorsDidChange(_:)),
    name: .colorsDidChange,
    object: nil
)
NotificationCenter.default.addObserver(
    self,
    selector: #selector(handleDifficultyDidChange(_:)),
    name: .difficultyDidChange,
    object: nil
)
```

- [ ] **Step 6: Add the three new notification handler methods**

Add in `// MARK: - Notification handlers`:

```swift
/// Starts a new game immediately if the old one is over; shows an alert if a game is in progress.
@objc private func handleNewGameRequested(_ notification: Notification) {
    if waitingForClick {
        startNewGame()
    } else {
        showNewGameAlert(informative: "Your current game will be lost.")
    }
}

/// Responds to a colour-count change: silent restart if game is over, alert if game is active.
@objc private func handleColorsDidChange(_ notification: Notification) {
    if waitingForClick {
        startNewGame()
    } else {
        showNewGameAlert(informative: "You changed the number of colors.")
    }
}

/// Responds to a difficulty change: silent restart if game is over, alert if game is active.
@objc private func handleDifficultyDidChange(_ notification: Notification) {
    if waitingForClick {
        startNewGame()
    } else {
        showNewGameAlert(informative: "You changed the difficulty.")
    }
}
```

- [ ] **Step 7: Build to verify no compile errors**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

- [ ] **Step 8: Run all unit tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "Test Suite|passed|failed"
```

Expected: 19 tests pass, 0 failures.

- [ ] **Step 9: Commit**

```bash
git add Quadromania/GamePlayScene.swift
git commit -m "feat: add startNewGame(), mid-game alert observers, click-to-restart on win/loss"
```

---

## Final verification

- [ ] **Build the full project one last time**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```

Expected: `** BUILD SUCCEEDED **`

- [ ] **Run all tests**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme QuadroTests -configuration Debug test 2>&1 | grep -E "Test Suite|passed|failed"
```

Expected: 19 tests pass, 0 failures.

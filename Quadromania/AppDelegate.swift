//
//  AppDelegate.swift
//  Quadromania
//
//  Created by Marco Janker on 12.03.26.
//

import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    // MARK: - Palette & accessibility state

    /// The currently active colour palette, kept in sync with the Palette menu checkmark.
    var activePalette: TilePalette = .spring
    /// Whether shape symbols are drawn on top of tiles for colour-blind accessibility.
    var symbolOverlayEnabled: Bool = false

    /// Lookup table so `selectPaletteItem` can toggle checkmarks without iterating the whole menu.
    private var paletteMenuItems: [TilePalette: NSMenuItem] = [:]
    private var symbolMenuItem: NSMenuItem?
    /// Retained so the panel can be reused without recreating it on every open.
    private var customPalettePanel: CustomPalettePanel?

    /// The currently active tile-rotation animation style.
    var transitionStyle: TransitionStyle = .ringSweep
    /// Lookup table used to toggle checkmarks when the transition style changes.
    private var transitionMenuItems: [TransitionStyle: NSMenuItem] = [:]

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

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Skip app initialization when running under XCTest.
        guard ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] == nil else { return }
        AppDelegate.shared = self
        NSApp.activate(ignoringOtherApps: true)
        SoundManager.shared.startMusic()
        updateMusicMenuItem()
        buildGameMenu()
        // Restore the previously chosen transition style from UserDefaults
        let savedStyle = UserDefaults.standard.integer(forKey: "transitionStyle")
        if let style = TransitionStyle(rawValue: savedStyle) {
            transitionStyle = style
            // Clear all checkmarks before re-marking the restored style
            transitionMenuItems.values.forEach { $0.state = .off }
            transitionMenuItems[style]?.state = .on
        }
        buildPaletteMenu()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

    // MARK: - Game menu

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

    /// Handles a tap on one of the Transition submenu items.
    /// Persists the new style in UserDefaults and posts `.transitionStyleDidChange`
    /// so live scenes can update their `TileGridNode` without a restart.
    @objc private func selectTransitionStyle(_ sender: NSMenuItem) {
        guard let style = TransitionStyle(rawValue: sender.tag) else { return }
        transitionStyle = style
        // Persist choice so it survives app restarts
        UserDefaults.standard.set(style.rawValue, forKey: "transitionStyle")
        // Update checkmarks: clear all, then re-mark the selected item
        transitionMenuItems.values.forEach { $0.state = .off }
        sender.state = .on
        NotificationCenter.default.post(name: .transitionStyleDidChange,
                                        object: nil,
                                        userInfo: ["style": style.rawValue])
    }

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

    /// Posts `.showInstructions` so whichever scene is active can navigate to InstructionsScene.
    @objc private func showInstructionsMenuAction(_ sender: NSMenuItem) {
        NotificationCenter.default.post(name: .showInstructions, object: nil)
    }

    // MARK: - Palette menu

    /// Builds and appends the "Palette" menu to the main menu bar.
    /// Includes one item per built-in palette, a custom palette entry, an edit action,
    /// and the "Color Symbols" accessibility toggle.
    private func buildPaletteMenu() {
        let menu = NSMenu(title: "Palette")

        // Built-in palettes — .custom is added separately with a preceding separator
        for palette in TilePalette.allCases where palette != .custom {
            let item = NSMenuItem(
                title: palette.displayName,
                action: #selector(selectPaletteItem(_:)),
                keyEquivalent: ""
            )
            item.tag = palette.rawValue
            // Reflect the current active palette at menu-build time
            item.state = (palette == activePalette) ? .on : .off
            item.target = self
            menu.addItem(item)
            paletteMenuItems[palette] = item
        }

        menu.addItem(.separator())
        let customItem = NSMenuItem(
            title: TilePalette.custom.displayName,
            action: #selector(selectPaletteItem(_:)),
            keyEquivalent: ""
        )
        customItem.tag    = TilePalette.custom.rawValue  // 4
        customItem.state  = (activePalette == .custom) ? .on : .off
        customItem.target = self
        // Must be in paletteMenuItems so toggle-off works when switching away from .custom
        paletteMenuItems[.custom] = customItem
        menu.addItem(customItem)

        menu.addItem(.separator())
        let editItem = NSMenuItem(
            title: "Edit Custom Colors…",
            action: #selector(openCustomPaletteEditor(_:)),
            keyEquivalent: ""
        )
        editItem.target = self
        menu.addItem(editItem)

        menu.addItem(.separator())

        let symItem = NSMenuItem(
            title: "Color Symbols",
            action: #selector(toggleSymbolOverlay(_:)),
            keyEquivalent: ""
        )
        symItem.state = symbolOverlayEnabled ? .on : .off
        symItem.target = self
        menu.addItem(symItem)
        symbolMenuItem = symItem

        let paletteMenuItem = NSMenuItem(title: "Palette", action: nil, keyEquivalent: "")
        paletteMenuItem.submenu = menu
        NSApp.mainMenu?.addItem(paletteMenuItem)
    }

    /// Handles a tap on a palette menu item.
    /// Updates `activePalette`, refreshes checkmarks, and posts `.paletteDidChange`
    /// so live scenes can swap colours without restarting.
    @objc private func selectPaletteItem(_ sender: NSMenuItem) {
        guard let palette = TilePalette(rawValue: sender.tag) else { return }
        activePalette = palette
        // Toggle checkmarks: clear all, then mark the selected item
        paletteMenuItems.values.forEach { $0.state = .off }
        sender.state = .on
        NotificationCenter.default.post(
            name: .paletteDidChange,
            object: nil,
            userInfo: ["palette": palette.rawValue]
        )
    }

    /// Opens (or re-focuses) the floating custom palette editor panel.
    /// The panel is lazily created on first use and reused on subsequent opens.
    @objc private func openCustomPaletteEditor(_ sender: NSMenuItem) {
        if customPalettePanel == nil {
            customPalettePanel = CustomPalettePanel()
        }
        // Reload wells so a reused panel always shows the current saved colours
        customPalettePanel?.reloadColorWells()
        customPalettePanel?.center()
        customPalettePanel?.makeKeyAndOrderFront(nil)
    }

    /// Toggles the shape-symbol accessibility overlay on all tiles and posts `.symbolOverlayDidChange`.
    @objc private func toggleSymbolOverlay(_ sender: NSMenuItem) {
        symbolOverlayEnabled.toggle()
        sender.state = symbolOverlayEnabled ? .on : .off
        NotificationCenter.default.post(
            name: .symbolOverlayDidChange,
            object: nil,
            userInfo: ["enabled": symbolOverlayEnabled]
        )
    }

    // MARK: - Sound menu

    @objc func toggleMusic(_ sender: NSMenuItem) {
        let sm = SoundManager.shared
        if sm.isMusicPlaying {
            sm.stopMusic()
        } else {
            sm.startMusic()
        }
        updateMusicMenuItem()
    }

    /// Flips the Sound menu item title between "Start Music" and "Stop Music" to reflect playback state.
    private func updateMusicMenuItem() {
        guard let item = musicMenuItem else { return }
        item.title = SoundManager.shared.isMusicPlaying ? "Stop Music" : "Start Music"
    }

    /// Locates the music toggle item regardless of whether it currently reads "Start" or "Stop".
    private var musicMenuItem: NSMenuItem? {
        NSApp.mainMenu?
            .item(withTitle: "Sound")?
            .submenu?
            .item(withTitle: "Stop Music") ??
        NSApp.mainMenu?
            .item(withTitle: "Sound")?
            .submenu?
            .item(withTitle: "Start Music")
    }
}

// MARK: - Notification names

extension Notification.Name {
    /// Posted when the user selects a new colour palette from the Palette menu.
    static let paletteDidChange          = Notification.Name("paletteDidChange")
    /// Posted when the "Color Symbols" accessibility toggle is flipped.
    static let symbolOverlayDidChange    = Notification.Name("symbolOverlayDidChange")
    /// Posted when the user taps "Instructions" in the Game menu.
    static let showInstructions          = Notification.Name("showInstructions")
    /// Posted when the user selects a new tile-rotation animation style.
    static let transitionStyleDidChange  = Notification.Name("transitionStyleDidChange")
    /// Posted when the user confirms edits in the Custom Palette editor panel.
    static let customPaletteDidChange    = Notification.Name("customPaletteDidChange")
    /// Posted when the user selects "New Game" from the Game menu.
    static let newGameRequested          = Notification.Name("newGameRequested")
    /// Posted when the user changes the colour count from the Colors submenu.
    static let colorsDidChange           = Notification.Name("colorsDidChange")
    /// Posted when the user changes difficulty from the Difficulty submenu.
    static let difficultyDidChange       = Notification.Name("difficultyDidChange")
}

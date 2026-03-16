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

    /// Retained so the panel can be reused without recreating it on every open.
    private var customPalettePanel: CustomPalettePanel?

    /// The currently active tile-rotation animation style.
    var transitionStyle: TransitionStyle = .ringSweep

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

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Skip app initialization when running under XCTest.
        guard ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] == nil else { return }
        AppDelegate.shared = self
        NSApp.activate()
        SoundManager.shared.startMusic()
        // Restore the previously chosen transition style from UserDefaults
        let savedStyle = UserDefaults.standard.integer(forKey: "transitionStyle")
        if let style = TransitionStyle(rawValue: savedStyle) {
            transitionStyle = style
        }
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

    /// Ensures the game window is always restorable from the Dock.
    /// Without this, clicking the Dock icon while the window is miniaturised
    /// activates the app but leaves the window hidden in the Dock.
    func applicationShouldHandleReopen(_ sender: NSApplication,
                                       hasVisibleWindows: Bool) -> Bool {
        if !hasVisibleWindows {
            sender.windows.forEach { $0.makeKeyAndOrderFront(self) }
        }
        return true
    }

    // MARK: - Menu validation

    /// Drives all dynamic menu state: checkmarks for radio groups and the music toggle title.
    /// Called by AppKit before displaying any menu whose items target AppDelegate.
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(selectColors(_:)):
            menuItem.state = menuItem.tag == selectedColors ? .on : .off
        case #selector(selectDifficulty(_:)):
            menuItem.state = menuItem.tag == selectedLevel ? .on : .off
        case #selector(selectTransitionStyle(_:)):
            menuItem.state = menuItem.tag == transitionStyle.rawValue ? .on : .off
        case #selector(selectPaletteItem(_:)):
            if let palette = TilePalette(rawValue: menuItem.tag) {
                menuItem.state = palette == activePalette ? .on : .off
            }
        case #selector(toggleSymbolOverlay(_:)):
            menuItem.state = symbolOverlayEnabled ? .on : .off
        case #selector(toggleMusic(_:)):
            menuItem.title = SoundManager.shared.isMusicPlaying ? "Stop Music" : "Start Music"
        default:
            break
        }
        return true
    }

    // MARK: - Game menu actions

    /// Posts `.newGameRequested` — handled by TitleScene (transitions) or GamePlayScene (alert).
    @objc private func newGame(_ sender: NSMenuItem) {
        NotificationCenter.default.post(name: .newGameRequested, object: nil)
    }

    /// Persists the chosen colour count and posts `.colorsDidChange`.
    @objc private func selectColors(_ sender: NSMenuItem) {
        selectedColors = sender.tag  // 2–5
        NotificationCenter.default.post(name: .colorsDidChange, object: nil)
    }

    /// Handles a tap on one of the Transition submenu items.
    /// Persists the new style in UserDefaults and posts `.transitionStyleDidChange`
    /// so live scenes can update their `TileGridNode` without a restart.
    @objc private func selectTransitionStyle(_ sender: NSMenuItem) {
        guard let style = TransitionStyle(rawValue: sender.tag) else { return }
        transitionStyle = style
        UserDefaults.standard.set(style.rawValue, forKey: "transitionStyle")
        NotificationCenter.default.post(name: .transitionStyleDidChange,
                                        object: nil,
                                        userInfo: ["style": style.rawValue])
    }

    /// Persists the chosen difficulty level and posts `.difficultyDidChange`.
    @objc private func selectDifficulty(_ sender: NSMenuItem) {
        selectedLevel = sender.tag  // 1, 5, or 10
        NotificationCenter.default.post(name: .difficultyDidChange, object: nil)
    }

    /// Posts `.showInstructions` so whichever scene is active can navigate to InstructionsScene.
    @objc private func showInstructionsMenuAction(_ sender: NSMenuItem) {
        NotificationCenter.default.post(name: .showInstructions, object: nil)
    }

    // MARK: - Palette menu actions

    /// Handles a tap on a palette menu item.
    /// Updates `activePalette` and posts `.paletteDidChange` so live scenes can swap colours.
    @objc private func selectPaletteItem(_ sender: NSMenuItem) {
        guard let palette = TilePalette(rawValue: sender.tag) else { return }
        activePalette = palette
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
        NotificationCenter.default.post(
            name: .symbolOverlayDidChange,
            object: nil,
            userInfo: ["enabled": symbolOverlayEnabled]
        )
    }

    // MARK: - Sound menu actions

    @objc func toggleMusic(_ sender: NSMenuItem) {
        let sm = SoundManager.shared
        if sm.isMusicPlaying {
            sm.stopMusic()
        } else {
            sm.startMusic()
        }
        sender.title = sm.isMusicPlaying ? "Stop Music" : "Start Music"
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

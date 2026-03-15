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

    var activePalette: TilePalette = .spring
    var symbolOverlayEnabled: Bool = false

    private var paletteMenuItems: [TilePalette: NSMenuItem] = [:]
    private var symbolMenuItem: NSMenuItem?

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Skip app initialization when running under XCTest.
        guard ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] == nil else { return }
        NSApp.activate(ignoringOtherApps: true)
        SoundManager.shared.startMusic()
        updateMusicMenuItem()
        buildGameMenu()
        buildPaletteMenu()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

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

    // MARK: - Palette menu

    private func buildPaletteMenu() {
        let menu = NSMenu(title: "Palette")

        for palette in TilePalette.allCases {
            let item = NSMenuItem(
                title: palette.displayName,
                action: #selector(selectPaletteItem(_:)),
                keyEquivalent: ""
            )
            item.tag = palette.rawValue
            item.state = (palette == activePalette) ? .on : .off
            item.target = self
            menu.addItem(item)
            paletteMenuItems[palette] = item
        }

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

    @objc private func selectPaletteItem(_ sender: NSMenuItem) {
        guard let palette = TilePalette(rawValue: sender.tag) else { return }
        activePalette = palette
        paletteMenuItems.values.forEach { $0.state = .off }
        sender.state = .on
        NotificationCenter.default.post(
            name: .paletteDidChange,
            object: nil,
            userInfo: ["palette": palette.rawValue]
        )
    }

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

    private func updateMusicMenuItem() {
        guard let item = musicMenuItem else { return }
        item.title = SoundManager.shared.isMusicPlaying ? "Stop Music" : "Start Music"
    }

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

extension Notification.Name {
    static let paletteDidChange       = Notification.Name("paletteDidChange")
    static let symbolOverlayDidChange = Notification.Name("symbolOverlayDidChange")
    static let showInstructions       = Notification.Name("showInstructions")
}

//
//  AppDelegate.swift
//  Quadromania
//
//  Created by Marco Janker on 12.03.26.
//


import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    private var toggleMusicItem: NSMenuItem!

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        NSApp.activate(ignoringOtherApps: true)

        // Kick off background music
        SoundManager.shared.startMusic()

        // Insert a "Sound" menu before the Window menu
        buildSoundMenu()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

    // MARK: - Sound menu

    private func buildSoundMenu() {
        guard let mainMenu = NSApp.mainMenu else { return }

        let soundMenu = NSMenu(title: "Sound")

        // Toggle music
        toggleMusicItem = NSMenuItem(
            title: "Stop Music",
            action: #selector(toggleMusic(_:)),
            keyEquivalent: "m"
        )
        toggleMusicItem.keyEquivalentModifierMask = [.command, .shift]
        toggleMusicItem.target = self
        soundMenu.addItem(toggleMusicItem)

        let soundMenuItem = NSMenuItem()
        soundMenuItem.submenu = soundMenu

        // Insert before the Window menu (second to last)
        let insertIndex = max(mainMenu.items.count - 2, 0)
        mainMenu.insertItem(soundMenuItem, at: insertIndex)
    }

    // MARK: - Actions

    @objc private func toggleMusic(_ sender: NSMenuItem) {
        let sm = SoundManager.shared
        if sm.isMusicPlaying {
            sm.stopMusic()
            toggleMusicItem.title = "Start Music"
        } else {
            sm.startMusic()
            toggleMusicItem.title = "Stop Music"
        }
    }
}

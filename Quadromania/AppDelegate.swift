//
//  AppDelegate.swift
//  Quadromania
//
//  Created by Marco Janker on 12.03.26.
//

import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        NSApp.activate(ignoringOtherApps: true)
        SoundManager.shared.startMusic()
        updateMusicMenuItem()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
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

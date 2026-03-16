// CustomPalettePanel.swift
// Floating NSPanel with 5 NSColorWell editors for the custom palette.

import Cocoa
import SpriteKit

/// A floating utility panel that allows editing the 5 custom palette colours.
/// Present by calling `center()` then `makeKeyAndOrderFront(nil)` from AppDelegate.
class CustomPalettePanel: NSPanel, NSWindowDelegate {
    /// The five colour wells, one per palette slot (index 0–4).
    private var colorWells: [NSColorWell] = []

    convenience init() {
        self.init(contentRect: CGRect(x: 0, y: 0, width: 360, height: 160),
                  styleMask: [.titled, .closable, .utilityWindow],
                  backing: .buffered,
                  defer: false)
        title = "Edit Custom Colors"
        // Prevent deallocation on close so the panel can be reused without re-creating it
        isReleasedWhenClosed = false
        delegate = self
        buildLayout()
    }

    private func buildLayout() {
        guard let contentView = contentView else { return }

        // Build a vertical stack (well + index label) for each of the 5 palette slots
        var wellStacks: [NSView] = []
        let storedColors = CustomPaletteStore.shared.colors
        for i in 0..<5 {
            let well = NSColorWell()
            well.color = storedColors[i]
            well.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                well.widthAnchor.constraint(equalToConstant: 44),
                well.heightAnchor.constraint(equalToConstant: 44)
            ])
            colorWells.append(well)

            // Label shows the colour index so users can relate wells to game tile colours
            let label = NSTextField(labelWithString: "\(i)")
            label.font = NSFont.systemFont(ofSize: 11)
            label.alignment = .center

            let stack = NSStackView(views: [well, label])
            stack.orientation = .vertical
            stack.alignment = .centerX
            stack.spacing = 4
            wellStacks.append(stack)
        }

        // Arrange the 5 stacks side by side
        let wellsRow = NSStackView(views: wellStacks)
        wellsRow.orientation = .horizontal
        wellsRow.spacing = 12
        wellsRow.translatesAutoresizingMaskIntoConstraints = false

        // OK accepts changes; Cancel discards them
        let okButton = NSButton(title: "OK", target: self, action: #selector(okClicked(_:)))
        okButton.keyEquivalent = "\r"
        let cancelButton = NSButton(title: "Cancel", target: self, action: #selector(cancelClicked(_:)))
        cancelButton.keyEquivalent = "\u{1b}"

        let buttonsRow = NSStackView(views: [cancelButton, okButton])
        buttonsRow.orientation = .horizontal
        buttonsRow.spacing = 8
        buttonsRow.translatesAutoresizingMaskIntoConstraints = false

        contentView.addSubview(wellsRow)
        contentView.addSubview(buttonsRow)

        NSLayoutConstraint.activate([
            wellsRow.centerXAnchor.constraint(equalTo: contentView.centerXAnchor),
            wellsRow.topAnchor.constraint(equalTo: contentView.topAnchor, constant: 16),

            buttonsRow.trailingAnchor.constraint(equalTo: contentView.trailingAnchor, constant: -12),
            buttonsRow.bottomAnchor.constraint(equalTo: contentView.bottomAnchor, constant: -12)
        ])
    }

    /// Saves the current well colours to CustomPaletteStore (posts .customPaletteDidChange) and closes.
    @objc private func okClicked(_ sender: NSButton) {
        // SKColor is a typealias for NSColor on macOS — no cast needed
        CustomPaletteStore.shared.save(colorWells.map { $0.color })
        close()
    }

    @objc private func cancelClicked(_ sender: NSButton) {
        close()
    }

    // Window close button also discards changes (same as Cancel)
    func windowShouldClose(_ sender: NSWindow) -> Bool {
        return true
    }
}

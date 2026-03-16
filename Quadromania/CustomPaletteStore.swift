// CustomPaletteStore.swift
// Persistent storage for the user's custom colour palette.

import SpriteKit

/// Singleton that stores and restores the 5-colour custom palette in UserDefaults.
class CustomPaletteStore {
    static let shared = CustomPaletteStore()
    private static let defaultsKey = "customPaletteColors"

    private init() {}

    /// Five SKColors for the custom palette. Reads from UserDefaults, falls back to Spring-like defaults.
    var colors: [SKColor] {
        get {
            // UserDefaults round-trips numeric arrays as [[Double]] via NSNumber bridging;
            // casting as [[CGFloat]] always returns nil even on 64-bit where CGFloat == Double.
            guard let data = UserDefaults.standard.array(forKey: Self.defaultsKey)
                    as? [[Double]], data.count == 5 else {
                return defaultColors
            }
            return data.map { SKColor(red: CGFloat($0[0]), green: CGFloat($0[1]),
                                      blue: CGFloat($0[2]), alpha: 1) }
        }
        set {
            // Store as [[Double]] — consistent with the [[Double]] read-back in the getter
            let data = newValue.map { c -> [Double] in
                var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
                c.getRed(&r, green: &g, blue: &b, alpha: &a)
                return [Double(r), Double(g), Double(b)]
            }
            UserDefaults.standard.set(data, forKey: Self.defaultsKey)
        }
    }

    /// Saves `colors` to UserDefaults and broadcasts `.customPaletteDidChange`.
    func save(_ colors: [SKColor]) {
        self.colors = colors
        NotificationCenter.default.post(name: .customPaletteDidChange, object: nil)
    }

    // Default: copy of Spring palette colours
    private let defaultColors: [SKColor] = [
        SKColor(red: 0.98, green: 0.80, blue: 0.84, alpha: 1),
        SKColor(red: 0.52, green: 0.88, blue: 0.67, alpha: 1),
        SKColor(red: 0.76, green: 0.66, blue: 0.94, alpha: 1),
        SKColor(red: 0.99, green: 0.92, blue: 0.42, alpha: 1),
        SKColor(red: 0.50, green: 0.78, blue: 0.97, alpha: 1),
    ]
}

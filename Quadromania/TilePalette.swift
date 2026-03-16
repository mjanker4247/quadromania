// TilePalette.swift
// Four named colour palettes for the tile grid.
// Index 0 is always the goal/win colour.

import SpriteKit

/// Identifies one of the built-in colour themes or the user-defined custom palette.
/// Stored as an Int raw value so the active selection can be persisted in UserDefaults.
enum TilePalette: Int, CaseIterable {
    /// Pastel spring tones: cherry blossom, mint, lilac, buttercup, sky blue.
    case spring
    /// Coastal tones: seafoam, ocean blue, deep teal, coral, sandy gold.
    case ocean
    /// Warm evening tones: golden, coral orange, rose, violet, warm red.
    case sunset
    /// Woodland tones: moss, forest green, mushroom, autumn orange, bark brown.
    case forest
    /// Five colours freely chosen by the user and persisted via CustomPaletteStore.
    case custom

    /// Human-readable name shown in the Palette menu and any UI labels.
    var displayName: String {
        switch self {
        case .spring: return "🌸 Spring"
        case .ocean:  return "🌊 Ocean"
        case .sunset: return "🌅 Sunset"
        case .forest: return "🌿 Forest"
        case .custom: return "🎨 Custom"
        }
    }

    /// Five colours: index 0 = goal state (all tiles solved), 1–4 = in-play tile colours.
    /// The `.custom` case delegates entirely to `CustomPaletteStore.shared.colors`,
    /// so any scene that reads this property will automatically use the latest saved values.
    var colors: [SKColor] {
        switch self {
        case .spring: return [
            SKColor(red: 0.98, green: 0.80, blue: 0.84, alpha: 1),  // 0 cherry blossom
            SKColor(red: 0.52, green: 0.88, blue: 0.67, alpha: 1),  // 1 mint
            SKColor(red: 0.76, green: 0.66, blue: 0.94, alpha: 1),  // 2 lilac
            SKColor(red: 0.99, green: 0.92, blue: 0.42, alpha: 1),  // 3 buttercup
            SKColor(red: 0.50, green: 0.78, blue: 0.97, alpha: 1),  // 4 sky blue
        ]
        case .ocean: return [
            SKColor(red: 0.72, green: 0.93, blue: 0.89, alpha: 1),  // 0 seafoam
            SKColor(red: 0.23, green: 0.56, blue: 0.78, alpha: 1),  // 1 ocean blue
            SKColor(red: 0.11, green: 0.48, blue: 0.41, alpha: 1),  // 2 deep teal
            SKColor(red: 0.96, green: 0.48, blue: 0.42, alpha: 1),  // 3 coral
            SKColor(red: 0.96, green: 0.77, blue: 0.38, alpha: 1),  // 4 sandy gold
        ]
        case .sunset: return [
            SKColor(red: 0.99, green: 0.91, blue: 0.54, alpha: 1),  // 0 golden
            SKColor(red: 0.98, green: 0.57, blue: 0.24, alpha: 1),  // 1 coral orange
            SKColor(red: 0.96, green: 0.45, blue: 0.71, alpha: 1),  // 2 rose
            SKColor(red: 0.65, green: 0.55, blue: 0.98, alpha: 1),  // 3 violet
            SKColor(red: 0.94, green: 0.27, blue: 0.27, alpha: 1),  // 4 warm red
        ]
        case .forest: return [
            SKColor(red: 0.82, green: 0.91, blue: 0.77, alpha: 1),  // 0 moss
            SKColor(red: 0.18, green: 0.49, blue: 0.23, alpha: 1),  // 1 forest green
            SKColor(red: 0.77, green: 0.66, blue: 0.51, alpha: 1),  // 2 mushroom
            SKColor(red: 0.83, green: 0.45, blue: 0.12, alpha: 1),  // 3 autumn orange
            SKColor(red: 0.55, green: 0.37, blue: 0.24, alpha: 1),  // 4 bark brown
        ]
        // Delegates to CustomPaletteStore so callers always see the latest user-edited colours.
        case .custom: return CustomPaletteStore.shared.colors
        }
    }
}

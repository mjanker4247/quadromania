// TransitionStyle.swift
// Three selectable rotation animation modes for the 3×3 block.

/// Animation style applied to each tile when the player clicks a rotation center.
/// Stored as an Int raw value so it can be persisted in UserDefaults.
enum TransitionStyle: Int, CaseIterable {
    /// Tiles animate outward from the center in concentric rings (center first, then edge ring).
    case ringSweep   = 0
    /// Tiles animate one after another in a deterministic scan order.
    case sequential  = 1
    /// All tiles in the block animate simultaneously with a scaled pulse effect.
    case radialPulse = 2

    /// Human-readable name shown in the Game → Transition submenu.
    var displayName: String {
        switch self {
        case .ringSweep:   return "Ring Sweep"
        case .sequential:  return "Sequential"
        case .radialPulse: return "Radial Pulse"
        }
    }
}

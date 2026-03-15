// TransitionStyle.swift
// Three selectable rotation animation modes for the 3×3 block.

enum TransitionStyle: Int, CaseIterable {
    case ringSweep   = 0
    case sequential  = 1
    case radialPulse = 2

    var displayName: String {
        switch self {
        case .ringSweep:   return "Ring Sweep"
        case .sequential:  return "Sequential"
        case .radialPulse: return "Radial Pulse"
        }
    }
}

# QuadroCore Static Library Target — Design Spec

**Date:** 2026-03-15
**Status:** Approved

## Problem

`GameModel.swift` and `PuzzleGenerator.swift` are compiled as part of the `Quadromania` app target alongside SpriteKit scenes. Swift 5.5+ infers `@MainActor` on classes compiled in the same unit as SpriteKit types, causing `GameModel` to become `@MainActor`-isolated. This forces `GameModelIntegrationTests` to carry a `@MainActor` annotation and a `nonisolated deinit {}` workaround in `GameModel` itself. Tests cannot be run against pure game logic in isolation from the UI.

## Goal

Separate game logic from UI code so that `QuadroTests` can build and run against game logic alone, with no SpriteKit in the compile unit.

## Chosen Approach

Add a `QuadroCore` static library target within the existing `Quadromania.xcodeproj`. Move the two logic source files into it. Both the app target and the test target link against `QuadroCore`.

## Target Structure

```
QuadroCore (static library, macOS, Swift, Foundation only)
    ↑ linked by
Quadromania (app target, SpriteKit)
    ↑ host for
QuadroTests (XCTest bundle) — links QuadroCore directly
```

### Files That Change Target Membership

| File | Old target | New target |
|------|------------|------------|
| `Quadromania/GameModel.swift` | Quadromania | QuadroCore |
| `Quadromania/PuzzleGenerator.swift` | Quadromania | QuadroCore |

Files stay at their current paths on disk. Only `.xcodeproj` membership changes.

## Implementation Steps

1. **Add `QuadroCore` static library target** in `Quadromania.xcodeproj`
   - Platform: macOS
   - Language: Swift
   - Deployment target: matches the app (macOS 14+)
   - No additional frameworks (Foundation via Swift stdlib)

2. **Reassign source files** — remove `GameModel.swift` and `PuzzleGenerator.swift` from the `Quadromania` compile sources build phase; add them to `QuadroCore`'s compile sources build phase

3. **Link `QuadroCore` into the `Quadromania` app target**
   - Add `QuadroCore` as an explicit target dependency
   - Add `libQuadroCore.a` to the app's "Link Binary With Libraries" build phase

4. **Retarget `QuadroTests`**
   - Change the test bundle's host from `Quadromania` to `QuadroCore` (or set it to none and link `QuadroCore` directly)
   - Replace `@testable import Quadromania` with `@testable import QuadroCore` in both test files

5. **Remove workarounds**
   - Delete `@MainActor` from `GameModelIntegrationTests`
   - Delete the `nonisolated deinit {}` and its explanatory comment from `GameModel`

6. **Verify build**
   - Build the `Quadromania` scheme (confirms app still compiles)
   - Run the `QuadroTests` target (confirms tests pass without `@MainActor`)

## Success Criteria

- `xcodebuild` for the `Quadromania` scheme succeeds with no errors
- `GameModelIntegrationTests` and `PuzzleGeneratorTests` pass without any `@MainActor` annotation on the test classes
- `GameModel.swift` contains no concurrency workarounds (`nonisolated deinit` is gone)
- `QuadroCore` has no SpriteKit import, directly or transitively

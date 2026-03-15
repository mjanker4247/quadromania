# QuadroCore Static Library Target — Design Spec

**Date:** 2026-03-15
**Status:** Approved

## Problem

`GameModel.swift` and `PuzzleGenerator.swift` are compiled as part of the `Quadromania` app target alongside SpriteKit scenes. Two build settings on the app target combine to propagate `@MainActor` to all types in the module:

- `SWIFT_APPROACHABLE_CONCURRENCY = YES` — enables strict concurrency checking, allowing `@MainActor` to propagate from SpriteKit types to other types in the same module
- `SWIFT_DEFAULT_ACTOR_ISOLATION = MainActor` — makes `@MainActor` the default isolation for every type in the module that does not declare otherwise

Together these cause `GameModel` to become `@MainActor`-isolated despite importing only `Foundation`. This forces `GameModelIntegrationTests` to carry a `@MainActor` annotation and a `nonisolated deinit {}` workaround in `GameModel` itself. Tests cannot be run against pure game logic in isolation from the UI.

## Goal

Separate game logic from UI code so that `QuadroTests` can build and run against game logic alone, with no SpriteKit in the compile unit.

## Chosen Approach

Add a `QuadroCore` static library target within the existing `Quadromania.xcodeproj`. Move the two logic source files into it. Both the app target and the test target link against `QuadroCore`.

## Target Structure

```
QuadroCore (static library, macOS, Swift, Foundation only)
    ↑ linked by
Quadromania (app target, SpriteKit)
    ↑ also linked by
QuadroTests (XCTest bundle) — links QuadroCore directly; Quadromania remains TEST_HOST
```

### Files That Move on Disk

The project uses `PBXFileSystemSynchronizedRootGroup` (Xcode 16+ automatic folder syncing). Files that remain under `Quadromania/` will continue to be offered to the `Quadromania` target automatically. To avoid this, the two logic files must move to a new top-level `QuadroCore/` folder that becomes its own synchronized group.

| File (current path) | New path |
|---------------------|----------|
| `Quadromania/GameModel.swift` | `QuadroCore/GameModel.swift` |
| `Quadromania/PuzzleGenerator.swift` | `QuadroCore/PuzzleGenerator.swift` |

## Implementation Steps

1. **Move source files on disk**
   - Create a `QuadroCore/` folder at the repository root (sibling to `Quadromania/`)
   - Move `GameModel.swift` and `PuzzleGenerator.swift` into it
   - Update any `#include`/`import` paths if needed (none expected — both files import only Foundation)

2. **Add `QuadroCore` static library target** in `Quadromania.xcodeproj`
   - Type: Static Library
   - Platform: macOS
   - Language: Swift
   - Deployment target: `14.6` (matches the project-level `MACOSX_DEPLOYMENT_TARGET` in the `.pbxproj`)
   - No additional frameworks (Foundation via Swift stdlib)
   - Add `QuadroCore/` as a `PBXFileSystemSynchronizedRootGroup` for this target
   - Set `ENABLE_TESTABILITY = YES` for both Debug and Release configurations
   - The `QuadroCore` target's Frameworks build phase must remain empty (no SpriteKit)

3. **Link `QuadroCore` into the `Quadromania` app target**
   - Add `QuadroCore` as an explicit target dependency
   - Add `libQuadroCore.a` to the app's "Link Binary With Libraries" build phase

4. **Update `QuadroTests`**
   - Keep `TEST_HOST` pointing to `Quadromania.app` (a static library cannot be a test host)
   - Add `QuadroCore` as an explicit target dependency of `QuadroTests`
   - Add `libQuadroCore.a` to `QuadroTests`'s "Link Binary With Libraries" build phase
   - In both test files, replace `@testable import Quadromania` with `@testable import QuadroCore`

5. **Remove workarounds**
   - Delete `@MainActor` from `GameModelIntegrationTests`
   - Delete the `nonisolated deinit {}` and its explanatory comment from `GameModel`

6. **Verify build**
   - Build the `Quadromania` scheme: `xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build`
   - Run the `QuadroTests` target and confirm all tests pass without any `@MainActor` annotation on the test classes

## Success Criteria

- `xcodebuild` for the `Quadromania` scheme succeeds with no errors or warnings
- `GameModelIntegrationTests` and `PuzzleGeneratorTests` pass without `@MainActor` on the test classes
- `GameModel.swift` contains no concurrency workarounds (`nonisolated deinit` is gone)
- The `QuadroCore` target's Frameworks build phase is empty (no SpriteKit reference)
- `GameModel.swift` and `PuzzleGenerator.swift` each contain no `import SpriteKit`

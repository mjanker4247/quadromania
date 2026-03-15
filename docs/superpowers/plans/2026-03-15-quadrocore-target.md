# QuadroCore Static Library Target Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extract `GameModel` and `PuzzleGenerator` into a `QuadroCore` static library so the test target compiles game logic without SpriteKit, eliminating `@MainActor` workarounds.

**Architecture:** Add a `QuadroCore` static library target alongside the existing `Quadromania` app target. The app links against `libQuadroCore.a`; `QuadroTests` takes `QuadroCore` as a target dependency only (no extra link step — the host app already provides the symbols via `BUNDLE_LOADER`). The app adds `import QuadroCore` to three UI files; tests change `@testable import Quadromania` to `@testable import QuadroCore`.

**Tech Stack:** Swift, Xcode 26, macOS 14.6 deployment target, `project.pbxproj` direct editing.

---

## File Map

| Action | Path | What changes |
|--------|------|--------------|
| Move | `Quadromania/GameModel.swift` → `QuadroCore/GameModel.swift` | Target membership changes |
| Move | `Quadromania/PuzzleGenerator.swift` → `QuadroCore/PuzzleGenerator.swift` | Target membership changes |
| Modify | `Quadromania/GameModel.swift` | Remove `nonisolated deinit {}` workaround |
| Modify | `Quadromania/GamePlayScene.swift` | Add `import QuadroCore` |
| Modify | `Quadromania/TitleScene.swift` | Add `import QuadroCore` |
| Modify | `Quadromania/TileGridNode.swift` | Add `import QuadroCore` |
| Modify | `QuadroTests/GameModelIntegrationTests.swift` | Change import; remove `@MainActor` |
| Modify | `QuadroTests/PuzzleGeneratorTests.swift` | Change import |
| Modify | `Quadromania.xcodeproj/project.pbxproj` | Add `QuadroCore` target, wire dependencies |

---

## Chunk 1: Move Source Files and Add App Imports

### Task 1: Create `QuadroCore/` folder and move source files

**Files:**
- Create: `QuadroCore/GameModel.swift` (moved from `Quadromania/GameModel.swift`)
- Create: `QuadroCore/PuzzleGenerator.swift` (moved from `Quadromania/PuzzleGenerator.swift`)
- Delete content from: `Quadromania/GameModel.swift`, `Quadromania/PuzzleGenerator.swift`

> **Why a new top-level folder?** The project uses `PBXFileSystemSynchronizedRootGroup` — Xcode 16+ auto-syncing that compiles everything inside `Quadromania/` into the `Quadromania` target. Moving files out of that folder is required to prevent automatic re-inclusion.

> All shell commands in Chunk 1 assume the working directory is the repository root (the directory containing `Quadromania.xcodeproj`).

- [ ] **Step 1: Create QuadroCore directory**

```bash
mkdir -p QuadroCore
```

- [ ] **Step 2: Move GameModel.swift**

Read `Quadromania/GameModel.swift`, write identical content to `QuadroCore/GameModel.swift`, then delete `Quadromania/GameModel.swift` using a filesystem tool (not `git rm` — Step 5 handles the git-layer removal).

- [ ] **Step 3: Move PuzzleGenerator.swift**

Read `Quadromania/PuzzleGenerator.swift`, write identical content to `QuadroCore/PuzzleGenerator.swift`, then delete `Quadromania/PuzzleGenerator.swift` using a filesystem tool (not `git rm` — Step 5 handles the git-layer removal).

- [ ] **Step 4: Verify files are in place**

```bash
ls QuadroCore/
```
Expected output:
```
GameModel.swift
PuzzleGenerator.swift
```

- [ ] **Step 5: Commit**

```bash
git add QuadroCore/GameModel.swift QuadroCore/PuzzleGenerator.swift
git rm Quadromania/GameModel.swift Quadromania/PuzzleGenerator.swift
git commit -m "refactor: move game logic to QuadroCore/ folder"
```

---

### Task 2: Add `import QuadroCore` to app files that reference `GameModel`

**Files:**
- Modify: `Quadromania/GamePlayScene.swift`
- Modify: `Quadromania/TitleScene.swift`
- Modify: `Quadromania/TileGridNode.swift`

- [ ] **Step 1: Add import to GamePlayScene.swift**

In `Quadromania/GamePlayScene.swift`, change:
```swift
import SpriteKit
```
to:
```swift
import SpriteKit
import QuadroCore
```

- [ ] **Step 2: Add import to TitleScene.swift**

In `Quadromania/TitleScene.swift`, change:
```swift
import SpriteKit
```
to:
```swift
import SpriteKit
import QuadroCore
```

- [ ] **Step 3: Add import to TileGridNode.swift**

Read `Quadromania/TileGridNode.swift` to find its import line, then add `import QuadroCore` below `import SpriteKit` (same pattern as above).

- [ ] **Step 4: Commit**

```bash
git add Quadromania/GamePlayScene.swift Quadromania/TitleScene.swift Quadromania/TileGridNode.swift
git commit -m "refactor: add import QuadroCore to app files"
```

---

## Chunk 2: Edit `project.pbxproj`

> **Important:** `project.pbxproj` is a plain text file. All edits are exact string replacements. Make one logical edit at a time and keep the surrounding structure intact. The UUIDs below were chosen for this plan — use them exactly as written to keep the steps consistent.

**UUID legend (use these exact strings throughout):**

| Object | UUID |
|--------|------|
| QuadroCore `PBXFileSystemSynchronizedRootGroup` | `AA000001000000000000AA01` |
| QuadroCore `PBXNativeTarget` | `AA000002000000000000AA02` |
| QuadroCore `PBXSourcesBuildPhase` | `AA000003000000000000AA03` |
| QuadroCore `PBXFrameworksBuildPhase` | `AA000004000000000000AA04` |
| `libQuadroCore.a` `PBXFileReference` | `AA000005000000000000AA05` |
| QuadroCore Debug `XCBuildConfiguration` | `AA000006000000000000AA06` |
| QuadroCore Release `XCBuildConfiguration` | `AA000007000000000000AA07` |
| QuadroCore `XCConfigurationList` | `AA000008000000000000AA08` |
| `PBXContainerItemProxy` for Quadromania→QuadroCore | `AA000009000000000000AA09` |
| `PBXTargetDependency` Quadromania→QuadroCore | `AA00000A000000000000AA0A` |
| `PBXContainerItemProxy` for QuadroTests→QuadroCore | `AA00000B000000000000AA0B` |
| `PBXTargetDependency` QuadroTests→QuadroCore | `AA00000C000000000000AA0C` |
| `PBXBuildFile` libQuadroCore.a in Quadromania Frameworks | `AA00000D000000000000AA0D` |

### Task 3: Add new sections to `project.pbxproj`

**Files:**
- Modify: `Quadromania.xcodeproj/project.pbxproj`

- [ ] **Step 1: Add `PBXBuildFile` section**

Find:
```
/* Begin PBXContainerItemProxy section */
```

Insert the new section immediately before it:
```
/* Begin PBXBuildFile section */
		AA00000D000000000000AA0D /* libQuadroCore.a in Frameworks */ = {isa = PBXBuildFile; fileRef = AA000005000000000000AA05 /* libQuadroCore.a */; };
/* End PBXBuildFile section */

```

- [ ] **Step 2: Add `PBXContainerItemProxy` entries for QuadroCore**

Find the end of the existing `PBXContainerItemProxy` section:
```
/* End PBXContainerItemProxy section */
```

Replace with:
```
		AA000009000000000000AA09 /* PBXContainerItemProxy */ = {
			isa = PBXContainerItemProxy;
			containerPortal = 1D550BC42F634CF100BD8ED6 /* Project object */;
			proxyType = 1;
			remoteGlobalIDString = AA000002000000000000AA02;
			remoteInfo = QuadroCore;
		};
		AA00000B000000000000AA0B /* PBXContainerItemProxy */ = {
			isa = PBXContainerItemProxy;
			containerPortal = 1D550BC42F634CF100BD8ED6 /* Project object */;
			proxyType = 1;
			remoteGlobalIDString = AA000002000000000000AA02;
			remoteInfo = QuadroCore;
		};
/* End PBXContainerItemProxy section */
```

- [ ] **Step 3: Add `libQuadroCore.a` to the `PBXFileReference` section**

Find:
```
		1D550C1C2F63E33800BD8ED6 /* Game purpose.md */ = {isa = PBXFileReference; lastKnownFileType = net.daringfireball.markdown; path = "Game purpose.md"; sourceTree = "<group>"; };
```

Replace with:
```
		1D550C1C2F63E33800BD8ED6 /* Game purpose.md */ = {isa = PBXFileReference; lastKnownFileType = net.daringfireball.markdown; path = "Game purpose.md"; sourceTree = "<group>"; };
		AA000005000000000000AA05 /* libQuadroCore.a */ = {isa = PBXFileReference; explicitFileType = archive.ar; includeInIndex = 0; path = libQuadroCore.a; sourceTree = BUILT_PRODUCTS_DIR; };
```

- [ ] **Step 4: Add `QuadroCore` to the `PBXFileSystemSynchronizedRootGroup` section**

Find:
```
/* End PBXFileSystemSynchronizedRootGroup section */
```

Replace with:
```
		AA000001000000000000AA01 /* QuadroCore */ = {
			isa = PBXFileSystemSynchronizedRootGroup;
			path = QuadroCore;
			sourceTree = "<group>";
		};
/* End PBXFileSystemSynchronizedRootGroup section */
```

- [ ] **Step 5: Add `PBXFrameworksBuildPhase` for QuadroCore**

Find:
```
/* End PBXFrameworksBuildPhase section */
```

Replace with:
```
		AA000004000000000000AA04 /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXFrameworksBuildPhase section */
```

- [ ] **Step 6: Add `libQuadroCore.a` to the `Products` group and `QuadroCore/` to the root group**

Find:
```
		1D550BC32F634CF100BD8ED6 = {
			isa = PBXGroup;
			children = (
				1D550C1C2F63E33800BD8ED6 /* Game purpose.md */,
				1D550BCE2F634CF100BD8ED6 /* Quadromania */,
				1D1A658B2F65405A00A3467A /* QuadroTests */,
				1D550BCD2F634CF100BD8ED6 /* Products */,
			);
			sourceTree = "<group>";
		};
```

Replace with:
```
		1D550BC32F634CF100BD8ED6 = {
			isa = PBXGroup;
			children = (
				1D550C1C2F63E33800BD8ED6 /* Game purpose.md */,
				1D550BCE2F634CF100BD8ED6 /* Quadromania */,
				AA000001000000000000AA01 /* QuadroCore */,
				1D1A658B2F65405A00A3467A /* QuadroTests */,
				1D550BCD2F634CF100BD8ED6 /* Products */,
			);
			sourceTree = "<group>";
		};
```

Find:
```
		1D550BCD2F634CF100BD8ED6 /* Products */ = {
			isa = PBXGroup;
			children = (
				1D550BCC2F634CF100BD8ED6 /* Quadromania.app */,
				1D1A658A2F65405A00A3467A /* QuadroTests.xctest */,
			);
			name = Products;
			sourceTree = "<group>";
		};
```

Replace with:
```
		1D550BCD2F634CF100BD8ED6 /* Products */ = {
			isa = PBXGroup;
			children = (
				1D550BCC2F634CF100BD8ED6 /* Quadromania.app */,
				1D1A658A2F65405A00A3467A /* QuadroTests.xctest */,
				AA000005000000000000AA05 /* libQuadroCore.a */,
			);
			name = Products;
			sourceTree = "<group>";
		};
```

- [ ] **Step 7: Add `QuadroCore` native target**

Find:
```
/* End PBXNativeTarget section */
```

Replace with:
```
		AA000002000000000000AA02 /* QuadroCore */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = AA000008000000000000AA08 /* Build configuration list for PBXNativeTarget "QuadroCore" */;
			buildPhases = (
				AA000003000000000000AA03 /* Sources */,
				AA000004000000000000AA04 /* Frameworks */,
			);
			buildRules = (
			);
			dependencies = (
			);
			fileSystemSynchronizedGroups = (
				AA000001000000000000AA01 /* QuadroCore */,
			);
			name = QuadroCore;
			packageProductDependencies = (
			);
			productName = QuadroCore;
			productReference = AA000005000000000000AA05 /* libQuadroCore.a */;
			productType = "com.apple.product-type.library.static";
		};
/* End PBXNativeTarget section */
```

- [ ] **Step 8: Add `QuadroCore` to the project's targets list and TargetAttributes**

Find:
```
			TargetAttributes = {
				1D1A65892F65405A00A3467A = {
					CreatedOnToolsVersion = 26.3;
					TestTargetID = 1D550BCB2F634CF100BD8ED6;
				};
				1D550BCB2F634CF100BD8ED6 = {
					CreatedOnToolsVersion = 26.3;
				};
			};
```

Replace with:
```
			TargetAttributes = {
				1D1A65892F65405A00A3467A = {
					CreatedOnToolsVersion = 26.3;
					TestTargetID = 1D550BCB2F634CF100BD8ED6;
				};
				1D550BCB2F634CF100BD8ED6 = {
					CreatedOnToolsVersion = 26.3;
				};
				AA000002000000000000AA02 = {
					CreatedOnToolsVersion = 26.3;
				};
			};
```

Find:
```
			targets = (
				1D550BCB2F634CF100BD8ED6 /* Quadromania */,
				1D1A65892F65405A00A3467A /* QuadroTests */,
			);
```

Replace with:
```
			targets = (
				1D550BCB2F634CF100BD8ED6 /* Quadromania */,
				1D1A65892F65405A00A3467A /* QuadroTests */,
				AA000002000000000000AA02 /* QuadroCore */,
			);
```

- [ ] **Step 9: Add `PBXSourcesBuildPhase` for QuadroCore**

Find:
```
/* End PBXSourcesBuildPhase section */
```

Replace with:
```
		AA000003000000000000AA03 /* Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXSourcesBuildPhase section */
```

- [ ] **Step 10: Add `PBXTargetDependency` entries for QuadroCore**

Find:
```
/* End PBXTargetDependency section */
```

Replace with:
```
		AA00000A000000000000AA0A /* PBXTargetDependency */ = {
			isa = PBXTargetDependency;
			target = AA000002000000000000AA02 /* QuadroCore */;
			targetProxy = AA000009000000000000AA09 /* PBXContainerItemProxy */;
		};
		AA00000C000000000000AA0C /* PBXTargetDependency */ = {
			isa = PBXTargetDependency;
			target = AA000002000000000000AA02 /* QuadroCore */;
			targetProxy = AA00000B000000000000AA0B /* PBXContainerItemProxy */;
		};
/* End PBXTargetDependency section */
```

- [ ] **Step 11: Add `XCBuildConfiguration` entries for QuadroCore**

Find:
```
/* End XCBuildConfiguration section */
```

Replace with:
```
		AA000006000000000000AA06 /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				CODE_SIGN_STYLE = Automatic;
				CURRENT_PROJECT_VERSION = 1;
				DEVELOPMENT_TEAM = 8J58VWJG83;
				ENABLE_TESTABILITY = YES;
				MACOSX_DEPLOYMENT_TARGET = 14.6;
				MARKETING_VERSION = 1.0;
				PRODUCT_NAME = QuadroCore;
				SWIFT_APPROACHABLE_CONCURRENCY = YES;
				SWIFT_EMIT_LOC_STRINGS = NO;
				SWIFT_UPCOMING_FEATURE_MEMBER_IMPORT_VISIBILITY = YES;
				SWIFT_VERSION = 5.0;
			};
			name = Debug;
		};
		AA000007000000000000AA07 /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				CODE_SIGN_STYLE = Automatic;
				CURRENT_PROJECT_VERSION = 1;
				DEVELOPMENT_TEAM = 8J58VWJG83;
				ENABLE_TESTABILITY = YES;
				MACOSX_DEPLOYMENT_TARGET = 14.6;
				MARKETING_VERSION = 1.0;
				PRODUCT_NAME = QuadroCore;
				SWIFT_APPROACHABLE_CONCURRENCY = YES;
				SWIFT_EMIT_LOC_STRINGS = NO;
				SWIFT_UPCOMING_FEATURE_MEMBER_IMPORT_VISIBILITY = YES;
				SWIFT_VERSION = 5.0;
			};
			name = Release;
		};
/* End XCBuildConfiguration section */
```

- [ ] **Step 12: Add `XCConfigurationList` for QuadroCore**

Find:
```
/* End XCConfigurationList section */
```

Replace with:
```
		AA000008000000000000AA08 /* Build configuration list for PBXNativeTarget "QuadroCore" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				AA000006000000000000AA06 /* Debug */,
				AA000007000000000000AA07 /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
/* End XCConfigurationList section */
```

### Task 4: Wire QuadroCore into Quadromania and QuadroTests

**Files:**
- Modify: `Quadromania.xcodeproj/project.pbxproj`

- [ ] **Step 1: Add QuadroCore dependency and libQuadroCore.a to Quadromania target**

Find:
```
		1D550BCB2F634CF100BD8ED6 /* Quadromania */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = 1D550BE02F634CF200BD8ED6 /* Build configuration list for PBXNativeTarget "Quadromania" */;
			buildPhases = (
				1D550BC82F634CF100BD8ED6 /* Sources */,
				1D550BC92F634CF100BD8ED6 /* Frameworks */,
				1D550BCA2F634CF100BD8ED6 /* Resources */,
			);
			buildRules = (
			);
			dependencies = (
			);
```

Replace with:
```
		1D550BCB2F634CF100BD8ED6 /* Quadromania */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = 1D550BE02F634CF200BD8ED6 /* Build configuration list for PBXNativeTarget "Quadromania" */;
			buildPhases = (
				1D550BC82F634CF100BD8ED6 /* Sources */,
				1D550BC92F634CF100BD8ED6 /* Frameworks */,
				1D550BCA2F634CF100BD8ED6 /* Resources */,
			);
			buildRules = (
			);
			dependencies = (
				AA00000A000000000000AA0A /* PBXTargetDependency */,
			);
```

Find the Quadromania Frameworks build phase (the one with `1D550BC92F634CF100BD8ED6`):
```
		1D550BC92F634CF100BD8ED6 /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
```

Replace with:
```
		1D550BC92F634CF100BD8ED6 /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
				AA00000D000000000000AA0D /* libQuadroCore.a in Frameworks */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
```

- [ ] **Step 2: Add QuadroCore dependency to QuadroTests target**

> `QuadroTests` is a hosted test bundle (`BUNDLE_LOADER = TEST_HOST`). The host app (`Quadromania.app`) already statically links `libQuadroCore.a`, so all `GameModel` and `PuzzleGenerator` symbols are present in the host process at test runtime. Linking `libQuadroCore.a` a second time into the test bundle would cause duplicate-symbol linker errors. We add only a target dependency (so `QuadroCore` builds before `QuadroTests` and the Swift module is available for `@testable import`) — no extra link step.

Find:
```
			dependencies = (
				1D1A658F2F65405A00A3467A /* PBXTargetDependency */,
			);
```

Replace with:
```
			dependencies = (
				1D1A658F2F65405A00A3467A /* PBXTargetDependency */,
				AA00000C000000000000AA0C /* PBXTargetDependency */,
			);
```

The QuadroTests Frameworks build phase (`1D1A65872F65405A00A3467A`) stays empty — no changes needed.

- [ ] **Step 3: Verify pbxproj is valid XML/plist**

```bash
plutil -lint Quadromania.xcodeproj/project.pbxproj
```
Expected: `Quadromania.xcodeproj/project.pbxproj: OK`

- [ ] **Step 4: Commit**

```bash
git add Quadromania.xcodeproj/project.pbxproj
git commit -m "chore: add QuadroCore static library target to xcodeproj"
```

---

## Chunk 3: Update Tests and Remove Workarounds

### Task 5: Update test file imports

**Files:**
- Modify: `QuadroTests/GameModelIntegrationTests.swift`
- Modify: `QuadroTests/PuzzleGeneratorTests.swift`

- [ ] **Step 1: Update GameModelIntegrationTests.swift**

In `QuadroTests/GameModelIntegrationTests.swift`, change:
```swift
import XCTest
@testable import Quadromania
```
to:
```swift
import XCTest
@testable import QuadroCore
```

- [ ] **Step 2: Remove `@MainActor` from GameModelIntegrationTests**

In `QuadroTests/GameModelIntegrationTests.swift`, change:
```swift
@MainActor
final class GameModelIntegrationTests: XCTestCase {
```
to:
```swift
final class GameModelIntegrationTests: XCTestCase {
```

- [ ] **Step 3: Update PuzzleGeneratorTests.swift**

In `QuadroTests/PuzzleGeneratorTests.swift`, change:
```swift
import XCTest
@testable import Quadromania
```
to:
```swift
import XCTest
@testable import QuadroCore
```

- [ ] **Step 4: Commit**

```bash
git add QuadroTests/GameModelIntegrationTests.swift QuadroTests/PuzzleGeneratorTests.swift
git commit -m "refactor: switch test imports from Quadromania to QuadroCore"
```

---

### Task 6: Remove `nonisolated deinit` workaround from GameModel

> **Prerequisite:** Chunk 1 must be complete — `GameModel.swift` is now at `QuadroCore/GameModel.swift`.

**Files:**
- Modify: `QuadroCore/GameModel.swift`

- [ ] **Step 1: Remove the workaround block**

In `QuadroCore/GameModel.swift`, find and delete the following block (including the blank line before it):

```swift
    // Explicitly non-isolated deinit to prevent Swift concurrency back-deployment
    // stub (swift_task_deinitOnExecutorMainActorBackDeploy) from running in
    // non-concurrency contexts such as XCTest — required because GameModel is
    // inferred @MainActor when compiled alongside SpriteKit scenes on macOS 14+
    // deployment targets.
    nonisolated deinit {}
```

The `// MARK: - Private` section that follows should remain.

- [ ] **Step 2: Commit**

```bash
git add QuadroCore/GameModel.swift
git commit -m "refactor: remove nonisolated deinit workaround from GameModel"
```

---

### Task 7: Build and verify

- [ ] **Step 1: Build the Quadromania app scheme**

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build 2>&1 | tail -5
```
Expected: `** BUILD SUCCEEDED **`

If it fails, check the error output. Common issues:
- "No such module 'QuadroCore'" in an app file → confirm `import QuadroCore` was added to all three files (Task 2)
- "Cannot find type 'GameModel'" → confirm pbxproj dependency wiring (Task 4)
- plist syntax error → re-run `plutil -lint` and fix whitespace/braces

- [ ] **Step 2: Run the tests**

> The `Quadromania` scheme is used here because no shared `.xcscheme` file for `QuadroTests` is committed to the repository. The test action on the `Quadromania` scheme runs the hosted `QuadroTests` bundle.

```bash
xcodebuild test -project Quadromania.xcodeproj -scheme Quadromania -destination 'platform=macOS' 2>&1 | grep -E "Test Suite|PASS|FAIL|error:"
```
Expected: all tests pass, no `error:` lines.

- [ ] **Step 3: Confirm no `@MainActor` annotation on test classes**

```bash
grep "@MainActor" QuadroTests/GameModelIntegrationTests.swift
```
Expected: no output.

- [ ] **Step 4: Confirm no `nonisolated deinit` in GameModel**

```bash
grep "nonisolated deinit" QuadroCore/GameModel.swift
```
Expected: no output.

- [ ] **Step 5: Confirm QuadroCore has no SpriteKit import**

```bash
grep "import SpriteKit" QuadroCore/GameModel.swift QuadroCore/PuzzleGenerator.swift
```
Expected: no output.

- [ ] **Step 6: Confirm working tree is clean**

```bash
git status
```
Expected: `nothing to commit, working tree clean`. All changes should have been committed in earlier tasks.

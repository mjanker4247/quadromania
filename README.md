# Quadromania

A macOS puzzle game — Swift/SpriteKit port of the original Quadromania by **Matthias Arndt** (ASM Software).

> Original C/SDL version © 2002/2003/2009/2010 Matthias Arndt <marndt@asmsoftware.de>
> http://www.asmsoftware.de/
> Licensed under the GNU General Public License.

---

## The Game

Restore an **18×13 grid** of colored tiles back to red by clicking the center of any 3×3 section. Each click cycles every tile in that section forward by one color. The computer scrambles the board at the start — you have a fixed number of turns to solve it.

- **Colors**: 1–4 (configurable), cycling back to red (0) at the maximum
- **Turn limit**: `initialRotations × maxColors`
- **Score**: `((limit − turns) × 10000) / turns` — fewer turns scores higher; exceeding the limit scores 0
- **Level rotations**: `56 + level × 13` (levels 1–10)

## Requirements

- macOS 14 or later
- Xcode 15 or later

## Build & Run

Open `Quadromania.xcodeproj` in Xcode and press **Cmd+R**, or from the terminal:

```bash
xcodebuild -project Quadromania.xcodeproj -scheme Quadromania -configuration Debug build
```

## Controls

| Action | Input |
|--------|-------|
| Rotate 3×3 section | Click the center tile |
| Navigate menus | Mouse hover + click |

## Menu Bar

| Menu | Item | Shortcut |
|------|------|----------|
| Game | New Game | Cmd+N |
| Game | Instructions | Cmd+I |
| Game | Highscores | Cmd+Shift+H |
| Sound | Stop/Start Music | Cmd+Shift+M |
| Window | Enter Full Screen | Ctrl+Cmd+F |

## Architecture

Built with Swift and SpriteKit, targeting macOS via Cocoa. Game logic is fully decoupled from the presentation layer.

```
AppDelegate
  └─ GameViewController (SKView 1280×960)
       └─ TitleScene          ← main menu
            └─ GamePlayScene  ← active gameplay
                 └─ TitleScene (on game end)
```

Key source files:

| File | Responsibility |
|------|----------------|
| `AppDelegate.swift` | App lifecycle, music startup |
| `GameViewController.swift` | Hosts SKView, handles menu bar actions |
| `GameModel.swift` | Pure game logic: grid, rotation, win/loss, scoring |
| `HighscoreManager.swift` | Persistent highscores via UserDefaults (10 levels × 10 entries) |
| `SoundManager.swift` | Background music + sound effects (AVFoundation) |
| `TitleScene.swift` | Main menu scene |
| `GamePlayScene.swift` | Active gameplay scene |
| `InstructionsScene.swift` | Instructions screen |
| `HighscoreScene.swift` | Highscore display |
| `TileGridNode.swift` | 18×13 tile grid node |

## Credits

Original game design and implementation by **Matthias Arndt** / ASM Software (2002–2010).
macOS Swift/SpriteKit port by Marco Janker (2026).

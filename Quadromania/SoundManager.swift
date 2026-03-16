import AVFoundation
import os.log

private let log = Logger(subsystem: "com.quadromania.macos", category: "Audio")

enum SoundEffect {
    case menu, turn, win, loose
}

/// AVFoundation wrapper — port of src/audio/sound.c.
class SoundManager {

    static let shared = SoundManager()

    private var menuPlayer: AVAudioPlayer?
    private var turnPlayer: AVAudioPlayer?
    private var winPlayer: AVAudioPlayer?
    private var loosePlayer: AVAudioPlayer?
    private var musicPlayer: AVAudioPlayer?

    /// Whether background music is currently playing.
    var isMusicPlaying: Bool { musicPlayer?.isPlaying ?? false }


    private init() {
        menuPlayer  = makePlayer(resource: "menu",  ext: "wav")
        turnPlayer  = makePlayer(resource: "turn",  ext: "wav")
        winPlayer   = makePlayer(resource: "win",   ext: "wav")
        loosePlayer = makePlayer(resource: "loose", ext: "wav")
        musicPlayer = makePlayer(resource: "music", ext: "m4a")
        musicPlayer?.numberOfLoops = -1   // loop forever
        startMusic()
    }

    func playEffect(_ effect: SoundEffect) {
        switch effect {
        case .menu:  fire(menuPlayer)
        case .turn:  fire(turnPlayer)
        case .win:   fire(winPlayer)
        case .loose: fire(loosePlayer)
        }
    }

    func startMusic()  { musicPlayer?.play()  }
    func stopMusic()   { musicPlayer?.stop()  }

    // MARK: - Private

    private func makePlayer(resource: String, ext: String) -> AVAudioPlayer? {
        guard let url = Bundle.main.url(forResource: resource, withExtension: ext) else {
            log.debug("Sound resource not found: \(resource).\(ext)")
            return nil
        }
        do {
            let p = try AVAudioPlayer(contentsOf: url)
            p.prepareToPlay()
            return p
        } catch {
            log.error("Failed to load \(resource).\(ext): \(error.localizedDescription)")
            return nil
        }
    }

    private func fire(_ player: AVAudioPlayer?) {
        guard let p = player else { return }
        if p.isPlaying { p.currentTime = 0 }
        p.play()
    }
}

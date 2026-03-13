// HighscoreManager.swift
// Persistent highscore tables — port of old_src/data/highscore.c
// Uses UserDefaults for storage. No SpriteKit or UIKit imports.

import Foundation

struct HighscoreEntry: Codable {
    var score: Int
    var name: String

    static let empty = HighscoreEntry(score: 0, name: "Nobody")
}

class HighscoreManager {

    // MARK: - Constants (from highscore.h)

    static let numberOfTables        = 10   // one per level
    static let entriesPerTable       = 8
    private static let userDefaultsKey = "QuadromaniHighscores"

    // MARK: - State

    private var tables: [[HighscoreEntry]]

    // MARK: - Singleton

    static let shared = HighscoreManager()

    private init() {
        // Pre-fill with empty entries
        tables = Array(
            repeating: Array(repeating: .empty, count: HighscoreManager.entriesPerTable),
            count: HighscoreManager.numberOfTables
        )
        load()
    }

    // MARK: - Public API

    /// All entries for a table (0-based level index).
    func entries(forTable table: Int) -> [HighscoreEntry] {
        guard table < HighscoreManager.numberOfTables else { return [] }
        return tables[table]
    }

    /// Returns the insertion index if `score` qualifies for the table, or nil.
    func position(forTable table: Int, score: Int) -> Int? {
        guard table < HighscoreManager.numberOfTables else { return nil }
        for (i, entry) in tables[table].enumerated() {
            if score > entry.score { return i }
        }
        return nil
    }

    /// Insert a score into the table at `position`, shifting lower entries down.
    func enterScore(_ score: Int, name: String, table: Int, at position: Int) {
        guard table < HighscoreManager.numberOfTables,
              position < HighscoreManager.entriesPerTable else { return }

        // Shift entries after the insertion point down by one (losing the last entry)
        if position < HighscoreManager.entriesPerTable - 1 {
            for i in stride(from: HighscoreManager.entriesPerTable - 1, through: position + 1, by: -1) {
                tables[table][i] = tables[table][i - 1]
            }
        }
        tables[table][position] = HighscoreEntry(score: score, name: name)
        save()
    }

    /// A name string built from the current timestamp ("YYYY-MM-DD HH:mm").
    var nameFromTimestamp: String {
        let f = DateFormatter()
        f.dateFormat = "yyyy-MM-dd HH:mm"
        return f.string(from: Date())
    }

    // MARK: - Persistence

    private func load() {
        guard let data = UserDefaults.standard.data(forKey: HighscoreManager.userDefaultsKey),
              let decoded = try? JSONDecoder().decode([[HighscoreEntry]].self, from: data),
              decoded.count == HighscoreManager.numberOfTables else { return }
        tables = decoded
    }

    private func save() {
        guard let data = try? JSONEncoder().encode(tables) else { return }
        UserDefaults.standard.set(data, forKey: HighscoreManager.userDefaultsKey)
    }
}

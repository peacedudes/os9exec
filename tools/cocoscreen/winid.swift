// winid.swift — print the CGWindowID of XRoar's emulator window.
//
// Companion to tools/cocoscreen.sh.  `screencapture -l <id>` needs a window
// ID, and there are no Python Quartz bindings on a stock macOS, so this does
// the CGWindowList lookup instead.
//
// XRoar publishes several windows (menu-bar strips and the like); the
// emulator display is the one whose kCGWindowName is exactly "XRoar".  Its ID
// changes every time XRoar restarts, so callers must re-query, never cache.
//
// Output: one tab-separated row per match — id, WxH, owner, name.
// The emulator window, when found, is printed first.
// Exit 0 if any window matched, 1 if none, 2 if the window list is unreadable.

import CoreGraphics
import Foundation

let target = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "xroar"

guard let windows = CGWindowListCopyWindowInfo([.optionAll], kCGNullWindowID) as? [[String: Any]] else {
    FileHandle.standardError.write("winid: cannot read the window list\n".data(using: .utf8)!)
    exit(2)
}

struct Match {
    let id: Int
    let width: Int
    let height: Int
    let owner: String
    let name: String
    /// The emulator display proper, as opposed to XRoar's auxiliary windows.
    var isDisplay: Bool { name == "XRoar" }
    var row: String { "\(id)\t\(width)x\(height)\t\(owner)\t\(name)" }
}

let matches: [Match] = windows.compactMap { window in
    let owner = window[kCGWindowOwnerName as String] as? String ?? ""
    let name = window[kCGWindowName as String] as? String ?? ""
    guard owner.localizedCaseInsensitiveContains(target)
            || name.localizedCaseInsensitiveContains(target) else { return nil }
    let bounds = window[kCGWindowBounds as String] as? [String: Any] ?? [:]
    return Match(id: window[kCGWindowNumber as String] as? Int ?? -1,
                 width: Int(bounds["Width"] as? Double ?? 0),
                 height: Int(bounds["Height"] as? Double ?? 0),
                 owner: owner,
                 name: name)
}

guard !matches.isEmpty else { exit(1) }

for match in matches.sorted(by: { $0.isDisplay && !$1.isDisplay }) {
    print(match.row)
}

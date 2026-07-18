// sendkey.swift — inject keystrokes into a running XRoar, by pid.
//
// Companion to tools/cocoscreen.sh.
//
// Why CGEvent and not AppleScript: System Events `key code` and `keystroke`
// both FAIL against XRoar.  They reach the process but arrive as the wrong
// key entirely (every attempt landed as a stray 'p' in the guest), and they
// steal window focus.  A real HID-level CGEvent posted with postToPid(_:)
// delivers correctly *and* needs no focus change, so it does not fight
// whoever is typing at the machine.
//
// XRoar maps host keys by position, so the host key at the US-layout location
// is what the emulated CoCo sees.  Notably the CoCo CLEAR key — which cycles
// between windows' screens under NitrOS-9 — is the host backtick.
//
// Usage:
//   sendkey <pid> key <name>...     press named keys in order
//   sendkey <pid> type <string>     type a literal ASCII string
//
// Exit 0 on success, 2 on a usage or unmappable-character error.

import CoreGraphics
import Foundation

// US-layout virtual keycodes for the characters XRoar can receive unshifted.
let unshifted: [Character: CGKeyCode] = [
    "a": 0, "s": 1, "d": 2, "f": 3, "h": 4, "g": 5, "z": 6, "x": 7, "c": 8,
    "v": 9, "b": 11, "q": 12, "w": 13, "e": 14, "r": 15, "y": 16, "t": 17,
    "1": 18, "2": 19, "3": 20, "4": 21, "6": 22, "5": 23, "=": 24, "9": 25,
    "7": 26, "-": 27, "8": 28, "0": 29, "]": 30, "o": 31, "u": 32, "[": 33,
    "i": 34, "p": 35, "l": 37, "j": 38, "'": 39, "k": 40, ";": 41, "\\": 42,
    ",": 43, "/": 44, "n": 45, "m": 46, ".": 47, "`": 50, " ": 49,
]

// Characters reachable only with Shift held, mapped to their base character.
let shifted: [Character: Character] = [
    "!": "1", "@": "2", "#": "3", "$": "4", "%": "5", "^": "6", "&": "7",
    "*": "8", "(": "9", ")": "0", "_": "-", "+": "=", "{": "[", "}": "]",
    "|": "\\", ":": ";", "\"": "'", "<": ",", ">": ".", "?": "/", "~": "`",
]

// Named keys, including the CoCo-specific spellings worth having aliases for.
let named: [String: CGKeyCode] = [
    "clear": 50, "backtick": 50, "grave": 50,   // CoCo CLEAR — cycles screens
    "enter": 36, "return": 36, "cr": 36,
    "space": 49, "tab": 48, "esc": 53, "escape": 53,
    "break": 53,                                 // CoCo BREAK
    "backspace": 51, "delete": 51,
    "left": 123, "right": 124, "down": 125, "up": 126,
]

let holdMillis: UInt32 = 40      // long enough for XRoar to sample the matrix
let gapMillis: UInt32 = 60       // between successive keys

guard let source = CGEventSource(stateID: .hidSystemState) else {
    FileHandle.standardError.write("sendkey: cannot create an event source\n".data(using: .utf8)!)
    exit(2)
}

func press(_ code: CGKeyCode, shift: Bool = false, to pid: pid_t) {
    guard let down = CGEvent(keyboardEventSource: source, virtualKey: code, keyDown: true),
          let up = CGEvent(keyboardEventSource: source, virtualKey: code, keyDown: false) else { return }
    if shift {
        down.flags = .maskShift
        up.flags = .maskShift
    }
    down.postToPid(pid)
    usleep(holdMillis * 1000)
    up.postToPid(pid)
    usleep(gapMillis * 1000)
}

func fail(_ message: String) -> Never {
    FileHandle.standardError.write("sendkey: \(message)\n".data(using: .utf8)!)
    exit(2)
}

let arguments = CommandLine.arguments
guard arguments.count >= 3, let pid = pid_t(arguments[1]) else {
    fail("usage: sendkey <pid> key <name>... | sendkey <pid> type <string>")
}

switch arguments[2] {
case "key":
    guard arguments.count > 3 else { fail("`key` needs at least one key name") }
    for name in arguments[3...] {
        guard let code = named[name.lowercased()] ?? unshifted[Character(name.lowercased())] else {
            fail("unknown key name: \(name)")
        }
        press(code, to: pid)
    }

case "type":
    guard arguments.count > 3 else { fail("`type` needs a string") }
    for character in arguments[3...].joined(separator: " ") {
        let lowered = Character(character.lowercased())
        if let code = unshifted[lowered] {
            press(code, shift: character.isUppercase, to: pid)
        } else if let base = shifted[character], let code = unshifted[base] {
            press(code, shift: true, to: pid)
        } else if character == "\n" {
            press(36, to: pid)
        } else {
            fail("cannot type character: \(character)")
        }
    }

default:
    fail("unknown command \(arguments[2]) — expected `key` or `type`")
}

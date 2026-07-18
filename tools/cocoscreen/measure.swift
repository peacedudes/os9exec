// measure.swift — report where the drawn pixels actually are in a screenshot.
//
// Companion to tools/cocoscreen.sh.  Verifying graphics claims by eyeballing a
// PNG produces hedged numbers ("about half the width"); this turns the same
// screenshot into measurements that can be quoted.
//
// Treats the most common colour in the image as background, everything else as
// ink, and reports the ink bounding box plus the ink columns and rows.  For a
// pattern of vertical lines that gives their exact X positions; for a shape it
// gives exact extents.
//
// Usage:
//   measure <file.png> [--columns] [--rows]
//
// Output is plain key: value lines, easy to read from a shell.

import AppKit
import Foundation

let arguments = CommandLine.arguments
guard arguments.count >= 2 else {
    FileHandle.standardError.write("usage: measure <file.png> [--columns] [--rows]\n".data(using: .utf8)!)
    exit(2)
}
let wantColumns = arguments.contains("--columns")
let wantRows = arguments.contains("--rows")

guard let image = NSImage(contentsOfFile: arguments[1]),
      let tiff = image.tiffRepresentation,
      let bitmap = NSBitmapImageRep(data: tiff) else {
    FileHandle.standardError.write("measure: cannot read \(arguments[1])\n".data(using: .utf8)!)
    exit(2)
}

let width = bitmap.pixelsWide
let height = bitmap.pixelsHigh

/// Pack a pixel into a comparable key; exact RGB equality is what we want here
/// since the emulator renders flat palette colours.
func key(_ x: Int, _ y: Int) -> UInt32 {
    guard let colour = bitmap.colorAt(x: x, y: y) else { return 0 }
    let r = UInt32(colour.redComponent * 255)
    let g = UInt32(colour.greenComponent * 255)
    let b = UInt32(colour.blueComponent * 255)
    return (r << 16) | (g << 8) | b
}

var histogram: [UInt32: Int] = [:]
var pixels = [UInt32](repeating: 0, count: width * height)
for y in 0..<height {
    for x in 0..<width {
        let value = key(x, y)
        pixels[y * width + x] = value
        histogram[value, default: 0] += 1
    }
}
guard let background = histogram.max(by: { $0.value < $1.value })?.key else { exit(2) }

// The capture includes XRoar's title bar and window border, which are not the
// emulated screen.  Counting "everything that is not background" as ink
// therefore spans the whole image and measures nothing useful — so allow the
// ink colour to be named explicitly, and always print the histogram so the
// right colour can be picked.
var inkColour: UInt32?
if let index = arguments.firstIndex(of: "--color"), index + 1 < arguments.count {
    inkColour = UInt32(arguments[index + 1].replacingOccurrences(of: "#", with: ""), radix: 16)
}

print("top colours:")
for entry in histogram.sorted(by: { $0.value > $1.value }).prefix(5) {
    print(String(format: "  #%06X  %d px", entry.key, entry.value))
}

var minX = width, maxX = -1, minY = height, maxY = -1
var inkPerColumn = [Int](repeating: 0, count: width)
var inkPerRow = [Int](repeating: 0, count: height)
var inkTotal = 0

/// Ink is either one named colour, or "anything but the background".
func isInk(_ value: UInt32) -> Bool {
    if let wanted = inkColour { return value == wanted }
    return value != background
}

for y in 0..<height {
    for x in 0..<width where isInk(pixels[y * width + x]) {
        inkTotal += 1
        inkPerColumn[x] += 1
        inkPerRow[y] += 1
        minX = min(minX, x); maxX = max(maxX, x)
        minY = min(minY, y); maxY = max(maxY, y)
    }
}

print("image: \(width)x\(height)")
print(String(format: "background: #%06X", background))
print("ink pixels: \(inkTotal)")
guard inkTotal > 0 else { print("ink bbox: none"); exit(0) }
print("ink bbox: x \(minX)..\(maxX) (w \(maxX - minX + 1)), y \(minY)..\(maxY) (h \(maxY - minY + 1))")

/// Collapse runs of adjacent marked indices into "start-end" spans, so a set of
/// vertical lines reads as a short list of positions rather than raw columns.
func spans(_ counts: [Int], threshold: Int) -> [String] {
    var result: [String] = []
    var start: Int?
    for index in counts.indices {
        if counts[index] >= threshold {
            if start == nil { start = index }
        } else if let begin = start {
            result.append(begin == index - 1 ? "\(begin)" : "\(begin)-\(index - 1)")
            start = nil
        }
    }
    if let begin = start {
        result.append(begin == counts.count - 1 ? "\(begin)" : "\(begin)-\(counts.count - 1)")
    }
    return result
}

if wantColumns {
    print("ink columns: \(spans(inkPerColumn, threshold: max(1, height / 20)).joined(separator: " "))")
}
if wantRows {
    print("ink rows: \(spans(inkPerRow, threshold: max(1, width / 20)).joined(separator: " "))")
}

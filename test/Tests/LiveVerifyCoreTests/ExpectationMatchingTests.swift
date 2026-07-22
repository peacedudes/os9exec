import XCTest
@testable import LiveVerifyCore

final class ExpectationMatchingTests: XCTestCase {

    // MARK: normalizeTranscript

    func testCollapsesCRLFPairToSingleNewline() {
        XCTAssertEqual(normalizeTranscript("line one\r\nline two\r\n"),
                        "line one\nline two\n")
    }

    func testLoneCRAlsoBecomesNewline() {
        // A bare CR (no paired LF) must still normalize, not survive as a
        // literal carriage return that would confuse a $-anchored regex.
        XCTAssertEqual(normalizeTranscript("a\rb"), "a\nb")
    }

    func testPlainLFIsLeftAlone() {
        XCTAssertEqual(normalizeTranscript("already\nfine\n"), "already\nfine\n")
    }

    // MARK: matches

    func testContainsMatchesSubstring() {
        let e = Expectation(contains: "PASS F$Time", matches: nil)
        XCTAssertTrue(matches(e, in: "noise\nPASS F$Time year-byte=123\nmore"))
    }

    func testContainsFailsWhenAbsent() {
        let e = Expectation(contains: "PASS F$Time", matches: nil)
        XCTAssertFalse(matches(e, in: "FAIL F$Time err=216"))
    }

    func testMatchesAppliesRegex() {
        let e = Expectation(contains: nil, matches: #"F\$ID raw d0/d1/d2: \d+\n 0\n 128"#)
        XCTAssertTrue(matches(e, in: "F$ID raw d0/d1/d2: 3\n 0\n 128\nPASS F$Time"))
    }

    func testMatchesRegexFailsOnWrongValue() {
        let e = Expectation(contains: nil, matches: #"F\$ID raw d0/d1/d2: \d+\n 0\n 128"#)
        // Priority printed as 129, not the doc-settled default of 128.
        XCTAssertFalse(matches(e, in: "F$ID raw d0/d1/d2: 3\n 0\n 129\n"))
    }

    // MARK: evaluate (AND semantics)

    func testEvaluateRequiresEveryExpectationToMatch() {
        let expectations = [
            Expectation(contains: "PASS A", matches: nil),
            Expectation(contains: "PASS B", matches: nil),
        ]
        XCTAssertTrue(evaluate(expectations, against: "PASS A\nPASS B\n"))
        XCTAssertFalse(evaluate(expectations, against: "PASS A\nFAIL B\n"))
    }

    // MARK: validate (manifest self-check, no emulator)

    func testValidateRejectsEmptyContainsPattern() {
        let entry = ManifestEntry(id: "bad", category: .solo,
                                  sources: [SourceFile(file: "x.a", moduleName: "x", primary: true)],
                                  roles: nil,
                                  expect: [Expectation(contains: "", matches: nil)],
                                  notes: "n")
        XCTAssertThrowsError(try validate(entry))
    }

    func testValidateRejectsInvalidRegex() {
        let entry = ManifestEntry(id: "bad", category: .solo,
                                  sources: [SourceFile(file: "x.a", moduleName: "x", primary: true)],
                                  roles: nil,
                                  expect: [Expectation(contains: nil, matches: "(unclosed")],
                                  notes: "n")
        XCTAssertThrowsError(try validate(entry))
    }

    func testValidateRejectsExpectationWithNeitherField() {
        let entry = ManifestEntry(id: "bad", category: .solo,
                                  sources: [SourceFile(file: "x.a", moduleName: "x", primary: true)],
                                  roles: nil,
                                  expect: [Expectation(contains: nil, matches: nil)],
                                  notes: "n")
        XCTAssertThrowsError(try validate(entry))
    }

    func testValidateRejectsSoloEntryWithNoPrimarySource() {
        let entry = ManifestEntry(id: "bad", category: .solo,
                                  sources: [SourceFile(file: "x.a", moduleName: "x", primary: false)],
                                  roles: nil,
                                  expect: [Expectation(contains: "ok", matches: nil)],
                                  notes: "n")
        XCTAssertThrowsError(try validate(entry))
    }

    func testValidateRejectsChoreographyEntryWithNoRoles() {
        let entry = ManifestEntry(id: "bad", category: .choreography,
                                  sources: [SourceFile(file: "x.bas", moduleName: "x", primary: true)],
                                  roles: nil,
                                  expect: [Expectation(contains: "ok", matches: nil)],
                                  notes: "n")
        XCTAssertThrowsError(try validate(entry))
    }

    func testValidateRejectsRoleWithNoMatchingSource() {
        let entry = ManifestEntry(id: "bad", category: .choreography,
                                  sources: [SourceFile(file: "x.bas", moduleName: "x", primary: true)],
                                  roles: [Role(moduleName: "nonexistent", delayBeforeStart: 0, background: false)],
                                  expect: [Expectation(contains: "ok", matches: nil)],
                                  notes: "n")
        XCTAssertThrowsError(try validate(entry))
    }

    func testValidateAcceptsAWellFormedSoloEntry() throws {
        let entry = ManifestEntry(id: "good", category: .solo,
                                  sources: [SourceFile(file: "x.a", moduleName: "x", primary: true)],
                                  roles: nil,
                                  expect: [Expectation(contains: "PASS", matches: nil)],
                                  notes: "n")
        XCTAssertNoThrow(try validate(entry))
    }
}

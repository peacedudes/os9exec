import XCTest
@testable import LiveVerifyCore

final class SoloExecutorTests: XCTestCase {

    var repoRoot: URL {
        URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().deletingLastPathComponent()
    }

    var fixturesDir: URL {
        Bundle.module.url(forResource: "Fixtures", withExtension: nil)!
    }

    func makeExecutor() throws -> SoloExecutor {
        SoloExecutor(runner: try OS9Runner(repoRoot: repoRoot), corpusDir: fixturesDir)
    }

    func makeScratch() throws -> URL {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("liveverify-soloexec-\(UUID().uuidString.prefix(8))")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        return dir
    }

    func testAssemblesLinksAndRunsAnAsmEntry() throws {
        let entry = ManifestEntry(
            id: "f-id-time-cmpnam", category: .solo,
            sources: [SourceFile(file: "batch1-01.a", moduleName: "batch101", primary: true)],
            roles: nil,
            expect: [
                Expectation(contains: "PASS F$Time year-byte=", matches: nil),
                Expectation(contains: "PASS F$CmpNam identical strings match", matches: nil),
                Expectation(contains: nil, matches: #"F\$ID raw d0/d1/d2: \d+\n 0\n 128"#),
            ],
            notes: "test fixture")
        let result = try makeExecutor().run(entry, scratchDir: try makeScratch(), timeout: 30)
        XCTAssertEqual(result, .pass)
    }

    func testCompilesCWithAnAsmDependencyAndRuns() throws {
        let entry = ManifestEntry(
            id: "osfork-bare-modname", category: .solo,
            sources: [
                SourceFile(file: "childprg68k.a", moduleName: "childprg68k", primary: false),
                SourceFile(file: "dogfood-osfork-modname.c", moduleName: "osfork", primary: true),
            ],
            roles: nil,
            expect: [
                Expectation(contains: "bare name 'childprg68k' forked ok", matches: nil),
                Expectation(contains: "status=77", matches: nil),
            ],
            notes: "test fixture")
        let result = try makeExecutor().run(entry, scratchDir: try makeScratch(), timeout: 60)
        XCTAssertEqual(result, .pass)
    }

    func testRunsABasEntryOnARamDisk() throws {
        let entry = ManifestEntry(
            id: "eoflock-deadlock", category: .solo,
            sources: [SourceFile(file: "dogfood-eoflock-deadlock.bas", moduleName: "eofdlk", primary: true)],
            roles: nil,
            expect: [Expectation(contains: "eofdlk: OK deadlock refused err=254", matches: nil)],
            notes: "test fixture")
        let result = try makeExecutor().run(entry, scratchDir: try makeScratch(), timeout: 30)
        XCTAssertEqual(result, .pass)
    }

    func testRunsAnExistingShRepro() throws {
        let entry = ManifestEntry(
            id: "bmode-repro", category: .solo,
            sources: [SourceFile(file: "bmode-repro.sh", moduleName: "bmode-repro.sh", primary: true)],
            roles: nil,
            expect: [Expectation(contains: "PASS", matches: nil)],
            notes: "test fixture")
        let result = try makeExecutor().run(entry, scratchDir: try makeScratch(), timeout: 60)
        XCTAssertEqual(result, .pass)
    }

    func testFailsCleanlyWhenExpectationDoesNotHold() throws {
        // Same real entry as above, but with a wrong expected priority
        // value (129 instead of the doc-settled 128) -- the "prove it can
        // fail" case for this executor.
        let entry = ManifestEntry(
            id: "f-id-time-cmpnam-broken", category: .solo,
            sources: [SourceFile(file: "batch1-01.a", moduleName: "batch101b", primary: true)],
            roles: nil,
            expect: [Expectation(contains: nil, matches: #"F\$ID raw d0/d1/d2: \d+\n 0\n 129"#)],
            notes: "deliberately wrong, proves the executor can fail")
        let result = try makeExecutor().run(entry, scratchDir: try makeScratch(), timeout: 30)
        if case .fail = result {} else { XCTFail("expected .fail, got \(result)") }
    }
}

extension ExecutionResult: Equatable {
    public static func == (lhs: ExecutionResult, rhs: ExecutionResult) -> Bool {
        switch (lhs, rhs) {
        case (.pass, .pass), (.timedOut, .timedOut): return true
        case (.fail, .fail), (.buildFailed, .buildFailed): return true
        default: return false
        }
    }
}

import XCTest
@testable import LiveVerifyCore

final class ChoreographyExecutorTests: XCTestCase {

    var repoRoot: URL {
        URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().deletingLastPathComponent()
    }

    var fixturesDir: URL {
        Bundle.module.url(forResource: "Fixtures", withExtension: nil)!
    }

    func makeExecutor() throws -> ChoreographyExecutor {
        ChoreographyExecutor(runner: try OS9Runner(repoRoot: repoRoot), corpusDir: fixturesDir)
    }

    func makeScratch() throws -> URL {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("liveverify-choreo-\(UUID().uuidString.prefix(8))")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        return dir
    }

    let lostupdateEntry = ManifestEntry(
        id: "lostupdate-trio", category: .choreography,
        sources: [
            SourceFile(file: "dogfood-lostupdate-init.bas", moduleName: "lostinit", primary: true),
            SourceFile(file: "dogfood-lostupdate-incrementer.bas", moduleName: "lostinc", primary: false),
            SourceFile(file: "dogfood-lostupdate-verify.bas", moduleName: "lostver", primary: false),
        ],
        roles: [
            Role(moduleName: "lostinit", delayBeforeStart: 0, background: false),
            Role(moduleName: "lostinc", delayBeforeStart: 0, background: true),
            Role(moduleName: "lostinc", delayBeforeStart: 0, background: true),
            Role(moduleName: "lostver", delayBeforeStart: 3, background: false),
        ],
        expect: [Expectation(contains: "lostver: final count=600", matches: nil)],
        notes: "RBF automatic record locking under concurrent read-modify-write")

    func testLostUpdateTrioShowsNoLostIncrements() throws {
        let result = try makeExecutor().run(lostupdateEntry, scratchDir: try makeScratch(), timeout: 60)
        XCTAssertEqual(result, .pass)
    }

    func testFailsCleanlyWhenExpectationDoesNotHold() throws {
        let broken = ManifestEntry(id: "lostupdate-trio-broken", category: .choreography,
                                   sources: lostupdateEntry.sources, roles: lostupdateEntry.roles,
                                   expect: [Expectation(contains: "lostver: final count=599", matches: nil)],
                                   notes: "deliberately wrong, proves the executor can fail")
        let result = try makeExecutor().run(broken, scratchDir: try makeScratch(), timeout: 60)
        if case .fail = result {} else { XCTFail("expected .fail, got \(result)") }
    }
}

import XCTest
@testable import LiveVerifyCore

final class OS9RunnerTests: XCTestCase {

    var repoRoot: URL {
        URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent()  // LiveVerifyCoreTests/
            .deletingLastPathComponent()  // Tests/
            .deletingLastPathComponent()  // test/
            .deletingLastPathComponent()  // repo root
    }

    func makeScratch() throws -> URL {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("liveverify-runnertest-\(UUID().uuidString.prefix(8))")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        return dir
    }

    func testRunsASingleImmediateCommand() throws {
        let runner = try OS9Runner(repoRoot: repoRoot)
        let scratch = try makeScratch()
        let outcome = runner.run(staged: [StagedInput(text: "echo HELLO_LIVEVERIFY\n", delay: 0)],
                                 scratchDir: scratch, timeout: 15)
        XCTAssertFalse(outcome.timedOut)
        XCTAssertTrue(outcome.transcript.contains("HELLO_LIVEVERIFY"))
    }

    func testStagedInputIsSentInOrderWithRealDelay() throws {
        let runner = try OS9Runner(repoRoot: repoRoot)
        let scratch = try makeScratch()
        let staged = [
            StagedInput(text: "echo FIRST\n", delay: 0),
            StagedInput(text: "echo SECOND\n", delay: 1),
        ]
        let start = Date()
        let outcome = runner.run(staged: staged, scratchDir: scratch, timeout: 15)
        XCTAssertGreaterThanOrEqual(Date().timeIntervalSince(start), 1.0)
        let firstIndex = outcome.transcript.range(of: "FIRST")?.lowerBound
        let secondIndex = outcome.transcript.range(of: "SECOND")?.lowerBound
        XCTAssertNotNil(firstIndex)
        XCTAssertNotNil(secondIndex)
        XCTAssertLessThan(firstIndex!, secondIndex!)
    }

    func testTimeoutIsReportedDistinctlyFromNormalCompletion() throws {
        let runner = try OS9Runner(repoRoot: repoRoot)
        let scratch = try makeScratch()
        // run() always appends ESC after the staged input (see its own doc
        // comment), so a real command reliably finishes given enough time --
        // an unreasonably short timeout (os9exec's own cold start alone
        // takes longer than this) is what forces a timeout here, not
        // anything about the staged content itself.
        let outcome = runner.run(staged: [StagedInput(text: "echo HELLO\n", delay: 0)],
                                 scratchDir: scratch, timeout: 0.01)
        XCTAssertTrue(outcome.timedOut)
    }
}

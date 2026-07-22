import XCTest
@testable import LiveVerifyCore

final class ManifestTests: XCTestCase {

    func testDecodesASoloEntry() throws {
        let json = """
        {
          "entries": [
            {
              "id": "f-id-time-cmpnam",
              "category": "solo",
              "sources": [
                {"file": "batch1-01.a", "moduleName": "batch101", "primary": true}
              ],
              "expect": [
                {"contains": "PASS F$Time year-byte="},
                {"matches": "F\\\\$ID raw d0/d1/d2: \\\\d+\\\\n 0\\\\n 128"}
              ],
              "notes": "F$ID/F$Time/F$CmpNam per 68k/syscall-reference.md"
            }
          ]
        }
        """
        let manifest = try JSONDecoder().decode(Manifest.self, from: Data(json.utf8))
        XCTAssertEqual(manifest.entries.count, 1)
        let entry = manifest.entries[0]
        XCTAssertEqual(entry.id, "f-id-time-cmpnam")
        XCTAssertEqual(entry.category, .solo)
        XCTAssertEqual(entry.sources.count, 1)
        XCTAssertEqual(entry.sources[0].file, "batch1-01.a")
        XCTAssertEqual(entry.sources[0].moduleName, "batch101")
        XCTAssertTrue(entry.sources[0].primary)
        XCTAssertEqual(entry.expect.count, 2)
        XCTAssertEqual(entry.expect[0].contains, "PASS F$Time year-byte=")
        XCTAssertNil(entry.expect[0].matches)
        XCTAssertNil(entry.roles)
    }

    func testDecodesAChoreographyEntryWithRoles() throws {
        let json = """
        {
          "entries": [
            {
              "id": "lostupdate-trio",
              "category": "choreography",
              "sources": [
                {"file": "dogfood-lostupdate-init.bas", "moduleName": "lostinit", "primary": true},
                {"file": "dogfood-lostupdate-incrementer.bas", "moduleName": "lostinc", "primary": false},
                {"file": "dogfood-lostupdate-verify.bas", "moduleName": "lostver", "primary": false}
              ],
              "roles": [
                {"moduleName": "lostinit", "delayBeforeStart": 0, "background": false},
                {"moduleName": "lostinc", "delayBeforeStart": 0, "background": true},
                {"moduleName": "lostinc", "delayBeforeStart": 0, "background": true},
                {"moduleName": "lostver", "delayBeforeStart": 3, "background": false}
              ],
              "expect": [{"contains": "lostver: final count=600"}],
              "notes": "RBF automatic record locking under a lost-update race"
            }
          ]
        }
        """
        let manifest = try JSONDecoder().decode(Manifest.self, from: Data(json.utf8))
        let entry = manifest.entries[0]
        XCTAssertEqual(entry.category, .choreography)
        XCTAssertEqual(entry.roles?.count, 4)
        XCTAssertEqual(entry.roles?[1].moduleName, "lostinc")
        XCTAssertEqual(entry.roles?[1].background, true)
        XCTAssertEqual(entry.roles?[3].delayBeforeStart, 3)
    }

    func testMissingRequiredFieldFailsToDecode() {
        let json = """
        {"entries": [{"id": "bad", "category": "solo", "sources": [], "notes": "x"}]}
        """
        XCTAssertThrowsError(try JSONDecoder().decode(Manifest.self, from: Data(json.utf8)))
    }
}

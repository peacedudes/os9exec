// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "OS9Tests",
    platforms: [.macOS(.v14)],
    targets: [
        .executableTarget(
            name: "OS9Tests",
            path: "Sources/OS9Tests"
        ),
        .target(
            name: "RBFHammerCore",
            path: "Sources/RBFHammerCore"
        ),
        .testTarget(
            name: "RBFHammerCoreTests",
            dependencies: ["RBFHammerCore"],
            path: "Tests/RBFHammerCoreTests",
            resources: [.copy("Fixtures")]
        ),
        .testTarget(
            name: "RBFHammerScenarioTests",
            dependencies: ["RBFHammerCore"],
            path: "Tests/RBFHammerScenarioTests"
        ),
        // The 6809 scenarios are a separate target because they are far slower
        // than the 68k ones -- a real emulated CoCo3 boots for about forty
        // seconds per run -- and because they need XRoar, ToolShed and the
        // NitrOS-9 image, none of which the 68k side requires.
        .testTarget(
            name: "RBFHammer6809Tests",
            dependencies: ["RBFHammerCore"],
            path: "Tests/RBFHammer6809Tests"
        )
    ]
)

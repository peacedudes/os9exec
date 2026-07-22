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
        // The soak/gate CLI: runs the hammer scenarios in a parallel pool, once
        // (--gate) or for many iterations (--soak). `swift run RBFHammer ...`.
        .executableTarget(
            name: "RBFHammer",
            dependencies: ["RBFHammerCore"],
            path: "Sources/RBFHammer"
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
        ),
        .target(
            name: "LiveVerifyCore",
            path: "Sources/LiveVerifyCore"
        ),
        .executableTarget(
            name: "LiveVerify",
            dependencies: ["LiveVerifyCore"],
            path: "Sources/LiveVerify"
        ),
        .testTarget(
            name: "LiveVerifyCoreTests",
            dependencies: ["LiveVerifyCore"],
            path: "Tests/LiveVerifyCoreTests"
        )
    ]
)

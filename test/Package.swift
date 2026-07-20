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
        )
    ]
)

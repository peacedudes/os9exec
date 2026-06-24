// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "OS9Tests",
    platforms: [.macOS(.v14)],
    targets: [
        .executableTarget(
            name: "OS9Tests",
            path: "Sources/OS9Tests"
        )
    ]
)

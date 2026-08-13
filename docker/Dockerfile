# Build stage
FROM ubuntu:24.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source
COPY . .

# Build os9exec
RUN make clean && make

# Runtime stage
FROM ubuntu:24.04

# Install runtime dependencies (minimal)
RUN apt-get update && apt-get install -y \
    libc6 \
    && rm -rf /var/lib/apt/lists/*

# Copy binary from builder
COPY --from=builder /build/os9exec /usr/local/bin/os9exec

# Run from a real directory, NOT from "/", and keep RBF IMAGE FILES out of the
# container root -- `mount -k=500K h9` at "/" reports success and the device is
# then unusable (`dir /h9` -> E_MNF).
#
# The reason is NOT that the host and OS-9 namespaces collide, which is what
# this comment used to say; that was measured wrong (2026-08-13). An image file
# whose parent directory is "/" is unusable from ANY working directory --
# `OS9DISK=/disk.img` fails just as `mount -k` at "/" does, while the same image
# one directory down works. A host DIRECTORY at the root is unaffected, which is
# why `-v /your/os9:/dd` (below) is fine and why this hid for so long.
#
# Two layers, both traced: AdjustPath's backward search for an existing ancestor
# (linuxfiles.c) writes NUL over the LEADING '/', so the prefix becomes "" and
# the root is never tried as an anchor; and even past that, GetRBFName
# (utilstuff.c) refuses a path that IS exactly the image file with E$FNA. Full
# write-up in ROADMAP-68k.md.
#
# So: mount a DIRECTORY of OS-9 files at /dd, or put an image file anywhere but
# the container root.
WORKDIR /work

# os9exec looks for ./dd by default
# User mounts their OS-9 binaries at runtime:
#   docker run -v /path/to/your/os9:/dd -it os9exec

ENTRYPOINT ["os9exec"]

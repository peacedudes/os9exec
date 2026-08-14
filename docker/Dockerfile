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

# Runs from a real directory rather than "/". This is now a tidiness choice --
# it keeps scratch disk images out of the container root -- and no longer a
# workaround: an RBF image sitting directly in "/" used to be unusable from any
# working directory (`OS9DISK=/disk.img` found nothing, `mount -k` at "/"
# produced a device that then failed E$MNF), which is what this line originally
# dodged. Fixed 2026-08-13; three root-directory blind spots, see ROADMAP-68k.md.
WORKDIR /work

# os9exec looks for ./dd by default
# User mounts their OS-9 binaries at runtime:
#   docker run -v /path/to/your/os9:/dd -it os9exec

ENTRYPOINT ["os9exec"]

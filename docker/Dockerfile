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

# Run from a real directory, NOT from "/". os9exec resolves an hX device by
# looking for a file of that name next to its working directory, so at "/" the
# image's host path ("/h9") is textually identical to the OS-9 device path
# ("/h9") -- the two namespaces collide and the device never registers:
# `mount -k=500K h9` reports success, then `dir /h9` fails with E_MNF. Anywhere
# else it works. Also keeps scratch disk images out of the container root.
WORKDIR /work

# os9exec looks for ./dd by default
# User mounts their OS-9 binaries at runtime:
#   docker run -v /path/to/your/os9:/dd -it os9exec

ENTRYPOINT ["os9exec"]

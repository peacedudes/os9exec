# Build stage
FROM ubuntu:24.04 as builder

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

# os9exec looks for ./dd by default
# User mounts their OS-9 binaries at runtime:
#   docker run -v /path/to/your/os9:/dd -it os9exec

ENTRYPOINT ["os9exec"]

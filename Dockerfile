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

WORKDIR /os9exec

# Copy binary from builder
COPY --from=builder /build/os9exec /usr/local/bin/os9exec

# Copy disk image and sample directory
COPY --from=builder /build/dd ./dd
COPY --from=builder /build/h0 ./h0

# Set environment
ENV OS9DISK=/os9exec/dd

# Default to shell (uses OS-9 PATH to find it)
ENTRYPOINT ["os9exec"]
CMD ["shell"]

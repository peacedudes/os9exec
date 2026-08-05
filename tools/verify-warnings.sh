#!/usr/bin/env bash
# verify-warnings.sh -- `make warnings`, but with a pass/fail exit status.
#
# `make warnings` prints per-toolchain scores for a human to read; it exits 0
# whatever they say, so it cannot gate anything. This runs it, and fails if any
# leg reported a warning, an error, or NOT BUILT -- that last one matters,
# because a 0/0 from a leg that produced no binary is not a pass.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
out=$(cd "$REPO" && make warnings 2>&1) || { echo "$out"; exit 1; }
echo "$out"

if grep -q "NOT BUILT" <<<"$out"; then
    echo "verify-warnings: a leg produced no binary -- its score means nothing" >&2
    exit 1
fi
if grep -q "BUILD FAILED\|BUILD OR SETUP FAILED" <<<"$out"; then
    echo "verify-warnings: a leg failed to build" >&2
    exit 1
fi
# Every reported leg must be 0/0.
if grep -E "warnings: +[0-9]+ +errors: +[0-9]+" <<<"$out" | grep -qvE "warnings: +0 +errors: +0"; then
    echo "verify-warnings: a toolchain reported warnings or errors" >&2
    exit 1
fi
exit 0

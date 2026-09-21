#!/usr/bin/env bash
# coverage.sh -- which of os9exec the test suite actually executes.
#
# Worth knowing before trusting a green suite: 337 tests passing says nothing
# about the code they never enter. Measured 2026-09-21, first run:
# OS9exec_core is 17,379 lines, 4,899 of them never executed -- 71.8% covered.
# The whole binary reads much lower (23%) only because the UAE instruction
# tables are generated code for 68k opcodes no test uses; read the core figure.
#
#   tools/coverage.sh            report per file, least-covered first
#   tools/coverage.sh --dirty    measure the WORKING tree instead of HEAD
#
# Builds in a scratch export, never in the working tree: an instrumented
# binary must not become the ./os9exec another session is dogfooding
# (project memory `feedback_never-clean-the-dogfood-tree`).
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
WORK=${TMPDIR:-/tmp}/os9-cov-$$
DIRTY=no
[ "${1:-}" = "--dirty" ] && DIRTY=yes

command -v xcrun >/dev/null || { echo "needs xcrun (llvm-profdata, llvm-cov)" >&2; exit 2; }
[ -n "${OS9DISK:-}" ] || {
    echo "no OS9DISK -- the suite is what drives this, and it needs the system disk" >&2
    exit 2; }

trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/prof" || { echo "could not make a scratch tree"; exit 1; }

if [ "$DIRTY" = yes ]; then
    ( cd "$REPO" && COPYFILE_DISABLE=1 tar -cf - Source GNUmakefile tools test ) \
        | tar -x -C "$WORK" || { echo "could not copy the working tree"; exit 1; }
else
    ( cd "$REPO" && git archive HEAD ) | tar -x -C "$WORK" \
        || { echo "could not export HEAD"; exit 1; }
fi

cd "$WORK" || exit 1
echo "== building instrumented =="
make -B -j4 CC="cc -fprofile-instr-generate -fcoverage-mapping -g -O0" >/dev/null 2>&1 \
  || { echo "  build FAILED"; exit 1; }
[ -x ./os9exec ] || { echo "  no binary was produced"; exit 1; }

echo "== running the suite =="
LLVM_PROFILE_FILE="$WORK/prof/os9-%p.profraw" \
    swift run --package-path test OS9Tests > "$WORK/suite.out" 2>&1
grep -aE "^FAIL:|Results" "$WORK/suite.out" | sed 's/^/  /'

# The suite REPLACES the child environment, so LLVM_PROFILE_FILE reaches
# os9exec only because main.swift passes it through alongside ASAN_OPTIONS.
# When that passthrough was missing, 8 processes out of ~380 wrote a profile
# and the report looked plausible and was meaningless -- so insist on a
# count that could only come from the whole suite.
n=$(ls "$WORK"/prof/*.profraw 2>/dev/null | wc -l | tr -d ' ')
[ "${n:-0}" -ge 100 ] || {
    echo "  only ${n:-0} profiles for a 337-test run -- LLVM_PROFILE_FILE is not"
    echo "  reaching the emulator, so this measurement would be a fiction"
    exit 1; }
echo "  $n profiles"

xcrun llvm-profdata merge -sparse "$WORK"/prof/*.profraw -o "$WORK/cov.profdata" 2>/dev/null \
  || { echo "  could not merge profiles"; exit 1; }

echo
echo "== OS9exec_core, least covered first =="
xcrun llvm-cov report ./os9exec -instr-profile="$WORK/cov.profdata" 2>/dev/null \
  | awk '$1 ~ /OS9exec_core.*\.c$/ {gsub("%","",$10);
         printf "  %6.2f%%  %6d unexecuted  %s\n", $10, $9, $1}' | sort -n

xcrun llvm-cov report ./os9exec -instr-profile="$WORK/cov.profdata" 2>/dev/null \
  | awk '$1 ~ /OS9exec_core.*\.c$/ {tot+=$8; unc+=$9}
         END {printf "\n  OS9exec_core: %d lines, %d never executed (%.1f%% covered)\n",
              tot, unc, 100*(tot-unc)/tot}'

#!/bin/sh
# Run os9exec under Valgrind over a spread of OS-9 workloads and report every
# memory error found.
#
# Why this does not just reuse the Swift test harness: that harness captures the
# emulator's stderr and discards it (test/Sources/OS9Tests/main.swift), which is
# exactly where Valgrind writes. Pointing DOCKER_IMAGE at a Valgrind image would
# run all 129 tests and silently throw away every finding. So this drives the
# emulator directly and keeps stderr.
#
# Each scenario is a separate emulator run, because Valgrind reports leaks at
# process exit and one long shell session would attribute everything to itself.
#
# Usage:  tools/valgrind-sweep.sh            (builds the image if needed)
#         tools/valgrind-sweep.sh --rebuild  (force image rebuild)
set -eu

repo=$(cd "$(dirname "$0")/.." && pwd)
image=os9exec:valgrind
out=${TMPDIR:-/tmp}/os9exec-valgrind

if [ "${1:-}" = "--rebuild" ] || ! docker image inspect "$image" >/dev/null 2>&1; then
    echo "building $image ..."
    docker build -q -f "$repo/docker/Dockerfile.valgrind" -t "$image" "$repo" >/dev/null
fi

mkdir -p "$out"
rm -f "$out"/*.log

# Scenario name, then the OS-9 shell commands piped into it. Chosen to cover the
# subsystems bugs keep surfacing in -- RBF, the RAM disk, pipes, module
# loading/linking -- plus plain startup/exit as a control.
run_scenario() {
    name=$1; shift
    # One command per line, then ESC: the OS-9 shell does NOT exit on EOF, it
    # sits at its prompt forever. ESC-newline is how it is told to quit -- the
    # same thing the Swift harness sends (see `input` in main.swift). The
    # in-container `timeout` is a second belt: without it, one wedged scenario
    # stalls the whole sweep with no output and nothing to look at.
    { printf '%s\n' "$@"; printf '\033\n'; } | docker run --rm -i \
        -v "$repo/h0:/dd" \
        --entrypoint sh \
        "$image" -c '
            timeout 300 valgrind \
                --tool=memcheck \
                --leak-check=full \
                --track-origins=yes \
                --error-exitcode=0 \
                --log-fd=2 \
                /build/os9exec -r shell' \
        >"$out/$name.out" 2>"$out/$name.log" || true
    # Valgrind prints ERROR SUMMARY only on a clean exit. If it is missing, the
    # run was killed (timeout, signal) and the log is TRUNCATED -- counting
    # error lines in it would report a reassuring "0 errors" for a scenario that
    # never actually finished. Say so instead; a silent false clean is worse
    # than a loud failure.
    if ! grep -q "ERROR SUMMARY" "$out/$name.log"; then
        printf '  %-14s DID NOT FINISH (timeout or crash) -- see %s\n' "$name" "$name.log"
        return
    fi
    errs=$(awk '/ERROR SUMMARY/{print $4; exit}' "$out/$name.log")
    lost=$(awk '/definitely lost:/{gsub(/,/,"",$4); print $4; exit}' "$out/$name.log")
    printf '  %-14s errors: %-4s definitely-lost: %s bytes\n' "$name" "${errs:-?}" "${lost:-0}"
}

echo "=== valgrind sweep ==="
run_scenario startup   "echo hello"
run_scenario dir       "dir /dd" "dir /dd/CMDS"
run_scenario modules   "mdir" "mfree" "procs"
run_scenario ramdisk   "mount -r=200 /ram9" "dir /ram9" "unmount ram9"
run_scenario pipes     "dir /dd ! sort ! tee /nil"
run_scenario rbf       "mount -k=500 /h8" "dir /h8" "free /h8" "unmount h8"

# Aimed by the coverage pass (ROADMAP item 1): these subsystems are among the
# least-executed reachable code, so they are where an unexercised memory bug is
# most likely to still be hiding.
run_scenario move      "mount -r=200 /ram9" "makdir /ram9/D" "build /ram9/a" "x" "" \
                       "move /ram9/a /ram9/D/a" "move -w=/ram9/D /ram9/D/a" \
                       "dir /ram9/D" "unmount ram9"
run_scenario copy      "mount -r=200 /ram9" "build /ram9/src" "content" "" \
                       "copy /ram9/src /ram9/dst" "list /ram9/dst" \
                       "del /ram9/dst" "unmount ram9"
# dsave copies a whole directory tree, so point it at a small scratch dir, NOT
# at /dd -- dsaving all of /dd overflows a 200-block RAM disk (E$Full) and then
# grinds past the timeout under Valgrind without ever reporting.
run_scenario dsave     "makdir /dd/t_vgsrc" "build /dd/t_vgsrc/f1" "content" "" \
                       "mount -r=200 /ram9" "chd /dd/t_vgsrc" "dsave -ive /ram9" \
                       "dir /ram9" "chd /dd" "unmount ram9" \
                       "del /dd/t_vgsrc/f1" "deldir -q /dd/t_vgsrc"
run_scenario errors    "dir /nonexistent" "del /dd/nosuchfile" "list /dd/nope" \
                       "move /dd/nope /dd/nope2" "unmount nosuchdev"

echo
echo "logs in $out"

#!/usr/bin/env bash
# verify-sshvm.sh -- build and test os9exec on a Linux VM reached over ssh.
#
# The multipass sibling (verify-linuxvm.sh) covers the aarch64 Ubuntu guest.
# This one covers the machines that exist only as UTM images reached on a
# forwarded port -- sparc64 and riscv64, which are why the project owns any
# big-endian and any non-x86/non-ARM coverage at all. They were driven by hand
# for the 2026-08-26 fleet sweep; a hand-driven leg is one nobody repeats, so
# it is written down here (see project memory `feedback_recipes-live-in-tools`).
#
#   tools/verify-sshvm.sh <port> [label]
#
# Reaches root@localhost:<port> with ~/.ssh/os9exec-vms, the standing project
# key (see memory `vm-inventory-and-credentials`: sparc64 is 2225, riscv64
# 2223). Source is COPIED in, never mounted, so a VM run cannot write into the
# working tree.
#
# These guests are SLOW -- sparc64 has one vCPU and takes tens of minutes for a
# full build plus both conformance legs. Start them first and run them
# concurrently with everything else.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
PORT=${1:?usage: verify-sshvm.sh <ssh-port> [label]}
LABEL=${2:-port $PORT}
KEY=${OS9_VM_KEY:-$HOME/.ssh/os9exec-vms}

# TWO forms on purpose. `-n` points stdin at /dev/null, which is right for a
# probe and FATAL for the heredoc below: with it, the remote `sh -s` reads
# nothing, does nothing, and exits 0 -- the script then printed a header and
# stopped, which reads as a slow VM rather than as a broken invocation. Cost an
# hour of two guests compiling nothing.
SSH_OPTS="-o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
          -o ConnectTimeout=10 -i $KEY -p $PORT"
SSH="ssh -n $SSH_OPTS root@localhost"      # probes: no stdin
SSH_IN="ssh $SSH_OPTS root@localhost"      # the run: stdin IS the script

arch=$($SSH 'uname -m' 2>/dev/null) || { echo "$LABEL: unreachable on port $PORT"; exit 1; }
echo "== $LABEL ($arch) =="

# COPYFILE_DISABLE stops macOS tar writing AppleDouble headers that GNU tar
# then reports with a NON-ZERO exit -- which under set -e in the guest aborted
# the run silently. Same trap as the multipass script documents.
COPYFILE_DISABLE=1 tar -czf /tmp/os9-sshvm-$PORT.tgz -C "$REPO" \
    Source GNUmakefile tools test/68k-conformance 2>/dev/null \
  || { echo "  could not package the source"; exit 1; }

scp -q -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
    -i "$KEY" -P "$PORT" /tmp/os9-sshvm-$PORT.tgz root@localhost:/root/src.tgz \
  || { echo "  could not copy the source in"; exit 1; }

LOG=/tmp/os9-sshvm-$PORT.out
$SSH_IN 'sh -s' <<'REMOTE' 2>&1 | tee "$LOG"
set -u
rm -rf ~/os9 && mkdir -p ~/os9
tar -xzf ~/src.tgz -C ~/os9 2>/dev/null || true   # header warnings are not failures
cd ~/os9 || exit 1
[ -f GNUmakefile ] || { echo "  source did not unpack in the VM"; exit 1; }
rc=0

for CC in gcc clang; do
    command -v $CC >/dev/null 2>&1 || { echo "  $CC: not installed, skipped"; continue; }
    rm -rf /tmp/b-$CC; mkdir -p /tmp/b-$CC
    # grep -c EXITS 1 WHEN IT COUNTS ZERO, so `|| true` -- without it the check
    # could only fail when the build was clean.
    n=$(make -B CC=$CC OBJDIR=/tmp/b-$CC EXE=/tmp/b-$CC/os9exec prod 2>&1 | grep -cE "warning:" || true)
    echo "  $CC -O2: $n warnings"
    [ "$n" = 0 ] || rc=1
done

make -B CC=gcc prod >/dev/null 2>&1 || { echo "  build FAILED"; exit 1; }
./tools/conformance.sh 68k                 | tail -2 | sed "s/^/  /" || rc=1
./tools/conformance.sh 68k --noshell --rbf | tail -2 | sed "s/^/  /" || rc=1
exit $rc
REMOTE
rc=${PIPESTATUS[0]}
# A leg that produced NO recognisable result has not passed -- it has failed to
# run. Without this the silent-stdin bug above looked like success: a header,
# no output, exit 0. Teed rather than captured so a slow guest shows progress.
grep -qE "warnings|passed|PASS" "$LOG" \
  || { echo "  NO RESULT -- the guest produced no output; the leg did not run"; exit 1; }
exit $rc

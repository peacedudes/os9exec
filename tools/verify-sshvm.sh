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
#   tools/verify-sshvm.sh <port> [label]             run it and wait
#   tools/verify-sshvm.sh <port> [label] --detach    start it INSIDE the guest
#   tools/verify-sshvm.sh <port> [label] --collect   read the detached result
#
# Reaches root@localhost:<port> with ~/.ssh/os9exec-vms, the standing project
# key (see memory `vm-inventory-and-credentials`: sparc64 is 2225, riscv64
# 2223). Source is COPIED in, never mounted, so a VM run cannot write into the
# working tree.
#
# These guests are SLOW -- sparc64 has ONE vCPU (its `sun4u` machine type is
# capped there; the config.plist may say anything and QEMU then refuses to
# start) and `cpuemu.c` alone runs the better part of an hour. That is longer
# than an agent harness will hold a foreground command open, which is what
# --detach is for: the work runs under setsid writing to a log inside the
# guest, so killing the ssh client -- or this tool, or the whole session --
# cannot leave a build running whose output goes nowhere. That happened on
# 2026-09-01 and cost sparc64 two hours of compiling for no result.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
PORT=${1:?usage: verify-sshvm.sh <ssh-port> [label] [--detach|--collect]}
shift
MODE=run
LABEL="port $PORT"
for a in "$@"; do
    case "$a" in
        --detach)  MODE=detach ;;
        --collect) MODE=collect ;;
        *)         LABEL="$a" ;;
    esac
done
KEY=${OS9_VM_KEY:-$HOME/.ssh/os9exec-vms}

SSH_OPTS="-o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
          -o ConnectTimeout=10 -i $KEY -p $PORT"
SSH="ssh -n $SSH_OPTS root@localhost"

arch=$($SSH 'uname -m' 2>/dev/null) || { echo "$LABEL: unreachable on port $PORT"; exit 1; }
echo "== $LABEL ($arch) =="

LOG=/tmp/os9-sshvm-$PORT.out
GUEST_LOG=/root/leg.log

# ---------------------------------------------------------------- collect ---
# Reading a detached run's log. An unfinished run has no LEG_EXIT line: say so
# rather than scoring the partial output, which would read as a pass.
if [ "$MODE" = collect ]; then
    $SSH "cat $GUEST_LOG" > "$LOG" 2>/dev/null \
      || { echo "  no detached run found on this guest"; exit 1; }
    grep -v "LEG_EXIT=" "$LOG" | sed 's/^/  /'
    grep -q "LEG_EXIT=" "$LOG" || { echo "  STILL RUNNING -- no result yet"; exit 2; }
    grep -qE "warnings|passed|PASS" "$LOG" \
      || { echo "  NO RESULT -- the guest produced no output; the leg did not run"; exit 1; }
    exit "$(sed -n 's/^LEG_EXIT=//p' "$LOG" | tail -1)"
fi

# Refuse to start a second build beside a running one: two makes on a one-core
# guest are slower than either alone, and the logs would interleave. Check this
# BEFORE shipping 20MB of source into a guest that is already busy -- the ship
# is minutes of its slow disk, spent to then decline.
#
# Ask a PID FILE, not `pgrep -f`: the pattern form matched its own ssh command
# line -- the remote shell's arguments contain the very string searched for --
# and so reported a running leg on an idle guest. Same self-match that makes
# `pkill -f` kill its own shell.
if [ "$MODE" = detach ] \
   && $SSH '[ -f /root/leg.pid ] && kill -0 "$(cat /root/leg.pid)" 2>/dev/null'; then
    echo "  a leg is ALREADY RUNNING on this guest -- collect it, or let it finish"
    exit 1
fi

# ------------------------------------------------------------------- ship ---
# COPYFILE_DISABLE stops macOS tar writing AppleDouble headers that GNU tar
# then reports with a NON-ZERO exit -- which under set -e in the guest aborted
# the run silently. Same trap as the multipass script documents.
COPYFILE_DISABLE=1 tar -czf /tmp/os9-sshvm-$PORT.tgz -C "$REPO" \
    Source GNUmakefile tools test/68k-conformance 2>/dev/null \
  || { echo "  could not package the source"; exit 1; }

# The leg travels as a FILE rather than as a heredoc on the ssh client's stdin.
# The heredoc form needed an ssh without `-n`, and getting that wrong pointed
# the remote shell's stdin at /dev/null: it read nothing, did nothing, exited
# 0, and the leg printed a header and stopped -- which reads as a slow VM
# rather than a broken invocation, and cost two guests an hour of compiling
# nothing. A shipped file cannot go empty that quietly.
LEG=/tmp/os9-leg-$PORT.sh
cat > "$LEG" <<'REMOTE'
set -u
# The suite runs as an ORDINARY user (rdoggett, 2026-09-16): that is what the
# people who use os9exec do, and os9exec has super-user branches -- module
# ownership, SS_FD owner checks, RBF directory permissions -- that a root run
# takes everywhere and so never tests. Say who ran it, and refuse root.
echo "  running as $(id -un) (uid $(id -u))"
[ "$(id -u)" != 0 ] || { echo "  REFUSED: the leg must not run as root"; exit 1; }
rm -rf ~/os9 && mkdir -p ~/os9
tar -xzf ~/src.tgz -C ~/os9 2>/dev/null || true   # header warnings are not failures
cd ~/os9 || exit 1
[ -f GNUmakefile ] || { echo "  source did not unpack in the VM"; exit 1; }
rc=0

# A serial make leaves every core but one idle, and the build is most of this
# leg's wall clock. Parallel ONLY with --output-sync, though: without it two
# compilers interleave mid-line, a split "warning:" stops matching, and the
# count silently under-reports -- the check could then no longer fail. If make
# is too old to sync, build serially instead.
# sparc64's emulated IDE is this fleet's weak point: a build's sustained write
# traffic has twice broken it -- once tripping ext4's errors=remount-ro so the
# guest went READ-ONLY mid-leg, once hanging the guest outright for hours. So
# build in RAM when there is room for it. Fewer disk writes, and faster.
BUILDROOT=/tmp
shm_free=$(df -Pm /dev/shm 2>/dev/null | awk 'NR==2{print $4}')
[ "${shm_free:-0}" -ge 1024 ] 2>/dev/null && BUILDROOT=/dev/shm
echo "  building in $BUILDROOT (${shm_free:-0}MB free in /dev/shm)"

J=$(nproc 2>/dev/null || echo 1)
PAR="-j$J -O"
make --help 2>/dev/null | grep -q -- --output-sync \
  || { echo "  make has no --output-sync: building serially to keep the warning count honest"; PAR=""; }

for CC in gcc clang; do
    command -v $CC >/dev/null 2>&1 || { echo "  $CC: not installed, skipped"; continue; }
    rm -rf $BUILDROOT/b-$CC; mkdir -p $BUILDROOT/b-$CC
    # grep -c EXITS 1 WHEN IT COUNTS ZERO, so `|| true` -- without it the check
    # could only fail when the build was clean.
    n=$(make $PAR -B CC=$CC OBJDIR=$BUILDROOT/b-$CC EXE=$BUILDROOT/b-$CC/os9exec prod 2>&1 | grep -cE "warning:" || true)
    echo "  $CC -O2: $n warnings"
    [ "$n" = 0 ] || rc=1
    # A build that dies on ERRORS prints no "warning:" lines, so the count alone
    # scores it 0. gcc was rescued by the conformance build below; clang had no
    # rescue at all. The binary is the proof.
    [ -x $BUILDROOT/b-$CC/os9exec ] || { echo "  $CC: NOT BUILT -- the count above means nothing"; rc=1; }
done

# The gcc leg above already built `prod` with these very flags, just into
# $BUILDROOT/b-gcc. Rebuilding it from scratch costs a ONE-CORE sparc64 guest
# another hour for a byte-identical binary, so reuse it when it is there.
if [ -x $BUILDROOT/b-gcc/os9exec ]; then
    cp $BUILDROOT/b-gcc/os9exec ./os9exec || { echo "  could not place the gcc binary"; exit 1; }
else
    make $PAR -B CC=gcc prod >/dev/null 2>&1 || { echo "  build FAILED"; exit 1; }
fi
[ -x ./os9exec ] || { echo "  no os9exec binary to test"; exit 1; }

# A PIPELINE'S STATUS IS ITS LAST COMMAND'S. `conformance.sh | tail | sed ||
# rc=1` therefore tested `sed`, which always succeeds -- a divergence printed
# "CONFORMANCE: divergence reported above" and the leg still exited 0, so that
# check could not fail. Run it, keep ITS status, then show the evidence: the
# totals line as well as the verdict, which `tail -2` also cut off.
run_conf() {
    ./tools/conformance.sh 68k "$@" > /tmp/conf.$$ 2>&1
    st=$?
    grep -E "CONF68K totals|CONFORMANCE|no OS9DISK" /tmp/conf.$$ | sed "s/^/  /"
    rm -f /tmp/conf.$$
    return $st
}
run_conf                 || rc=1
run_conf --noshell --rbf || rc=1
exit $rc
REMOTE

scp -q -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
    -i "$KEY" -P "$PORT" /tmp/os9-sshvm-$PORT.tgz root@localhost:/root/src.tgz \
  || { echo "  could not copy the source in"; exit 1; }
scp -q -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
    -i "$KEY" -P "$PORT" "$LEG" root@localhost:/root/leg.sh \
  || { echo "  could not copy the leg script in"; exit 1; }

# Root only to PREPARE: make the ordinary account if the guest lacks one and
# hand it the source and the leg; the leg itself then runs as that account.
LEGUSER=os9test
PREP="id $LEGUSER >/dev/null 2>&1 || useradd -m -s /bin/sh $LEGUSER; \
      install -o $LEGUSER -m 644 /root/src.tgz /home/$LEGUSER/src.tgz && \
      install -o $LEGUSER -m 644 /root/leg.sh  /home/$LEGUSER/leg.sh"
$SSH "$PREP" || { echo "  could not prepare the $LEGUSER account"; exit 1; }
RUNLEG="runuser -u $LEGUSER -- sh /home/$LEGUSER/leg.sh"

# ----------------------------------------------------------------- detach ---
if [ "$MODE" = detach ]; then
    # -f backgrounds the client; setsid detaches the remote job from the ssh
    # session, so neither a killed client nor a closed session takes it down.
    #
    # The LOCAL redirect matters as much as the remote one. The backgrounded
    # client lives on for the whole leg holding this script's stdout, so
    # `--detach | tail` waited hours for an end-of-file and silently stalled
    # everything chained after it (2026-09-13). The exit status still reports
    # a client that could not start.
    ssh -n -f $SSH_OPTS root@localhost \
        "setsid sh -c 'echo \$\$ > /root/leg.pid; $RUNLEG > $GUEST_LOG 2>&1; \
                       echo LEG_EXIT=\$? >> $GUEST_LOG; rm -f /root/leg.pid' \
         < /dev/null > /dev/null 2>&1" > /dev/null 2>&1 \
      || { echo "  could not start the detached leg"; exit 1; }
    echo "  detached -- collect with: tools/verify-sshvm.sh $PORT --collect"
    exit 0
fi

# -------------------------------------------------------------------- run ---
$SSH "$RUNLEG" 2>&1 | tee "$LOG"
rc=${PIPESTATUS[0]}
# A leg that produced NO recognisable result has not passed -- it has failed to
# run. Teed rather than captured so a slow guest shows progress.
grep -qE "warnings|passed|PASS" "$LOG" \
  || { echo "  NO RESULT -- the guest produced no output; the leg did not run"; exit 1; }
exit $rc

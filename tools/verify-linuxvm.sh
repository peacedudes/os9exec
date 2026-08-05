#!/usr/bin/env bash
# verify-linuxvm.sh -- build and test os9exec on a real Linux VM.
#
# A multipass Ubuntu guest, not a container: its own kernel, its own glibc,
# hardware-virtualised. That distinction has already paid for itself -- glibc
# marks write()/fread() warn_unused_result where macOS and mingw do not, and
# the two warnings only this machine emitted led to an uninitialised RBF
# signature buffer and, by sweeping the class, to a global-buffer-overflow in
# egetenv that no compiler anywhere had ever mentioned.
#
# Builds with BOTH gcc and clang, because they disagree about what to warn
# about, and runs the conformance suite on a host directory and on a real RBF
# image. Source is COPIED in, never mounted: a VM run must not be able to
# write into the working tree.
#
# Setup (see project memory `vm-inventory-and-credentials`):
#   multipass launch 24.04 --name os9-ubuntu --cpus 4 --memory 4G --disk 20G
#   multipass exec os9-ubuntu -- sudo apt-get install -y build-essential clang
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
VM=${OS9_LINUX_VM:-os9-ubuntu}

command -v multipass >/dev/null 2>&1 || { echo "multipass not installed"; exit 1; }

state=$(multipass list 2>/dev/null | awk -v v="$VM" '$1==v {print $2}')
[ -n "$state" ] || { echo "no multipass VM named $VM"; exit 1; }
if [ "$state" != "Running" ]; then
    echo "starting $VM ..."
    multipass start "$VM" >/dev/null 2>&1 || { echo "could not start $VM"; exit 1; }
fi

multipass exec "$VM" -- sh -c 'command -v gcc >/dev/null && command -v clang >/dev/null' \
  || { echo "$VM lacks gcc and/or clang -- see the header of this script"; exit 1; }

# COPYFILE_DISABLE stops macOS tar writing AppleDouble/xattr headers that GNU
# tar then complains about -- and complains with a NON-ZERO exit, which under
# `set -e` in the guest aborted the whole run silently.
COPYFILE_DISABLE=1 tar -czf /tmp/os9-linuxvm-src.tgz -C "$REPO" \
    Source GNUmakefile tools test/68k-conformance 2>/dev/null \
  || { echo "could not package the source"; exit 1; }
multipass transfer /tmp/os9-linuxvm-src.tgz "$VM":/home/ubuntu/src.tgz >/dev/null 2>&1 \
  || { echo "could not copy the source to $VM"; exit 1; }

multipass exec "$VM" -- sh -c '
set -e
rm -rf ~/os9 && mkdir -p ~/os9
tar -xzf ~/src.tgz -C ~/os9 2>/dev/null || true   # header warnings are not failures
cd ~/os9
[ -f GNUmakefile ] || { echo "  source did not unpack in the VM"; exit 1; }
rc=0

# Both compilers, at the flags that ship. A warning on either is a failure:
# this VM exists precisely to see what the other platforms cannot.
for CC in gcc clang; do
    rm -rf /tmp/b-$CC; mkdir -p /tmp/b-$CC
    # `|| true` because grep -c EXITS 1 WHEN IT COUNTS ZERO. Without it, and
    # under set -e, this script aborted silently exactly when the build was
    # clean -- a check that could only fail when there was nothing wrong.
    n=$(make -B CC=$CC OBJDIR=/tmp/b-$CC EXE=/tmp/b-$CC/os9exec prod 2>&1 | grep -cE "warning:" || true)
    echo "  $CC -O2: $n warnings"
    [ "$n" = 0 ] || rc=1
done

# Then prove the thing RUNS here, on both device types.
make -B CC=gcc prod >/dev/null 2>&1
./tools/conformance.sh 68k              | tail -2 | sed "s/^/  /" || rc=1
./tools/conformance.sh 68k --noshell --rbf | tail -2 | sed "s/^/  /" || rc=1
exit $rc
' || exit 1

exit 0

#!/bin/sh
# Reproduce + verify that a host-native directory reports its own FD, so a
# ".."-walking getwd() can name the directory it is standing in.
#
# Runs the same guest program twice, changing only the file manager under the
# current directory:
#   host-native (/h5, a plain host directory) -- the case ported bash fails in
#   RBF (a `mount -k` image)                  -- the control, known to work
#
# Usage:  test/68k-live-verification/getwd-dirfd-repro.sh
# Exit 0 = both report PASS; 1 = the host-native case cannot identify itself.
# A FAIL in the RBF control means the harness is broken, not the emulator.
set -e

REPO=$(cd "$(dirname "$0")/../.." && pwd)
OS9EXEC="$REPO/os9exec"
DISK="${OS9DISK:?set OS9DISK to an OS-9 system disk}"
SRC="$(dirname "$0")/getwd-dirfd.a"
SCR=$(mktemp -d)
trap 'rm -rf "$SCR"' EXIT
TIMEOUT=$(command -v gtimeout || command -v timeout)

mkdir "$SCR/SUB"
cp "$SRC" "$SCR/getwdfd.a"
flip -m "$SCR/getwdfd.a"

# cd into the scratch dir so `mount -k`'s image lands there, not in the repo
cd "$SCR"

out=$( printf 'load /dd/CMDS/r68 /dd/CMDS/l68\nchd /h5\nr68 getwdfd.a -o=/h5/gwfd.r\nl68 /h5/gwfd.r -o=/h5/gwfd\nchd /h5/SUB\n/h5/gwfd\nmount -k=200K h8\nmakdir /h8/SUB\nchd /h8/SUB\n/h5/gwfd\nstop\n' \
       | "$TIMEOUT" 120 env OS9STOP=1 OS9DISK="$DISK" OS9H5="$SCR" "$OS9EXEC" -r /dd/CMDS/shell 2>&1 | tr '\r' '\n' )

lines=$( printf '%s\n' "$out" | grep -a 'GETWD-DIRFD' || true )
host=$( printf '%s\n' "$lines" | sed -n 1p )
rbf=$(  printf '%s\n' "$lines" | sed -n 2p )

echo "host-native: ${host:-(no output)}"
echo "RBF control: ${rbf:-(no output)}"

case "$rbf" in
    *PASS*) ;;
    *) echo "HARNESS BROKEN: the RBF control did not pass -- results are not evidence"
       printf '%s\n' "$out" | grep -aE 'Error|error' | head
       exit 1 ;;
esac

case "$host" in
    *PASS*) echo "PASS: a host-native directory reports an FD its parent's entry matches"
            exit 0 ;;
    *) echo "FAIL: a host-native directory cannot be identified from its parent"
       exit 1 ;;
esac

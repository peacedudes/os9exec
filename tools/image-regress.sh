#!/usr/bin/env bash
# image-regress.sh -- open every RBF disk image you have with an older
# os9exec and with this one, and report any image the two see differently.
#
#   tools/image-regress.sh <old-os9exec> <dir-to-search>...
#
# Why: a new validity check on data read from an image can refuse something
# older software legitimately wrote. The suite's images are all made fresh, so
# it cannot see that; only real old disks can. v4.1.0's sector-size check did
# exactly this (an image with no sector size recorded at $68 was refused), and
# a user's own disk found it, not a test.
#
# Each image is copied to a scratch directory and opened from there as /h9
# (a file named h9 in the working directory), so originals are only read. Both
# builds list the root with `dir -e` and run `dcheck`; the outputs are compared
# with the listing's clock time removed. Needs OS9DISK for the shell, dir and
# dcheck. Only images os9exec recognises (the OS-9/68000 "Cruz" mark at $60)
# can differ; the rest are reported as not opened by either.
set -u
old=${1:?usage: image-regress.sh <old-os9exec> <dir>...}; shift
[ $# -gt 0 ] || { echo "usage: image-regress.sh <old-os9exec> <dir>..." >&2; exit 2; }
repo=$(cd "$(dirname "$0")/.." && pwd)
new=$repo/os9exec
[ -n "${OS9DISK:-}" ] || { echo "OS9DISK is not set" >&2; exit 2; }
work=$(mktemp -d "${TMPDIR:-/tmp}/imgregress.XXXXXX") || exit 1
trap 'rm -rf "$work"' EXIT

run() {   # run <binary> <image> -> what the binary sees
    rm -f "$work/h9"; cp "$1" "$work/h9" 2>/dev/null || return
    printf 'dir -e /h9\ndcheck /h9\n\033\n' \
      | ( cd "$work" && env -i HOME="$HOME" PATH=/usr/bin:/bin OS9DISK="$OS9DISK" "$2" -r shell ) 2>&1 \
      | tr '\r' '\n' | grep -av '^#' | sed -E 's/(Directory of .*) [0-9:]+$/\1/'
}

n=0; differ=0; opened=0
while IFS= read -r -d '' img; do
    n=$((n+1))
    a=$(run "$img" "$old"); b=$(run "$img" "$new")
    grep -q 'Directory of' <<<"$b" && opened=$((opened+1))
    if [ "$a" != "$b" ]; then
        differ=$((differ+1))
        echo "DIFFERS: $img"
        diff <(echo "$a") <(echo "$b") | sed 's/^/    /' | head -12
    fi
done < <(find "$@" -type f -size +20k \( -iname '*.dsk' -o -iname '*.img' -o -iname '*.dd' \
                                         -o -iname '*.rbf' -o -name 'h[0-9a-z]' \) -print0)

echo "$n files, $opened opened as RBF disks, $differ seen differently"
[ "$differ" -eq 0 ]

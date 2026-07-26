#!/usr/bin/env bash
# verify-image.sh <image> -- host-side structural assertions on a built
# conformance image, using ToolShed as an oracle independent of the
# emulator that built it.
set -uo pipefail
IMG="${1:?usage: verify-image.sh <image>}"
OS9=${OS9:-/usr/local/bin/os9}
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SRCDIR="$REPO/test/6809-conformance"
fails=0
fail() { printf 'ASSERT-FAIL %s\n' "$*"; fails=$((fails+1)); }

[ -f "$IMG" ] || { printf 'ASSERT-FAIL image %s does not exist\n' "$IMG"; exit 1; }

listing=$("$OS9" dir -e "$IMG," 2>&1)

# Every required directory is present.
for d in CMDS SRC DOCS SCRATCH RESULTS REBUILT; do
    printf '%s\n' "$listing" | grep -aq "[[:space:]]$d\$" || fail "missing directory $d"
done

# Nothing on the image is owned by group 0. Microware defines the super user
# as ANY user in group zero, so a group-0 owner would ship privileged files.
if printf '%s\n' "$listing" | grep -aqE '^[[:space:]]*0\.'; then
    fail "found group-0 (super user) ownership in root listing"
fi

## Assertions added in Task 4, once the files they check exist.
## Until then this script must not assert them: a check for something no task
## has created yet fails for the wrong reason and teaches you to ignore it.

# Every staged module actually landed on the image, byte-for-byte. The OS-9
# shell does not abort a build procedure on a failed command -- a `copy`
# whose source is missing prints an Error # line and the procedure runs to
# completion regardless, leaving the destination silently absent. The
# BUILD-STRUCTURE-DONE marker in build.log proves only that the procedure
# ran, never that each step worked, so this compares the build's own
# manifest.txt (written as it staged each module) against what actually
# exists in CMDS/ on the finished image.
MANIFEST="$(dirname "$IMG")/manifest.txt"
if [ -f "$MANIFEST" ]; then
    cmds=$("$OS9" dir -e "$IMG,/CMDS" 2>&1)
    while read -r name bytes; do
        [ -z "$name" ] && continue
        line=$(printf '%s\n' "$cmds" | grep -aE "[[:space:]]${name}\$") \
            || { fail "manifest entry $name missing from CMDS/ on image"; continue; }
        got=$(printf '%s\n' "$line" | awk '{print $(NF-1)}')
        [ "$got" = "$bytes" ] \
            || fail "manifest entry $name size mismatch: image has $got bytes, manifest says $bytes"
    done < "$MANIFEST"
else
    fail "manifest not found at $MANIFEST"
fi

# The build log itself must show no failed command. A silently-failed copy
# or attr is otherwise invisible: its test module is just missing from the
# suite's report, which reads as a shorter suite rather than as a fault.
BUILDLOG="$(dirname "$IMG")/build.log"
if [ -f "$BUILDLOG" ]; then
    grep -aE 'Error #' "$BUILDLOG" >/dev/null \
        && fail "build.log contains an Error # line -- a build step failed silently"
else
    fail "build log not found at $BUILDLOG"
fi

# Every required top-level file is present, lowercase per OS-9 convention.
for f in readme runall runone rebuild; do
    printf '%s\n' "$listing" | grep -aq "[[:space:]]$f\$" || fail "missing file $f"
done

# runall, runone and rebuild must all invoke the same set of tests. The shell
# has no positional-parameter substitution, so all three are hand-maintained
# in lockstep; without this check a test added to some but not all of them
# drifts silently forever, with no failure signal anywhere.
ra=$(grep -aoE '^t[0-9]+[a-z]*' "$SRCDIR/text/runall" | sort)
ro=$(grep -aoE '^t[0-9]+[a-z]*' "$SRCDIR/text/runone" | sort)
rb=$(grep -aoE '^t[0-9]+[a-z]*' "$SRCDIR/text/rebuild" | sort)
[ "$ra" = "$ro" ] || fail "runall and runone invoke different tests"
[ "$ra" = "$rb" ] || fail "runall and rebuild invoke different tests"

# claims.md must cite the manual, not quote it. A quoted document or section
# title (a handful of words) keeps a citation checkable; a quoted sentence is
# a verbatim lift of copyrighted Microware text -- forbidden by this file's
# own header and already the subject of one prior fix (commit 75de504, a
# two-word lift). Six words is the line between the two.
long=$(grep -oE '"[^"]*"' "$SRCDIR/DOCS/claims.md" \
       | awk '{ gsub(/"/,""); if (NF>6) print }')
[ -z "$long" ] || fail "claims.md quotes verbatim text: $long"

# Public read, or the disk is unreadable to anyone but our build account.
for f in readme runall runone rebuild; do
    printf '%s\n' "$listing" | grep -a "[[:space:]]$f\$" | grep -aq '\-\-\-\-r' \
        || fail "$f lacks public read"
done

# Text files carry no non-ASCII bytes and no LF. Extract and check the real
# bytes: assembled modules legitimately contain high bytes, so this is
# per-file, never a scan of the whole image.
tmp=$(mktemp -d)
for f in readme runall runone rebuild; do
    if ! "$OS9" copy "$IMG,/$f" "$tmp/$f" >/dev/null 2>&1; then
        fail "cannot extract $f"; continue
    fi
    # grep exits 0 = found, 1 = not found, >=2 = error. Treat an error as a
    # failed assertion: `grep ... && fail` alone silently passes on exit 2,
    # which is a check that cannot fail.
    LC_ALL=C grep -q $'[\x80-\xff]' "$tmp/$f"
    case $? in 0) fail "$f contains non-ASCII bytes";; 1) ;; *) fail "cannot scan $f";; esac
    # NOT `grep -q $'\n' "$tmp/$f"` as originally specified: on this host's
    # BSD grep (2.6.0-FreeBSD, macOS default), `grep` splits input into
    # records on \n before matching, so a pattern that IS the record
    # separator matches every file -- proven live, including a file with
    # zero bytes of any kind. That is a check that cannot NOT fire: it
    # would flag every genuinely CR-only file as a false failure, which is
    # the same "vacuous check" defect as a check that can never fail, just
    # inverted. `wc -l` counts raw \n bytes directly and is unambiguous.
    lf_count=$(LC_ALL=C wc -l < "$tmp/$f" 2>/dev/null | tr -d ' ')
    case "$lf_count" in
        '') fail "cannot scan $f" ;;
        0) ;;
        *) fail "$f contains LF -- must be CR-only" ;;
    esac
done
rm -rf "$tmp"

if [ "$fails" -eq 0 ]; then printf 'VERIFY-OK %s\n' "$IMG"; exit 0; fi
printf 'VERIFY-FAILED %d assertion(s)\n' "$fails"; exit 1

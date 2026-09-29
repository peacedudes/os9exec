#!/usr/bin/env bash
# release-check.sh -- test the EXACT files a release publishes, the way a
# user meets them, and say GO or NO-GO.
#
#   tools/release-check.sh --run <CI run id> [--version V4.11]   candidate, before tagging
#   tools/release-check.sh --tag v4.1.1      [--version V4.11]   what is published
#
# Why this exists. v4.1.0 was published on 2026-09-29 after every gate
# passed, and within the hour two faults turned up that no gate could see:
#   * CONF68K's verdict was unreadable at a terminal -- its lines ended in a
#     bare CR, so each overwrote the one before. Every check had captured
#     the output through a pipe, where the lines look separate.
#   * On Windows about one conformance run in three said "Please tell us"
#     (a lock timeout that fired late). The Windows check ran once per sweep
#     and passed often enough to hide it.
# So this looks at what a user looks at: the published binaries, not the
# local build; a real terminal (a pty), not a pipe; and every run repeated,
# with a fresh disk each time, because one green run is not a result.
#
# Checks, each printed ok/FAILED:
#   the release files are all there and not empty
#   each binary reports the expected version (-ih)
#   macOS arm64 (this host), Linux x64 and Linux i386 (docker, `-t` for a
#   pty): conf68k.dsk's shell-less runner, RUNS times each, must say
#   "Nothing to send", and no line of its screen may be overwritten
#   Windows x64 (the UTM VM, see verify-winvm.sh): the runner WINRUNS times
# Exit 0 only on GO. tools/release.sh will not tag without it.
set -u
REPO="$(cd "$(dirname "$0")/.." && pwd)"
RUNS=${RUNS:-3}
WINRUNS=${WINRUNS:-5}
mode="" ref="" want=""
while [ $# -gt 0 ]; do
    case "$1" in
        --run)     mode=run; ref="$2"; shift 2 ;;
        --tag)     mode=tag; ref="$2"; shift 2 ;;
        --version) want="$2"; shift 2 ;;
        *) echo "usage: $0 --run <id> | --tag <vX.Y.Z> [--version V4.11]" >&2; exit 2 ;;
    esac
done
[ -n "$mode" ] || { echo "usage: $0 --run <id> | --tag <vX.Y.Z> [--version V4.11]" >&2; exit 2; }

# v4.1.1 -> V4.11: os9exec prints its version "V%x.%02x", major.minor-and-patch
if [ -z "$want" ] && [ "$mode" = tag ]; then
    IFS=. read -r maj min pat <<<"${ref#v}"
    want="V${maj}.${min}${pat}"
fi

W=$(mktemp -d "${TMPDIR:-/tmp}/release-check-XXXXXX")
fails=0
ok()   { printf '  %-58s ok\n' "$1"; }
bad()  { printf '  %-58s FAILED  %s\n' "$1" "$2"; fails=$((fails+1)); }

echo "== fetching the $mode $ref =="
if [ "$mode" = tag ]; then
    gh release download "$ref" -R peacedudes/os9exec -D "$W/files" >/dev/null 2>&1 \
        || { echo "could not download release $ref"; exit 1; }
else
    gh run download "$ref" -R peacedudes/os9exec -D "$W/art" >/dev/null 2>&1 \
        || { echo "could not download the artifacts of run $ref"; exit 1; }
    mkdir -p "$W/files"
    # each artifact is a directory named for what the release calls the file
    for d in "$W/art"/*/; do
        name=$(basename "$d")
        f=$(find "$d" -type f | head -1)
        [ -n "$f" ] && cp "$f" "$W/files/$name"
    done
    [ -f "$W/files/conf68k-disk" ] && mv "$W/files/conf68k-disk" "$W/files/conf68k.dsk"
fi
F="$W/files"
chmod +x "$F"/os9exec-* 2>/dev/null

echo "== the files =="
for f in os9exec-macos-arm64 os9exec-linux-x64 os9exec-linux-i386 os9exec-windows-x64.exe conf68k.dsk; do
    [ -s "$F/$f" ] && ok "$f" || bad "$f" "missing or empty"
done

# A screen line is overwritten when a carriage return is followed by text:
# \r\n and \r\r\n are fine, \r then a letter puts it over what was there.
overwrites() { python3 -c 'import re,sys; d=open(sys.argv[1],"rb").read(); sys.exit(0 if re.search(rb"\r[^\r\n]",d) else 1)' "$1"; }

# $1 label, $2 file holding one run's screen
judge_screen() {
    if ! grep -q "Nothing to send" "$2"; then
        why=$(tr '\r' '\n' <"$2" | grep -E 'CONF68K totals|Please tell us' | tr '\n' ' ')
        [ -n "$why" ] || why=$(tr '\r' '\n' <"$2" | grep -v '^[[:space:]]*$' | head -1)
        bad "$1" "${why:-no output at all}"
    elif overwrites "$2"; then
        bad "$1" "a line of the verdict overwrites another (bare CR)"
    else
        ok "$1"
    fi
}

echo "== macOS arm64, on a terminal =="
if [ -n "$want" ]; then
    v=$("$F/os9exec-macos-arm64" -ih 2>&1 | head -1)
    grep -q "OS9exec $want " <<<"$v" && ok "version $want" || bad "version $want" "says: $v"
fi
for i in $(seq 1 "$RUNS"); do
    cp "$F/conf68k.dsk" "$W/mac.dsk"
    script -q /dev/null env OS9DISK="$W/mac.dsk" "$F/os9exec-macos-arm64" -r /dd/CMDS/run \
        </dev/null >"$W/mac.$i" 2>&1
    judge_screen "run $i of $RUNS" "$W/mac.$i"
done

# Docker: `-t` gives the program a pty, the way a user's terminal would.
linux_leg() {   # $1 label, $2 binary, $3 image, $4 platform
    echo "== $1, on a terminal (docker) =="
    docker info >/dev/null 2>&1 || { bad "$1" "docker is not running"; return; }
    if [ -n "$want" ]; then
        v=$(docker run --rm --platform "$4" -v "$F:/r:ro" "$3" /r/"$2" -ih 2>&1 | grep "OS9exec V" | head -1)
        grep -q "OS9exec $want " <<<"$v" && ok "version $want" || bad "version $want" "says: $v"
    fi
    for i in $(seq 1 "$RUNS"); do
        # A new directory every run, never one deleted and made again: on
        # macOS Docker a host-side delete of a bind-mounted path is invisible
        # to the NEXT container, which then finds no disk ("E_MNF /dd/CMDS/run"
        # in 2 runs of 6 the first time this ran).
        lx="$W/lx.$2.$i"; mkdir -p "$lx"; cp "$F/conf68k.dsk" "$lx/c.dsk"
        docker run --rm -t --platform "$4" -v "$F:/r:ro" -v "$lx:/w" -e OS9DISK=/w/c.dsk \
            "$3" /r/"$2" -r /dd/CMDS/run </dev/null >"$W/$2.$i" 2>&1
        judge_screen "run $i of $RUNS" "$W/$2.$i"
    done
}
linux_leg "Linux x64"  os9exec-linux-x64  ubuntu:24.04      linux/amd64
linux_leg "Linux i386" os9exec-linux-i386 i386/ubuntu:20.04 linux/386

echo "== Windows x64 (VM), $WINRUNS runs from a fresh disk =="
KEY="$HOME/.ssh/os9exec_winvm"
SSHOPTS="-i $KEY -o IdentitiesOnly=yes -o ConnectTimeout=10 -o StrictHostKeyChecking=no"
SSH="ssh $SSHOPTS -p 2222 claude@localhost"
if ! $SSH "echo up" >/dev/null 2>&1; then
    bad "Windows VM" "not reachable -- start it (never force-stop it) and run this again"
else
    $SSH 'cmd /c if not exist C:\relchk mkdir C:\relchk' >/dev/null 2>&1
    scp -q $SSHOPTS -P 2222 "$F/os9exec-windows-x64.exe" claude@localhost:C:/relchk/os9exec.exe
    scp -q $SSHOPTS -P 2222 "$F/conf68k.dsk" claude@localhost:C:/relchk/pristine.dsk
    if [ -n "$want" ]; then
        v=$($SSH 'C:\relchk\os9exec.exe -ih' 2>&1 | grep "OS9exec V" | head -1)
        grep -q "OS9exec $want " <<<"$v" && ok "version $want" || bad "version $want" "says: $v"
    fi
    for i in $(seq 1 "$WINRUNS"); do
        $SSH 'Copy-Item C:\relchk\pristine.dsk C:\relchk\c.dsk -Force
$env:OS9DISK="C:/relchk/c.dsk"
cd C:\relchk
& C:\relchk\os9exec.exe -r /dd/CMDS/run 2>&1 | Out-String' 2>/dev/null >"$W/win.$i"
        # Windows' console is not a pty here; the screen check is the other legs'
        if grep -q "Nothing to send" "$W/win.$i"; then ok "run $i of $WINRUNS"
        else bad "run $i of $WINRUNS" "$(tr '\r' '\n' <"$W/win.$i" | grep -E 'CONF68K totals' | tr '\n' ' ')"; fi
    done
fi

echo
if [ "$fails" = 0 ]; then
    echo "== GO: $mode $ref passed every check =="
    rm -rf "$W"; exit 0
fi
echo "== NO-GO: $fails check(s) failed; screens kept in $W =="
exit 1

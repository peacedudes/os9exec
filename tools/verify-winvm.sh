#!/usr/bin/env bash
# verify-winvm.sh -- build os9exec for Windows, ship it to the UTM Windows VM,
# and run the whole CONF68K suite there. On a real Windows kernel, not wine and
# not a container.
#
# The VM is Windows 11 ARM64 and the binary is x86_64; Windows' own emulation
# covers that, and it is what a user with an ARM Surface would be running too.
#
# Setup this depends on (see project memory `windows-vm-access`):
#   - UTM guest named os9exec-windows, SSH forwarded to localhost:2222
#   - key auth as user `claude` (~/.ssh/os9exec_winvm)
#
# FORWARD SLASHES in OS9DISK are not a style choice: with backslashes the
# emulator finds nothing at all and every test reports E$PNNF. That is a real
# defect, recorded in ROADMAP-68k.md; this script sidesteps it deliberately
# rather than accidentally.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
VM=os9exec-windows
SSH="ssh -o ConnectTimeout=10 -o StrictHostKeyChecking=no -p 2222 claude@localhost"
BUILD=/tmp/verify-winvm

command -v utmctl >/dev/null 2>&1 || { echo "utmctl not installed"; exit 1; }

started_here=no
if [ "$(utmctl status $VM 2>/dev/null)" != "started" ]; then
    echo "starting $VM ..."
    utmctl start $VM >/dev/null 2>&1 || { echo "could not start $VM"; exit 1; }
    started_here=yes
fi

# Wait for sshd rather than guessing a boot time.
up=no
for _ in $(seq 1 40); do
    if $SSH "echo UP" 2>/dev/null | grep -q UP; then up=yes; break; fi
    sleep 10
done
[ "$up" = yes ] || { echo "$VM never answered on ssh"; exit 1; }

# Cross-build into a scratch dir -- never into the repo root, which holds the
# binary the user actually runs.
rm -rf "$BUILD"; mkdir -p "$BUILD"
( cd "$REPO" && make -B OS=Windows_NT CC=x86_64-w64-mingw32-gcc \
     OBJDIR="$BUILD" EXE="$BUILD/os9exec.exe" prod ) >"$BUILD/build.log" 2>&1 \
  || { echo "windows cross-build failed -- $BUILD/build.log"; exit 1; }

$SSH 'if (Test-Path C:\verify) { Remove-Item -Recurse -Force C:\verify }; New-Item -ItemType Directory C:\verify | Out-Null' >/dev/null 2>&1
scp -q -o StrictHostKeyChecking=no -P 2222 "$BUILD/os9exec.exe" claude@localhost:C:/verify/os9exec.exe || exit 1
scp -q -r -o StrictHostKeyChecking=no -P 2222 "$REPO/test/68k-conformance" claude@localhost:C:/verify/ || exit 1

# Run every test module as its own boot program and tally the RESULT lines,
# which is exactly what conformance.sh's --noshell leg does on Unix.
out=$($SSH 'cd C:\verify\68k-conformance; $env:OS9DISK="C:/verify/68k-conformance"
$mods = Get-ChildItem CMDS | Where-Object { $_.Name -ne "tally" -and $_.Name -ne "mark" } | Sort-Object Name
$res=@(); foreach ($m in $mods) {
  $o = (C:\verify\os9exec.exe -r ("/dd/CMDS/" + $m.Name) 2>&1 | Out-String)
  $res += (($o -replace "`r","`n") -split "`n" | Where-Object { $_ -match "^RESULT " })
}
Write-Output ("TOTAL=" + $res.Count)
Write-Output ("PASS="  + ($res | Where-Object {$_ -match " PASS "}).Count)
Write-Output ("FAIL="  + ($res | Where-Object {$_ -match " FAIL "}).Count)
Write-Output ("SKIP="  + ($res | Where-Object {$_ -match " SKIP "}).Count)
Write-Output ("ERROR=" + ($res | Where-Object {$_ -match " ERROR "}).Count)
$res | Where-Object {$_ -match " FAIL | ERROR "}' 2>/dev/null | tr -d '\r')

echo "$out"
# Shut the guest down FROM INSIDE, and never force it.
#
# `utmctl stop` defaults to --force, which is a power-off event -- pulling the
# plug on a running Windows. Doing that after every run corrupted this VM's
# boot volume on 2026-08-06 ("Your device ran into a problem and couldn't be
# repaired"). A test harness must not be able to damage the machine it tests.
#
# So: ask Windows to shut itself down, wait for sshd to go quiet, and if it is
# still up after that, LEAVE IT RUNNING. A VM left on is a nuisance; a VM
# left broken is somebody's evening.
if [ "$started_here" = yes ]; then
    $SSH "shutdown /s /t 0" >/dev/null 2>&1
    for _ in $(seq 1 18); do
        $SSH "echo up" >/dev/null 2>&1 || break
        sleep 5
    done
    if $SSH "echo up" >/dev/null 2>&1; then
        echo "note: $VM did not shut down; leaving it running rather than forcing it" >&2
    else
        utmctl stop --request $VM >/dev/null 2>&1   # ACPI, in case it lingers
    fi
fi

total=$(sed -n 's/^TOTAL=//p' <<<"$out"); fail=$(sed -n 's/^FAIL=//p' <<<"$out")
err=$(sed -n 's/^ERROR=//p'  <<<"$out")

# A silent run is a failed run: if no module reported, the suite did not run.
[ -n "$total" ] && [ "$total" -gt 0 ] || { echo "no RESULT lines came back"; exit 1; }
[ "${fail:-1}" = 0 ] && [ "${err:-1}" = 0 ] || exit 1
exit 0

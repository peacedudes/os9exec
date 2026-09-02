#!/usr/bin/env bash
# qemu-guest.sh -- start or stop a headless test guest under the HOST's QEMU.
#
# Not UTM. UTM was how sparc64 and riscv64 were run until 2026-09-02, and for
# sparc64 it made the guest unusable: UTM wires the disk to the `sun4u`
# machine's cmd646 IDE controller, which loses interrupts under a build's write
# load until ext4 gives up and remounts read-only. Three legs died that way --
# one hung for sixteen hours. Nothing about it was fixable from inside the
# guest; `libata.dma=0` made no difference because it was already in PIO.
#
# The same image, same kernel, same initrd, booted by Homebrew's QEMU with the
# disk on VIRTIO instead: up in 40 seconds, zero disk errors. `root=LABEL=`
# finds it as /dev/vda1 without any change to the guest. UTM also bundles an
# older QEMU and splits its argument strings on spaces, so a multi-word value
# needs literal quotes embedded in the plist -- see project memory
# `sparc64-one-vcpu-and-the-ata-storm`.
#
#   tools/qemu-guest.sh start sparc64      boot it, wait for ssh
#   tools/qemu-guest.sh stop  sparc64      poweroff from inside, wait
#   tools/qemu-guest.sh status sparc64
#
# Guests live in ~/Developer/os9/vms/<name>/ as kernel, initrd and
# overlay.qcow2 -- a COPY-ON-WRITE overlay over the pristine image, made with
#
#   qemu-img create -f qcow2 -b <image.qcow2> -F qcow2 overlay.qcow2
#
# so a run cannot damage the original and a reset is `rm overlay.qcow2` and
# make it again. Copying the 1.9G image instead took over sixteen minutes.
set -uo pipefail

ACTION=${1:?usage: qemu-guest.sh <start|stop|status> <guest>}
NAME=${2:?usage: qemu-guest.sh <start|stop|status> <guest>}
DIR=${OS9_VM_DIR:-$HOME/Developer/os9/vms}/$NAME
KEY=${OS9_VM_KEY:-$HOME/.ssh/os9exec-vms}

case "$NAME" in
    sparc64) QEMU=qemu-system-sparc64; MACHINE=sun4u;  PORT=2225; MEM=4096; CPUS=1 ;;
    riscv64) QEMU=qemu-system-riscv64; MACHINE=virt;   PORT=2223; MEM=4096; CPUS=4 ;;
    *) echo "unknown guest '$NAME' -- add it to the case in $0"; exit 2 ;;
esac

SSH="ssh -n -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
     -o ConnectTimeout=10 -i $KEY -p $PORT root@localhost"

alive() { [ -f "$DIR/qemu.pid" ] && kill -0 "$(cat "$DIR/qemu.pid" 2>/dev/null)" 2>/dev/null; }
# By PID, from the pidfile. NEVER `pkill -f qemu` -- the pattern matches this
# script's own shell, and other sessions run guests too.

case "$ACTION" in
status)
    alive && echo "$NAME: running (pid $(cat "$DIR/qemu.pid"))" || echo "$NAME: not running"
    $SSH 'uptime' 2>/dev/null | sed 's/^/  /'
    exit 0 ;;

stop)
    alive || { echo "$NAME: not running"; exit 0; }
    # From INSIDE the guest. Killing QEMU is pulling the plug on a live
    # filesystem, and this project has paid for that once already.
    $SSH 'nohup poweroff >/dev/null 2>&1 &' >/dev/null 2>&1
    for _ in $(seq 1 60); do alive || { echo "$NAME: stopped"; rm -f "$DIR/qemu.pid"; exit 0; }; sleep 5; done
    echo "$NAME: STILL RUNNING after 5 minutes -- it did not take the poweroff"
    exit 1 ;;

start) ;;
*) echo "unknown action '$ACTION'"; exit 2 ;;
esac

command -v "$QEMU" >/dev/null 2>&1 || { echo "$QEMU not installed (brew install qemu)"; exit 1; }
for f in kernel initrd overlay.qcow2; do
    [ -f "$DIR/$f" ] || { echo "missing $DIR/$f -- see the header for how to stage a guest"; exit 1; }
done
alive && { echo "$NAME: already running (pid $(cat "$DIR/qemu.pid"))"; exit 0; }

# -serial to a FILE, not stdio: this runs unattended, and a console nobody
# reads is still the only place a boot failure explains itself.
: > "$DIR/console.log"
"$QEMU" -machine "$MACHINE" -m "$MEM" -smp "$CPUS" \
    -kernel "$DIR/kernel" -initrd "$DIR/initrd" \
    -append "root=LABEL=rootfs console=ttyS0" \
    -drive if=none,id=d0,file="$DIR/overlay.qcow2",format=qcow2 \
    -device virtio-blk-pci,drive=d0 \
    -netdev user,id=n0,hostfwd=tcp:127.0.0.1:$PORT-:22 -device e1000,netdev=n0 \
    -display none -serial file:"$DIR/console.log" -pidfile "$DIR/qemu.pid" -daemonize \
  || { echo "$NAME: QEMU refused to start"; exit 1; }

echo "$NAME: booting (pid $(cat "$DIR/qemu.pid" 2>/dev/null))"
for i in $(seq 1 60); do
    $SSH 'true' >/dev/null 2>&1 && { echo "$NAME: up after ~$((i*5))s on port $PORT"; break; }
    alive || { echo "$NAME: QEMU DIED while booting -- tail of console:"; tail -5 "$DIR/console.log" | sed 's/^/  /'; exit 1; }
    sleep 5
done
$SSH 'true' >/dev/null 2>&1 || { echo "$NAME: no ssh after 5 minutes -- tail of console:"; tail -5 "$DIR/console.log" | sed 's/^/  /'; exit 1; }

# A guest that boots but whose disk is already complaining will fail a leg an
# hour from now instead of here. Say so at the start.
errs=$(grep -ciE "lost interrupt|I/O error|EXT4-fs error" "$DIR/console.log" 2>/dev/null)
[ "${errs:-0}" = 0 ] && echo "  disk clean (0 errors on the console)" \
                     || echo "  WARNING: $errs disk errors already on the console"
$SSH 'mount | grep " / "' 2>/dev/null | sed 's/^/  /'

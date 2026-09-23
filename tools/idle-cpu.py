#!/usr/bin/env python3
"""Measure an os9exec binary's CPU while idle at a prompt.

  idle-cpu.py <os9exec> [seconds]

An idle emulator should cost almost nothing: it has nothing to run, and it is
woken by the host when something arrives. It used to cost 1.6% of a core by
polling every millisecond; it costs about 0.1% now (2026-09-20).

Three things this does deliberately, each of which went wrong once:

  * a real pty, because an idle emulator reading a PIPE is a different animal
    (it sees EOF, and the wait it takes is not the one a person would);
  * the child's pid straight from pty.fork(), never a pattern match -- a
    `pgrep -x os9exec | tail -1` once matched the operator's own shell and
    killed it;
  * the load average printed before and after, because the first version of
    these numbers was measured with six orphaned spinners on the machine and
    nobody noticed until a peer session asked why everything was slow. A load
    average you did not cause is a measurement you cannot trust.

Typical use is a before/after pair, interleaved, on a quiet machine:

    tools/idle-cpu.py /path/to/old/os9exec 20
    tools/idle-cpu.py ./os9exec 20
"""
import os, pty, sys, time, signal, subprocess

exe   = sys.argv[1]
secs  = float(sys.argv[2]) if len(sys.argv) > 2 else 20.0

def load():
    return open('/proc/loadavg').read().split()[0] if os.path.exists('/proc/loadavg') \
           else subprocess.run(['uptime'], capture_output=True, text=True).stdout.split('averages:')[-1].strip()

def cpu(pid):
    return subprocess.run(['ps', '-o', 'time=', '-p', str(pid)],
                          capture_output=True, text=True).stdout.strip()

def secs_of(t):
    """ps's cumulative CPU time in seconds: macOS prints "0:00.42" (MM:SS.ss),
    Linux "00:00:01" or "1-02:03:04" ([DD-]HH:MM:SS, whole seconds)."""
    days, _, clock = t.strip().rpartition('-')
    secs = 0.0
    for part in clock.split(':'):
        secs = secs * 60 + float(part)
    return secs + int(days or 0) * 86400

pid, fd = pty.fork()
if pid == 0:
    os.execv(exe, [exe, 'shell'])

time.sleep(4)                        # let it reach a prompt and settle
before_load = load()
t0 = cpu(pid)
time.sleep(secs)
t1 = cpu(pid)
after_load = load()

# Politely, and then REAP it. This script is the child's parent (pty.fork), so
# an exited child becomes a zombie -- and kill(pid, 0) SUCCEEDS on a zombie.
# Testing liveness that way said "ignored SIGTERM for 10s" about an emulator
# that had in fact stopped in 0.6s, which is a scare, and the kind that sends
# somebody looking for a bug in the thing being measured. waitpid is the
# question worth asking: has it exited, and has its status been collected.
os.kill(pid, signal.SIGTERM)
stopped = False
for _ in range(100):
    time.sleep(0.1)
    if os.waitpid(pid, os.WNOHANG)[0] == pid:
        stopped = True
        break
if not stopped:
    print("  WARNING: pid %d ignored SIGTERM for 10s; killing it" % pid)
    os.kill(pid, signal.SIGKILL)
    os.waitpid(pid, 0)

used = secs_of(t1) - secs_of(t0)
print("  %-44s %.2fs over %.0fs = %.2f%% of a core   [load %s -> %s]"
      % (os.path.basename(os.path.dirname(exe)) + '/' + os.path.basename(exe),
         used, secs, 100.0 * used / secs, before_load, after_load))

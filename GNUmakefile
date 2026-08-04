CC      ?= clang
CORE    = Source/OS9exec_core
NATIVE  = Source/NATIVE
UAE     = Source/OS9AppEmu/UAE68emulator
APPEMU  = Source/OS9AppEmu
OBJDIR  = build

# Per-OS platform file: $(OS) is set to "Windows_NT" by cmd.exe/PowerShell
# on a native Windows build (e.g. mingw-w64 installed on a windows-latest
# CI runner); everything else (macOS, Linux) uses the POSIX/dirent-based
# LINUX platform file, which macOS also happens to work with unmodified.
ifeq ($(OS),Windows_NT)
  PLAT    = Source/Platforms/WIN32
  PLATSRC = winfiles.c
  EXE     = os9exec.exe
  # -pthread: nanosleep() (consio.c, procstuff.c) lives in mingw-w64's
  # winpthreads, not linked by default on every mingw-w64 distribution --
  # confirmed missing on Ubuntu's apt mingw-w64 package even though
  # Homebrew's on macOS pulls it in implicitly. Explicit for portability.
  LDFLAGS = -static -pthread
else
  PLAT    = Source/Platforms/LINUX
  PLATSRC = linuxfiles.c
  EXE     = os9exec
  LDFLAGS =
endif

CFLAGS  = -g -Wall -fcommon \
          -DTERMINAL_CONSOLE \
          -DINT_CMD \
          -DRAM_SUPPORT \
          -DREUSE_MEM \
          -I$(CORE) \
          -I$(CORE)/os9defs \
          -I$(PLAT) \
          -ISource/Platforms \
          -I$(UAE) \
          -I$(APPEMU)

SRCS = \
    Source/OS9execMPW/os9.c \
    $(CORE)/alarms.c \
    $(CORE)/c_access.c \
    $(CORE)/consio.c \
    $(CORE)/debug.c \
    $(CORE)/events.c \
    $(CORE)/fcalls.c \
    $(CORE)/fileaccess.c \
    $(CORE)/filescsi.c \
    $(CORE)/file_rbf.c \
    $(CORE)/filestuff.c \
    $(CORE)/filters.c \
    $(CORE)/funcdispatch.c \
    $(CORE)/hostterm.c \
    $(CORE)/icalls.c \
    $(CORE)/int_move.c \
    $(CORE)/int_rename.c \
    $(CORE)/intcommand.c \
    $(CORE)/memstuff.c \
    $(CORE)/modstuff.c \
    $(CORE)/os9exec_nt.c \
    $(CORE)/os9main.c \
    $(CORE)/os9_tick.c \
    $(CORE)/pipefiles.c \
    $(CORE)/printer.c \
    $(CORE)/procstuff.c \
    $(CORE)/telnetaccess.c \
    $(CORE)/utilstuff.c \
    $(CORE)/vmod.c \
    $(PLAT)/$(PLATSRC) \
    $(APPEMU)/os9_uae.c \
    $(APPEMU)/luzstuff.c \
    $(UAE)/newcpu.c \
    $(UAE)/cpuemu.c \
    $(UAE)/cpustbl.c \
    $(UAE)/cpudefs.c \
    $(UAE)/readcpu.c \
    $(UAE)/memory.c \
    $(UAE)/fpp.c \
    $(UAE)/support.c

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SRCS)))

# Warnings suppressed only for auto-generated / legacy UAE files that we
# cannot realistically clean up without regenerating or rewriting them.
UAE_SUPPRESS = -Wno-unused-variable -Wno-unused-but-set-variable \
               -Wno-deprecated-non-prototype -Wno-format \
               -Wno-unused-function

VPATH = $(CORE):$(PLAT):Source/OS9execMPW:$(APPEMU):$(UAE)

.PHONY: all prod clean test test-notick test-linux warnings hammer hammer-soak hammer-6809 conformance

all: $(OBJDIR) $(EXE)

# Production build: optimised, no debug symbols.
# Usage: make prod   (rebuilds from scratch with -O2)
prod:
	$(MAKE) -B CFLAGS="-O2 -Wall -fcommon \
	          -DREUSE_MEM \
          -DTERMINAL_CONSOLE \
	          -DINT_CMD \
	          -DRAM_SUPPORT \
	          -I$(CORE) \
	          -I$(CORE)/os9defs \
	          -I$(PLAT) \
	          -ISource/Platforms \
	          -I$(UAE) \
	          -I$(APPEMU)"


# Header dependency tracking. Without this, editing a header rebuilds NOTHING,
# and you silently link objects compiled against DIFFERENT versions of a struct.
# That is not a theoretical worry: changing a field in process_typ (os9exec_nt.h)
# left most objects on the old layout, and the emulator crashed in unrelated code
# with a garbage path type -- it looked exactly like a logic bug. `make clean` was
# the only defence, and relying on remembering it is not a defence.
#
# -MMD: emit a .d file of each object's header prerequisites as a side effect of
#       compiling (project headers only; system headers are not our problem).
# -MP:  also emit a phony target per header, so DELETING or renaming a header
#       doesn't wedge make with "No rule to make target".
# Kept out of CFLAGS on purpose: the `prod` target replaces CFLAGS wholesale and
# would otherwise silently drop dependency tracking in exactly the build we ship.
DEPFLAGS = -MMD -MP
DEPS     = $(OBJS:.o=.d)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lm

$(OBJDIR)/cpuemu.o: $(UAE)/cpuemu.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(UAE_SUPPRESS) -c $< -o $@

$(OBJDIR)/newcpu.o: $(UAE)/newcpu.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(UAE_SUPPRESS) -c $< -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Pull in the generated dependencies (absent on a clean tree -- hence `-include`).
-include $(DEPS)

clean:
	rm -rf $(OBJDIR) os9exec os9exec.exe

test: $(EXE)
	swift run --package-path test OS9Tests

# The same suite with the system tick OFF ("-q"). The tick is on by default, so
# this is the one configuration users can select that `make test` never covers,
# and it is the fallback if pre-emption ever breaks a workload -- a safety net
# nobody exercises is one that frays quietly. Not part of `test`: it doubles the
# runtime (~1 min each) for a mode that is deliberately not the supported one.
test-notick: $(EXE)
	OS9_FLAGS=-q swift run --package-path test OS9Tests

# Guest-side conformance suites: run one on a guest and CHECK the RESULT lines
# it produced against the recorded results in the suite's DOCS/expected. This
# is what makes a divergence fail a build rather than merely print -- before
# tools/conformance.sh existed, both suites could be run and neither could
# fail. Both device types are run because they genuinely disagree: a
# host-native directory is a shim with no real RBF underneath, and t10 found a
# real I$Create defect by passing on one and failing on the other.
# The 6809 arm (./tools/conformance.sh 6809) needs XRoar and is deliberately
# not wired in here -- it drives a live emulator and would collide with any
# concurrent session using the same tooling.
conformance: $(EXE)
	./tools/conformance.sh 68k
	./tools/conformance.sh 68k --rbf

# Wraps the 68k live-verification corpus (test/68k-live-verification/) into
# a runnable PASS/FAIL suite -- see docs/superpowers/specs/
# 2026-07-21-live-verification-suite-design.md. 6809 is deliberately not
# wired in here (see that design doc's "6809 is deferred" section).
live-verify: $(EXE)
	swift run --package-path test LiveVerify

# ── RBF integrity hammer ────────────────────────────────────────────────────
# A multiprocess stress test aimed at RBF: concurrent writers on shared and
# separate files, on every backend, verified for BOTH file content and on-disk
# structure (dcheck/free). Two entry points:
#
#   make hammer        the GATE: every scenario once, a few seconds -- a
#                      pre-commit check.
#   make hammer-soak   the SOAK: HAMMER_ITER iterations in a parallel pool of
#                      HAMMER_JOBS, to shake out intermittent faults a single
#                      run hides. HAMMER_ITER=2000 is roughly ten minutes.
#   make hammer-6809   the same against real NitrOS-9 under XRoar (needs the
#                      XRoar/ToolShed/NitrOS-9 setup; slow -- ~15s per run).
#
# The record-lock, destructive, and cross-target tests live in the test targets
# (`swift test`); these run the contended scenarios that verify generically.
HAMMER_ITER ?= 500
HAMMER_JOBS ?= 8

hammer: $(EXE)
	swift run --package-path test RBFHammer --target 68k --gate

hammer-soak: $(EXE)
	swift run --package-path test RBFHammer --target 68k \
		--iterations $(HAMMER_ITER) --jobs $(HAMMER_JOBS)

hammer-6809:
	swift run --package-path test RBFHammer --target 6809 --iterations 5 --jobs 2

# Self-hosted OS-9/6809 conformance suite: builds a small RBF image
# (build/selfhost6809/conf6809.dsk) that anyone with real 6809 hardware can
# mount and run. See .superpowers/sdd/2026-07-25-6809-selfhosted-conformance-suite/.
selfhost-6809:
	./tools/selfhost6809/build-image.sh
	./tools/selfhost6809/verify-image.sh build/selfhost6809/conf6809.dsk

# Run the same integration suite against a real Linux build, in Docker.
#
# Worth doing even from macOS -- the platforms disagree in ways that hide bugs:
# on Linux/ARM plain `char` is UNSIGNED (it is signed on macOS, even on arm64),
# and gcc-on-Linux warns about things clang and mingw-gcc both miss. The first
# run of this target found os9exec spinning forever on every file read, because
# `char c = fgetc(...)` could never equal EOF where char is unsigned.
test-linux:
	docker build -f docker/Dockerfile -t os9exec:linux .
	DOCKER_IMAGE=os9exec:linux swift run --package-path test OS9Tests

# Compile-only sweep across all four toolchains. Each one sees bugs the others
# do not: mingw x86_64 (LLP64) catches host pointers truncated through 32-bit
# ints, mingw i686 (ILP32) catches the mirror image -- a width hardcoded to 64
# bits where the pointer is 32 -- plus every __stdcall omission that x86_64
# Windows is free to ignore, gcc-on-Linux catches NULL/format issues clang
# ignores, and clang catches its own. All four must be warning-clean.
#
# The i686 leg earns its slot twice over. It is the only target here where
# pointer width AND `long` width both differ from every other leg, and it found
# two real defects the day it was first run (aca7b7f, d7a4ebd) that no other
# toolchain in this matrix can see. It is also the ONLY evidence 32-bit Windows
# works at all: that target is supported but has never been executed -- there is
# no 32-bit Windows host here to run it on -- so a clean compile is the whole of
# the assurance. Treat a regression here as a broken platform, not as noise from
# a toolchain nobody uses.
#
# Each leg writes its build log to a file, then tallies it. Do NOT go back to
# piping the build straight into `grep -c "warning:"`: a build that dies with
# compile ERRORS emits no "warning:" lines, so it scored a clean "warnings: 0"
# and a broken toolchain was indistinguishable from a passing one. Likewise the
# docker leg's setup used to be an `&&` chain, so a failed `apt-get` skipped the
# build and printed nothing at all -- a leg that never ran looked like a leg
# that passed. Every leg must now print a tally line and shout if it failed.
# A tally of 0/0 means "clean" ONLY if the leg actually built. When a leg dies
# before compiling -- a missing toolchain, or the package mirror this used to
# depend on returning 400 -- there are no "warning:"/"error:" lines to count, so
# a bare count reports a perfect score for a build that never happened. That is
# the same vacuous-green trap as `make warnings` once scoring 0 on a hard
# compile error. So the artifact is the proof: no binary, no score.
TALLY = awk '/warning:/{w++} /error:/{e++} \
  END{printf "  warnings: %d  errors: %d\n", w+0, e+0}'

warnings:
	@echo "=== host ($(CC)) ==="
	@$(MAKE) -B --no-print-directory >/tmp/os9exec-host.log 2>&1 \
	  || echo "  BUILD FAILED -- see /tmp/os9exec-host.log"
	@$(TALLY) /tmp/os9exec-host.log
	@echo "=== linux (gcc, in docker) ==="
# Seen 2026-08-04: the container's cp failed with "File exists", so the leg
# reported NOT BUILT even though it had just compiled and linked 0/0 clean.
# The destination is a FIXED path in /tmp shared by every run, and several
# sessions share this repo -- two overlapping `make warnings` collide, one's rm
# racing the other's cp. Root ownership was the first theory and is NOT the
# cause: on macOS the bind mount maps ownership and the host rm removes the file
# fine (checked). The rm below narrows the window; it does not close it. Closing
# it wants a per-run unique path for the binary AND the four logs, which is a
# rewrite of this target -- noted in ROADMAP-68k.md instead of done half-way.
# The `test -x` guard already fails LOUD rather than scoring a phantom 0/0, so
# this misreports in the safe direction.
	@rm -f /tmp/os9exec-linux-built 2>/dev/null || true
	@docker run --rm -v "$(CURDIR)/Source:/src/Source:ro" -v "$(CURDIR)/GNUmakefile:/src/GNUmakefile:ro" \
	  -v /tmp:/out -w /src gcc:13 sh -c 'set -e; mkdir -p /tmp/b; \
	  make CC=gcc OBJDIR=/tmp/b EXE=/tmp/b/os9exec 2>&1; \
	  rm -f /out/os9exec-linux-built; \
	  cp /tmp/b/os9exec /out/os9exec-linux-built' >/tmp/os9exec-linux.log 2>&1 \
	  || echo "  BUILD OR SETUP FAILED -- see /tmp/os9exec-linux.log"
	@test -x /tmp/os9exec-linux-built \
	  || echo "  NOT BUILT -- no Linux binary produced; the score below means nothing"
	@$(TALLY) /tmp/os9exec-linux.log
	@echo "=== windows x86_64 (mingw-w64, LLP64) ==="
	@rm -f /tmp/os9exec-win/os9exec.exe
	@$(MAKE) -B --no-print-directory OS=Windows_NT CC=x86_64-w64-mingw32-gcc \
	  OBJDIR=/tmp/os9exec-win EXE=/tmp/os9exec-win/os9exec.exe \
	  >/tmp/os9exec-win.log 2>&1 \
	  || echo "  BUILD FAILED -- see /tmp/os9exec-win.log"
	@test -f /tmp/os9exec-win/os9exec.exe \
	  || echo "  NOT BUILT -- no Windows binary produced; the score below means nothing"
	@$(TALLY) /tmp/os9exec-win.log
	@echo "=== windows i686 (mingw-w64, ILP32) ==="
	@rm -f /tmp/os9exec-win32/os9exec.exe
	@$(MAKE) -B --no-print-directory OS=Windows_NT CC=i686-w64-mingw32-gcc \
	  OBJDIR=/tmp/os9exec-win32 EXE=/tmp/os9exec-win32/os9exec.exe \
	  >/tmp/os9exec-win32.log 2>&1 \
	  || echo "  BUILD FAILED -- see /tmp/os9exec-win32.log"
	@test -f /tmp/os9exec-win32/os9exec.exe \
	  || echo "  NOT BUILT -- no Windows binary produced; the score below means nothing"
	@$(TALLY) /tmp/os9exec-win32.log

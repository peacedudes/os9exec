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

# -fno-strict-aliasing is REQUIRED, not a preference. The 68k register halves
# are reached by punning a 32-bit register through a ushort*/byte* --
# loword/hiword/lobyte/retword in os9_ll.h, 125 call sites, 39 of them as
# assignment targets. That is exactly what the hardware does and the macros are
# already correct for both byte orders, but it is undefined behaviour under C's
# strict-aliasing rule, which gcc -O2 is entitled to optimise on. gcc says so
# 59 times at -O2; clang says nothing and has the same licence, so a quiet
# build is not evidence of safety. The alternative is retyping the register
# file as a union across all 125 sites -- a real option, and a separate change.
CFLAGS  = -g -Wall -fcommon -fno-strict-aliasing \
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

.PHONY: all prod clean test test-notick test-linux warnings hammer hammer-soak hammer-6809 conformance verify verify-vms verify-quick selfhost68k selfhost68k-verify

all: $(OBJDIR) $(EXE)

# Production build: optimised, no debug symbols.
# Usage: make prod   (rebuilds from scratch with -O2)
prod:
	$(MAKE) -B CFLAGS="-O2 -Wall -fcommon -fno-strict-aliasing \
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
# The self-contained CONF68K disk: one RBF image carrying the whole suite,
# mountable on real OS-9/68000 hardware. Mirrors tools/selfhost6809 for the
# 6809 side. `selfhost68k-verify` proves every file landed byte for byte AND
# that all 44 tests run off the image.
selfhost68k:
	@tools/selfhost68k/build-image.sh

selfhost68k-verify: selfhost68k
	@tools/selfhost68k/verify-image.sh

# One button. Every gate, one verdict, non-zero exit if any of them failed.
#   make verify        host + docker
#   make verify-vms    ... and the real UTM machines
#   make verify-quick  host only
verify:
	@tools/verify.sh

verify-vms:
	@tools/verify.sh --vms

verify-quick:
	@tools/verify.sh --quick

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

# Every leg's log, object directory and binary used to be a FIXED path in /tmp,
# shared by every concurrent run -- and several sessions share this repo, so two
# overlapping `make warnings` runs are expected rather than hypothetical. The
# binary was guarded (`test -x` -> a loud NOT BUILT, which is how this was
# noticed 2026-08-04), but the LOGS were not, and they are what TALLY counts.
# Each is opened with `>`, which truncates at open, so one run starting its host
# leg while another is about to tally wipes the file being counted and that run
# scores "warnings: 0  errors: 0" for a build that had warnings -- a phantom
# pass, the exact trap the rest of this comment block exists to prevent. The two
# mingw legs were worse in kind: their /tmp paths are OBJDIRs used with -B, so
# two runs wrote .o files into one directory and either could link a
# half-written object.
#
# One directory per make invocation fixes all of it. `:=` evaluates the $$ once,
# at parse time, so every recipe line below sees the SAME value -- a plain `=`
# would re-expand per line and hand each leg a different directory.
WARNDIR := /tmp/os9exec-warnings-$(shell echo $$$$)

# Swept at PROD flags (-O2), not the default -g -O0 build. This target used to
# run the plain `all` target, and -Wstrict-aliasing / -Wstringop-truncation /
# -Wrestrict need optimisation to fire at all -- so it reported 0/0 on a
# configuration nobody ships while `make prod` (what CI releases) emitted 69
# warnings, two of them real overlapping-strcpy bugs. A sweep that cannot see
# the shipping build is a sweep that cannot fail.
#
# The linux leg is pinned to --platform linux/amd64. Unpinned, whatever gcc:13
# happens to be cached locally is used: on this machine that was a ppc64le
# image running under QEMU, so the leg named "linux" was compiling for
# powerpc64le and not for the architecture CI builds.
warnings:
	@mkdir -p $(WARNDIR)
	@echo "=== host ($(CC)) ==="
	@$(MAKE) -B --no-print-directory prod >$(WARNDIR)/host.log 2>&1 \
	  || echo "  BUILD FAILED -- see $(WARNDIR)/host.log"
	@$(TALLY) $(WARNDIR)/host.log
	@echo "=== linux (gcc, in docker) ==="
	@docker run --rm --platform linux/amd64 -v "$(CURDIR)/Source:/src/Source:ro" -v "$(CURDIR)/GNUmakefile:/src/GNUmakefile:ro" \
	  -v $(WARNDIR):/out -w /src gcc:13 sh -c 'set -e; mkdir -p /tmp/b; \
	  make CC=gcc OBJDIR=/tmp/b EXE=/tmp/b/os9exec prod 2>&1; \
	  cp /tmp/b/os9exec /out/linux-built' >$(WARNDIR)/linux.log 2>&1 \
	  || echo "  BUILD OR SETUP FAILED -- see $(WARNDIR)/linux.log"
	@test -x $(WARNDIR)/linux-built \
	  || echo "  NOT BUILT -- no Linux binary produced; the score below means nothing"
	@$(TALLY) $(WARNDIR)/linux.log
	@echo "=== windows x86_64 (mingw-w64, LLP64) ==="
	@$(MAKE) -B --no-print-directory OS=Windows_NT CC=x86_64-w64-mingw32-gcc prod \
	  OBJDIR=$(WARNDIR)/win EXE=$(WARNDIR)/win/os9exec.exe \
	  >$(WARNDIR)/win.log 2>&1 \
	  || echo "  BUILD FAILED -- see $(WARNDIR)/win.log"
	@test -f $(WARNDIR)/win/os9exec.exe \
	  || echo "  NOT BUILT -- no Windows binary produced; the score below means nothing"
	@$(TALLY) $(WARNDIR)/win.log
	@echo "=== windows i686 (mingw-w64, ILP32) ==="
	@$(MAKE) -B --no-print-directory OS=Windows_NT CC=i686-w64-mingw32-gcc prod \
	  OBJDIR=$(WARNDIR)/win32 EXE=$(WARNDIR)/win32/os9exec.exe \
	  >$(WARNDIR)/win32.log 2>&1 \
	  || echo "  BUILD FAILED -- see $(WARNDIR)/win32.log"
	@test -f $(WARNDIR)/win32/os9exec.exe \
	  || echo "  NOT BUILT -- no Windows binary produced; the score below means nothing"
	@$(TALLY) $(WARNDIR)/win32.log
	@rm -rf $(WARNDIR)/win $(WARNDIR)/win32 $(WARNDIR)/linux-built
	@echo "logs: $(WARNDIR)"

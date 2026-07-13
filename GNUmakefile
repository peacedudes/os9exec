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
    $(CORE)/icalls.c \
    $(CORE)/int_move.c \
    $(CORE)/int_rename.c \
    $(CORE)/intcommand.c \
    $(CORE)/memstuff.c \
    $(CORE)/modstuff.c \
    $(CORE)/os9exec_nt.c \
    $(CORE)/os9main.c \
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

.PHONY: all prod clean test

all: $(OBJDIR) $(EXE)

# Production build: optimised, no debug symbols.
# Usage: make prod   (rebuilds from scratch with -O2)
prod:
	$(MAKE) -B CFLAGS="-O2 -Wall -fcommon \
	          -DTERMINAL_CONSOLE \
	          -DINT_CMD \
	          -DRAM_SUPPORT \
	          -I$(CORE) \
	          -I$(CORE)/os9defs \
	          -I$(PLAT) \
	          -ISource/Platforms \
	          -I$(UAE) \
	          -I$(APPEMU)"


$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lm

$(OBJDIR)/cpuemu.o: $(UAE)/cpuemu.c
	$(CC) $(CFLAGS) $(UAE_SUPPRESS) -c $< -o $@

$(OBJDIR)/newcpu.o: $(UAE)/newcpu.c
	$(CC) $(CFLAGS) $(UAE_SUPPRESS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) os9exec os9exec.exe

test: $(EXE)
	swift run --package-path test

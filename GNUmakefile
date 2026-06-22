CC      = clang
CORE    = Source/OS9exec_core
PLAT    = Source/Platforms/LINUX
NATIVE  = Source/NATIVE
OBJDIR  = build

CFLAGS  = -g -Wall \
          -DTERMINAL_CONSOLE \
          -I$(CORE) \
          -I$(CORE)/os9defs \
          -I$(PLAT) \
          -ISource/Platforms

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
    $(CORE)/int_dir.c \
    $(CORE)/int_move.c \
    $(CORE)/int_rename.c \
    $(CORE)/intcommand.c \
    $(CORE)/memstuff.c \
    $(CORE)/modstuff.c \
    $(CORE)/os9exec_nt.c \
    $(CORE)/os9main.c \
    $(CORE)/os9_llm_unix.c \
    $(CORE)/pipefiles.c \
    $(CORE)/printer.c \
    $(CORE)/procstuff.c \
    $(CORE)/telnetaccess.c \
    $(CORE)/utilstuff.c \
    $(CORE)/vmod.c \
    $(PLAT)/linuxfiles.c

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SRCS)))

VPATH = $(CORE):$(PLAT):Source/OS9execMPW

.PHONY: all clean

all: $(OBJDIR) os9exec

os9exec: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) os9exec

#!/usr/bin/env bash
# build.sh -- compile test/68k-utils/SRC/*.c into test/68k-utils/CMDS/.
#
# These are clean-room reimplementations of OS-9/68000 utilities, written from
# the published manuals.  The SOURCE is ours and is committed; the compiled
# module is not (see .gitignore), because the vendor's C runtime is linked into
# it and is not ours to redistribute.
#
#   tools/68k-utils/build.sh          build everything in SRC/
#   tools/68k-utils/build.sh load     build just this one
#
# WHAT IT NEEDS
#
#   OS9DISK   a licensed OS-9/68000 system disk.  Supplies /dd -- the shell this
#             script drives, and DEFS, which carries module.h and the other
#             headers.  Headers are text and contribute no bytes to a module.
#
#   OS9SDK    OPTIONAL, and the difference between a module you can give away
#             and one you cannot.  A disk carrying a COMPLETE C library set whose
#             cstart.r has no licensee watermark.  When set, the libraries and
#             cstart both come from here -- a MATCHED set, which is what makes
#             this the route to prefer.  Not a newer compiler: the compilers are
#             byte-identical (see below).
#
#   OS9CSTART OPTIONAL: an OS-9 path to a watermark-free cstart.r to link in
#             place of $CLIB/cstart.r, keeping everything else on $OS9DISK.
#             Needs no second disk.  On this system that is
#             /dd/USR/DOG/cstart.r, and it is the same generation as the
#             stamped one -- see below.  This is the route to reach for first.
#
# THE WATERMARK, AND WHY OS9SDK EXISTS
#
# cstart.r is linked into every C program the compiler builds, and on a
# personalised system disk it carries a banner naming the licensee -- literally
# ">>>> from the disk of <name> <<<<", sitting in the code psect.  Every binary
# built against it is stamped with that name, which is the last thing you want
# on something being handed to a freeware collection.
#
# It is NOT a compiler difference.  Measured with ident: cc, cpp, c68, o68, r68
# and l68 are byte-identical across the disks here -- same size, same edition,
# same CRC.
#
# Nor is /dd/USR/DOG/cstart.r an older or foreign cstart, which is what its file
# mtime suggests and what an earlier version of this comment claimed.  Host
# mtimes record when a file was COPIED.  The build date is in the ROF header at
# offset 12, and both of these are 1989:
#
#   /dd/LIB/cstart.r      1355 bytes, STAMPED     built 1989-06-05
#   /dd/USR/DOG/cstart.r  1274 bytes, unstamped   built 1989-04-07
#
# They are the same source.  cstart.a carries the banner as a plain data block
# at the head of the psect --
#
#     Author: dc.b ">>>>>>>>>>>>>>>>"
#             dc.b "from the disk of"
#             dc.b " <the licensee> "
#             dc.b "<<<<<<<<<<<<<<<<"
#
# -- a label nothing references, preceded by filler words.  Assembling that
# source with the four dc.b lines and the filler removed reproduces
# /dd/USR/DOG/cstart.r EXACTLY: 1274 bytes, and the only bytes that differ are
# the six-byte assembly date in the ROF header.  So the unstamped cstart is not
# something to be wary of pairing with this disk's libraries; it is this disk's
# own cstart with the nameplate left off.
#
# Only cstart.r carries it: cio.l, clibn.l, math.l and sys.l were all checked
# and are clean.  But the watermark-free cstart.r copies available here (the
# SDK's own, and ansi_cstart.r/acstart.r) are all NEWER than the personalised
# disk's linker, which rejects them outright:
#
#   l68: error - psect 'cstart_a' in file '...' created by assembler too new
#        for this linker
#
# The fix is therefore a cstart the linker will actually accept.  Two work:
#
#   OS9CSTART  the unstamped cstart against this disk's own libraries.  4148
#              bytes.  Preferred: no second disk, and nothing is mixed.
#
#   OS9SDK     a second disk carrying a DIFFERENT build of the C libraries
#              (clibn.l 35466 against this disk's 38523).  4110 bytes.  Smaller,
#              but it is a different library build, not a newer cstart, and it
#
#              is missing exactly THREE functions by exact-symbol comparison:
#              fgetc, fputc and setvbuf.  Nothing is in it that this disk's
#              library lacks.  getc/putc are macros in stdio.h and unaffected,
#              but all three missing names ARE declared there, so a program
#              calling one compiles clean and fails at LINK time on an
#              unresolved symbol.  If that happens, this is why.
#
# With neither, this still builds; the result simply carries the watermark, and
# the check at the end says so rather than letting it pass unnoticed.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SRC="$REPO/test/68k-utils/SRC"
OUT="$REPO/test/68k-utils/CMDS"
EXE="$REPO/os9exec"
TIMEOUT=$(command -v gtimeout || command -v timeout)

[ -n "$TIMEOUT" ] || { echo "need gtimeout or timeout on PATH" >&2; exit 2; }
[ -x "$EXE" ]     || { echo "no os9exec at $EXE -- run make first" >&2; exit 2; }

# Refuse rather than guess.  Silently building against the wrong disk produces a
# module that looks right and was linked against another system's library, and
# nothing downstream would say so.
if [ -z "${OS9DISK:-}" ] || [ ! -d "$OS9DISK" ]; then
    echo "build.sh: OS9DISK must name an OS-9/68000 system disk." >&2
    echo "          export OS9DISK=/path/to/system/disk" >&2
    exit 2
fi
[ -f "$OS9DISK/CMDS/shell" ] || { echo "build.sh: no CMDS/shell on \$OS9DISK ($OS9DISK)" >&2; exit 2; }

# Where the toolchain comes from.  /h7 is the clean SDK when there is one and
# is simply not mounted otherwise, in which case /dd supplies everything.
sdk="${OS9SDK:-}"
cstart="${OS9CSTART:-}"
if [ -n "$sdk" ] && [ ! -e "$sdk" ]; then
    echo "build.sh: OS9SDK is set but '$sdk' does not exist" >&2
    exit 2
fi
if [ -n "$sdk" ]; then
    tools=/h7/CMDS; libs=/h7/LIB; cstart=""
    echo "toolchain: \$OS9SDK ($sdk) -- matched set"
else
    tools=/dd/CMDS; libs=/dd/LIB
    [ -f "$OS9DISK/CMDS/cc" ] || { echo "build.sh: no CMDS/cc on \$OS9DISK and no \$OS9SDK" >&2; exit 2; }
    if [ -n "$cstart" ]; then
        echo "toolchain: \$OS9DISK, with cstart from \$OS9CSTART ($cstart)"
    elif grep -a -q 'from the disk of' "$OS9DISK/LIB/cstart.r" 2>/dev/null; then
        echo "toolchain: \$OS9DISK -- its LIB/cstart.r is TAGGED; set OS9CSTART or OS9SDK"
    else
        echo "toolchain: \$OS9DISK (LIB/cstart.r is untagged)"
    fi
fi

mkdir -p "$OUT" "$REPO/test/68k-utils/SCRATCH"

names=("$@")
if [ ${#names[@]} -eq 0 ]; then
    names=()
    for f in "$SRC"/*.c; do
        [ -e "$f" ] || continue
        names+=("$(basename "$f" .c)")
    done
fi
[ ${#names[@]} -gt 0 ] || { echo "build.sh: nothing to build in $SRC" >&2; exit 1; }

# One shell session for the lot.
#
# chx is what makes cc able to fork cpp/c68/o68/r68/l68 by bare name, so it has
# to point at the toolchain being used, not merely at a place that has one.
#
# -I links against the `cio` C-I/O TRAP HANDLER instead of copying the C library
# into the module.  It is worth 4x -- load is 4112 bytes this way and 16848 built
# statically -- because stdio then lives once in a shared module rather than once
# per program, which is the point of a trap handler and how OS-9's own utilities
# are built.  The cost is that the module needs cio reachable from its execution
# directory at run time, and dies with "Can't install trap handler" where it is
# not.  The osk-freeware disk ships cio with Microware's permission, and an
# ordinary system disk has it in CMDS.
#
# -FD= puts the module where the DATA directory says rather than where the
# EXECUTION directory does; a bare -o=/-F= would land it in the toolchain's own
# CMDS, which is the classic way to write into somebody's licensed disk while
# believing the build failed.
log=$(mktemp)
{
    echo "setenv CLIB $libs"
    echo "setenv CDEF /dd/DEFS"
    echo "chx $tools"
    echo "chd /h8/SRC"
    for n in "${names[@]}"; do
        if [ -n "$cstart" ]; then
            # Two steps, because cc always puts $CLIB/cstart.r at the front of
            # its own link line and offers no way to say otherwise.  The l68
            # line below is cc -I's own, copied from `cc -I -BP` output, with
            # only the cstart swapped -- so this is the same link, not a
            # reconstruction of one.
            echo "cc -I -r=/h8/SCRATCH $n.c"
            echo "l68 $cstart /h8/SCRATCH/$n.r -O=/h8/CMDS/$n -l=$libs/cio.l -l=$libs/clibn.l -l=$libs/math.l -l=$libs/sys.l -a"
        else
            echo "cc -I $n.c -FD=/h8/CMDS/$n"
        fi
    done
    echo stop
} | $TIMEOUT 600 env OS9STOP=1 OS9DISK="$OS9DISK" OS9H7="$sdk" \
      OS9H8="$REPO/test/68k-utils" "$EXE" -r /dd/CMDS/shell > "$log" 2>&1

# The compiler names its phases as it runs them and says nothing on success, so
# "no error text" is not evidence.  Check for the file, and check the warnings
# separately: this tree is warning-clean and a new one is a defect, not noise.
rc=0
if grep -a -q 'warning' "$log"; then
    echo "build.sh: compiler warnings -- this tree is meant to be warning-clean:" >&2
    tr '\r' '\n' < "$log" | grep -a -A2 'warning' | sed 's/^/  /' >&2
    rc=1
fi
for n in "${names[@]}"; do
    if [ ! -s "$OUT/$n" ]; then
        echo "  $n did not build -- full log in $log" >&2
        rc=1
        continue
    fi
    # l68 writes through the host filesystem as 0644, dropping the execute bit
    # -- and a host mode bit is what becomes the OS-9 `e` attribute on the way
    # out.  Without this the module cannot be forked by name.
    chmod 755 "$OUT/$n"

    # Say what came out, every time.  A module that is quietly stamped is one
    # that gets given away stamped; the whole reason OS9SDK exists is this line.
    if grep -a -q 'from the disk of' "$OUT/$n"; then
        printf '  built %-12s %6s bytes  WATERMARKED -- not for redistribution\n' \
            "$n" "$(wc -c < "$OUT/$n" | tr -d ' ')"
    else
        printf '  built %-12s %6s bytes  clean\n' \
            "$n" "$(wc -c < "$OUT/$n" | tr -d ' ')"
    fi
done

rm -f "$REPO"/test/68k-utils/SCRATCH/*.r
[ $rc -eq 0 ] && rm -f "$log"
exit $rc

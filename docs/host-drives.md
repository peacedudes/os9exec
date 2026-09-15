# Host drives: how they work, and how they fail

A device such as `/dd` or `/h5` can be one of two things:

- an **RBF image**: a file holding a real OS-9 disk. os9exec runs its own RBF
  file manager over it, with sectors, file descriptors, an allocation map,
  record locking and owner checks.
- a **host drive**: an ordinary directory on the machine running os9exec. Every
  OS-9 file is a host file, and every OS-9 call becomes a host call made at that
  moment.

A host drive is a convenience bridge. It is immediate: edit a file on the host
and the next OS-9 open sees it. It has no counterpart on real OS-9, so most of
what RBF promises is imitated or missing. This page lists what a host drive
does, and exactly where a program written for real OS-9 will go wrong on one.

**If a program depends on locking, ownership, permissions or disk structure,
put its files in an RBF image** (`mount -k=<size> h5` makes a blank one).

Unless stated otherwise, the behaviour below was measured on 2026-09-15
(os9exec 78d03c1, macOS arm64). The Linux case behaviour was measured in the
`os9exec:linux` Docker image, on the container's own filesystem.

## At a glance

| | RBF image | Host drive |
|---|---|---|
| Record locking, EOF lock | yes | **none** |
| `SS_Lock` on a file | works | `E$UnkSvc` |
| Delete a file another path has open | `E$Share` | succeeds (Unix); refused by the host (Windows) |
| Owner | stored, checked | always `0.0`, never checked |
| Attributes | stored in the FD, checked by RBF | host mode bits, checked by the host kernel |
| Super user bypass | reads and writes anything | none, except read on files it creates |
| File descriptor sector | real | synthesized on each request |
| "Sector" numbers (`dir -e`, `PD_FD`) | disk LSNs | numbers for host path names |
| Identification sector, allocation map | real | sector 0 invented; nothing else |
| Directory contents | a file of 32-byte entries | synthesized from the host listing |
| Name length | 28 characters | shown cut to 27 |
| Name case | ignored | ignored by lookup; the host keeps the spelling |
| `del` of an empty directory | refused, `E$FNA` | **removed** |
| Line endings | bytes | bytes; no translation either way |

## How a host drive works

### Files

A file open is a host `fopen`. Each OS-9 path gets its own C stdio stream,
and every `I$Write` and `I$WritLn` is flushed to the host before the call
returns. Nothing is cached between opens: the host file is the only copy.

An open with access mode 0 (neither read nor write, as `attr` uses) opens no
host stream at all. `SS_Size`, `SS_Pos`, `SS_EOF` and seek answer from the
host's file information, and reads and writes are `E$BMode`. This is what lets
`attr` restore access to a file whose read permission has been removed.

### Directories

A host directory has no entries to read, so os9exec makes them up. Reading a
directory returns 32-byte RBF-style entries built from the host listing:

- the first entry is `..`, the second `.`, then the host's own order (not
  sorted, and not necessarily the same order twice);
- `.AppleDouble` and `.DS_Store` are left out;
- names longer than 27 characters are cut to 27, and spaces show as `_`;
- on the Linux build, a host name starting `:2e` (netatalk's spelling of a
  leading dot) shows as `.`;
- the 3-byte file-descriptor field holds a number standing for that host path
  (see [Sector numbers](#sector-numbers-are-path-names)).

At a device root the `..` entry carries the root's own number, as an RBF root
does. That is how `pd` and `dsave` know to stop.

### Name lookup

A pathlist is matched to the host in two steps, per component:

1. the name exactly as given (with any space turned into `_`);
2. if that does not exist, a scan of the host directory for the **first**
   entry whose shown name (27 characters, spaces as `_`) matches, ignoring
   case.

So lookup ignores case on every host, Linux included: `list /h5/sub/file.txt`
opens `Sub/File.txt`. A long host name can be opened by its 27-character
shown name, and a space name by its `_` spelling.

### File descriptors

`SS_FD` builds a 16-byte file descriptor on each request:

- owner `0.0`;
- attributes from the host mode bits (below);
- modification date from the host, to the minute;
- "creation" date from the host's `st_ctime`, which on Unix is the last
  status change, not the creation date;
- link count 1, and no segment list.

Setting the FD applies only the modification date and the attribute byte.

### Attributes

| OS-9 bit | Unix host | Windows host |
|---|---|---|
| owner read / write / execute | user `r` / `w` / `x` | read: an ACL deny for Everyone; write: the read-only flag; execute: always shown on |
| public read / write / execute | group and other `r` / `w` / `x` | the same single flags as owner |
| `s` (single user) | not stored | not stored |
| `d` | from the host's file type | from the host's file type |

A directory always shows every permission. `makdir` creates the host directory
with mode `0700`. A file created from OS-9 gets the attribute byte its
`I$Create` asked for, applied to the host file. The shell's `>` redirect gives
owner read and write, host mode `0600`.

### The raw device

Opening `/h5@` returns one invented identification sector (volume name, host
capacity in `DD_TOT`), so the C library's `stat()` works. Reading past it is
`E$EOF`. There is no allocation map, FD sector or segment to read.

## How it fails

### No locking: concurrent writers lose data

RBF locks the record a read-for-update path reads, until that path writes.
It also holds an EOF lock, so a reader following a writer waits at the end of
file. **A host drive has none of this.** Nothing coordinates two processes,
whether both are OS-9 processes in one emulator, two os9exec runs, or a host
program.

- **Lost updates.** Two processes that each read a record, change it and write
  it back will both succeed, and the second write silently undoes the first.
  Databases, counters and sequence files are exposed.
- **Premature end of file.** A reader following a file that another process is
  still writing gets `E$EOF` when it catches up, instead of waiting.
- **Stale reads.** Every write is flushed, but a reader's own stdio buffer can
  still hold bytes it read before the write.
- **An explicit lock is refused honestly.** `SS_Lock` on a host file is
  `E$UnkSvc`, so a program that asks for a lock learns it did not get one.
  What goes missing silently is the automatic locking nobody asked for.

This is deliberate (DECISIONS-68k.md, "Host-native record locking"). A host
lock could only ever be half-true while host tools can change the file.

### Deleting an open file

RBF refuses with `E$Share`. On a Unix host the delete succeeds: the name is
gone, and paths already open keep reading and writing the unlinked data until
they close. On Windows the host itself refuses (recorded in DECISIONS-68k.md,
not re-measured here).

### No user IDs

Every file reports owner `0.0`, and nothing compares owners. Whichever OS-9
user is logged in, every file access is made by one host user: the one running
os9exec. Group and user distinctions do not exist.

- A non-super user who `copy`s from a host drive into an RBF image without
  `-n` gets `E$PERMIT`, because copying the FD means giving the new file owner
  `0.0`. The data is still copied. Use `copy -n`.
- A file its owner made read-only stays read-only for that owner.

### Permissions are real host permissions

Attributes are enforced by the host kernel, not by OS-9 rules, and the kernel
knows nothing about OS-9 users:

- **The super user is not exempt.** On RBF, group 0 may open any file. On a
  host drive a file without read permission cannot be read by anyone. The one
  exception: a file the super user creates always keeps owner read (78d03c1),
  so a program that creates its output with attributes `$0000` can read it
  back. It never gets write.
- **A non-super user creating with attributes 0** makes a file nobody can read
  until `attr` restores it.
- **`e` is reported, not enforced** for loading or running a module.
- **Docker bind mounts on macOS ignore permissions entirely**: `chmod` is
  recorded and not enforced, and ownership is faked to match the caller.

### Sector numbers are path names

The number `dir -e` shows as "Sector", and the `PD_FD` `SS_Opt` returns, is a
number for a host **path**. It lives only in the running emulator and is not a
disk address:

- renaming a file changes its number;
- two hard links to one host file get two different numbers, so a program that
  compares FD numbers to find the same file will not see they are one;
- `SS_FD`-by-sector calls work only for numbers this run has handed out;
- tools that read sectors (`dcheck`, `free`, `format`, freeware's `dinfo`,
  `freeb`, `dam`, `howfrag`) have no disk to read.

### Names

- **Long names.** A host name longer than 27 characters shows cut to 27, and
  opens only by that cut name: the full name is `E$PNNF`. Two host names with
  the same first 27 characters show as two identical entries, and only the
  first one the host lists can be opened.
- **Spaces.** A host name with spaces shows with `_`. If the host also has a
  name spelt with `_`, that exact file wins, and the spaced one cannot be
  reached.
- **Case.** Lookup ignores case, and a new name is created exactly as spelt.
  On a case-sensitive host (Linux), `makdir sub` next to an existing `Sub` is
  `E$CEF`. When two host names differ only in case, the exact spelling reaches
  its own file, and any other spelling reaches whichever the host lists first.
- **Dot names on Linux.** The Linux build respells a leading `.` in a host
  name as `:2e` for netatalk. RBF images are not affected.

### Writing a directory

A program may rewrite a host directory's entries only in the ways that mean a
rename (this is how Microware's `rename()`, `move`, `wndex` and `upperdir`
work):

- rewrite an existing entry with a new name: the host file is renamed;
- `move`'s append-then-clear: the host file moves to this directory.

Everything else is `E$BMode`: new entries, clearing an entry to delete it,
another file's FD number, partial writes. A new name that another entry
already answers to, ignoring case, is `E$CEF`, because a host rename would
replace that file. Device roots, host names the entry does not spell exactly
(long or spaced names), and a mounted image's file are refused. Paths open on a
renamed file, and processes whose current directory is inside it, follow the
rename.

### `del` on a directory

RBF refuses `del` on any directory, `E$FNA`. A host drive **removes an empty
directory** silently, and refuses a non-empty one with `E$DNE`. Measure `del`
behaviour on an image, never on a host drive.

### Line endings are not translated

A host drive passes bytes unchanged. `I$ReadLn` stops only at a carriage return
(`$0D`), so a host file with LF line endings reads as one long line:
`linecount` reports 0 lines for a three-line LF file. Files written from OS-9
contain CR. Convert host-edited text with `flip -m` (and `flip -t` first, since
running `-m` on a file that is already CR-only collapses it).

### The host can change files underneath

Nothing stops host programs, other emulators or sync tools from changing a
host drive while OS-9 uses it:

- a host editor that saves by writing a new file and renaming it leaves an
  OS-9 path that had the file open reading the old copy;
- `dir` reads the host listing by position, so a listing taken while files are
  created or removed can skip or repeat entries (from the code; not measured);
- a file removed on the host while an OS-9 program holds its sector number
  leaves that number pointing at nothing.

### Symbolic and hard links

Host links are followed.

- A symlink whose target is inside a configured device works like the target.
- A **file** symlink pointing outside every device is `E$FNA`.
- **Known defect:** a **directory** symlink pointing outside every device
  silently resolves to the device root. `dir /h5/out` lists `/h5` itself, and
  `chd /h5/out` then `pd` prints `/h5`, so anything written "through" the link
  lands in the root.
- `del` of a symlink removes the link, not the target.
- Hard links: see [sector numbers](#sector-numbers-are-path-names). Renaming
  one name leaves the other.

### Where a device may live

- **Known defect:** a device whose host path contains a space cannot be used.
  `OS9H5="/tmp/has space"` gives `E$PNNF` for `dir /h5`, because path
  resolution turns every space in the host path into `_`, including the part
  that names the device itself.
- `..` at a device root stays at the root. Nested devices are the exception:
  if one device's host directory lies inside another's, `..` from the inner
  root walks into the outer device.
- A path outside every configured device is `E$PNNF`.

### Docker and virtiofs

On a macOS directory bind-mounted into a Linux container:

- permissions are not enforced, and ownership is faked (above);
- a case-only rename reports success and keeps the old case (os9exec renames
  through a temporary name to get around it);
- a file deleted on the host can still appear to the next container.

### Windows

- There is no execute bit, and write permission is one flag for everyone.
- `attr` shows read as the C runtime sees it, which does not reflect the ACL
  that actually denies read.
- The host refuses to delete a file that is open (DECISIONS-68k.md, "`E$Share`
  on deleting an open host-native file").
- Names Windows reserves (`con`, `nul`, `aux` and the like) are not mapped to
  anything else, so the host decides what happens to them.

### Limits

- File sizes are 32-bit, as on OS-9: a host file of 4 GB or more reports a
  wrong size.
- Dates carry minutes, not seconds.

## Choosing

Use a **host drive** to move files in and out, edit sources on the host, and
run programs that only read and write their own files one at a time.

Use an **RBF image** when correctness matters: multi-process access to shared
files, anything with user accounts or permissions, disk utilities, backups by
sector, and any test of what real OS-9 does.

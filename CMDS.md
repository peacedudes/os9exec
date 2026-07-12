# OS-9/68k SDK — CMDS catalog

Commands from the Microware OS-9/68k SDK (`OS9/68000/CMDS`), tested under
the os9exec arm64 emulator.

**✓** confirmed working · **—** not functional in this environment (reason in description) · blank = untested or requires arguments to do anything useful

---

### File and directory tools

| Command | What it does | Status |
|---------|-------------|--------|
| `attr` | Show or set file attributes and permissions | ✓ |
| `chown` | Change file ownership (group.user) | ✓ |
| `cmp` | Compare two files byte-by-byte | ✓ |
| `copy` | Copy files or directories | ✓ |
| `del` | Delete files | ✓ |
| `deldir` | Delete a directory | ✓ |
| `dir` | List directory contents | ✓ |
| `dsave` | Generate a shell script to recreate a directory tree | ✓ |
| `free` | Report free space on a disk device; works on RBF images (`/h0`, etc.); host-directory paths (`/dd`) return a clean "File Not Accessible" error | ✓ |
| `makdir` | Create a directory | ✓ |
| `mv` | Move a file (os9exec uses its built-in `mv`; the OS-9 binary only works on RBF) | ✓ |
| `pd` | Print current working directory | ✓ |
| `rename` | Rename a file within a directory | ✓ |
| `touch` | Create a file or update its timestamp | ✓ |
| `undel` | Undelete a file (RBF-specific; works on RBF image, no-op on host directories) | ✓ |

### Text and data tools

| Command | What it does | Status |
|---------|-------------|--------|
| `build` | Build a short text file from standard input | ✓ |
| `compress` | Compress a file in place (LZH format) | ✓ |
| `count` | Count lines, words, and bytes in a file | ✓ |
| `dump` | Hex dump of a file | ✓ |
| `edt` | Line-oriented text editor; works without TERM | ✓ |
| `expand` | Decompress a file compressed by `compress` | ✓ |
| `grep` | Search files for a regular expression | ✓ |
| `list` | Display a text file (like `cat`) | ✓ |
| `merge` | Concatenate files | ✓ |
| `pr` | Format and paginate a text file | ✓ |
| `qsort` | In-memory quick sort (reads stdin or file) | ✓ |
| `tar` | Create, list, or extract tar archives | ✓ |
| `tee` | Copy stdin to stdout and to a file simultaneously | ✓ |
| `tr` | Translate or delete characters | ✓ |
| `umacs` | Micro-Emacs editor (requires TERM environment variable) | — |
| `what` | Extract embedded `@(#)` version strings from a binary | ✓ |

### Module and binary tools

| Command | What it does | Status |
|---------|-------------|--------|
| `binex` | Convert binary module to Motorola S-record (hex) format | ✓ |
| `cudo` | Convert OS-9/68k module to OS-9000 format (modifies file in place) | ✓ |
| `dcheck` | Verify RBF disk integrity; works on RBF images (`/h0`, etc.); host-directory paths (`/dd`) return a clean "File Not Accessible" error | ✓ |
| `editmod` | Show or edit module header fields | ✓ |
| `exbin` | Convert Motorola S-record back to binary module | ✓ |
| `fixmod` | Recalculate and fix module header CRC and parity | ✓ |
| `ident` | Display module header information | ✓ |
| `link` | Link a module into memory by name | ✓ |
| `load` | Load a module from disk into memory | ✓ |
| `mkdatmod` | Package a file into an OS-9 data module | ✓ |
| `moded` | Module field editor (requires `moded.fields` config file, not included) | — |
| `padrom` | Pad a file to a target size with `0xFF` | ✓ |
| `romsplit` | Split a ROM image into interleaved files | |
| `save` | Save an in-memory module to a file | ✓ |
| `unlink` | Unlink a module from memory | ✓ |

### System information

| Command | What it does | Status |
|---------|-------------|--------|
| `date` | Display the current date and time | ✓ |
| `debug` | Launch OS-9 symbolic debugger front-end | ✓ |
| `deiniz` | Detach (de-initialize) a device | ✓ |
| `devs` | List mounted devices (device table is empty — OS-9 devices are not populated in the emulator, but the command runs without crashing) | ✓ |
| `events` | List OS-9 system events | ✓ |
| `help` | Display help text for OS-9 utilities | ✓ |
| `iniz` | Initialize (attach) a device | ✓ |
| `irqs` | Display IRQ assignments (`F$SysID` syscall not implemented) | — |
| `maps` | Show SSM memory allocation map (SSM not emulated) | — |
| `mdir` | List all loaded modules and their attributes | ✓ |
| `mfree` | Show total free RAM | ✓ |
| `paths` | List open paths for all processes | ✓ |
| `printenv` | Display environment variables | ✓ |
| `procs` | List running OS-9 processes | ✓ |
| `setime` | Set system time (time is taken from the host; any value entered is silently ignored — works as it did in the 1990s) | ✓ |

### Shell and process tools

| Command | What it does | Status |
|---------|-------------|--------|
| `break` | Halt timesharing and enter the OS-9 debugger | ✓ |
| `echo` | Write text to stdout | ✓ |
| `make` | Build targets from a Makefile; fails gracefully if no `makefile` present | ✓ |
| `os9gen` | Write OS-9 boot track to a device (requires raw device access) | — |
| `shell` | Start a new interactive OS-9 shell | ✓ |
| `sleep` | Suspend execution for N seconds | ✓ |

### Disk and storage tools

| Command | What it does | Status |
|---------|-------------|--------|
| `backup` | Back up an RBF disk to tape (requires tape hardware) | — |
| `diskcache` | Configure disk sector cache (operates silently with no output) | ✓ |
| `dpsplit` | Split DPIO device descriptors into sub-parts | ✓ |
| `format` | Low-level format an RBF disk image | |
| `frestore` | Restore a filesystem from tape (requires tape hardware) | — |
| `fsave` | Dump a filesystem to tape (requires tape hardware) | — |
| `mount` | Mount an RBF disk image as a device | ✓ |
| `p2init` | Link and initialize an OS-9 Phase 2 module | ✓ |
| `partdgen` | Generate a PC-format partition descriptor | ✓ |
| `partition` | Partition a large (>4GB) hard disk | ✓ |
| `pcformat` | Format a PC-style floppy (requires hardware) | — |
| `tape` | Tape drive control (requires hardware) | — |
| `tapegen` | Generate a tape boot track (requires `/mt0` tape device) | — |
| `tapestart` | Start tape streaming (requires `/mt0` tape device) | — |

### Terminal and I/O tools

| Command | What it does | Status |
|---------|-------------|--------|
| `bfed` | Screen-oriented binary file editor (requires TERM environment variable) | — |
| `cfp` | Floating-point coprocessor utility | ✓ |
| `cio` | Communications I/O module (module not present in SDK distribution) | — |
| `code` | Return hex keycode of terminal keypresses; type keys one at a time, ^E to exit | ✓ |
| `com` | Serial port communication (requires hardware) | — |
| `kermit` | Kermit file transfer over serial (requires hardware) | — |
| `tmode` | Show or set terminal mode parameters | ✓ |
| `tsmon` | Timesharing terminal monitor | ✓ |
| `xmode` | Show or set extended SCF/GFM device descriptor parameters (needs descriptor modules loaded) | ✓ |

### Miscellaneous

| Command | What it does | Status |
|---------|-------------|--------|
| `csl` | C Shell (requires external csl module not included) | — |
| `math` | Math library (requires external math module not included) | — |
| `maui` | Microware MAUI graphics (requires MAUI module not included) | — |
| `pwrstat` | Power management utility | ✓ |
| `login` | Login with password authentication; reads `/dd/SYS/password`; launches user's shell from the password entry | ✓ |
| `su` | Switch user identity (requires `/dd/SYS/password` file) | ✓ |

### Network tools

These require the SPF network stack and TCP/IP daemons, which are not present
in this emulated environment.  None will connect to anything.

| Command | What it does |
|---------|-------------|
| `arp` | Display or modify ARP cache |
| `beam` | Send a short UDP message to a host |
| `bootpd` / `bootptest` | BOOTP server / test client |
| `chat` | Modem/serial chat script runner |
| `dhcp` | DHCP client |
| `dird` | Remote `dir` server |
| `ex1_rcv` / `ex1_snd` / `example3` | ITE network programming examples |
| `exportfs` | NFS export manager |
| `ftp` / `ftpd` / `ftpdc` | FTP client / daemon / connector |
| `hostname` | Get or set system hostname |
| `idbdump` / `idbgen` | Dump or generate internet database module |
| `ifconfig` | Configure network interfaces |
| `inetd` | Internet super-daemon |
| `ipstart` | Start the IP network stack |
| `lmm` | Load module manager |
| `mbdump` / `mbinstall` | Network buffer statistics / installer |
| `mountd` | NFS mount daemon |
| `mrecv` / `msend` / `msgd` / `rmsg` | Interprocess messaging |
| `mshell` | Remote network shell |
| `ndbmod` | Network database module tool |
| `ndpio` | Network DPIO module |
| `netstat` | Show network statistics |
| `nfsc` / `nfsd` / `nfsstat` | NFS client / server / statistics |
| `on` | Execute command on remote host |
| `pcnfsd` | PC NFS daemon |
| `ping` | ICMP echo test |
| `portmap` | RPC port mapper |
| `pppauth` / `pppd` | PPP authentication / daemon |
| `rcopy` | Copy file over network |
| `rdir` | Remote directory listing |
| `rexd` / `rexdc` | Remote execution daemon / connector |
| `rldd` / `rload` | Remote loader |
| `route` / `routed` | Routing table / routing daemon |
| `rpcdbgen` / `rpcdump` / `rpcgen` / `rpchost` / `rpcinfo` | RPC tools |
| `rpr` | Remote print |
| `rsort` / `sortd` | Remote sort / sort daemon |
| `rstatd` / `rup` / `rusers` / `rusersd` | Remote status daemons |
| `showmount` | Show NFS mounts |
| `spf_test` | SPF network stack test |
| `spfndpd` / `spfndpdc` / `spfnppd` / `spfnppdc` | SPF network protocol daemons |
| `spray` / `sprayd` | Network packet spray test |
| `target` | Network target service |
| `tcprecv` / `tcpsend` | TCP send / receive test programs |
| `telnet` / `telnetd` / `telnetdc` | Telnet client / daemon / connector |
| `tftpd` / `tftpdc` | TFTP daemon / connector |
| `undpd` / `undpdc` | UDP daemon / connector |

---

## dd/CMDS/SHARE — freeware and public-domain utilities

These are in `dd/CMDS/SHARE/`, on the default PATH (`SHARE:/h0/CMDS`).
Sources: Microware OS-9 archive, John-Titor/os9-m68k-ports, and the `smallutils` package.
See `dd/SOURCES.txt` in the disk image for full provenance.

**✓** confirmed working · **!** works with caveats · **—** known broken · blank = not yet tested

| Command | What it does | Status |
|---------|-------------|--------|
| `VI` | Full-screen vi editor — the recommended editor for this environment | ✓ |
| `vi_cio` | PVic v1.0a — vi-compatible editor (cio variant) | ✓ |
| `vi_nocio` | PVic v1.0a — vi-compatible editor (no-cio variant) | ✓ |
| `less` | Pager; requires `TERM` | ✓ |
| `emacs` | MicroEmacs 4.00; requires `TERM` and `emacs.mm1` | ✓ |
| `beav` | Binary Editor And Viewer; requires `TERM` | ✓ |
| `cat` | Concatenate and print files | ✓ |
| `tail` | Print last N lines of a file | ✓ |
| `basename` | Strip directory and suffix from a path | ✓ |
| `dirname` | Extract directory component of a path | ✓ |
| `exist` | Test whether a file exists (exits 0/1, no output) | ✓ |
| `mexist` | Test whether a module is loaded (exits 0/1, no output) | ✓ |
| `getenv` | Print value of a named environment variable | ✓ |
| `date` | Display current date and time (public-domain, edition 29) | ✓ |
| `ar` | OS-9/68k archive librarian | ✓ |
| `arc` | ARC archive compression/extraction | ✓ |
| `gzip` | GNU zip compression | ✓ |
| `lha` | LZH archiver (OS-9/68k port) | ✓ |
| `shar` | Create self-extracting shell archives | ✓ |
| `tar` | Tape archive; requires one of c/r/t/x/d flags | ✓ |
| `unzip` | Extract ZIP archives | ✓ |
| `zip` | Create and update ZIP archives (Info-ZIP v1.9) | ✓ |
| `zipnote` | View and edit ZIP archive comments (Info-ZIP v1.9) | ✓ |
| `zipsplit` | Split ZIP archives into smaller files (Info-ZIP v1.9) | ✓ |
| `ctags` | Generate vi tags file from C source | ✓ |
| `cal` | Display calendars with custom formatting and date ranges | ✓ |
| `rechne` | RPN calculator (German UI: "rechne" = calculate) | ✓ |
| `bash` | Bourne-Again Shell (OS-9/68k port); opens interactive session | ! |
| `sh` | Bourne shell v7.5 (OS-9/68k port); opens interactive session | ! |
| `ckermit` | C-Kermit v5A(190) — terminal emulator and file transfer | ! |
| `aprocs` | Enhanced process monitor; runs continuously until Ctrl-C | ! |
| `dpark` | Park a process in the background | ✓ |
| `fc` | Re-execute or list shell command history | ! |
| `sbreak` | Set or clear SS_Break signal on a path | ✓ |
| `setime` | Set system time; prompts for YYMMDDHHMMSS interactively | ! |
| `wysecrack` | Wyse terminal baud-rate detection; interactive | ! |
| `file` | File type identifier; determines file format from magic bytes | ✓ |
| `hexedit` | Hex editor for binary files; screen-oriented | ✓ |
| `screen` | Terminal multiplexer; multiple virtual terminals in one session | ✓ |
| `todos` | Convert text files from OS-9 to DOS line endings | ✓ |
| `toos9` | Convert text files from DOS to OS-9 line endings | ✓ |
| `setimex` | Set time-execute flag on files | ✓ |
| `upperdir` | Convert directory and filenames to uppercase | — |
| `autolf` | Auto line-feed utility | ✓ |
| `kermit` | Kermit variant; file transfer and terminal emulation | ! |
| `aterm` | Alternative terminal emulator v2.6 | ! |
| `gs33` | Ghostscript PostScript/PDF interpreter v3.3 | ! |
| **Mtools utilities** | MS-DOS filesystem utilities (17 commands) | |
| `msattrib` | Get/set MS-DOS file attributes | ✓ |
| `msbadblocks` | List bad sectors on MS-DOS disk | ✓ |
| `mscd` | Change MS-DOS directory | ✓ |
| `mscheck` | Check MS-DOS filesystem integrity | ✓ |
| `mscopy` | Copy files on MS-DOS disk | ✓ |
| `msdel` | Delete files on MS-DOS disk | ✓ |
| `msdeltree` | Delete directory tree on MS-DOS disk | ✓ |
| `msdir` | List MS-DOS directory contents | ✓ |
| `msformat` | Format MS-DOS disk | ✓ |
| `msinfo` | Show MS-DOS disk information | ✓ |
| `mslabel` | Get/set MS-DOS disk label | ✓ |
| `msmd` | Create MS-DOS directory | ✓ |
| `msmove` | Move/rename files on MS-DOS disk | ✓ |
| `msrd` | Remove MS-DOS directory | ✓ |
| `msread` | Read sectors from MS-DOS disk | ✓ |
| `msren` | Rename files on MS-DOS disk | ✓ |
| `mstoolstest` | Mtools test utility | ✓ |
| `mstype` | Display/copy MS-DOS text files | ✓ |
| `mswrite` | Write sectors to MS-DOS disk | ✓ |
| `mtools` | Mtools meta-command (displays available commands) | ✓ |
| **Games and Entertainment** | Interactive games and game engines | |
| `advent` | Colossal Cave Adventure (classic text game) | ! |
| `advent0` | Colossal Cave Adventure variant | ! |
| `advent1.txt` | Game data file (part of advent) | |
| `advent2.txt` | Game data file (part of advent) | |
| `advent3.txt` | Game data file (part of advent) | |
| `advent4.txt` | Game data file (part of advent) | |
| `gnuchess` | GNU Chess game engine | ✓ |
| `gnuchessr` | GNU Chess with reduced features | ✓ |
| `gnuchessn` | GNU Chess variant | ✓ |
| `infocom` | Infocom Z-machine interpreter for interactive fiction games | ✓ |
| `infocom.tcap` | Infocom with termcap support | ✓ |
| `paranoia` | Paranoia role-playing game system | ! |
| **Additional tools** | Expanded utility set from archives | |
| `ed` | GNU ed v0.2 — line-oriented text editor | ✓ |
| `find` | File finder v1.1.5 — search directories by criteria | ✓ |
| `makeinfo` | GNU Texinfo documentation generator | ✓ |

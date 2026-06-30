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
| `free` | Report free space on a disk device (works on RBF; zero-divide crash on native host directory) | ✓ |
| `makdir` | Create a directory | ✓ |
| `mv` | Move a file (requires RBF directory-write; fails E_BMODE on native `/dd`; use built-in `mv` / `move` instead) | — |
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
| `dcheck` | Verify RBF disk integrity (works on RBF image; requires `/hX` with a disk image) | ✓ |
| `editmod` | Show or edit module header fields | ✓ |
| `exbin` | Convert Motorola S-record back to binary module | ✓ |
| `fixmod` | Recalculate and fix module header CRC and parity | ✓ |
| `ident` | Display module header information | ✓ |
| `link` | Link a module into memory by name | ✓ |
| `load` | Load a module from disk into memory | ✓ |
| `mkdatmod` | Package a file into an OS-9 data module | ✓ |
| `moded` | Module field editor (requires `moded.fields` config file, not included) | — |
| `padrom` | Pad a file to a target size with `0xFF` | ✓ |
| `romsplit` | Split a ROM image into interleaved files (hangs when tested; likely expects specific ROM format) | — |
| `save` | Save an in-memory module to a file | ✓ |
| `unlink` | Unlink a module from memory | ✓ |

### System information

| Command | What it does | Status |
|---------|-------------|--------|
| `date` | Display the current date and time | ✓ |
| `debug` | Launch OS-9 symbolic debugger front-end | ✓ |
| `deiniz` | Detach (de-initialize) a device | ✓ |
| `devs` | List mounted devices | ✓ |
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
| `setime` | Set system date and time (loops on invalid input; avoid in non-interactive use) | — |

### Shell and process tools

| Command | What it does | Status |
|---------|-------------|--------|
| `break` | Halt timesharing and enter debugger (leaves terminal in raw mode; unsuitable for non-interactive testing) | — |
| `echo` | Write text to stdout | ✓ |
| `make` | Build targets from a Makefile; fails gracefully if no `makefile` present | ✓ |
| `on` | Execute a command on a remote host (requires SPF network stack) | — |
| `os9gen` | Write OS-9 boot track to a device (requires raw device access) | — |
| `shell` | Start a new interactive OS-9 shell | ✓ |
| `sleep` | Suspend execution for N seconds | ✓ |

### Disk and storage tools

| Command | What it does | Status |
|---------|-------------|--------|
| `backup` | Back up an RBF disk to tape (requires tape hardware) | — |
| `diskcache` | Configure disk sector cache (operates silently with no output) | ✓ |
| `dpsplit` | Split DPIO device descriptors into sub-parts | ✓ |
| `format` | Low-level format a disk (requires hardware) | — |
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
| `code` | Return hex keycode of a terminal keypress (requires interactive terminal input; cannot test non-interactively) | — |
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
| `su` | Switch user identity (requires `/dd/password` file) | — |

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
| `login` | Login with password authentication |
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

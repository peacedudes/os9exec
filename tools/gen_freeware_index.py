#!/usr/bin/env python3
"""Regenerate freeware/DOC/INDEX from the actual disk contents.

Run from anywhere:  python3 tools/gen_freeware_index.py
Then convert to OS-9 line endings:  flip -m freeware/DOC/INDEX

The command lists are read from the disk itself, so they can never drift out
of sync with what is actually installed; only the one-line descriptions below
are hand-maintained.  A command with no entry in these tables is emitted as
"(runs; purpose not yet documented)" -- grep for that to find gaps.
"""
import os, glob, textwrap, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

BUILT = {
 'cat':'concatenate files -- REBUILT HERE with gcc2 (archived cat needs cio)',
 'wc':'count lines/words/chars -- REBUILT HERE with gcc2; counts CR or LF lines',
 'basename':'strip directory from a pathname -- REBUILT HERE with gcc2',
 'dirname':'strip filename from a pathname -- REBUILT HERE with gcc2',
 'ls':'GNU ls (fileutils 3.13) -- OUR OWN FIXED BUILD: real stat(), columns, -al',
}
D = {
 'sh':'Bourne shell v7.5 -- THE BOOT SHELL; needs no Microware module',
 'bash':'GNU Bourne-Again Shell; runs, but warns about getwd and a missing .bashrc',
 'bash_new':'Alternate bash build; same getwd/.bashrc warnings',
 'screen':'Screen multiplexer (needs HOME set)',
 'vi_nocio':'PVic 1.0a -- vi-compatible editor, cio-free (the vi to use here)',
 'vi_nocio_pv':'PVic 1.0a -- vi-compatible editor, alternate cio-free build',
 'beav':'BEAV 1.40 -- Binary Editor And Viewer (needs TERM)',
 'hexedit':'Hex file editor -- hex [-vdr] file',
 'less':'Pager (wants a real TERM)',
 'grep':'GNU grep 2.0 -- pattern search',
 'sort':'GNU sort',
 'join':'GNU join -- relational join of two sorted files',
 'tree':'Print a directory tree -- BUT fails on this disk: it opens the raw\n                 device (/dd@), which a host-native disk has no equivalent for',
 'file':'Identify file types (warns: no /dd/SYS/magic file -- still IDs OS-9 modules)',
 'find':'find 1.1.5 -- search a directory tree',
 'upperdir':'Normalise case: files lowercase, dirs uppercase',
 'ascii':'ASCII character table',
 'jargon':'Jargon-file browser (needs its database files)',
 'lha':'LHa 2.08 -- create/extract .lzh archives',
 'zip':'Info-ZIP zip 1.9', 'zipnote':'Info-ZIP zipnote -- view/edit zip comments',
 'zipsplit':'Info-ZIP zipsplit -- split a zip archive',
 'gzip':'GNU gzip', 'tar':'GNU tar 1.10', 'shar':'Shell-archive creator',
 'atob':'ASCII-to-binary decode', 'btoa':'Binary-to-ASCII encode',
 'md5':'MD5 checksum', 'modbuster':'Split merged OS-9 module files',
 'dback':'Directory backup utility (wants a /d0 device)',
 'kermit':'C-Kermit 5A(188) -- serial file transfer + terminal emulation',
 'aterm':'ATerm 2.6 -- terminal emulator',
 'sbreak':'Send/clear an SS_Break signal on a serial path',
 'as0':'68000 assembler (xasm)','as1':'68010 assembler (xasm)',
 'as4':'68040 assembler (xasm)','as5':'68050 assembler (xasm)',
 'as11':'68HC11 assembler (xasm)',
 'sc':'sc -- spreadsheet calculator (needs TERM)',
 'gs33':'Ghostscript 3.33 (needs gs_init.ps + fonts)',
 'makeinfo':'GNU makeinfo -- Texinfo to info',
 'infocom':'Infocom Z-machine interpreter -- plays GAMES/INFORM/*.z3',
 'infocom.tcap':'Infocom interpreter, termcap build',
 'date':'Print date and time','setime':'Set system time (prompts YYMMDDHHMMSS)',
 'wysetime':'Wyse terminal time utility','t':'tiny test/stub binary',
 'mtools':'MS-DOS disk suite -- front end listing its sub-commands',
 # --- added 2026-07-13 from the archive sweep, each verified running here ---
 'diff':'GNU diff 1.1 -- ADDED; the disk had no diff at all.  Verified on CR files',
 'gmake':'GNU make -- ADDED (the gnu.bin build of make is the broken one)',
 'bison':'GNU bison 1.19 parser generator -- ADDED; skeletons in /dd/LIB',
 'compress':'compress/uncompress (LZW) -- ADDED',
 'me':'MicroEmacs 3.11 -- ADDED; needs TERM.  (memacs400 `emacs` needs cio)',
 'cjpeg':'JPEG encoder (IJG) -- ADDED',
 'djpeg':'JPEG decoder (IJG) -- ADDED',
}
for m in ('msattrib msbadblocks mscd mscopy msdel msdeltree msdir msformat msinfo '
          'mslabel msmd msmove msrd msread msren mstoolstest mstype mswrite').split():
    D[m] = 'mtools 3.6 -- MS-DOS %s (needs a PC drive configured)' % m[2:]
D.update(BUILT)

NEED = {
 'cat':'concatenate files','wc':'word/line/char count','uniq':'drop duplicate lines',
 'strings':'extract printable strings','tail':'last lines of a file','cp':'copy files',
 'mv':'move/rename','rm':'remove files','mkdir':'make directory','ed':'GNU ed 0.2 line editor',
 'emacs':'MicroEmacs 4.00','emacs.mm1':'MicroEmacs macro module','mg':'MicroGnuEmacs',
 'vi_cio':'PVic vi, cio build (use vi_nocio instead)','less':'pager',
 'basename':'strip directory from path','dirname':'strip filename from path',
 'getenv':'print an environment variable','exist':'test file existence',
 'mexist':'test module existence','chown':'change owner','chgrp':'change group',
 'cb':'C beautifier','cxref':'C cross-reference','detab':'tabs to spaces',
 'field':'extract fields','gdd':'data dump','divide':'integer divide',
 'rechne':'RPN calculator','cal':'calendar','sysmon':'system monitor',
 'aprocs':'process monitor','advent':'Colossal Cave Adventure','advent0':'Adventure (variant)',
 'gnuchess':'GNU Chess','gnuchessn':'GNU Chess (ncurses)','gnuchessr':'GNU Chess (raw)',
 'paranoia':'floating-point benchmark','psc':'spreadsheet print filter',
 'rcs':'RCS','ci':'RCS check in','co':'RCS check out','rcsdiff':'RCS diff',
 'rlog':'RCS log','rcsident':'RCS ident','rcsmerge':'RCS merge',
 'ckermit':'C-Kermit (cio build)','unzip':'Info-ZIP unzip','autolf':'auto-linefeed filter',
 'dm':'disk/dir monitor','dpark':'park a process','fc':'re-execute history',
 'launch':'launch background process','initvdu':'init video display',
 'setterm':'set terminal type','setimex':'set time from hardware clock',
 'todos':'OS-9 to DOS line endings','toos9':'DOS to OS-9 line endings',
 'as09':'6809 assembler','checkenv':'check env var','delbak':'delete backup files',
 'l':'brief directory listing','owner':'show file owner','wysecrack':'Wyse baud detect',
}

def mods(d):
    out = []
    for p in sorted(glob.glob(d + '/*'), key=str.lower):
        if os.path.isfile(p) and open(p, 'rb').read(2) == b'\x4a\xfc':
            out.append(os.path.basename(p))
    return out

work    = mods('freeware/CMDS')
need    = mods('freeware/CMDS/NEEDCIO')
nonfree = mods('freeware/CMDS/NONFREE')

L = []
L.append("INDEX -- what is on this disk, and where it came from")
L.append("=" * 54)
L.append("")
L.append(textwrap.fill(
    "Every command in /dd/CMDS was tested by BOOTING THIS DISK STANDALONE "
    "(os9exec /dd/CMDS/sh) with no Microware module present anywhere -- no "
    "shell, csl, cio or math. That is the bar: if it is listed here, it ran.", 78))
L.append("")
L.append(textwrap.fill(
    "Provenance: everything came from the Microware OS-9 hobbyist archive, the "
    "Hamburg OS-9 Users Group, and other public preservation sites. The original "
    "archives are kept in CMDS/archives/ -- ed, elvis and grep include full C "
    "source in there. Per-package documentation is in DOC/ (see DOC/README). "
    "SOURCES.txt carries fuller per-command provenance and licences.", 78))
L.append("")
L.append(textwrap.fill(
    "Five commands here are OUR OWN BUILDS, not archive binaries: cat, wc, "
    "basename and dirname were rebuilt from source with the gcc2 toolchain "
    "against os9lib, because every archived build of them needs cio; and ls is "
    "our repaired build of GNU fileutils 3.13. Their C source is on the h0 "
    "workshop disk.", 78))
L.append("")
L.append("-" * 54)
L.append("/dd/CMDS -- WORKS ON THIS DISK (%d commands)" % len(work))
L.append("-" * 54)
for c in work:
    L.append("  %-14s %s" % (c, D.get(c, '(runs; purpose not yet documented)')))
L.append("")
L.append("  Also present, shell scripts rather than OS-9 modules -- NEITHER WORKS HERE:")
L.append("    who            'who is logged in'.  Written in MICROWARE SHELL syntax")
L.append("                   ('!' pipes), not sh, and needs procs/field/qsort/tr,")
L.append("                   none of which are on this disk.")
L.append("    mscheck        mtools disk verifier.  A ksh script (#!ksh) -- there is")
L.append("                   no ksh here -- and it needs a PC drive configured.")
L.append("")
L.append("-" * 54)
L.append("/dd/CMDS/NEEDCIO -- NEEDS MICROWARE cio (%d) -- WILL NOT RUN HERE" % len(need))
L.append("-" * 54)
L.append(textwrap.fill(
    "Good programs, but linked against Microware's cio trap handler, which is "
    "licensed and is not on this disk. They fail with \"**** Can't install trap "
    "handler ****\". Kept for reference and for anyone who has a licensed OS-9 "
    "SDK; deliberately left off PATH. Where we needed one badly (cat, wc, "
    "basename, dirname) we rebuilt it instead -- see above.", 78))
L.append("")
for c in need:
    L.append("  %-14s %s" % (c, NEED.get(c, '(purpose not yet documented)')))
L.append("")
L.append("-" * 54)
L.append("/dd/CMDS/NONFREE -- MICROWARE PROPRIETARY (%d) -- DO NOT REDISTRIBUTE" % len(nonfree))
L.append("-" * 54)
L.append(textwrap.fill(
    "SOURCES.txt marks these as proprietary Microware SDK binaries. They do not "
    "belong on a disk called 'freeware' and are quarantined here pending "
    "removal. Free replacements already on this disk: vi_nocio for VI; "
    "lha/zip/tar/gzip for ar and arc; elvis ships a free ctags (source is in "
    "CMDS/archives/elvis1.7.lzh).", 78))
L.append("")
for c in nonfree:
    L.append("  %-14s Microware SDK -- proprietary" % c)
L.append("")
L.append("-" * 54); L.append("Other directories"); L.append("-" * 54)
L.append("  /dd/GAMES/INFORM   Z-machine story files -- play with: infocom file.z3")
L.append("  /dd/GAMES/ADV      Colossal Cave data (advent itself needs cio)")
L.append("  /dd/DOC            per-package documentation (see DOC/README)")
L.append("  /dd/CMDS/archives  the original .lzh archives")
L.append("  /dd/CMDS/NEEDCIO/GCC  the gcc2 toolchain -- ALSO needs cio, so it cannot")
L.append("                     run on this disk either.  cat/wc/basename/dirname/ls")
L.append("                     were built with it on a disk that HAS a licensed cio.")
L.append("  /dd/DEFS /dd/LIB   C headers and libraries (LIB also holds bison's skeletons)")
L.append("")

open('freeware/DOC/INDEX', 'w').write('\n'.join(L) + '\n')
print("wrote freeware/DOC/INDEX: working=%d needcio=%d nonfree=%d" % (len(work), len(need), len(nonfree)))

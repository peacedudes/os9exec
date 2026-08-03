# Building the release image

`release/dd` is a 182 MB OS-9 RBF disk image holding the whole freeware
collection, with ~138 MB free for score files, saves and your own work.
It is a build ARTIFACT — generated from `freeware/`, never hand-edited, and
gitignored. The tree is what gets maintained; the image is what gets shipped.

## Build it

    release/mkimage.sh  freeware  release/dd

Takes a while — roughly 5 seconds per file through the emulator.

## Use it

    OS9DISK=/path/to/dd  os9exec /dd/CMDS/bash_new        # standalone
    OS9DISK=/path/to/dd  OS9H1=/your/os9  os9exec -r /h1/CMDS/shell

**The image file must currently be named `dd`** (or `h0`…`hz` for a secondary
device). os9exec resolves an image by FILENAME, so `freeware.rbf` fails with
`E_FNA`. There is a fix for this in the working tree, uncommitted; once it
lands the name stops mattering. See `ROADMAP.md`.

## How it works, and why not `mount -k`

`mount -k` builds the entire image in the 68k arena before writing it, so it
cannot create anything near the 32 MB arena — a 50 MB collection is out of
reach. Nothing about a *finished* image needs memory (`Open_Image` reads one
sector), so `mkblank.py` writes the blank image host-side instead: a port of
os9exec's own `BuildBlankImage()`, **verified byte-for-byte identical to
`mount -k` output at 4 M, 12 M and 28 M**, with no ceiling.

Above ~128 MB the allocation map outgrows its 16-bit size field, so the
cluster size doubles automatically. Sector size stays 256: os9exec's
`RAM_zero` template is a 256-byte array and it `memcpy`s `sctSize` bytes out
of it, so a 512-byte sector would read past the end.

Files are copied in batches of 250, each in a fresh os9exec session, because
os9exec leaks arena memory across process creations — a straight run dies with
"No more memory !!!" after about 487 copies, on a file of a few hundred bytes.
Both that and the `mount -k` cap are in `ROADMAP.md`.

## Attributes

Copied files inherit HOST permissions, which is meaningless on OS-9 — one
binary arrived `--e-rewr` and another `-----ewr` purely because one had `o+x`
on the Mac. So the builder normalises at the end: every OS-9 module (4AFC
magic) gets `e+pe+r+pr`, everything else `r+pr` and no execute. That is the
main reason to ship an image rather than a directory — the bits become real
and travel with the file.

Volume name: set `RBF_VOLNAME`, else it inherits `mount -k`'s default of
"Ram Disk (Caution: Volatile)", which is wrong for a distribution disk. The
shipped image reads "OS-9 Freeware Collection".

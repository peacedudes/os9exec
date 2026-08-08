#!/usr/bin/env python3
"""audit-patterns.py -- static sweeps for the bug classes that recur in os9exec.

These scans were run ad-hoc across several sessions and only their FINDINGS were
kept; the recipes were lost.  This is them, made rerunnable.

    tools/audit-patterns.py              # scan, print findings, exit 1 if any new
    tools/audit-patterns.py --selftest   # prove every scan can still fail
    tools/audit-patterns.py --all        # include allowlisted (already-cleared) hits

WHAT THIS DOES NOT COVER -- on purpose:

  * Host pointer truncated through a 32-bit int.  The mingw (LLP64) build is the
    instrument for that class, not grep: `long` is 32 bits there, so GCC flags
    the truncations that LP64 macOS/Linux compile in silence.  Use
    `make warnings` / tools/verify-warnings.sh.  See memory pointer-width-audit.
  * printf/scanf TYPE mismatches (%d for a char*, a format string built at
    runtime).  PRINTF_FMT in debug.h puts __attribute__((format(printf,...))) on
    all six wrappers and the compiler owns that permanently.  Keep it on.
    It does NOT cover the WIDTH question, which is why the format-* scans below
    exist: `%lu` against a `ulong` type-checks cleanly on every platform and is
    still wrong on one of them, because `ulong` is `unsigned long` on LP64 and
    `unsigned long long` under MINGW.  Only a cross-platform build or these
    scans see that.

A hit is a QUESTION, not a defect.  Every scan here is deliberately over-broad;
the allowlist carries sites already investigated and cleared, each with a reason,
so a rerun shows only what is new.
"""

import argparse
import pathlib
import re
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent
SRC = REPO / "Source"

# Guest 68k memory is BIG-ENDIAN.  Any multi-byte access must go through these;
# a line mentioning one is doing the swap and is not a finding.
SWAP = r"os9_word|os9_long|os9_byte|GET_OS9|SET_OS9|os9_get_|os9_set_"

# os9exec spells guest-struct fields with a leading underscore (pd._cid).
FIELD = r"[.>]_[a-z]\w*"


def scan_endian_compare(line):
    """Raw guest field compared against a host scalar.

    Swap-INVARIANT comparisons are fine and must be excluded: ==0/!=0 (zero
    survives any swap) and field-to-field compares (same endianness both sides).
    The bug is a raw guest field vs a HOST scalar -- this is the `_cid != newpid`
    shape fixed in cb4c843.
    """
    m = re.search(FIELD + r"\s*(==|!=|<=|>=|<|>)\s*([A-Za-z_][\w.\[\]>-]*)", line)
    if not m:
        return None
    if re.search(SWAP, line):
        return None
    rhs = m.group(2)
    # Field-to-field is same-endian on both sides -- swap-invariant, not a bug.
    # The RHS must be taken WHOLE (`procs[k].pd._group`), not just its first
    # identifier, or every such compare is reported.
    if re.search(FIELD, rhs):
        return None
    if rhs in ("NULL", "true", "false", "NUL"):
        return None
    return m.group(0)


def scan_endian_ptr_deref(line):
    """Multi-byte pointer-cast dereference of guest memory without a swap."""
    m = re.search(r"\*\s*\(\s*(ushort|uint16_t|ulong|uint32_t|short|long)\s*\*\s*\)", line)
    if not m or re.search(SWAP, line):
        return None
    return m.group(0)


def scan_endian_array_index(line):
    """Indexing an array with a RAW guest field selects the wrong element."""
    m = re.search(r"\[[^]]*" + FIELD + r"\s*\]", line)
    if not m or re.search(SWAP, line):
        return None
    return m.group(0)


def scan_array_stride(line):
    """&arrayfield + N advances by N*sizeof(element), not N bytes.

    This is the device-descriptor option-read trap: `&mod->_mdtype + PD_SSize`
    where _mdtype is uint8_t[128] steps 128 bytes per unit.  `&arr[N]` is right.
    """
    m = re.search(r"&\w+(?:->|\.)_?\w+\s*\+\s*[A-Za-z_]\w*", line)
    return m.group(0) if m else None


def scan_char_eof(line):
    """(char)EOF == EOF is false where char is unsigned (ARM Linux).

    Two same-line shapes are detectable: storing getc()'s int into a char (which
    destroys the EOF sentinel), and casting to char before comparing.  A char
    declared on one line and compared to EOF on another is NOT caught here --
    that needs whole-function analysis.  fileaccess.c:396 is the known-correct
    site (`int c`, with a comment saying why).
    """
    m = re.search(r"\b(?:char|byte)\s+\**\w+\s*=\s*(?:getc|fgetc|getchar)\s*\(", line)
    if m:
        return m.group(0)
    m = re.search(r"\(\s*(?:unsigned\s+|signed\s+)?char\s*\)[^;]*?(?:==|!=)\s*(?:EOF|-1)\b", line)
    return m.group(0) if m else None


# printf-family, including the project's six wrappers (debug.h).
PRINTF_FN = (r"\b(?:s?n?printf|fprintf|_debugprintf|debugprintf|"
             r"up[eoh]_printf|uph[eo]_printf)\s*\(")
SCANF_FN = r"\b(?:sscanf|fscanf|scanf)\s*\("


def scan_format_long_width(line):
    """`%l` has no width that is right on both LP64 and LLP64.

    `ulong` is the project's pointer-width word: `unsigned long` on LP64 but
    deliberately `unsigned long long` under MINGW.  So `%lu` is correct on
    macOS/Linux and wrong on Windows, and no single spelling fixes both.
    The rule is: never re-spell the conversion -- CAST THE ARGUMENT, `(uint32_t)`
    for 68k values or `%p` + `(void*)` for host pointers.
    """
    if not re.search(PRINTF_FN, line):
        return None
    m = re.search(r"%[-+ #0-9.]*l[diouxX]", line)
    if not m or "%ll" in line:
        return None
    # An explicit cast to a fixed width is the documented fix, not a finding.
    if re.search(r"\(\s*(?:unsigned\s+)?long\s*\)|\(\s*u?int32_t\s*\)|\(\s*u?int64_t\s*\)", line):
        return None
    return m.group(0)


def scan_format_sizeof(line):
    """sizeof yields size_t; %d/%u/%x mis-read it on at least one platform.

    `snprintf(buf, sizeof(buf), "...")` is the CORRECT idiom -- there the sizeof
    is the size parameter, not a printed argument.  Only a sizeof appearing
    AFTER the format string is a real candidate.
    """
    call = re.search(PRINTF_FN, line)
    if not call or "sizeof" not in line:
        return None
    fmt = re.search(r'"', line[call.end():])
    if not fmt:
        return None
    args = line[call.end() + fmt.end():]
    close = args.rfind('"')
    args = args[close + 1:] if close >= 0 else args
    if "sizeof" not in args:
        return None
    m = re.search(r"%[-+ #0-9.]*[diouxX]\b", line)
    if not m or re.search(r"%[-+ #0-9.]*z[diouxX]", line):
        return None
    return m.group(0) + " with sizeof"


def scan_format_pointer(line):
    """%p must be fed a (void*); anything else is undefined by the standard."""
    if not re.search(PRINTF_FN, line) or "%p" not in line:
        return None
    return None if "(void*)" in line or "(void *)" in line else "%p without (void*)"


def scan_format_scanf_width(line):
    """scanf writes THROUGH the pointer, so a cast cannot repair a width.

    The length modifier must match the destination's real width exactly.
    `sscanf("%lu", &ulong)` filled 4 of 8 bytes on Windows and left the rest
    uninitialised -- it hit `-M`, the 68k arena size.  Fix with a plain
    `unsigned long` temporary, then assign.
    """
    if not re.search(SCANF_FN, line):
        return None
    m = re.search(r"%[-+ #0-9.]*(?:l|h|ll|hh)[diouxX]", line)
    return m.group(0) if m else None


def scan_platform_linux_chain(line):
    """`#elif defined linux` / `#ifndef linux` give mingw the WRONG branch.

    Under mingw, linux/MACOSX/windows32/macintosh are ALL undefined, so an
    #elif chain keyed on linux hands it no branch at all and #ifndef linux hands
    it a branch written for the Mac.  This silently broke deldir on Windows.
    Key POSIX-generic branches on UNIX instead.
    """
    m = re.search(r"#\s*(?:elif\s+defined|ifndef)\s+\(?\s*linux\b", line)
    if not m:
        return None
    # A branch that names MINGW or UNIX has already accounted for Windows.
    if re.search(r"\bMINGW\b|\bUNIX\b", line):
        return None
    return m.group(0)


def scan_platform_chains(lines):
    """Whole-chain version of the mingw-falls-through trap.

    Asking this line-at-a-time is wrong: `#elif defined linux` is perfectly safe
    when an earlier `#elif defined MINGW` in the SAME chain already caught
    Windows.  The real defect is a chain that dispatches on platform, names
    linux, and never names MINGW/windows32/UNIX anywhere -- there mingw either
    falls to an #else written for the Mac, or gets no branch at all.  That is
    what made deldir silently do nothing on Windows.

    Returns (lineno, text) for the head of each offending chain.
    """
    hits, stack = [], []
    for n, line in enumerate(lines, 1):
        m = re.match(r"\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)", line)
        if not m:
            continue
        kind, rest = m.group(1), m.group(2)
        if kind in ("if", "ifdef", "ifndef"):
            # If an ENCLOSING chain already dispatched Windows elsewhere (its
            # `#if defined MINGW` branch), this nested chain is unreachable on
            # Windows and cannot be the bug.
            inherited = any(
                re.search(r"\bMINGW\b|\bwindows32\b", a["cond"]) for a in stack)
            stack.append({"line": n, "text": line.strip(), "cond": rest,
                          "platform": False, "branches": 0,
                          "inherited": inherited})
            if kind == "ifndef" and re.search(r"\blinux\b", rest):
                stack[-1]["platform"] = True
        elif stack:
            stack[-1]["cond"] += " " + rest
            if kind in ("elif", "else"):
                stack[-1]["branches"] += 1
        if not stack:
            continue
        if re.search(r"\blinux\b|\bMACOSX\b|\bmacintosh\b", stack[-1]["cond"]):
            stack[-1]["platform"] = True
        if kind == "endif" and stack:
            ch = stack.pop()
            covers_windows = re.search(r"\bMINGW\b|\bwindows32\b|\bUNIX\b", ch["cond"])
            # A single-branch #ifdef is ADDITIVE (mingw just skips it, usually
            # right).  Only a multi-branch chain DISPATCHES, and that is where
            # Windows silently lands on someone else's branch.
            if (ch["platform"] and ch["branches"] and not covers_windows
                    and not ch["inherited"]):
                hits.append((ch["line"], ch["text"]))
    return hits


FILE_SCANS = [
    ("platform-chain-nowin", scan_platform_chains,
     "platform #if chain with no MINGW/UNIX branch"),
]


def scan_dead_macro_gate(line):
    """Code gated on a macro no shipping build ever defines is dead code."""
    dead = ("windows32", "THREAD_SUPPORT", "SERIAL_INTERFACE", "PTOC_SUPPORT",
            "NATIVE_SUPPORT", "NET_SUPPORT", "REUSE_MEM")
    m = re.search(r"#\s*(?:if|ifdef|elif)\s+.*?\b(" + "|".join(dead) + r")\b", line)
    if not m:
        return None
    # `defined windows32 || defined MINGW` is the CORRECT "real Windows" idiom.
    if "MINGW" in line:
        return None
    return m.group(0)


def scan_sprintf_self_source(line):
    """sprintf(buf, "...", buf, ...) -- destination as its own source is UB."""
    m = re.match(r"\s*(?:\w+\s*=\s*)?s(?:n)?printf\s*\(\s*([A-Za-z_]\w*)\s*,", line)
    if not m:
        return None
    dst = m.group(1)
    return f"sprintf({dst}, ..., {dst}, ...)" if re.search(
        r",\s*" + re.escape(dst) + r"\s*[,)]", line[m.end():]) else None


def scan_memset_transposed(line):
    """memset(p, size, 0) -- value and length swapped; clears nothing."""
    m = re.search(r"\bmemset\s*\([^;]*,\s*0\s*\)\s*;", line)
    if not m:
        return None
    args = re.search(r"\bmemset\s*\((.*)\)\s*;", line)
    if not args or args.group(1).count(",") != 2:
        return None
    return m.group(0).strip()


SCANS = [
    ("endian-compare",       scan_endian_compare,       "raw guest field vs host scalar"),
    ("endian-ptr-deref",     scan_endian_ptr_deref,     "multi-byte cast deref, no swap"),
    ("endian-array-index",   scan_endian_array_index,   "array indexed by raw guest field"),
    ("array-stride",         scan_array_stride,         "&arrayfield + N stride trap"),
    ("char-eof",             scan_char_eof,             "char compared to EOF/-1"),
    ("dead-macro-gate",      scan_dead_macro_gate,      "gated on a never-defined macro"),
    ("sprintf-self-source",  scan_sprintf_self_source,  "dest passed as its own source"),
    ("memset-transposed",    scan_memset_transposed,    "memset(p, size, 0)"),
    ("format-long-width",    scan_format_long_width,    "%l differs LP64 vs LLP64"),
    ("format-sizeof",        scan_format_sizeof,        "size_t printed as %d/%u"),
    ("format-pointer",       scan_format_pointer,       "%p without (void*)"),
    ("format-scanf-width",   scan_format_scanf_width,   "scanf width must match dest"),
]

# Sites investigated and CLEARED, with the reason.  Matched as substrings against
# the offending line, scoped to one scan and one file, so they cannot mask a new
# bug elsewhere.  Sources: memories endian-swap-sweep, conditional-compilation-audit.
ALLOW = [
    ("endian-ptr-deref", "modstuff.c", "*(short*)",
     "guest->guest byte copy, order-preserving"),
    ("endian-compare", "debug.c", "_msync",
     "compared to sync_id=os9_word(0x4afc); constant pre-swapped instead of field"),
    ("endian-compare", "os9exec_nt.c", "_mexcpt",
     "zero-check; the value use below it is wrapped"),
]


# The four format scans duplicate what -Wformat already enforces -- but ONLY for
# files the build actually compiles.  Proven live on 2026-08-07: injecting
# `uphe_printf("%08lX", (char*)0)` into fileaccess.c produced
#   "format specifies type 'unsigned long' but the argument has type 'char *'"
# under the real compile line, and all three shipping builds are 0 warnings.
# So in built files these are noise; in never-compiled files they are the only
# check there is.
FORMAT_SCANS = {"format-long-width", "format-sizeof", "format-pointer",
                "format-scanf-width"}
COMPILER_COVERED = "compiler-checked: -Wformat, 3 platforms, all 0 warnings"


def built_sources():
    """Files the GNUmakefile compiles.  Headers count -- they are included."""
    try:
        text = (REPO / "GNUmakefile").read_text()
    except OSError:
        return set()
    names = set(re.findall(r"([A-Za-z0-9_]+\.c)", text))
    return names


BUILT = built_sources()


def allowed(scan, path, line):
    if scan in FORMAT_SCANS and (path.suffix == ".h" or path.name in BUILT):
        return COMPILER_COVERED
    for a_scan, a_file, a_sub, reason in ALLOW:
        if scan == a_scan and path.name == a_file and a_sub in line:
            return reason
    return None


def sources():
    for p in sorted(SRC.rglob("*")):
        if p.suffix in (".c", ".h") and p.is_file():
            yield p


def strip_comment(line):
    """Single-line comment removal (used by the self-test)."""
    line = re.sub(r"/\*.*?\*/", "", line)
    return re.sub(r"//.*$", "", line)


def strip_comments(lines):
    """Blank out comments across a whole file, preserving line numbering.

    Block comments MUST be tracked across lines: this codebase documents its own
    past bugs in prose ("(char)EOF == EOF is false", "*(uint32_t*)sp = ..."), and
    a line-at-a-time stripper reports those explanations as fresh findings.
    """
    out, in_block = [], False
    for raw in lines:
        line, res, i = raw, [], 0
        while i < len(line):
            if in_block:
                end = line.find("*/", i)
                if end < 0:
                    i = len(line)
                    break
                in_block, i = False, end + 2
                continue
            start = line.find("/*", i)
            slash = line.find("//", i)
            if slash >= 0 and (start < 0 or slash < start):
                res.append(line[i:slash])
                i = len(line)
                break
            if start < 0:
                res.append(line[i:])
                break
            res.append(line[i:start])
            in_block, i = True, start + 2
        out.append("".join(res))
    return out


def run(show_all=False):
    hits = {name: [] for name, _, _ in SCANS}
    hits.update({name: [] for name, _, _ in FILE_SCANS})
    suppressed = 0
    for path in sources():
        try:
            text = path.read_text(errors="replace").splitlines()
        except OSError:
            continue
        stripped = strip_comments(text)
        for name, fn, _ in FILE_SCANS:
            for n, snippet in fn(stripped):
                rel = path.relative_to(REPO)
                hits[name].append((f"{rel}:{n}", snippet[:100], None))
        for n, (raw, line) in enumerate(zip(text, stripped), 1):
            if not line.strip():
                continue
            for name, fn, _ in SCANS:
                found = fn(line)
                if not found:
                    continue
                why = allowed(name, path, line)
                if why and not show_all:
                    suppressed += 1
                    continue
                rel = path.relative_to(REPO)
                hits[name].append((f"{rel}:{n}", raw.strip()[:100], why))
    return hits, suppressed


# Each entry must trip exactly the scan it names.  A scan that cannot fail is
# worse than no scan -- this repo has found three of those.
SELFTEST = [
    ("endian-compare",       "    if (cp->pd._cid != newpid) return 1;"),
    ("endian-ptr-deref",     "    v = *(ushort*)p;"),
    ("endian-array-index",   "    procs[pd->_pid].state = 0;"),
    ("array-stride",         "    q = &mod->_mdtype + PD_SSize;"),
    ("char-eof",             "    char c = getc(fp);"),
    ("platform-linux-chain", "#elif defined linux"),
    ("dead-macro-gate",      "#ifdef windows32"),
    ("sprintf-self-source",  '    sprintf(buf, "%s/%s", buf, name);'),
    ("memset-transposed",    "    memset(base, size, 0);"),
    ("format-long-width",    '    upo_printf("size %lu\\n", arenasize);'),
    ("format-sizeof",        '    upo_printf("%d\\n", sizeof(process_typ));'),
    ("format-pointer",       '    upo_printf("at %p\\n", ptr);'),
    ("format-scanf-width",   '    sscanf(arg, "%lu", &memsize);'),
]


def selftest():
    print("self-test -- every scan must fire on a known-bad line\n")
    ok = True
    for name, fn, _ in SCANS:
        sample = next(s for n, s in SELFTEST if n == name)
        fired = fn(strip_comment(sample))
        # It must also NOT fire on the corrected form, where one exists.
        print(f"  {'PASS' if fired else 'FAIL'}  {name:22} <- {sample.strip()[:56]}")
        ok = ok and bool(fired)
    negatives = [
        ("endian-compare", "    if (cp->pd._cid != os9_word(newpid)) return 1;"),
        ("endian-compare", "    if (pd->_pid == 0) return 1;"),
        ("endian-ptr-deref", "    v = GET_OS9W(buf, 4);"),
        ("dead-macro-gate", "#if defined windows32 || defined MINGW"),
        # The documented fix for each width class must not re-flag.
        ("format-long-width", '    upo_printf("size %u\\n", (uint32_t)arenasize);'),
        ("format-sizeof", '    upo_printf("%zu\\n", sizeof(process_typ));'),
        ("format-pointer", '    upo_printf("at %p\\n", (void*)ptr);'),
        ("format-scanf-width", '    sscanf(arg, "%u", &n32);'),
        ("char-eof", "    int c = getc(fp);"),
    ]
    print("\n  negative controls -- these must NOT fire")
    for name, sample in negatives:
        fn = next(f for n, f, _ in SCANS if n == name)
        fired = fn(strip_comment(sample))
        print(f"  {'FAIL' if fired else 'PASS'}  {name:22} <- {sample.strip()[:56]}")
        ok = ok and not fired
    # The chain scanner replaced a line-local check that reported five safe
    # sites, because it could not see the MINGW branch above the linux one.
    bad_chain = ["#if defined MACOS9", "  a();", "#elif defined MACOSX",
                 "  b();", "#elif defined linux", "  c();", "#endif"]
    good_chain = ["#if defined windows32 || defined MINGW", "  w();",
                  "#elif defined MACOSX", "  b();", "#elif defined linux",
                  "  c();", "#endif"]
    fires = bool(scan_platform_chains(bad_chain))
    quiet = not scan_platform_chains(good_chain)
    print("\n  platform #if chains (whole-chain, not line-local)")
    print(f"  {'PASS' if fires else 'FAIL'}  chain with no MINGW/UNIX branch is reported")
    print(f"  {'PASS' if quiet else 'FAIL'}  chain whose first branch is MINGW is NOT reported")
    ok = ok and fires and quiet

    # The block-comment stripper is load-bearing: this codebase describes its own
    # past bugs in prose, and those descriptions look exactly like the bugs.
    prose = [
        "/* Historical note:",
        " * aarch64: (char)EOF == EOF is false.",
        " * `*(uint32_t*)sp = ...` is a misaligned store -- undefined.",
        " */",
        "    v = *(ushort*)p;",          # real code, MUST still be found
    ]
    stripped = strip_comments(prose)
    commented_clean = all(
        not scan_char_eof(s) and not scan_endian_ptr_deref(s) for s in stripped[:4])
    code_still_found = bool(scan_endian_ptr_deref(stripped[4]))
    print("\n  block-comment handling")
    print(f"  {'PASS' if commented_clean else 'FAIL'}  prose inside /* */ is not reported")
    print(f"  {'PASS' if code_still_found else 'FAIL'}  real code after */ is still reported")
    ok = ok and commented_clean and code_still_found

    print("\nself-test", "OK" if ok else "BROKEN -- fix before trusting a clean run")
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--all", action="store_true", help="include allowlisted hits")
    args = ap.parse_args()
    if args.selftest:
        return selftest()

    hits, suppressed = run(args.all)
    total = sum(len(v) for v in hits.values())
    for name, _, desc in SCANS + FILE_SCANS:
        rows = hits[name]
        print(f"\n== {name} -- {desc}  [{len(rows)}]")
        for loc, code, why in rows:
            print(f"   {loc}\n       {code}" + (f"\n       (allowed: {why})" if why else ""))
        if not rows:
            print("   clean")
    print(f"\n{total} finding(s); {suppressed} allowlisted hit(s) suppressed")
    print("A hit is a question, not a verdict -- read the site before changing it.")
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())

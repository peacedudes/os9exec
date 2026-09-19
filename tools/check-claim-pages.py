#!/usr/bin/env python3
"""check-claim-pages.py -- every page a CONF68K claim cites must hold that call.

    tools/check-claim-pages.py <v2.4 Technical Reference Manual as text>

DOCS/claims.md cites the manual by call and page ("F$Send, FUNCTION, page
1 - 48"). A page number is easy to get wrong and nothing else notices: when
this was first run (2026-09-19) ten citations were off, from 1 - 22 given for
F$CmpNam (it is 1 - 9) to three written that same day; the error-appendix
check added after it found nine more, most one page early. The manual's own index
lists the page each call STARTS on; a call runs until the next call starts,
so a citation is accepted anywhere in that range, the page a later paragraph
of a long call (F$Event, I$GetStt) sits on included.

Only the pairing "<call> ... page N - M" within one citation clause (the text
between semicolons) is checked, and only for calls the index knows. Error
codes are checked the same way: "entry for E$X, page Error Codes - N" must
name the appendix page E$X's entry is printed on. The
manual is not in this repository, so its path is an argument. Exits 1 on any
citation outside its call's pages.
"""
import re
import sys
from pathlib import Path

CLAIMS = Path(__file__).resolve().parent.parent / "test/68k-conformance/DOCS/claims.md"


def call_starts(manual):
    """Index lines look like `F$CRC   Generate CRC ...... 1-11`."""
    starts = {}
    for line in open(manual, errors="replace"):
        m = re.match(r"\s*([FI]\$\w+)\s+.*?\.{3,}\s*([12])-(\d+)\s*$", line)
        if m:
            starts.setdefault(m.group(1), (int(m.group(2)), int(m.group(3))))
    return starts


def error_pages(manual):
    """Appendix entries look like `000:221       E$MNF      MODULE NOT FOUND`, and
    a page ends with a footer naming its number, so an entry is on the page of
    the first footer after it."""
    pages, pending = {}, []
    for line in open(manual, errors="replace"):
        m = re.match(r"\s*\d{3}:\d{3}\s+(E\$\w+)", line)
        if m:
            pending.append(m.group(1))
        f = re.search(r"Error Codes - (\d+)", line)
        if f and pending:
            for name in pending:
                pages.setdefault(name, int(f.group(1)))
            pending = []
    return pages


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__.strip().splitlines()[2].strip())
    starts = call_starts(sys.argv[1])
    if len(starts) < 50:
        sys.exit(f"only {len(starts)} calls found in the index -- is that the v2.4 Technical Reference Manual?")
    pages = sorted(set(starts.values()))

    def span(call):
        first = starts[call]
        later = [p for p in pages if p > first and p[0] == first[0]]
        return first, (later[0] if later else (first[0], 999))

    errpages = error_pages(sys.argv[1])
    checked = bad = 0
    for row in CLAIMS.read_text().split("\n"):
        m = re.match(r"\| (t\d+) \| ([^|]*)\|", row)
        if not m:
            continue
        for clause in m.group(2).split(";"):
            em = re.search(r"(E\$\w+), page Error Codes - (\d+)", clause)
            if em and em.group(1) in errpages:
                checked += 1
                if int(em.group(2)) != errpages[em.group(1)]:
                    bad += 1
                    print(f"{m.group(1)}: {em.group(1)} cited on Error Codes - {em.group(2)}; "
                          f"its entry is on Error Codes - {errpages[em.group(1)]}")
                continue
            calls = re.findall(r"([FI]\$\w+)", clause)
            cited = re.findall(r"page ([12]) - (\d+)", clause)
            if not calls or len(cited) != 1 or calls[0] not in starts:
                continue
            checked += 1
            page = (int(cited[0][0]), int(cited[0][1]))
            lo, hi = span(calls[0])
            if not lo <= page <= hi:
                bad += 1
                print(f"{m.group(1)}: {calls[0]} cited as page {page[0]} - {page[1]}; "
                      f"it runs {lo[0]} - {lo[1]} to {hi[0]} - {hi[1]}")
    print(f"{checked} citations checked, {bad} outside their call's pages")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Word-shingle overlap check for derivative reference docs vs. their sources.

Usage: shingle_scan.py <derivative.md> <source1.txt> [source2.txt ...]

Reports N-word runs (default 6) that appear identically in both the
derivative file and a source, as a first-pass copyright/paraphrase-quality
check before publishing a card-extracted reference file. A hit doesn't
automatically mean a violation -- short factual/notational runs (syntax
argument lists, numeric tables, standard technical terms) are expected and
fine; a hit that reproduces a full sentence's structure is the real signal.
Follow up on any hit by reading the surrounding context in both files.
"""
import re
import sys

N = 6  # shingle length in words


def shingles(words, n):
    return {tuple(words[i:i + n]) for i in range(len(words) - n + 1)}


def load_words(path):
    with open(path, "r", errors="ignore") as f:
        text = f.read().lower()
    return re.findall(r"[a-z0-9]+", text)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    target_path, source_paths = sys.argv[1], sys.argv[2:]
    target_shingles = shingles(load_words(target_path), N)
    print(f"Target file: {target_path}")
    print(f"Unique {N}-word shingles: {len(target_shingles)}\n")

    for src in source_paths:
        src_shingles = shingles(load_words(src), N)
        hits = sorted(" ".join(s) for s in (target_shingles & src_shingles))
        print(f"=== {src} ===")
        print(f"  {N}-word shingle matches: {len(hits)}")
        for h in hits[:200]:
            print(f"    MATCH: {h}")
        print()


if __name__ == "__main__":
    main()

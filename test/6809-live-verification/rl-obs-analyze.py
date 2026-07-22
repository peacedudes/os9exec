#!/usr/bin/env python3
"""Analyze rlrace3o observation dumps (see rl-race3o.bas / rl-dump.bas).

Usage: rl-obs-analyze.py <csv>   where <csv> holds the L<r>,<i>,<v>,<w>,<j>
lines scraped from an rldump run, e.g.:
  tmux capture-pane -t <session>:chan -p -S -1500 | sed -n '/OMARKx/,$p' | grep '^L[12],' > obs.csv
Stage 1 prints duplicate clusters (each duplicated v = one lost update) and
reader-source tallies; stage 2 prints divergence-window structure and
same-stamp birth events.  Used 2026-07-21 to root-cause the NitrOS-9 6809
RBF lost update -- see docs/nitros9-rbf-lostupdate-HANDOFF.md.

Each line: L<r>,<i>,<v>,<w>,<j>
  r = racer id (log file), i = its GET number (1..200)
  v = rec(1) the GET observed          (counter)
  w = rec(2) the GET observed          (id of the PUT that produced this state)
  j = rec(3) the GET observed          (that writer's iteration number)

Invariants of a correct serialization:
  - all 400 observed v are distinct (each GET sees the current counter,
    then writes v+1; two GETs seeing the same v = one lost update)
  - the triple is internally consistent: v == (what writer w read at its
    iteration j) + 1, because rec(1..3) travel in the same 10-byte PUT.
    (exception: v==0 rows observed the initial record, w=j=0)
"""
import sys, re, collections

lines = open(sys.argv[1]).read().splitlines()
obs = {}   # (racer, i) -> (v, w, j)
for ln in lines:
    m = re.match(r'^L([12]),(-?\d+),(-?\d+),(-?\d+),(-?\d+)$', ln.strip())
    if m:
        r, i, v, w, j = map(int, m.groups())
        obs[(r, i)] = (v, w, j)

per = {1: {}, 2: {}}
for (r, i), t in obs.items():
    per[r][i] = t
print(f"rows: L1={len(per[1])} L2={len(per[2])}")
# racer 1 row i=1 may be clipped by capture; it is the very first GET (v=0,w=0,j=0)
if 1 not in per[1] and len(per[1]) == 199:
    per[1][1] = (0, 0, 0); obs[(1, 1)] = (0, 0, 0)
    print("(assumed clipped L1,1 = 0,0,0)")

# --- duplicates: the lost updates ---
byv = collections.defaultdict(list)
for (r, i), (v, w, j) in sorted(obs.items()):
    byv[v].append((r, i, w, j))
dups = {v: rs for v, rs in byv.items() if len(rs) > 1}
n_extra = sum(len(rs) - 1 for rs in dups.values())
print(f"distinct observed v: {len(byv)}   duplicated v: {len(dups)}   extra reads (=losses): {n_extra}")

# --- classify each duplicate ---
def prev_own(r, i):
    return per[r].get(i - 1, (None, None, None))[0]

kinds = collections.Counter()
print("\n--- duplicate clusters (v: readers...) ---")
for v in sorted(dups):
    rs = dups[v]
    tags = []
    for (r, i, w, j) in rs:
        own = prev_own(r, i)
        selfstale = (w == r and j == i - 1)   # saw own previous PUT
        othstale  = (w == (3 - r))            # saw sibling's PUT
        init      = (w == 0)
        tag = f"L{r}#{i} w={w} j={j}"
        if selfstale: tag += " [SELF]"
        elif othstale: tag += " [OTH]"
        elif init: tag += " [INIT]"
        tags.append(tag)
    # who is the *stale* reader?  In a dup pair the later-serialized read is stale.
    print(f"v={v}: " + " | ".join(tags))
    for (r, i, w, j) in rs:
        if w == r and j == i - 1: kinds["reader saw its OWN previous write"] += 1
        elif w == 3 - r:          kinds["reader saw sibling's write"] += 1
        elif w == 0:              kinds["reader saw initial record"] += 1
        else:                     kinds["other"] += 1

print("\n--- reader-source tally over duplicate rows ---")
for k, c in kinds.items():
    print(f"  {c:4d}  {k}")

# --- torn-record check: v must equal (writer w's read at j) + 1 ---
torn = []
for (r, i), (v, w, j) in sorted(obs.items()):
    if w in (1, 2) and j >= 1:
        src = per[w].get(j)
        if src is not None and v != src[0] + 1:
            torn.append((r, i, v, w, j, src[0]))
print(f"\ntorn-triple rows (v != writer's_read+1): {len(torn)}")
for t in torn[:20]:
    r, i, v, w, j, sv = t
    print(f"  L{r}#{i} saw v={v} stamped (w={w},j={j}) but writer read {sv} (so wrote {sv+1})")

# --- per-racer monotonicity of observed v ---
for r in (1, 2):
    seq = [per[r][i][0] for i in sorted(per[r])]
    drops = [(i + 1, seq[i - 1], seq[i]) for i in range(1, len(seq)) if seq[i] < seq[i - 1]]
    print(f"\nL{r}: v monotonic non-decreasing: {not drops}; drops={drops[:5]}")
    # stale-depth for [OTH] readers: how far behind the final count of that writer
print()

print("="*70)
print("WINDOW / BIRTH STRUCTURE (stage 2)")
print("="*70)
import sys, re, collections

obs = {}
for ln in open(sys.argv[1]):
    m = re.match(r'^L([12]),(-?\d+),(-?\d+),(-?\d+),(-?\d+)$', ln.strip())
    if m:
        r, i, v, w, j = map(int, m.groups())
        obs[(r, i)] = (v, w, j)
per = {1: {}, 2: {}}
for (r, i), t in obs.items():
    per[r][i] = t
if 1 not in per[1] and len(per[1]) == 199:
    per[1][1] = (0, 0, 0); obs[(1, 1)] = (0, 0, 0)

# every PUT: (writer, iter) wrote value v+1 where v = what it read
puts = {}
for r in (1, 2):
    for i, (v, w, j) in per[r].items():
        puts[(r, i)] = v + 1
# which PUTs were ever observed by a subsequent GET?
seen_stamps = collections.Counter()
for (r, i), (v, w, j) in obs.items():
    seen_stamps[(w, j)] += 1
unread = [(r, i, puts[(r, i)]) for (r, i) in sorted(puts) if (r, i) not in seen_stamps]
print(f"PUTs never observed by any GET: {len(unread)} (of 400; the final PUTs of each racer and pre-done-phase tails are legitimately unread)")

byv = collections.defaultdict(list)
for (r, i), (v, w, j) in sorted(obs.items()):
    byv[v].append((r, i, w, j))
dups = sorted(v for v, rs in byv.items() if len(rs) > 1)

# classify pairs
same_stamp, diff_stamp = [], []
for v in dups:
    rs = byv[v]
    stamps = {(w, j) for (_, _, w, j) in rs}
    (same_stamp if len(stamps) == 1 else diff_stamp).append(v)
print(f"dup values: {len(dups)}  same-stamp pairs: {len(same_stamp)}  diff-stamp pairs: {len(diff_stamp)}")

# group dups into consecutive-v windows
windows = []
cur = [dups[0]]
for v in dups[1:]:
    if v == cur[-1] + 1:
        cur.append(v)
    else:
        windows.append(cur); cur = [v]
windows.append(cur)
print(f"\nwindows of consecutive duplicated v: {len(windows)}")
for wnd in windows:
    lo, hi = wnd[0], wnd[-1]
    body = []
    for v in wnd:
        rs = byv[v]
        stamps = {(w, j) for (_, _, w, j) in rs}
        body.append("S" if len(stamps) == 1 else "D")
    # what follows the window: who observed hi+1 and with what stamp
    nxt = byv.get(hi + 1, [])
    nxts = " ".join(f"L{r}#{i}<-({w},{j})" for (r, i, w, j) in nxt)
    print(f"  v={lo}..{hi} len={len(wnd)} pattern={''.join(body)}  next v={hi+1}: {nxts}")

# for each same-stamp pair: order the two readers by who read the state FIRST.
# The stamped writer (w,j) belongs to one racer; its OWN next GET (w, j+1) tells
# what the writer saw after: if the writer's next GET saw v+1 with its own stamp,
# the writer's branch continued privately.
print("\nsame-stamp birth details:")
for v in same_stamp:
    rs = byv[v]
    (w, j) = next((w, j) for (_, _, w, j) in rs)
    readers = " ".join(f"L{r}#{i}" for (r, i, _, _) in rs)
    wr_next = per.get(w, {}).get(j + 1)
    print(f"  v={v} state=({w},{j}) readers: {readers}; writer's own next GET saw {wr_next}")

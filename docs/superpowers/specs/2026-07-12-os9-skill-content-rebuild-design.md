# OS-9 Skill Content Rebuild — Design

## Background

`~/.claude/skills/os9-dev/` and its sibling `~/.claude/skills/os9-systems-dev/`
are shared Claude Code skills for OS-9/6809 and OS-9/68000 development,
intended for public release alongside this project. The 68k content went
through two prior passes: an initial condensation of *The OS-9 Guru* book
(deleted entirely after the user judged it too structurally close to the
book's own chapter organization — "guru-notes/"), then a "structural reorg"
that re-extracted from 11 primary `txtResources/68k` manuals and rewrote
content task-oriented rather than book-chapter-oriented.

The user has judged that reorg still insufficiently transformed relative to
its sources and wants a full redo, this time using a stricter method
(atomic, tagged "index cards" rather than free-form condensation notes) that
was separately developed and proven during the 6809 phase. The user is
willing to spend the tokens to do this once, correctly, since the result is
meant to be stable for a long time ("baked in stone").

6809 content was already built via primary vendor manuals (never the Guru
book) using an early version of the card method, and is not being redone —
its synthesized output is reused directly as clean input to this rebuild.

## Scope

**In scope:** full re-extraction and resynthesis of 68k-facing prose content
in `os9-dev` (`common/`, `c/`, `basic09/` 68k-relevant files,
`68k/os9-68k-assembly.md`) and `os9-systems-dev`'s 68k-facing files.

**Out of scope:** 6809 extraction (reused as-is); OS-9000 content (doesn't
exist yet); CD-i driver content (separately scoped, deferred).

**Source set:** every primary document already triaged into
`txtResources/68k`'s high-value tier (8 official manuals) + secondary tier
(4 more), same triage as the prior pass. **The OS-9 Guru book is included
this time as an ordinary source** — no longer excluded, no longer
privileged. Every source, including the Guru book, gets the same
card-and-cite discipline: nothing verbatim except genuine data tables
(syscall codes, error numbers, byte offsets), everything else atomically
re-expressed.

## Card format

Adopted from an external design review (Opus), used as a flexible working
format — **cards are an intermediate artifact for Claude's own processing,
not a human-facing deliverable**, so the schema may be amended during
processing if a better shape emerges; fidelity to the exact template below
is not an end in itself.

```
--- CARD ---
id:        <short-slug>
type:      FACT | BEHAVIOR | CONCEPT | GOTCHA | ROSETTA
target:    6809 | 68k | all
verify:    VERIFIED | from-manual | authored
topic:     <cluster tag>
claim:     <the fact, in our own words, or a cited verbatim table for FACT>
context:   <caveat that keeps the claim from being misleading, or "-">
source:    <doc> §<locator>, or "authored"
--- END ---
```

Hard rules:
- `target: all` only if the fact is identical on 6809 and 68k. Any real
  divergence splits into sibling cards (`<id>-6809`, `<id>-68k`) — never
  average two different behaviors into one card.
- `FACT` cards may quote official Microware tables verbatim, cited.
  Everything else must be re-expressed, not paraphrased-in-place.
- No card may mirror a source document's own chapter/section structure as
  its organizing principle — cards are atomic and unordered until the sift
  phase clusters them by topic.
- Every card must omit facts a competent LLM already knows generically
  (K&R C, generic BASIC syntax, generic Unix). The entire value of this
  skill is the *delta* from what Claude would already assume — about
  BASIC09 vs. generic BASIC, 6809 vs. 68k, and OS-9 vs. Unix/Linux. Cards
  should foreground that delta, not restate the baseline.

## Process

### 1. Extraction (Haiku, sequential)

One subagent per source document, dispatched sequentially (parallel
dispatch was tried during the 6809 phase and the user found it slower in
practice, not faster — not repeating that). Each agent reads its assigned
document and emits cards of **all five types**, not just facts — including
`CONCEPT` and `GOTCHA` cards wherever the source document itself states or
clearly implies a mental-model point, not only discrete syscall/table facts.

### 2. Concept/Rosetta authorship

Primarily Claude's own contribution, via two channels:
- Cards surfaced directly during extraction (channel above), where the
  source text itself states the concept.
- Cards written afterward from this project's own accumulated hands-on OS-9
  experience — live REPL verification, syscall behavior confirmed by
  compiling and running real programs, the kind of operational knowledge
  already scattered through this project's memory files.

The user will contribute some CONCEPT/ROSETTA cards but as a minority
contributor, not the primary author — do not block on the user supplying
these.

### 3. BNF-style grammar cards

BASIC09 only (not C, not 68k assembly — those are already well-served by
generic syntax knowledge Claude already has). Informal shorthand, not
strict formal BNF — skip leaf productions that are obvious. Each grammar
snippet paired with one tiny concrete example. Framed around *how BASIC09
differs from generic BASIC*, not as a syntax reference for its own sake.

### 3a. Error code table (the "errno" of OS-9)

Called out explicitly because it's the single cleanest distributable
artifact this rebuild produces: a table of error numbers and mnemonics
(`E$PNNF`, `E$BPNam`, ...) is functional data, not copyrightable
expression, the same way libc's `errno.h` is freely redistributable. Treat
it as its own `FACT`-card group, not folded anonymously into general
extraction:

- Rebuild from scratch by cross-referencing every primary source that
  lists OS-9 error codes (at minimum the v2.4 Technical Reference and one
  independent source, matching the two-source cross-check already proven
  once for the existing `common/error-codes.md`).
- One `FACT` card per code (or small contiguous range), `target: 6809 |
  68k | all` as appropriate (6809 and 68k share almost the entire 200-255
  range under the same kernel design; the one known real divergence —
  68k's `E$BPNum` covering both 200 and 201 vs. 6809's distinct
  `E$PthFul` at 200 — is confirmed genuine, not a source bug, and gets a
  `context:` note rather than a "fix").
- Cite every source per code so the final table's provenance is
  independently checkable, not just asserted.
- This card group is a good pilot/smoke-test for the whole extraction
  pipeline before running it across the full document set — small,
  well-bounded, and the correctness of the result is easy to verify against
  the two-source cross-check already on record.

### 4. Sift / merge

All new 68k cards plus the existing 6809 synthesized docs (treated as
already-clean input, not re-carded) go into one pile, clustered by topic.
Merge only where a fact is genuinely identical across targets; split on any
real divergence (see hard rules above). This is also where the
already-partially-generalized `common/` files (`unix-differences.md`,
`os9-mental-model.md`, `error-codes.md`) get folded in rather than
redone from scratch.

### 5. Final repack

Topic-clustered docs, two tiers:
- **Eager core** — SKILL.md plus the essential mental-model/Rosetta
  material. Crushed hard: delta-only, no filler.
- **Load-on-demand reference** — syscall tables, error codes, module
  layouts, other lookup data. Kept complete and intact — this tier is
  exactly where Claude would otherwise hallucinate confidently if content
  is thinned, so it is not subject to the same crushing discipline as the
  eager core.

Architecture tags (6809/68k/all) preserved inline throughout. `INDEX.md`
updated to route by topic. Old 68k files are replaced wholesale once new
versions land — this is a full rebuild, not an incremental patch.

## Testing / verification

There's no automated test suite for prose skill content. Verification means:
- Spot-check a sample of cards against their cited source to confirm they
  read as genuine re-expression, not shortened paraphrase (the same
  paraphrase-detection concern that caught the original Guru-book problem).
- Where a `BEHAVIOR` card's claim can be checked against the live 68k
  emulator/REPL, do so and upgrade `verify: from-manual` → `verify:
  VERIFIED`.
- Final structural check: no remaining file organizes itself around a
  source document's own chapter/section order.

## Open items deferred, not blocking this work

- CD-i section for the 68k skill (separately scoped, re-mining
  already-extracted material).
- Unsolicited "who carries OS-9 forward today" shout-out — needs the user
  to name the companies, not guessed.

# Extraction prompt template

Fill in {{DOC_NAME}}, {{OUTPUT_PATH}}, {{DOC_CAVEATS}} before dispatching.

---

You are extracting facts from one OS-9 reference document for a public,
freely-distributable Claude Code skill. You have no context beyond this
prompt — do not assume anything about prior sessions.

**Your source document:** locate the file matching "{{DOC_NAME}}" under
`/Users/rdoggett/mine/os9/XXX/os9exec/os9/txtResources/68k/` (plain text,
already extracted from PDF via pdftotext -layout — read it directly, do not
re-run any extraction tool). If more than one file could match, read enough
of each to confirm which one is "{{DOC_NAME}}" before proceeding.

**Known caveats for this specific document:** {{DOC_CAVEATS}}

**First step, always:** confirm this document's actual architecture scope
(6809 vs 68000) by reading enough of it to check for register names,
addressing modes, or explicit statements — filenames and even manual titles
have been wrong before (a "68k" folder RMA assembler manual turned out to be
pure 6809). If this document turns out to be 6809-only or otherwise out of
scope for a 68k skill, STOP and report that finding instead of extracting.

**What to produce:** read the entire document and emit a flat, unordered
list of cards, one per atomic fact. Do NOT organize your output by the
document's own chapter or section structure — that structural mirroring is
exactly what caused a prior version of this skill to be rejected as too
close to its source. Shuffle mentally; there is no "right order."

Each card:

```
--- CARD ---
id:        <short-kebab-slug, unique within this file>
type:      FACT | BEHAVIOR | CONCEPT | GOTCHA | ROSETTA
target:    68k | all
verify:    from-manual
topic:     <short cluster tag, e.g. process-mgmt, io, modules, basic09-syntax>
claim:     <the fact, in your own words — 1-4 lines>
context:   <the caveat that keeps this from being misleading if read alone,
            or "-" if none applies>
source:    {{DOC_NAME}} §<page/section locator if the text has one, else a
            short quoted anchor phrase>
--- END ---
```

Rules:
- `type: FACT` may quote an official data table (syscall codes, error
  numbers, byte offsets, register layouts) verbatim inside `claim:` — cite
  it. Every other type must be written in your own words. If you catch
  yourself producing a sentence that's just a shortened version of the
  source's own sentence, rewrite it from the underlying fact instead.
- Emit `CONCEPT` and `GOTCHA` cards too, not just `FACT`/`BEHAVIOR` — many
  reference manuals state mental-model points directly ("the module is the
  fundamental unit of...", "unlike file systems that..."). Capture those.
- Do NOT emit a card for anything a competent programmer already knows
  generically (standard C syntax, standard Unix concepts, standard BASIC
  keywords). Only OS-9-specific facts, or places where OS-9 diverges from
  what a Unix/Linux-experienced or generic-BASIC-experienced person would
  expect, are worth a card. When in doubt, ask: "would a Claude model
  trained broadly already know this?" — if yes, skip it.
- Set `target: 68k` for anything specific to the 68000 architecture or this
  manual's specific claims; set `target: all` only if you have independent
  reason to believe the fact holds on 6809 too (rare — you have not read
  any 6809 material, so default to `target: 68k` unless the document
  itself states the fact is architecture-independent).
- No fixed count — extract everything worth a card, however many that is.

**Output:** write the full card list to {{OUTPUT_PATH}}. Do not summarize or
truncate. Report back only a one-paragraph summary of what you found and any
architecture-scope surprises.

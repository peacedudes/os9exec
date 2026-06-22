# OS9exec project rules

## Protected file access — os9defs

The `Source/OS9exec_core/os9defs/` directory will contain proprietary OS-9
headers (errno.h, module.h, procid.h, sgstat.h and their adapted variants).
These are Microware copyright material. A PreToolUse hook blocks direct access,
but cannot detect path construction via shell variable splitting or other
runtime indirection.

**Hard rule — no exceptions, no judgment calls:**

Before executing any Bash command where a file path is assembled at runtime
across multiple statements (variable concatenation, string splitting, indirect
expansion, subshell tricks, or any other technique where the final path is not
visible as a literal string in the command), you must:

1. State the fully-expanded path you intend to access
2. Explain why you need it
3. Wait for explicit user approval

This applies even when individual statements look innocuous in isolation.

**Why this rule exists:**

The hook does static string matching. Variable splitting — e.g.:
  D="os9def"
  D="${D}s"
  cat "Source/OS9exec_core/${D}/errno.h"
— bypasses it because "os9defs" never appears literally. Filesystem permissions
cannot close this gap either: Claude runs with the owner's permissions, so
chmod 400 on the files allows Claude the same read access the owner has.
This behavioral rule is the only available defence against that vector.

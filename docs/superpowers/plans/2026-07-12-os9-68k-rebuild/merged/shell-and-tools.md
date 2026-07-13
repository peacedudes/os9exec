# The OS-9 shell and tools

34 raw cards in -> 22 cards out. 10 pruned (2 mis-bucketed dev-tools/toolchain cards,
5 mis-bucketed BASIC09-editor-specific cards, 3 generic-shell-101/filler restatements),
2 merged as true duplicates (unsetenv pair, `$`-prompt pair). All surviving cards are
architecture-independent (`target: all`); several were retargeted from `68k` to `all`
because the underlying manual happened to be a 68k-era edition, not because the
behavior is 68k-specific -- no genuine 6809/68k shell-behavior split was found in this
card set.

<!-- from guru-book.md -->
--- CARD ---
id:        shell-not-builtin
type:      CONCEPT
target:    all
verify:    from-manual
topic:     shell
claim:     Shell is a separate program (not built into the OS); it reads command lines and executes programs or built-in commands. Multiple shells can run concurrently for the same or different users.
context:   Users can run alternate command line interpreters (e.g., mshell) instead of shell.
source:    The OS-9 Guru, section 2.2, page 21
--- END ---

<!-- from guru-book.md -->
--- CARD ---
id:        startup-file-private-changes
type:      GOTCHA
target:    all
verify:    from-manual
topic:     shell
claim:     The startup file's shell incarnation is separate from the command-prompt shell incarnation; private changes made during startup (current directories, prompt string) don't carry over to the command shell.
context:   Current directories (chx/chd) and prompt changes (via -p=...) are process-private.
source:    The OS-9 Guru, section 2.1, page 19
--- END ---

## Shell command syntax and control flow

<!-- from using-professional-os9.md -->
--- CARD ---
id:        semicolon-sequential-execution
type:      FACT
target:    all
verify:    from-manual
topic:     command-separators
claim:     Semicolon (;) separator specifies sequential command execution: each command waits for the previous one to complete before starting.
context:   The separator character is stripped from parameters; must be quoted if passing as a literal program argument.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        ampersand-concurrent-execution
type:      FACT
target:    all
verify:    from-manual
topic:     multitasking
claim:     & (ampersand) separator enables concurrent execution: multiple commands run simultaneously as independent background processes.
context:   Enables multi-tasking directly from a single shell command line.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

<!-- from os9-primer.md -->
--- CARD ---
id:        ex-command
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     The ex command terminates the current shell and starts a replacement command (e.g., ex MShell replaces shell with MShell).
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---

<!-- from os9-primer.md -->
--- CARD ---
id:        logout-command
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     The logout command terminates the shell and ends the user session.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---

<!-- from os9-primer.md -->
--- CARD ---
id:        profile-command
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     The profile command reads a file of shell commands and executes them sequentially; environment changes made within the profile file persist in the shell that ran it.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---

## Environment variables and prompt configuration

<!-- from os9-primer.md + using-professional-os9.md -->
--- CARD ---
id:        setenv-unsetenv
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     Environment variables are set via setenv and unset via unsetenv (e.g., unsetenv PATH); setenv values containing spaces must be quoted. unsetenv both clears the value and removes the variable from environment storage entirely.
context:   -
source:    The OS-9 Primer, Chapter 3: "Environment variables"; Using Professional OS-9 v2.4, "The Shell", Changing the Shell Environment section, p. 5-5
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        printenv-display-variables
type:      FACT
target:    all
verify:    from-manual
topic:     shell-commands
claim:     printenv prints all environment variables and their values to standard output.
context:   Useful for inspecting the current shell environment.
source:    Using Professional OS-9 v2.4, "The Shell", Changing the Shell Environment section, p. 5-5
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        environment-variable-case-sensitive
type:      GOTCHA
target:    all
verify:    from-manual
topic:     shell-environment
claim:     Environment variables are case-sensitive; OS-9 does not recognize a variable if the proper case is not used.
context:   Example: PATH and path are different variables.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        prompt-at-macro-shell-level
type:      FACT
target:    all
verify:    from-manual
topic:     shell-configuration
claim:     The PROMPT environment variable specifies the current prompt; an @ sign in it is a replaceable macro for the shell nesting level number, tracked via the _sh variable. E.g., "@howdy: " with _sh=0 displays "howdy:" at level 0 and "1.howdy:" at level 1.
context:   Allows visual tracking of shell nesting depth.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        set-builtin-shell-options
type:      FACT
target:    all
verify:    from-manual
topic:     shell-commands
claim:     set <options> built-in command sets options for the shell; a hyphen (-) is unnecessary before letter options (e.g., set np turns off the prompt, equivalent to -np).
context:   Alternative to specifying options at shell invocation; affects shell behavior for the session.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        shell-prompt-dollar-default
type:      FACT
target:    all
verify:    from-manual
topic:     shell-interface
claim:     After a successful boot and time/date setup, the shell displays a $ prompt (the default system prompt) indicating OS-9 is active and ready for command input.
context:   The prompt is customizable via the PROMPT environment variable; $ is only the shipped default.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Booting OS-9 section, p. 2-2
--- END ---

## Control keys and line editing

<!-- from using-professional-os9.md -->
--- CARD ---
id:        control-A-repeat-previous-line
type:      FACT
target:    all
verify:    from-manual
topic:     line-editing
claim:     <control>A redisplays the previous input line without executing it; the cursor is placed at line end and the line can be edited by backspacing and typing over old characters.
context:   One of several line-editing control keys for error correction, standing in for the arrow-key/history editing found on other systems.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        control-X-delete-entire-line
type:      FACT
target:    all
verify:    from-manual
topic:     line-editing
claim:     <control>X deletes the entire current input line.
context:   Useful for abandoning a partially-typed command before pressing return.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        control-W-page-pause-output
type:      FACT
target:    all
verify:    from-manual
topic:     terminal-control
claim:     <control>W temporarily halts output so the user can read the screen before data scrolls off; output resumes when any other key is pressed.
context:   Distinct from the permanent page-pause mode set via tmode.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

## tmode and terminal configuration

<!-- from using-professional-os9.md -->
--- CARD ---
id:        tmode-control-key-customization
type:      FACT
target:    all
verify:    from-manual
topic:     system-configuration
claim:     The tmode utility redefines the function of control keys; the correspondence between control keys and their functions is changeable, allowing customization to specific keyboard layouts.
context:   -
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-6
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        page-pause-feature-screen-halt
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     terminal-output
claim:     The page-pause feature counts output lines until a full screen is displayed, then halts output until any key is pressed, repeating per screen. It can be fooled by lines longer than the physical screen width (wrapped lines aren't counted properly).
context:   Toggled on/off and configured via tmode's pag parameter.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", The Page Pause Feature section, p. 3-6
--- END ---

## Common utilities

<!-- from using-professional-os9.md -->
--- CARD ---
id:        help-utility-online-reference
type:      FACT
target:    all
verify:    from-manual
topic:     utilities
claim:     help is the on-line quick reference manual; type help <utility name> to display function, syntax, and available options. Every utility also supports the <utility name> -? option as an equivalent.
context:   Essential for quick lookup without printed documentation.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", The Help Utility section, p. 3-8
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        build-utility-text-file-creation
type:      FACT
target:    all
verify:    from-manual
topic:     utilities
claim:     build creates a file from standard input; it displays a ? prompt for each input line, and an empty line or EOF (typically <escape>) terminates file creation.
context:   Useful for creating short text files interactively without a full editor.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", build section, p. 11
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        cmp-binary-file-comparison
type:      FACT
target:    all
verify:    from-manual
topic:     utilities
claim:     cmp compares two binary files; it displays file offset (hex address), hexadecimal values, and ASCII characters for any differences encountered, plus an outputs byte-count and difference-count summary.
context:   -b sets buffer size, -s enables silent mode.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", cmp section, p. 15
--- END ---

<!-- from using-professional-os9.md -->
--- CARD ---
id:        setime-clock-startup-requirement
type:      FACT
target:    all
verify:    from-manual
topic:     utilities
claim:     After boot, the setime utility must start the system clock; the clock must be running for multi-tasking to function. setime prompts for year/month/day/hour/minute/second and an optional am/pm, and accepts flexible delimiters (space, colon, semicolon, comma, slash).
context:   If the system has a battery-backed clock, use setime -s instead of the interactive prompt.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Booting OS-9 section, p. 2-2
--- END ---

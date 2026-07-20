#!/bin/sh
# Run clang-tidy over the os9exec core in a container.
#
# A static-analysis axis distinct from GCC -fanalyzer and the sanitizers: its
# bugprone-*/cert-*/portability-* checks catch different shapes (too-small loop
# variables, integer division in a float context, unchecked library results,
# macro-hygiene). The UAE 68k core (Source/OS9AppEmu) is deliberately excluded
# -- its warnings are analysed and left unfixed (see ROADMAP.md).
#
# NOTE ON WHAT IT DOES *NOT* CATCH: there is no stock clang-tidy check for
# "ctype function (isalnum/tolower/...) called on a signed char", which is UB
# for bytes >=0x80. That class is real here (~38 sites, F$PrsNam fixed in
# 49bbd59) and must be swept separately -- do not assume a clean clang-tidy run
# clears it.
#
# clang-tidy writes diagnostics to STDERR, and multi-file mode needs a compile
# database; this runs it one file at a time with the build's own flags instead.
#
# Usage: tools/clang-tidy.sh [full]
#   (default) high-signal checks only -- the set worth triaging
#   full      every bugprone-*/cert-*/portability- check, incl. the noisy ones
set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
cd "$repo" || exit 1
image=silkeh/clang:latest

# High-signal by default: drop the checks that are almost always style noise here
# (easily-swappable-parameters, narrowing-conversions, cert-err33/34-c,
# assignment-in-if-condition, branch-clone, switch-missing-default-case).
if [ "${1:-}" = full ]; then
    checks='-*,bugprone-*,cert-*,portability-*'
else
    checks='-*,bugprone-*,cert-*,portability-*,-bugprone-easily-swappable-parameters,-bugprone-narrowing-conversions,-cert-err33-c,-cert-err34-c,-bugprone-assignment-in-if-condition,-bugprone-branch-clone,-bugprone-switch-missing-default-case,-bugprone-implicit-widening-of-multiplication-result,-bugprone-multi-level-implicit-pointer-conversion'
fi

incs='-DTERMINAL_CONSOLE -DINT_CMD -DRAM_SUPPORT -ISource/OS9exec_core -ISource/OS9exec_core/os9defs -ISource/Platforms/LINUX -ISource/Platforms -ISource/OS9AppEmu/UAE68emulator -ISource/OS9AppEmu'

out=$(docker run --rm -v "$repo":/src:ro -w /src "$image" sh -c "
for f in Source/OS9exec_core/*.c Source/NATIVE/*.c Source/Platforms/LINUX/*.c Source/OS9execMPW/os9.c; do
  clang-tidy -checks='$checks' \"\$f\" -- $incs 2>&1
done
")

n=$(printf '%s\n' "$out" | grep -c 'warning:')
echo "clang-tidy: $n warnings (checks: ${1:-high-signal})"
echo "=== by check ==="
printf '%s\n' "$out" | grep 'warning:' | grep -oE '\[[a-z0-9-]+\]$' | sort | uniq -c | sort -rn
echo "=== sites ==="
printf '%s\n' "$out" | grep 'warning:' | sed 's|/src/||'

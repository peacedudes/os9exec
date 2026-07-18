#!/usr/bin/env bash
# Run a command inside the MSYS2 CLANGARM64 environment, from this repo's root.
# Usage (from an MSYS2 usr/bin/bash.exe shell on the guest): ./.claude/armbuild.sh make CC=clang
export MSYSTEM=CLANGARM64
export MSYS2_PATH_TYPE=inherit
source /etc/profile
cd "$(dirname "${BASH_SOURCE[0]}")/.." || exit 1
exec "$@"

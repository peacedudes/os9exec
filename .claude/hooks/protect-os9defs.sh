#!/bin/bash
set -u
DIR='os9def'
DIR="${DIR}s"
PROT='errno\.h|module\.h|procid\.h|sgstat\.h|errno_adapted\.h|procid_adapted\.h'
deny() {
  REASON="Blocked by repository policy: the OS-9 DEFS headers in Source/OS9exec_core/${DIR}/ are proprietary (copyright) and must never be read by any model or session. Listing filenames with the Glob tool is fine; reading contents is not. Work from compiler diagnostics instead."
  printf '%s' "{\"hookSpecificOutput\":{\"hookEventName\":\"PreToolUse\",\"permissionDecision\":\"deny\",\"permissionDecisionReason\":\"${REASON}\"}}"
  exit 0
}
allow() { exit 0; }
payload="$(cat)"
READERS='cat|tac|rev|nl|head|tail|sed|awk|gawk|grep|egrep|fgrep|rg|ag|ack|less|more|most|od|xxd|hexdump|hd|strings|cut|tr|fold|fmt|expand|unexpand|bat|view|vi|vim|nvim|nano|pico|emacs|base64|uuencode|cp|mv|dd|rsync|install|tar|cpio|pax|perl|python|python3|python2|ruby|node|php|lua|tclsh|git|find|xargs|wc|diff|patch|sort|uniq|zip|unzip|7z|ar|ditto|jar|gzip|gunzip|zcat|bzip2|bunzip2|bzcat|xz|unxz|xzcat|zstd|lz4|compress|uncompress|cc|gcc|clang|cpp|curl|wget|nc|netcat|socat|ncat|scp|sftp|openssl|osascript|swift|swiftc|objdump|otool'
READER_RE="(^|[^[:alnum:]_/])($READERS)([^[:alnum:]_]|$)"
GIT_RE="[^[:alnum:]]git[^[:alnum:]].*(show|cat-file|diff|log)"
if command -v jq >/dev/null 2>&1; then
  tool="$(printf '%s' "$payload" | jq -r '.tool_name // ""')"
  case "$tool" in
    Read|Grep)
      target="$(printf '%s' "$payload" | jq -r '.tool_input.file_path // .tool_input.path // ""')"
      printf '%s' "$target" | grep -qE "$DIR|$PROT" && deny
      allow
      ;;
    Bash)
      cmd="$(printf '%s' "$payload" | jq -r '.tool_input.command // ""')"
      if printf '%s' "$cmd" | grep -q "$DIR"; then
        printf '%s' "$cmd" | grep -Eq "$READER_RE" && deny
        printf '%s' "$cmd" | grep -Eq "<[[:space:]]*[^|;&<>]*$DIR" && deny
        printf '%s' "$cmd" | grep -Eq "$GIT_RE" && deny
      fi
      if printf '%s' "$cmd" | grep -Eq "$PROT"; then
        printf '%s' "$cmd" | grep -Eq "$READER_RE" && deny
        printf '%s' "$cmd" | grep -Eq "<[[:space:]]*[^|;&<>]*($PROT)" && deny
        printf '%s' "$cmd" | grep -Eq "$GIT_RE" && deny
      fi
      allow
      ;;
    *)
      allow
      ;;
  esac
else
  if printf '%s' "$payload" | grep -qE "$DIR|$PROT"; then
    printf '%s' "$payload" | grep -Eq "$READER_RE" && deny
    printf '%s' "$payload" | grep -Eq "$GIT_RE" && deny
  fi
  allow
fi

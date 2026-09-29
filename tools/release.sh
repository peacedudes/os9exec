#!/usr/bin/env bash
# release.sh -- the only way a v* tag should be made.
#
#   tools/release.sh <version> <branch>             check the candidate; stop at GO/NO-GO
#   tools/release.sh <version> <branch> --publish   ...and on GO, merge, tag, publish, recheck
#
#   e.g. tools/release.sh 4.1.1 release-v4.1.1 --publish
#
# The tag is what publishes a release (build.yml's create-release), and on
# 2026-09-29 v4.1.0 was tagged by hand after the gates passed -- and shipped
# two faults that only the published files, looked at the way a user looks
# at them, would have shown. So this does, in order, and stops at the first
# failure:
#   1. the branch tip is pushed, clean, and carries docs/release-notes-v<version>.md
#   2. CI builds that exact commit (the same build the tag will make)
#   3. tools/release-check.sh on CI's artifacts: GO, or nothing is tagged
#   4. --publish only: fast-forward master to the commit, tag v<version>,
#      wait for the release job, then release-check.sh on what was published
# Freeware's own verification of the commit is separate and comes before
# --publish: send them the hash and wait for their result.
set -u
REPO="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO" || exit 1
version="${1:-}" branch="${2:-}" publish="${3:-}"
[ -n "$version" ] && [ -n "$branch" ] \
    || { echo "usage: $0 <version, e.g. 4.1.1> <branch> [--publish]" >&2; exit 2; }
tag="v$version"
IFS=. read -r maj min pat <<<"$version"
want="V${maj}.${min}${pat}"
WF="Build os9exec (all platforms)"

die() { echo "STOP: $*"; exit 1; }

echo "== 1. the candidate =="
git fetch -q github || die "cannot fetch github"
commit=$(git rev-parse "github/$branch" 2>/dev/null) || die "no branch github/$branch"
echo "  $branch is $commit"
git ls-remote --tags github "refs/tags/$tag" | grep -q . && die "$tag already exists on github"
git cat-file -e "$commit:docs/release-notes-$tag.md" 2>/dev/null \
    || die "docs/release-notes-$tag.md is not in $commit (it becomes the release text)"
git merge-base --is-ancestor github/master "$commit" \
    || die "master is not an ancestor of $commit: the merge would not be a fast-forward"

echo "== 2. CI on $commit =="
gh workflow run "$WF" --ref "$branch" >/dev/null || die "could not start CI"
id=""
for _ in $(seq 1 60); do
    id=$(gh run list --workflow "$WF" --branch "$branch" --limit 5 --json databaseId,headSha,event \
         --jq ".[] | select(.headSha==\"$commit\" and .event==\"workflow_dispatch\") | .databaseId" | head -1)
    [ -n "$id" ] && break; sleep 5
done
[ -n "$id" ] || die "CI run did not appear"
echo "  run $id"
gh run watch "$id" --exit-status >/dev/null 2>&1 || die "CI run $id failed"
echo "  CI green"

echo "== 3. the release gate on CI's artifacts =="
"$REPO/tools/release-check.sh" --run "$id" --version "$want" || die "release-check said NO-GO: nothing tagged"

if [ "$publish" != "--publish" ]; then
    echo "== GO. Not published (no --publish). To publish: $0 $version $branch --publish =="
    exit 0
fi

echo "== 4. publishing $tag =="
git push github "$commit:refs/heads/master" || die "could not fast-forward master"
git tag -a "$tag" "$commit" -m "os9exec $tag" || die "could not tag"
git push github "$tag" || die "could not push $tag"
rid=""
for _ in $(seq 1 60); do
    rid=$(gh run list --workflow "$WF" --limit 5 --json databaseId,headBranch \
          --jq ".[] | select(.headBranch==\"$tag\") | .databaseId" | head -1)
    [ -n "$rid" ] && break; sleep 5
done
[ -n "$rid" ] || die "the release run for $tag did not appear"
gh run watch "$rid" --exit-status >/dev/null 2>&1 || die "the release run $rid failed: CHECK THE RELEASE PAGE NOW"
echo "== 5. the release gate on what was PUBLISHED =="
"$REPO/tools/release-check.sh" --tag "$tag" --version "$want" \
    || die "the PUBLISHED $tag failed release-check: fix forward at once"
echo "== $tag is published and passed release-check =="

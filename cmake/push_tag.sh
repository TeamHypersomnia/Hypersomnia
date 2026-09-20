#!/usr/bin/env bash
set -e

if [ -z "$1" ]; then
	echo "Usage: $0 <tag>"
	exit 1
fi

TAG="$1"

git fetch origin master

if git merge-base --is-ancestor HEAD origin/master; then
	echo "Error: current commit is already on origin/master — nothing new to push."
	exit 1
fi

if git tag -l "$TAG" | grep -q "^${TAG}$"; then
	git tag -d "$TAG"
fi

git tag "$TAG"

# The tag goes first, and the order matters - do not swap these two.
#
# The workflows are triggered by the push to master and ignore tag pushes
# entirely (see tags-ignore in .github/workflows/*_build.yml), so pushing
# the tag alone starts nothing - it only makes the tag available.
#
# The build then derives the version from
#
#     git describe --exact-match --tags HEAD
#
# (CMakeLists.txt, cmake/version_file_generator, cmake/generate_archives.sh),
# which has no fallback: if CI checked out a commit whose tag is not on the
# remote yet, the version generation would simply fail.
#
# Hence the tag must already be there by the time the push to master
# sets the workflows off.

git push origin "$TAG" --force
git push origin master

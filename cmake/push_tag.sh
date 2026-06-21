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
git push origin "$TAG" --force
git push origin master

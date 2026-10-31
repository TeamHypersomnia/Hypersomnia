#!/usr/bin/env bash
# build_claude.sh <config> [worktree] - an agent's own native build: configures build/<dir> of the worktree (default: the
# checkout this script lies in) the first time, then builds it. Never build/current (the user's) - the build MCP and
# cmake/build.sh work there.
#
#   debug     build/Debug-x64-clang-claude            fast Debug, no debug info (like the user's cmkdf)
#   headless  build/Debug-x64-clang-claude-headless   the dedicated server (tournaments, crash recovery)
#   release   build/Release-x64-clang-claude
#   steam     build/Debug-x64-clang-claude-steam      needs the Steamworks SDK unpacked (see remote.md)
#
# One build on the machine at a time: it waits for the machine's build lock (wait_build_slot in lib.sh - any other ninja
# running counts too). ccache when installed (sudo pacman -S ccache): one cache for every worktree
# (CCACHE_BASEDIR=$HOME), so a fresh worktree builds in minutes. JOBS (10), CMAKE_ARGS (more -D flags),
# TARGET (ninja's target, default all), SUFFIX (a folder of its own for a variant), e.g. the STL that breaks ties
# like the Windows client's MSVC STL: SUFFIX=libstdcxx CMAKE_ARGS="-DPREFER_LIBCXX=0 -DHAVE_WFUNCTION_EFFECTS=0"
# (HAVE_WFUNCTION_EFFECTS=0 drops the bundled OpenAL's -Werror that fails with libstdc++).
set -e
CONFIG=$1
HERE=$(cd "$(dirname "$0")" && pwd)
TREE=$(cd "${2:-$HERE/../../../..}" && pwd)
REPO=$TREE
. "$HERE/lib.sh"

case "$CONFIG" in
	debug) DIR=Debug-x64-clang-claude; FLAGS="-DCMAKE_BUILD_TYPE=Debug -DGENERATE_DEBUG_INFORMATION=0" ;;
	headless) DIR=Debug-x64-clang-claude-headless; FLAGS="-DCMAKE_BUILD_TYPE=Debug -DGENERATE_DEBUG_INFORMATION=0 -DHEADLESS=1" ;;
	release) DIR=Release-x64-clang-claude; FLAGS="-DCMAKE_BUILD_TYPE=Release -DGENERATE_DEBUG_INFORMATION=0" ;;
	steam) DIR=Debug-x64-clang-claude-steam; FLAGS="-DCMAKE_BUILD_TYPE=Debug -DGENERATE_DEBUG_INFORMATION=0 -DDEVELOP_STEAM_INTEGRATION=1 -DUSE_SYSTEM_FREETYPE=0" ;;
	*) echo "usage: build_claude.sh debug|headless|release|steam [worktree]"; exit 2 ;;
esac

B=$TREE/build/$DIR${SUFFIX:+-$SUFFIX}
mkdir -p $B

# A fresh worktree has empty submodules: copied from the main checkout when at the same commits (no .git - a copied .git
# file breaks every git command there)
MAIN=$(cd "$(git -C $TREE rev-parse --path-format=absolute --git-common-dir)/.." && pwd)

if [ "$MAIN" != "$TREE" ]; then
	git -C $TREE submodule status | while read -r sha path rest; do
		[ -n "$(ls -A $TREE/$path 2>/dev/null)" ] && continue
		sha=${sha#[-+U]}

		if [ "$(git -C $MAIN/$path rev-parse HEAD 2>/dev/null)" != "$sha" ]; then
			echo "submodule $path: the main checkout has another commit - git submodule update --init $path"
			exit 1
		fi

		echo "submodule $path: copied from $MAIN"
		rsync -a --exclude=.git $MAIN/$path/ $TREE/$path/
	done
fi

LAUNCHER=""

if command -v ccache > /dev/null; then
	export CCACHE_BASEDIR=$HOME CCACHE_NOHASHDIR=1
	LAUNCHER="-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
else
	echo "(no ccache - sudo pacman -S ccache; building without it)"
fi

# Configured once; again when ccache came since (the launcher changes every command - one full rebuild fills the cache)
# or CMAKE_ARGS are given
if [ ! -f $B/build.ninja ] || [ -n "$CMAKE_ARGS" ] || { [ -n "$LAUNCHER" ] && ! grep -q '^CMAKE_CXX_COMPILER_LAUNCHER:.*ccache' $B/CMakeCache.txt; }; then
	(cd $B && CC=clang CXX=clang++ cmake -G Ninja -DARCHITECTURE=x64 -D_CMAKE_TOOLCHAIN_PREFIX=llvm- \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 $FLAGS $LAUNCHER $CMAKE_ARGS $TREE > $B/cmake_claude.log 2>&1) ||
		{ tail -30 $B/cmake_claude.log; exit 1; }
fi

wait_build_slot

echo "building $B"
nice -n 19 ninja -C $B -j${JOBS:-10} ${TARGET:-}

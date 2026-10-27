#!/bin/bash
# build_webs.sh <tag> - play, itch and CrazyGames web builds at exactly that tag, the version file made anew
TAG=$1
L=${LOGS:-~/.cache/hs-release}; mkdir -p $L
cd "$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
export CC=clang CXX=clang++
# Every version file made anew - once, before all the builds
rm -f build/Release-Web-clang-release-web/generators_output/generated/hypersomnia_version.cpp build/Release-Web-clang-release-web-itch/generators_output/generated/hypersomnia_version.cpp build/Release-Web-clang-release-crazygames/generators_output/generated/hypersomnia_version.cpp
for v in "release-web|-DUSE_O3=1 -DGENERATE_DEBUG_INFORMATION=0" "release-web-itch|-DUSE_O3=1 -DBUILD_ITCH=1 -DGENERATE_DEBUG_INFORMATION=0" "release-crazygames|-DUSE_O3=1 -DBUILD_CRAZYGAMES=1 -DGENERATE_DEBUG_INFORMATION=0"; do
	S=${v%%|*}; F=${v#*|}
	D=build/Release-Web-clang-$S
	[ "$(git describe --exact-match --tags HEAD)" = "$TAG" ] || { echo "HEAD not at $TAG"; exit 1; }
	BUILD_FOLDER_SUFFIX=$S cmake/build.sh Release Web "$F" > $L/web_$S.log 2>&1 && (cd $D && nice -n 10 ninja >> $L/web_$S.log 2>&1)
	echo "$S EXIT $? $(grep -m1 commit_tag $D/generators_output/generated/hypersomnia_version.cpp)"
done
echo ALLDONE

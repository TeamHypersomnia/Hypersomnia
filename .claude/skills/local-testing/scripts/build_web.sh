#!/bin/bash
# Web build in its own folder, the same steps as cmake/build.sh Release Web, without touching build/current.
# VARIANT=crazygames builds the CrazyGames build instead, in its own folder.
set -e
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
SRC=$REPO
VARIANT=${VARIANT:-}
B=$SRC/build/Release-Web-clang-claude-web${VARIANT:+-$VARIANT}
EXTRA=""
[ "$VARIANT" = crazygames ] && EXTRA="-DBUILD_CRAZYGAMES=1"
if [ -n "$EMSDK" ]; then
	source $EMSDK/emsdk_env.sh > /dev/null 2>&1
fi

command -v emcmake > /dev/null || { echo "emsdk not found: set EMSDK to the emsdk directory or put emcmake in PATH"; exit 1; }

mkdir -p $B/prebuilt-version $B/prebuilt-introspector
cd $B/prebuilt-version
CC=clang CXX=clang++ cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release $SRC/cmake/version_file_generator -G Ninja > cmake.log 2>&1
ninja > build.log 2>&1
cd $B/prebuilt-introspector
CC=clang CXX=clang++ cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release $SRC/cmake/Introspector-generator -G Ninja > cmake.log 2>&1
ninja > build.log 2>&1

cd $B
CC=clang CXX=clang++ emcmake cmake -DARCHITECTURE=Web -DCMAKE_BUILD_TYPE=Release \
	-DVERSION_FILE_GENERATOR_PATH=$B/prebuilt-version/version_file_generator \
	-DINTROSPECTOR_GENERATOR_PATH=$B/prebuilt-introspector/Introspector-generator \
	-D_CMAKE_TOOLCHAIN_PREFIX=llvm- -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DUSE_O3=1 -DGENERATE_DEBUG_INFORMATION=0 $EXTRA $SRC -G Ninja > cmake.log 2>&1
nice -n 19 ninja -j10 > build.log 2>&1 || { grep -E "error:" build.log | sort -u | head -20; exit 1; }
echo "web build OK"
ls -la $B/Hypersomnia.* | awk '{print $5, $NF}'

#!/usr/bin/env bash 
CONFIGURATION=$1
ARCHITECTURE=$2
C_COMPILER=$3
CXX_COMPILER=$4

if [[ ! -z "$3" ]] && [[ -z "$4" ]]
then
	echo "You must specify both a C and a C++ compiler, or leave both unspecified."
fi

if [[ -z "$ARCHITECTURE" ]]
then
	ARCHITECTURE="x64"
fi

if [[ -z "$C_COMPILER" ]]
then
	C_COMPILER="clang"
	CXX_COMPILER="clang++"
	ADDITIONAL_FLAGS="-D_CMAKE_TOOLCHAIN_PREFIX=llvm-"
fi

TARGET_DIR="build/${CONFIGURATION}-${ARCHITECTURE}-${C_COMPILER}"

if [[ ! -z "$BUILD_FOLDER_SUFFIX" ]]
then
	TARGET_DIR="${TARGET_DIR}-${BUILD_FOLDER_SUFFIX}"
fi

echo "Building into $TARGET_DIR"

mkdir --parents $TARGET_DIR
cd $TARGET_DIR

export CC=$C_COMPILER
export CXX=$CXX_COMPILER

cmake -DARCHITECTURE=$ARCHITECTURE -DCMAKE_BUILD_TYPE=$CONFIGURATION $ADDITIONAL_FLAGS $@ $OLDPWD -G Ninja

#!/usr/bin/env bash 
CONFIGURATION=$1
ARCHITECTURE=$2
C_COMPILER=$CC
CXX_COMPILER=$CXX

shift
shift

if [[ ! -z "$C_COMPILER" ]] && [[ -z "$CXX_COMPILER" ]]
then
	echo "You must specify both a C and a C++ compiler, or leave both unspecified."
fi

if [[ ! -z "$CXX_COMPILER" ]] && [[ -z "$C_COMPILER" ]]
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
fi

if [[ "$C_COMPILER" = "clang" ]]
then
	ADDITIONAL_FLAGS="-D_CMAKE_TOOLCHAIN_PREFIX=llvm-"
fi

ADDITIONAL_FLAGS="$ADDITIONAL_FLAGS -DCMAKE_POLICY_VERSION_MINIMUM=3.5"

BUILD_DIR="build"
TARGET_FOLDER_NAME="${CONFIGURATION}-${ARCHITECTURE}-${C_COMPILER}"

if [[ ! -z "$BUILD_FOLDER_SUFFIX" ]]
then
	TARGET_FOLDER_NAME="${TARGET_FOLDER_NAME}-${BUILD_FOLDER_SUFFIX}"
fi

TARGET_DIR="$BUILD_DIR/$TARGET_FOLDER_NAME"

echo "Building into $TARGET_DIR"

mkdir -p $TARGET_DIR
cd $TARGET_DIR

export CC=$C_COMPILER
export CXX=$CXX_COMPILER

if [[ "$ARCHITECTURE" = "Web" ]]
then
	echo "Building for Web. Pre-building version and introspector generators."

	mkdir prebuilt-version
	mkdir prebuilt-introspector

	SRCPWD=$OLDPWD
	echo "SRCPWD: $SRCPWD"

	pushd prebuilt-version
		cmake -DCMAKE_BUILD_TYPE=Release $SRCPWD/cmake/version_file_generator -G Ninja
		ninja
		VERSION_FILE_GENERATOR_PATH=$(pwd)/version_file_generator
	popd

	pushd prebuilt-introspector
		cmake -DCMAKE_BUILD_TYPE=Release $SRCPWD/cmake/Introspector-generator -G Ninja
		ninja
		INTROSPECTOR_GENERATOR_PATH=$(pwd)/Introspector-generator
	popd

	echo "Calling emconfigure. Building with emscripten."

	emcmake cmake -DARCHITECTURE=$ARCHITECTURE \
		-DCMAKE_BUILD_TYPE=$CONFIGURATION \
		-DVERSION_FILE_GENERATOR_PATH=$VERSION_FILE_GENERATOR_PATH \
		-DINTROSPECTOR_GENERATOR_PATH=$INTROSPECTOR_GENERATOR_PATH \
		$ADDITIONAL_FLAGS $@ $SRCPWD -G Ninja
else
	cmake -DARCHITECTURE=$ARCHITECTURE -DCMAKE_BUILD_TYPE=$CONFIGURATION $ADDITIONAL_FLAGS $@ $OLDPWD -G Ninja
fi

pushd ../
	# For simplicity of subsequent scripts, create a symlink to the last created build
	rm -f current
	ln -s $TARGET_FOLDER_NAME current
popd

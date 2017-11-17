source sh/unix/common.sh

if [[ $ARCHITECTURE == "x64" ]] 
then
	PASSED_CMAKE_CXX = "-m64"
	PASSED_CMAKE_C = "-m64"
elif [[ $ARCHITECTURE == "x86" ]]
then
	# Nothing needed
else
	echo "Unknown/unsupported architecture: $ARCHITECTURE"
	return 1
fi

TARGET_DIR=$(build_dir)
echo "Building into $TARGET_DIR"

mkdir --parents $TARGET_DIR
pushd $TARGET_DIR
cmake -DUNIX=ON -DBUILD_IN_CONSOLE_MODE=1  -DCMAKE_CXX_FLAGS="$PASSED_CMAKE_CXX" -DCMAKE_C_FLAGS="$PASSED_CMAKE_C" -DCMAKE_BUILD_TYPE=${CONFIGURATION} ${OLDPWD}
popd

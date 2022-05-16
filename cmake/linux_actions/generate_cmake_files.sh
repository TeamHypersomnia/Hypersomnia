c_compiler=clang-13
cxx_compiler=clang++-13
other_cmake_flags="-DGENERATE_DEBUG_INFORMATION=0 -DBUILD_IN_CONSOLE_MODE=0"

${c_compiler} -v && ${cxx_compiler} -v && cmake --version
export CC=${c_compiler}
export CXX=${cxx_compiler}
export LDFLAGS="${LDFLAGS} -L /usr/local/lib"

cmake/build.sh Release x64 "${other_cmake_flags}"

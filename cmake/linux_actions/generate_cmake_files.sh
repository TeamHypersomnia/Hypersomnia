c_compiler=clang-14
cxx_compiler=clang++-14
other_cmake_flags="-DGENERATE_DEBUG_INFORMATION=1 -DBUILD_DEBUGGER_SETUP=0"

${c_compiler} -v && ${cxx_compiler} -v && cmake --version
export CC=${c_compiler}
export CXX=${cxx_compiler}
export LDFLAGS="${LDFLAGS} -L /usr/local/lib"

cmake/build.sh Release x64 "${other_cmake_flags}"

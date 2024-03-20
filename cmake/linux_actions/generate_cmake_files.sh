c_compiler=clang-16
cxx_compiler=clang++-16
other_cmake_flags="-DLINK_STEAM_INTEGRATION=1 -DUSE_GLFW=1"

${c_compiler} -v && ${cxx_compiler} -v && cmake --version
export CC=${c_compiler}
export CXX=${cxx_compiler}
export LDFLAGS="${LDFLAGS} -L /usr/local/lib"

cmake/build.sh Release x64 "${other_cmake_flags}"

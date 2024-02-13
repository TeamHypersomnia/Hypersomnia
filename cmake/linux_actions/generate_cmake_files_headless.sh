c_compiler=clang-16
cxx_compiler=clang++-16
other_cmake_flags="-DHEADLESS=1 -DOPENSSL_ROOT_DIR=/usr/local -DOPENSSL_LIBRARIES=/usr/local/lib64"

${c_compiler} -v && ${cxx_compiler} -v && cmake --version
export CC=${c_compiler}
export CXX=${cxx_compiler}
export LDFLAGS="${LDFLAGS} -L /usr/local/lib"

cmake/build.sh Release x64 "${other_cmake_flags}"

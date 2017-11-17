source sh/unix/common.sh

pushd $(build_dir)
make -j4
popd

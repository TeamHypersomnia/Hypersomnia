source sh/unix/common.sh

ulimit -c unlimited -S

pushd hypersomnia
$(build_dir)/$(executable_name) --unit-tests-only
popd

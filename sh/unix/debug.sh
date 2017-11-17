source sh/unix/common.sh
source sh/unix/make.sh

ulimit -c unlimited -S

pushd hypersomnia
gdb $OLDPWD/$(build_dir)/$(executable_name) --unit-tests-only
popd

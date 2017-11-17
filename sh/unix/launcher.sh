#!/bin/bash

source sh/unix/common.sh
sh/unix/make.sh

ulimit -c unlimited -S

pushd hypersomnia

EXECUTABLE_PATH="$OLDPWD/$(build_dir)/$(executable_name)"

echo "Executable path: $EXECUTABLE_PATH"
echo "Executable working dir: $PWD"

$1 $EXECUTABLE_PATH 
popd

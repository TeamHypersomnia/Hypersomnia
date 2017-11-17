#!/bin/bash

source sh/unix/common.sh

pushd $(build_dir)
make -j4 
MAKE_EXIT_CODE=$?
popd
exit $MAKE_EXIT_CODE

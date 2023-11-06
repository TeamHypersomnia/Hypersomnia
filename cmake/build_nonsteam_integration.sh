#!/usr/bin/env bash 
pushd cmake/steam_integration
rm -rf build
mkdir build
pushd build
cmake -DBUILD_STEAM=0 -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja install
popd
popd

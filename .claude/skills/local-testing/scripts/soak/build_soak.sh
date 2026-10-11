#!/bin/bash
# Builds the soak client from the game's own netcode (the yojimbo submodule) and the system libsodium into $SCRATCH/soak/soak.
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
Y=$REPO/src/3rdparty/yojimbo
mkdir -p $S/soak
clang -O1 -std=c11 -D_DEFAULT_SOURCE -I$Y/netcode "$(dirname "$0")/soak.c" $Y/netcode/netcode.c -o $S/soak/soak -lsodium -lm -lpthread && echo "built $S/soak/soak"

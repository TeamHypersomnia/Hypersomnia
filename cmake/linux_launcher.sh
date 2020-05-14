#!/usr/bin/env bash

cd "`dirname $0`"

./.Hypersomnia "$@"

while [ $? -eq 42 ]; do
    ./.Hypersomnia "$@" --upgraded-successfully
done

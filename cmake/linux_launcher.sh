#!/usr/bin/env bash

./.Hypersomnia "$@"

while [ $? -eq 42 ]; do
    ./.Hypersomnia --upgraded-successfully
done

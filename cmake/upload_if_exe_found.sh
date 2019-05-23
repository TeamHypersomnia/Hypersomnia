#!/usr/bin/env bash 
EXE_PATH="build/current/Hypersomnia"

if [ -f "$EXE_PATH" ]; then
	echo "Exe found. Uploading."

	cp build/current/Hypersomnia hypersomnia
	pushd hypersomnia
	tar -czf Hypersomnia-x64.tar.gz arenas content scripts web config.lua Hypersomnia
	python ../cmake/upload.py Hypersomnia-x64.tar.gz $1
else
	echo "No exe found. Not uploading."
fi

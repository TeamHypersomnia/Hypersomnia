#!/usr/bin/env bash 
EXE_PATH="build/current/Hypersomnia"

if [ -f "$EXE_PATH" ]; then
	echo "Exe found. Uploading."

	API_KEY=$1
	PLATFORM=Linux
	COMMIT_HASH=$(git rev-parse HEAD)
	COMMIT_NUMBER=$(git rev-list --count master)
	VERSION="1.0.$COMMIT_NUMBER"
	FILE_PATH="Hypersomnia-for-$PLATFORM.sfx"
	UPLOAD_URL="https://hypersomnia.xyz/upload_artifact.php"

	. cmake/linux_launcher_install.sh
	cp build/current/Hypersomnia hypersomnia/.Hypersomnia
	pushd hypersomnia
	rm -r cache
	popd
	7z a -sfx $FILE_PATH hypersomnia
	curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" $UPLOAD_URL
else
	echo "No exe found. Not uploading."
fi

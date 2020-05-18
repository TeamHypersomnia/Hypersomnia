#!/usr/bin/env bash 
EXE_PATH="build/current/Hypersomnia"

PLATFORM=$1
GIT_BRANCH=$2

if [ "$GIT_BRANCH" != "master" ]; then
	echo "Branch is $GIT_BRANCH. Skipping upload."
	exit 0
fi

if [ -f "$EXE_PATH" ]; then
	echo "Exe found. Uploading."

	COMMIT_HASH=$(git rev-parse HEAD)
	COMMIT_NUMBER=$(git rev-list --count master)
	COMMIT_MESSAGE=$(git log -1 --pretty=%B)
	VERSION="1.0.$COMMIT_NUMBER"
	FILE_PATH="Hypersomnia-for-$PLATFORM.sfx"
	UPLOAD_URL="https://hypersomnia.xyz/upload_artifact.php"

	cp build/current/Hypersomnia hypersomnia/Hypersomnia

	pushd hypersomnia
	rm -r cache logs user
	popd
	
	if [[ "$PLATFORM" = "MacOS" ]]
	then
		echo "Preparing the application bundle."

		APP_PATH="Hypersomnia.app"
		DMG_PATH="Hypersomnia-for-$PLATFORM.dmg"
		SFX_PATH="Hypersomnia-for-$PLATFORM.app.sfx"
		CONTENTS_DIR="$APP_PATH/Contents"

		mkdir $APP_PATH

		cp -R cmake/macos_contents $CONTENTS_DIR
		cp -R hypersomnia "$CONTENTS_DIR/MacOS"
		7z a -sfx $SFX_PATH $APP_PATH

		echo "Uploading a highly compressed sfx archive for updates on MacOS."
		FILE_PATH=$SFX_PATH
		curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" -F "commit_message=$COMMIT_MESSAGE" $UPLOAD_URL

		create-dmg $APP_PATH $DMG_PATH

		echo "Uploading the dmg file for first-time downloads on MacOS."
		FILE_PATH=$DMG_PATH
		curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" -F "commit_message=$COMMIT_MESSAGE" $UPLOAD_URL
	fi

	if [[ "$PLATFORM" = "Linux" ]]
	then
		echo "Uploading a highly compressed sfx archive for updates on Linux."
		7z a -sfx $FILE_PATH hypersomnia
		curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" -F "commit_message=$COMMIT_MESSAGE" $UPLOAD_URL

		echo "Uploading a tar.gz archive for first-time downloads on Linux."
		FILE_PATH="Hypersomnia-for-$PLATFORM.tar.gz"
		tar -czf $FILE_PATH hypersomnia

		curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" -F "commit_message=$COMMIT_MESSAGE" $UPLOAD_URL
	fi
else
	echo "No exe found. Not uploading."
fi

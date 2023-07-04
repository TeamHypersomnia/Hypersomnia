#!/usr/bin/env bash 
EXE_PATH="build/current/Hypersomnia"

PLATFORM=$1
GIT_BRANCH=$2

if [ "$GIT_BRANCH" != "master" ]; then
	echo "Branch is $GIT_BRANCH. Skipping archivization."
	exit 0
fi

if [ -f "$EXE_PATH" ]; then
	echo "Executable exists. Generating archives."

	COMMIT_HASH=$(git rev-parse HEAD)
	COMMIT_NUMBER=$(git rev-list --count master)
	COMMIT_MESSAGE=$(git log -1 --pretty=%B)
	VERSION="1.2.$COMMIT_NUMBER"
	RELEASE_NOTES_FILENAME="release_notes.txt"

	cp build/current/Hypersomnia hypersomnia/Hypersomnia

	pushd hypersomnia
	rm -r cache logs user

	echo "Creating $RELEASE_NOTES_FILENAME"

	echo "$VERSION" > $RELEASE_NOTES_FILENAME
	echo "$COMMIT_HASH" >> $RELEASE_NOTES_FILENAME
	echo "$COMMIT_MESSAGE" >> $RELEASE_NOTES_FILENAME

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

		echo "Generating a highly compressed .sfx archive for updates on MacOS."
		7z a -sfx $SFX_PATH $APP_PATH

		echo "Generating a .dmg for first-time downloads on MacOS."
		create-dmg $APP_PATH
		mv "Hypersomnia undefined.dmg" $DMG_PATH
	fi

	if [[ "$PLATFORM" = "Linux" ]]
	then
		SFX_PATH="Hypersomnia-for-$PLATFORM.sfx"
		TAR_PATH="Hypersomnia-for-$PLATFORM.tar.gz"

		echo "Generating a highly compressed .sfx archive for updates on Linux."
		7z a -sfx $SFX_PATH hypersomnia

		echo "Generating a tar.gz for first-time downloads on Linux."
		tar -czf $TAR_PATH hypersomnia
	fi
else
	echo "No exe found. Nothing to archivize."
fi

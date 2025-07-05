#!/usr/bin/env bash 
EXE_PATH="build/current/Hypersomnia"

PLATFORM=$1
GIT_BRANCH=$2

if [ -f "$EXE_PATH" ]; then
	echo "Executable exists. Generating archives."

	COMMIT_HASH=$(git rev-parse HEAD)
	COMMIT_MESSAGE=$(git log -1 --pretty=%B)
	
	GIT_TAG=$(git describe --exact-match --tags HEAD 2>/dev/null || echo "")
	
	if [ ! -z "$GIT_TAG" ] && [[ ! "$GIT_TAG" =~ fatal ]]; then
		VERSION="$GIT_TAG"
	else
		VERSION="0.0.0"
	fi
	
	RELEASE_NOTES_FILENAME="release_notes.txt"

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

		cp build/current/Hypersomnia "$CONTENTS_DIR/MacOS"
		cp build/current/libsteam_integration.dylib "$CONTENTS_DIR/MacOS"

		echo "Generating a highly compressed .sfx archive for updates on MacOS."
		7z a -sfx $SFX_PATH $APP_PATH

		mount | grep Hypersomnia
		ls /Volumes/

		hdiutil info

		echo "Generating a .dmg for first-time downloads on MacOS."

		i=0
		until [[ -e "${DMG_PATH}" ]]; do
		  echo "Attempt $((i+1)): Running create-dmg..."
		  create-dmg "${DMG_PATH}" "${APP_PATH}" || true
		  if [[ $i -eq 30 ]]; then
			echo 'Error: create-dmg did not succeed even after 30 tries.'
			exit 1
		  fi
		  sleep 2
		  i=$((i+1))
		done
	fi
else
	echo "No exe found. Nothing to archivize."
fi

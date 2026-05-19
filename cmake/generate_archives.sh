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

		# Linker auto-signs arm64 Mach-Os ad-hoc, but the bundle has no
		# _CodeSignature/CodeResources. With quarantine, Gatekeeper sees the
		# inconsistency and reports "is damaged" instead of "unidentified
		# developer". Bundle-wide ad-hoc codesign creates CodeResources covering
		# the unsigned data under Contents/MacOS (content/, detail/, json) and
		# makes the bundle internally consistent.
		if ! command -v codesign >/dev/null 2>&1; then
			echo "ERROR: codesign is required to produce a valid MacOS bundle."
			exit 1
		fi

		echo "Sanitizing extended attributes before signing."
		xattr -cr "$APP_PATH" 2>/dev/null || true

		echo "Removing any pre-existing signatures."
		codesign --remove-signature "$APP_PATH/Contents/MacOS/Hypersomnia" 2>/dev/null || true
		codesign --remove-signature "$APP_PATH/Contents/MacOS/libsteam_integration.dylib" 2>/dev/null || true

		echo "Applying ad-hoc codesign inside-out."
		codesign --force --sign - --timestamp=none "$APP_PATH/Contents/MacOS/libsteam_integration.dylib"
		codesign --force --sign - --timestamp=none "$APP_PATH/Contents/MacOS/Hypersomnia"
		codesign --force --deep --sign - --timestamp=none "$APP_PATH"

		echo "Verifying bundle signature (strict)."
		codesign --verify --deep --strict --verbose=2 "$APP_PATH"

		echo "Signature details:"
		codesign --display --verbose=4 "$APP_PATH" || true

		echo "Gatekeeper assessment (ad-hoc is expected to be rejected; informational):"
		spctl --assess --type execute --verbose "$APP_PATH" || true

		echo "Generating a highly compressed .sfx archive for updates on MacOS."
		7z a -sfx $SFX_PATH $APP_PATH

		mount | grep Hypersomnia
		ls /Volumes/

		hdiutil info

		echo "Generating a .dmg for first-time downloads on MacOS."

		i=0
		until [[ -e "${DMG_PATH}" ]]; do
		  echo "Attempt $((i+1)): Running create-dmg..."
		  create-dmg \
		    --volname "Hypersomnia" \
		    --window-pos 200 120 \
		    --window-size 600 400 \
		    --icon-size 100 \
		    --icon "Hypersomnia.app" 150 200 \
		    --app-drop-link 450 200 \
		    --hide-extension "Hypersomnia.app" \
		    "${DMG_PATH}" \
		    "${APP_PATH}" || true
		  if [[ $i -eq 30 ]]; then
			echo 'Error: create-dmg did not succeed even after 30 tries.'
			exit 1
		  fi
		  sleep 2
		  i=$((i+1))
		done

		echo "Ad-hoc signing the .dmg itself."
		codesign --force --sign - "${DMG_PATH}" || true
		codesign --verify --verbose=2 "${DMG_PATH}" || true
	fi
else
	echo "No exe found. Nothing to archivize."
fi

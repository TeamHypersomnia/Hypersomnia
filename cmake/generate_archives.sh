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

		# Steam variant of the bundle.
		#
		# It must be assembled AND code-signed here on macOS: the Steam
		# upload script runs on Linux, where modifying the bundle would
		# both break the ad-hoc signature and (worse) leave an
		# arch-mismatched dylib inside. So we produce a ready-to-ship,
		# already-signed Steam .sfx that the Linux side only extracts.
		#
		# The non-Steam DMG/SFX above embed the stub libsteam_integration.dylib
		# (BUILD_STEAM=0). For Steam we swap in the real, Steam-enabled
		# integration dylib plus the Steam API dylib, both prebuilt privately
		# with the Steamworks SDK and committed under cmake/steam_integration/bin/macos.
		echo "Preparing the Steam application bundle."

		STEAM_SFX_PATH="Hypersomnia-for-$PLATFORM-Steam.app.sfx"
		STEAM_PACK_DIR="steam_pack"
		STEAM_APP_PATH="$STEAM_PACK_DIR/Hypersomnia.app"
		STEAM_MACOS_DIR="$STEAM_APP_PATH/Contents/MacOS"

		PREBUILT_MACOS="cmake/steam_integration/bin/macos"

		rm -rf "$STEAM_PACK_DIR"
		mkdir -p "$STEAM_PACK_DIR"
		cp -R "$APP_PATH" "$STEAM_APP_PATH"

		cp "$PREBUILT_MACOS/libsteam_integration.dylib" "$STEAM_MACOS_DIR/"
		cp "$PREBUILT_MACOS/libsteam_api.dylib"         "$STEAM_MACOS_DIR/"

		echo "Verifying the Steam integration dylib has an arm64 slice."
		lipo -info "$STEAM_MACOS_DIR/libsteam_integration.dylib"
		if ! lipo -archs "$STEAM_MACOS_DIR/libsteam_integration.dylib" | grep -q arm64; then
			echo "ERROR: $PREBUILT_MACOS/libsteam_integration.dylib has no arm64 slice."
			echo "Rebuild it in the steambuilders repo (universal arm64;x86_64) and re-commit."
			exit 1
		fi

		# Guard against shipping the non-Steam stub in the Steam bundle.
		# The stub (BUILD_STEAM=0) does not link libsteam_api, so steam_init()
		# returns DISABLED at runtime -> the game runs as a non-Steam session
		# (e.g. nickname falls back to "root"). We key on the libsteam_api
		# dependency (a stable redistributable filename) rather than on any
		# SteamAPI symbol name, which would break on SDK renames.
		echo "Verifying the Steam integration dylib is Steam-enabled (links libsteam_api)."
		if ! otool -L "$STEAM_MACOS_DIR/libsteam_integration.dylib" | grep -q libsteam_api; then
			echo "ERROR: libsteam_integration.dylib does not link libsteam_api — it is the non-Steam stub."
			echo "The non-Steam build step likely overwrote $PREBUILT_MACOS; restore the real dylib before packaging."
			exit 1
		fi

		echo "Sanitizing extended attributes before signing (Steam bundle)."
		xattr -cr "$STEAM_APP_PATH" 2>/dev/null || true

		echo "Removing any pre-existing signatures (Steam bundle)."
		codesign --remove-signature "$STEAM_MACOS_DIR/libsteam_integration.dylib" 2>/dev/null || true
		codesign --remove-signature "$STEAM_MACOS_DIR/libsteam_api.dylib" 2>/dev/null || true
		codesign --remove-signature "$STEAM_MACOS_DIR/Hypersomnia" 2>/dev/null || true

		echo "Applying ad-hoc codesign inside-out (Steam bundle)."
		codesign --force --sign - --timestamp=none "$STEAM_MACOS_DIR/libsteam_integration.dylib"
		codesign --force --sign - --timestamp=none "$STEAM_MACOS_DIR/libsteam_api.dylib"
		codesign --force --sign - --timestamp=none "$STEAM_MACOS_DIR/Hypersomnia"
		codesign --force --deep --sign - --timestamp=none "$STEAM_APP_PATH"

		echo "Verifying Steam bundle signature (strict)."
		codesign --verify --deep --strict --verbose=2 "$STEAM_APP_PATH"

		echo "Generating the Steam .sfx archive."
		pushd "$STEAM_PACK_DIR"
			7z a -sfx "../$STEAM_SFX_PATH" Hypersomnia.app
		popd
	fi
else
	echo "No exe found. Nothing to archivize."
fi

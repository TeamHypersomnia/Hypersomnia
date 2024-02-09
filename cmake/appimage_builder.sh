#!/bin/bash
set -e

BUILD_TYPE="non-steam"
if [ "$1" == "--steam" ]; then
  BUILD_TYPE="steam"
fi

# In case we're testing locally
rm -rf /tmp/AppDir

mkdir /tmp/AppDir/usr/bin -p
mkdir /tmp/AppDir/usr/lib -p

cp build/current/Hypersomnia /tmp/AppDir/usr/bin
strip /tmp/AppDir/usr/bin/Hypersomnia

if [ "$BUILD_TYPE" == "steam" ]; then
  cp cmake/steam_integration/bin/linux/libsteam_api.so /tmp/AppDir/usr/lib
  cp cmake/steam_integration/bin/linux/libsteam_integration.so /tmp/AppDir/usr/lib
  strip /tmp/AppDir/usr/lib/libsteam_api.so
  strip /tmp/AppDir/usr/lib/libsteam_integration.so

  pushd /tmp/AppDir/usr/bin
  # So linuxdeploy can find it
  ln -s ../lib/libsteam_integration.so
  ln -s ../lib/libsteam_api.so
  popd
else
	if [ -f "build/current/libsteam_integration.so" ]; then
		cp build/current/libsteam_integration.so /tmp/AppDir/usr/lib
		strip /tmp/AppDir/usr/lib/libsteam_integration.so

		pushd /tmp/AppDir/usr/bin
		# So linuxdeploy can find it
		ln -s ../lib/libsteam_integration.so
		popd
	else
		echo "Warning: libsteam_integration.so was not found!"
		echo "Game might have been built with DEVELOP_STEAM_INTEGRATION=1"
		echo "In that case, we must add a steam_appid.txt to the .AppImage."

		echo "2660970" > hypersomnia/steam_appid.txt
	fi
fi

cp hypersomnia/content/gfx/cyan_scythe.png /tmp/AppDir/

if [ ! -f linuxdeploy-x86_64.AppImage ]; then
	# Download linuxdeploy to create the AppImage
	curl -O --location 'https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'
	chmod +x linuxdeploy-x86_64.AppImage
fi

mkdir -p /tmp/AppDir/usr/share/ /tmp/AppDir/usr/bin
cp -r hypersomnia/ /tmp/AppDir/usr/share/

# In case we're testing locally
rm -rf /tmp/AppDir/usr/share/hypersomnia/user
rm -rf /tmp/AppDir/usr/share/hypersomnia/logs
rm -rf /tmp/AppDir/usr/share/hypersomnia/cache

if [ "$BUILD_TYPE" == "steam" ]; then
  OUTPUT_NAME="Hypersomnia-Steam.AppImage"
else
  OUTPUT_NAME="Hypersomnia.AppImage"
fi

# Create the AppImage.
OUTPUT=$OUTPUT_NAME ARCH=x86_64 ./linuxdeploy-x86_64.AppImage --appdir=/tmp/AppDir --icon-file=hypersomnia/content/gfx/metropolis_square_logo.png --icon-filename=Hypersomnia --desktop-file=cmake/Hypersomnia.desktop --executable=/tmp/AppDir/usr/bin/Hypersomnia --custom-apprun=cmake/AppRun --output=appimage

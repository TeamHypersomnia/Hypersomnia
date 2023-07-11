#!/bin/bash
set -e

# In case we're testing locally
rm -rf /tmp/AppDir

mkdir /tmp/AppDir/usr/bin -p
cp build/current/Hypersomnia /tmp/AppDir/usr/bin
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

# Create the AppImage.
OUTPUT="Hypersomnia.AppImage" ARCH=x86_64 ./linuxdeploy-x86_64.AppImage --appdir=/tmp/AppDir --icon-file=hypersomnia/content/gfx/metropolis_square_logo.png --icon-filename=Hypersomnia --desktop-file=cmake/Hypersomnia.desktop --executable=build/current/Hypersomnia --custom-apprun=cmake/AppRun --output=appimage

#!/bin/bash

set -e

# Save the version of the executable we are packing for later use for the update check
current_version=$(./build/current/Hypersomnia --version | grep "Hypersomnia version " | cut -d' ' -f3)
current_version_major=$(echo "$current_version" | cut -d'.' -f1)
current_version_minor=$(echo "$current_version" | cut -d'.' -f2)
current_version_patchlevel=$(echo "$current_version" | cut -d'.' -f3)

mkdir /tmp/AppDir/usr/bin -p
cp build/current/Hypersomnia /tmp/AppDir/usr/bin
cp hypersomnia/content/gfx/cyan_scythe.png /tmp/AppDir/

# An AppImage needs a desktop file. This may show up in the program launcher.
cat <<EOF > Hypersomnia.desktop
[Desktop Entry]
Type=Application
Categories=Game
Name=Hypersomnia
Exec=Hypersomnia
Icon=Hypersomnia
StartupNotify=false
Terminal=false
EOF

# AppRun is the script that gets executed when running the AppImage
cat <<EOF > AppRun
#!/bin/sh

current_version_major="$current_version_major"
current_version_minor="$current_version_minor"
current_version_patchlevel="$current_version_patchlevel"

# Figure out the XDG_CONFIG_HOME where to store the dynamic data
if [ -z \$XDG_CONFIG_HOME ]; then
  if [ -z \$HOME ]; then
    HOME=\$(getent passwd "\$(whoami)" | cut -d':' -f 6)
  fi

  XDG_CONFIG_HOME="\$HOME/.config"
fi
config_home="\${XDG_CONFIG_HOME}/Hypersomnia"

# Get the latest version from the website
latest_version=\$(curl --silent 'https://hypersomnia.xyz/builds/latest/version-Linux.txt' | head -n1)
latest_version_major=\$(echo "\$latest_version" | cut -d'.' -f1)
latest_version_minor=\$(echo "\$latest_version" | cut -d'.' -f2)
latest_version_patchlevel=\$(echo "\$latest_version" | cut -d'.' -f3)

# If that worked and the version on the website is newer, run the included AppImageUpdate
# to update this AppImage and then launch the new version
if [ -n \$latest_version ]; then
  if [ \$latest_version_major -gt \$current_version_major ] || \\
     { [ \$latest_version_major -eq \$current_version_major ] && [ \$latest_version_minor -gt \$current_version_minor ]; } || \\
     { [ \$latest_version_major -eq \$current_version_major ] && [ \$latest_version_minor -eq \$current_version_minor ] && [ \$latest_version_patchlevel -gt \$current_version_patchlevel ]; }; then
    "\${APPDIR}"/usr/bin/AppImageUpdate-x86_64.AppImage \$APPIMAGE
    if [ -d "\${config_home}.old" ]; then
      rm -rf "\${config_home}.old"
    fi
    mv "\${config_home}" "\${config_home}.old"
    exec "\$APPIMAGE" \$@
    exit 0
  fi
fi

# If there isn't already data in our config dir, copy it from the mounted
# AppImage to the config dir. If an old config exists, copy the user and
# logs directories from there.
if ! [ -d "\${config_home}" ]; then
  mkdir -p "\${config_home}"
  cp -r "\${APPDIR}"/usr/share/hypersomnia/* "\${config_home}"
  if [ -d "\${config_home}.old"/user ]; then
    cp -r "\${config_home}.old"/user "\${config_home}"
  fi
  if [ -d "\${config_home}.old"/logs ]; then
    cp -r "\${config_home}.old"/logs "\${config_home}"
  fi
fi

# Run the actual executable. The --keep-cwd is needed since we need to
# run from the confi dir since the AppImage is only mounted read-only.
# Also disable the updater from Hypersomnia itself. That won't work on
# a read-only file-system
cd "\${config_home}"
"\${APPDIR}"/usr/bin/Hypersomnia --keep-cwd --no-update \$@
EOF

# Download linuxdeploy to create the AppImage
curl -O --location 'https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'
chmod +x linuxdeploy-x86_64.AppImage
# Include AppImageUpdate to update the AppImage automatically
curl -O --location 'https://github.com/AppImageCommunity/AppImageUpdate/releases/download/continuous/AppImageUpdate-x86_64.AppImage'
chmod +x AppImageUpdate-x86_64.AppImage

mkdir -p /tmp/AppDir/usr/share/ /tmp/AppDir/usr/bin
cp -r hypersomnia/ /tmp/AppDir/usr/share/
cp AppImageUpdate-x86_64.AppImage /tmp/AppDir/usr/bin

# Create the AppImage. This will also create the zsync file neded for updating.
OUTPUT="Hypersomnia.AppImage" UPDATE_INFORMATION="zsync|https://hypersomnia.xyz/builds/latest/Hypersomnia.AppImage.zsync" ARCH=x86_64 ./linuxdeploy-x86_64.AppImage --appdir=/tmp/AppDir --icon-file=hypersomnia/content/gfx/metropolis_square_logo.png --icon-filename=Hypersomnia --desktop-file=Hypersomnia.desktop --executable=build/current/Hypersomnia --custom-apprun=AppRun --output=appimage

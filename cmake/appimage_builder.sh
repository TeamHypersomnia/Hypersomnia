#!/bin/bash

set -e

current_version=$(./build/current/Hypersomnia --version | grep "Hypersomnia version " | cut -d' ' -f3)
current_version_major=$(echo "$current_version" | cut -d'.' -f1)
current_version_minor=$(echo "$current_version" | cut -d'.' -f2)
current_version_patchlevel=$(echo "$current_version" | cut -d'.' -f3)

mkdir /tmp/AppDir/usr/bin -p
cp build/current/Hypersomnia /tmp/AppDir/usr/bin
cp hypersomnia/content/gfx/cyan_scythe.png /tmp/AppDir/

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

cat <<EOF > AppRun
#!/bin/sh

current_version_major="$current_version_major"
current_version_minor="$current_version_minor"
current_version_patchlevel="$current_version_patchlevel"

if [ -z \$XDG_CONFIG_HOME ]; then
  if [ -z \$HOME ]; then
    HOME=\$(getent passwd "\$(whoami)" | cut -d':' -f 6)
  fi

  XDG_CONFIG_HOME="\$HOME/.config"
fi

config_home="\${XDG_CONFIG_HOME}/Hypersomnia"
latest_version=\$(curl --silent 'https://hypersomnia.xyz/builds/latest/version-Linux.txt' | head -n1)
latest_version_major=\$(echo "\$latest_version" | cut -d'.' -f1)
latest_version_minor=\$(echo "\$latest_version" | cut -d'.' -f2)
latest_version_patchlevel=\$(echo "\$latest_version" | cut -d'.' -f3)

if [ -n \$latest_version ]; then
  if [ \$latest_version_major -gt \$current_version_major ] || \\
     { [ \$latest_version_major -eq \$current_version_major ] && [ \$latest_version_minor -gt \$current_version_minor ]; } || \\
     { [ \$latest_version_major -eq \$current_version_major ] && [ \$latest_version_minor -eq \$current_version_minor ] && [ \$latest_version_patchlevel -gt \$current_version_patchlevel ]; }; then
    "\${APPDIR}"/usr/bin/AppImageUpdate-x86_64.AppImage \$APPIMAGE
    rm -rf "\${config_home}"
    exec "\$APPIMAGE" \$@
    exit 0
  fi
fi

if ! [ -d "\${config_home}" ]; then
  mkdir -p "\${config_home}"
  cp -r "\${APPDIR}"/usr/share/hypersomnia/* "\${config_home}"
fi
cd "\${config_home}"
"\${APPDIR}"/usr/bin/Hypersomnia --keep-cwd --no-update \$@
EOF

curl -O --location 'https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'
chmod +x linuxdeploy-x86_64.AppImage
curl -O --location 'https://github.com/AppImageCommunity/AppImageUpdate/releases/download/continuous/AppImageUpdate-x86_64.AppImage'
chmod +x AppImageUpdate-x86_64.AppImage

mkdir -p /tmp/AppDir/usr/share/ /tmp/AppDir/usr/bin
cp -r hypersomnia/ /tmp/AppDir/usr/share/
cp AppImageUpdate-x86_64.AppImage /tmp/AppDir/usr/bin

OUTPUT="Hypersomnia.AppImage" UPDATE_INFORMATION="zsync|https://hypersomnia.xyz/builds/latest/Hypersomnia.AppImage.zsync" ARCH=x86_64 ./linuxdeploy-x86_64.AppImage --appdir=/tmp/AppDir --icon-file=hypersomnia/content/gfx/metropolis_square_logo.png --icon-filename=Hypersomnia --desktop-file=Hypersomnia.desktop --executable=build/current/Hypersomnia --custom-apprun=AppRun --output=appimage

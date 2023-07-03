#!/bin/bash

set -e

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
if [ -z \$XDG_CONFIG_HOME ]; then
  if [ -z \$HOME ]; then
    HOME=\$(getent passwd "\$(whoami)" | cut -d':' -f 6)
  fi

  XDG_CONFIG_HOME="\$HOME/.config"
fi

config_home="\${XDG_CONFIG_HOME}/Hypersomnia"
if ! [ -d "\${config_home}" ]; then
  mkdir -p "\${config_home}"
  cp -r "\${APPDIR}"/usr/share/hypersomnia/* "\${config_home}"
fi
cd "\${config_home}"
"\${APPDIR}"/usr/bin/Hypersomnia --keep-cwd --no-update \$@
EOF

curl -O --location 'https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'
chmod +x linuxdeploy-x86_64.AppImage

mkdir -p /tmp/AppDir/usr/share/
cp -r hypersomnia/ /tmp/AppDir/usr/share/

OUTPUT="Hypersomnia.AppImage" ARCH=x86_64 ./linuxdeploy-x86_64.AppImage --appdir=/tmp/AppDir --icon-file=hypersomnia/content/gfx/metropolis_square_logo.png --icon-filename=Hypersomnia --desktop-file=Hypersomnia.desktop --executable=build/current/Hypersomnia --custom-apprun=AppRun --output=appimage

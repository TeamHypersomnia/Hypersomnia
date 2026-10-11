#!/bin/bash
# account_linked_as.txt: a key linked to Steam is shown as the Steam account from the start.
# One start of a key client to the main menu, Settings - Advanced - Network screenshotted.
# Usage: account_linked_as_check.sh <appdata dir with user/account.key> <broker url> <screenshot.png>
export ALSOFT_DRIVERS=null VISUAL=true
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
A=$1
mkdir -p $A/user/conf.d
cat > $A/user/conf.d/test.json <<J
{ "identity_broker_url": "$2", "server_list_provider": "http://127.0.0.1:1", "launch_at_startup": "MAIN_MENU",
  "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "linkedas" } }
J
cd $REPO/hypersomnia
Xvfb :83 -screen 0 1280x720x24 > /dev/null 2>&1 &
XV=$!
sleep 1
DISPLAY=:83 timeout 60 $BIN --appdata-dir $A --no-update-on-launch > $A/run.out 2>&1 &
G=$!
sleep 11
export DISPLAY=:83
python3 $HERE/xinput.py click 156 555; sleep 1.5
python3 $HERE/xinput.py click 333 246; sleep 1
python3 $HERE/xinput.py click 425 278; sleep 1
import -window root $3
kill $G; sleep 1; kill $XV

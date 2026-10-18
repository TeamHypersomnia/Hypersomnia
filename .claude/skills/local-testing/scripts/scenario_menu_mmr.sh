#!/bin/bash
# After scenario_ranked_report.sh: the player "alice" starts the game into the main menu with the leaderboards
# of the local site - the header must show her place and MMR. Then she goes to a casual... (not needed):
# the leaderboards are downloaded once at start and not again on coming back to the menu.
# Usage: [PLAYER=bob] [BIN=<steam build>] scenario_menu_mmr.sh <screenshot.png>
. "$(dirname "$0")/test_env.sh"
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
A=$S/ranked_report/${PLAYER:-alice}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
[ -d $A/user ] || { echo "run scenario_ranked_report.sh first"; exit 1; }
cat > $A/user/conf.d/menu.json <<J
{ "launch_at_startup": "MAIN_MENU", "window": { "fullscreen": false, "size": [1280, 720] },
  "main_menu": { "leaderboards_provider_url": "$BROKER_URL/leaderboards" }, "website_url": "$BROKER_URL" }
J
cd $REPO/hypersomnia
Xvfb :89 -screen 0 1280x720x24 > /dev/null 2>&1 &
XV=$!
sleep 1
DISPLAY=:89 timeout 40 $BIN --appdata-dir $A $GAME_TEST_FLAGS --mute > $A/menu.out 2>&1 &
G=$!
sleep 18
DISPLAY=:89 import -window root ${1:-$S/menu_mmr.png}
wait $G
kill $XV
rm $A/user/conf.d/menu.json
echo "leaderboards downloads: $(grep -c 'Launching future_response' $A/menu.out)"

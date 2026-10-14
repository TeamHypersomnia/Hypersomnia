#!/bin/bash
# The user's running Steam: the Steam build joins a local ranked server with a cert the local broker made from a real
# Steam Web API ticket. Needs broker.sh started with STEAM_API_KEY_FILE and BROKER_URL / BROKER_PUBLIC_KEY exported.
# Usage: scenario_steam.sh
export ALSOFT_DRIVERS=null VISUAL=true
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
# A fresh directory every run - never removed recursively
T=$S/steam_test_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
SBIN=${STEAM_BIN:-$REPO/build/Debug-x64-clang-claude-steam/Hypersomnia}
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] || { echo "need BROKER_URL and BROKER_PUBLIC_KEY"; exit 1; }
pgrep -x steam > /dev/null || { echo "Steam is not running"; exit 1; }

for port in 10042; do
	if ss -tuln | grep -q ":$port "; then echo "port $port busy, abort"; exit 1; fi
done

mkdir -p $T/ranked/user/conf.d $T/steam/user/conf.d
cat > $T/ranked/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": 10042, "slots": 16 },
  "server": { "server_name": "steam test ranked", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "ranked": { "autostart_when": "TEAMS_EQUAL" },
              "webrtc_port_range_begin": 19500, "webrtc_port_range_end": 19505 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "http://127.0.0.1:1/report_match" } }
J
cat > $T/steam/user/conf.d/test.json <<J
{ "identity_broker_url": "$BROKER_URL", "website_url": "$BROKER_URL", "server_list_provider": "http://127.0.0.1:1",
  "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "steamtest" } }
J
cd $REPO/hypersomnia
timeout 120 nice -n 10 $BIN --dedicated-server --appdata-dir $T/ranked --no-update-on-launch > $T/ranked.out 2>&1 &
sleep 6
Xvfb :81 -screen 0 1280x720x24 > /dev/null 2>&1 &
XV=$!
sleep 1
DISPLAY=:81 timeout ${STEAM_SECS:-40} $SBIN --connect 127.0.0.1:10042 --appdata-dir $T/steam --no-update-on-launch > $T/steam.out 2>&1
echo "=== steam client"
grep -hE "identity cert|Connecting as|Received initial state|Reason|refused" $T/steam.out | sed 's/^\[[0-9:]*\] //' | head -6
echo "=== server"
grep -hE "Authenticated|need an account" $T/ranked.out | sed 's/^\[[0-9:]*\] //' | head -3
kill $XV
for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done

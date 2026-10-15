#!/bin/bash
# The CrazyGames web build in Chromium (pw_driver.js under Xvfb) with a stubbed CrazyGames SDK whose signed-in user's token
# is signed with a test key, joining a local official ranked server over WebRTC with the crazygames_ cert from the broker.
# The broker must run with CRAZYGAMES_PUBLIC_KEY_PEM=<CG_KEY's public part> and CRAZYGAMES_GAME_ID=$CG_GAME_ID (broker.sh).
# Usage: CG_KEY=<private key pem> [CG_GAME_ID=hypersomnia-test] scenario_crazygames.sh [wait secs]
export ALSOFT_DRIVERS=null VISUAL=true
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/cg_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
CG_BUILD=${CG_BUILD:-$REPO/build/Release-Web-clang-claude-web-crazygames}
CG_GAME_ID=${CG_GAME_ID:-hypersomnia-test}
PW=${PW_NODE_PATH:-$HOME/.cache/hs-playwright/node_modules}
WAIT=${1:-100}
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] && [ -f "$CG_KEY" ] || { echo "need BROKER_URL, BROKER_PUBLIC_KEY and CG_KEY"; exit 1; }

for port in 18000 18410 18420 18430 10002 8766 9334; do
	if ss -tuln | grep -q ":$port "; then echo "port $port busy, abort"; exit 1; fi
done

rm -rf $T
mkdir -p $T/official_servers.d
mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<J
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
J
mkconf server <<J
{ "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "instances": { "ranked": 1 }, "server_start": { "ip": "0.0.0.0", "port": 10002, "slots": 16 },
  "server": { "server_name": "[PL] Ranked test", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { "start_when": "TEAMS_EQUAL" },
              "webrtc_port_range_begin": 19000, "webrtc_port_range_end": 19050 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "$BROKER_URL/report_match" } }
J

"$HERE/prepare_web.sh" "$CG_BUILD" $T/web > /dev/null
python3 "$HERE/crazygames_sdk_stub.py" "$CG_KEY" "$CG_GAME_ID" cgUser_e2e $T/cg_sdk_stub.js

cd $REPO/hypersomnia
$BIN --print-server-key --appdata-dir $T/server 2>/dev/null | grep -E '^[0-9a-f]{64}$' > $T/official_servers.d/pl
timeout $((WAIT + 60)) $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
timeout $((WAIT + 55)) nice -n 10 $BIN --dedicated-server --appdata-dir $T/server --no-update-on-launch > $T/server.out 2>&1 &
timeout $((WAIT + 50)) python3 "$HERE/webserve.py" $T/web 8766 > $T/web.out 2>&1 &
sleep 14

Xvfb :82 -screen 0 1280x800x24 > /dev/null 2>&1 &
XV=$!
sleep 1
DISPLAY=:82 NODE_PATH=$PW PW_ROUTES="{\"https://sdk.crazygames.com/crazygames-sdk-v3.js\": \"$T/cg_sdk_stub.js\"}" \
	timeout $((WAIT + 10)) node "$HERE/pw_driver.js" http://127.0.0.1:8766/game/ranked-pl 9334 > $T/pw.out 2>&1 &
sleep $WAIT
curl -s "localhost:9334/shot?f=$T/screen.png" > /dev/null

echo "=== browser console (game)"
curl -s localhost:9334/console | grep -E "SDK stub|identity cert|Connecting as|client connected|Received initial state|last_disconnect_reason|Reason" | sed 's/^\[[^]]*\] //' | head -10
echo "=== server"
grep -hE "Authenticated|need an account|kicked|requested nickname" $T/server.out | sed 's/^\[[0-9:]*\] //' | head -6

curl -s localhost:9334/quit > /dev/null
kill $XV 2>/dev/null
for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
for p in $(pgrep -f "webserve.py $T/web"); do kill $p; done
echo "=== screenshot: $T/screen.png"

#!/bin/bash
# The web build in Firefox (under Xvfb) joining a local official server over WebRTC, with a cert from the local broker.
# Start the broker first (broker.sh start) and prepare the build with prepare_web.sh - it points the game at both.
# Everything on localhost: test master on 18xxx, server on 10002 (WebRTC 19000-19050), web on 8765, broker on 13500, Xvfb :78.
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
export ALSOFT_DRIVERS=null
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
# A fresh directory every run - never removed recursively
T=$S/web_test_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
GAME=$REPO/hypersomnia
WEB=${WEB:-$S/webserve}
CONNECT=${1:-pl}
BROKER_URL=${BROKER_URL:-http://127.0.0.1:13500}
BROKER_PUBLIC_KEY=${BROKER_PUBLIC_KEY:-}
WAIT=${2:-120}
# TAB_AWAY=<s>: the game's tab left for another one that long, then back (screen_back.png); KILL_SERVER=1: see the end
EXTRA=$(( ${TAB_AWAY:-0} + ${KILL_SERVER:+30} + 0 ))

for p in $(pgrep -f "^$BIN .*--appdata-dir $S/web_test"); do kill $p; done
for p in $(pgrep -f "webserve.py $WEB"); do kill $p; done
for p in $(pgrep -f "firefox --no-remote --profile $S/web_test"); do kill $p; done
for p in $(pgrep -f "^Xvfb :78"); do kill $p; done
sleep 1

for port in 18000 18410 18420 18430 10002 8765; do
	if ss -tulpn 2>/dev/null | grep -q ":$port\b"; then echo "port $port busy, abort"; exit 1; fi
done

mkdir -p $T/official_servers.d $T/ff

mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF

RANKED=NEVER
NAME="[PL] Casual test"
if [[ "$CONNECT" == ranked* ]]; then RANKED=TEAMS_EQUAL; NAME="[PL] Ranked test"; fi

mkconf server <<EOF
{
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "instances": { "ranked": $([ "$RANKED" = NEVER ] && echo 0 || echo 1) }, "server_start": { "ip": "0.0.0.0", "port": 10002, "slots": 16 },
  "server": { "server_name": "$NAME", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { "start_when": "${RANKED/NEVER/ALWAYS}" },
              "webrtc_port_range_begin": 19000, "webrtc_port_range_end": 19050 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "$BROKER_URL/report_match" }
}
EOF

cat > $T/ff/user.js <<'EOF'
user_pref("browser.shell.checkDefaultBrowser", false);
user_pref("browser.startup.homepage_override.mstone", "ignore");
user_pref("datareporting.policy.dataSubmissionEnabled", false);
user_pref("toolkit.telemetry.reportingpolicy.firstRun", false);
user_pref("webgl.force-enabled", true);
user_pref("media.autoplay.default", 0);
user_pref("devtools.console.stdout.content", true);
user_pref("media.volume_scale", "0.0");
EOF

cd $GAME
# Official by its public key
$BIN --print-server-key --appdata-dir $T/server 2>/dev/null | grep -E '^[0-9a-f]{64}$' > $T/official_servers.d/pl
# "@KEY" in the connect string is the server's key - e.g. "pl,@KEY", as the site's Browser links are
CONNECT=${CONNECT//@KEY/$(cat $T/official_servers.d/pl)}

timeout $((WAIT + EXTRA + 60)) $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
timeout $((WAIT + EXTRA + 55)) nice -n 10 $BIN --dedicated-server --appdata-dir $T/server --no-update-on-launch > $T/server.out 2>&1 &
timeout $((WAIT + EXTRA + 50)) python3 "$HERE/webserve.py" $WEB 8765 > $T/web.out 2>&1 &
sleep 14

echo "=== list"
curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], 'webrtc_id=', e.get('webrtc_id'), 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'))"

Xvfb :78 -screen 0 1280x720x24 > /dev/null 2>&1 &
sleep 2
DISPLAY=:78 timeout $((WAIT + EXTRA)) firefox --no-remote --profile $T/ff --new-instance "http://localhost:8765/game/$CONNECT" > $T/firefox.out 2>&1 &
sleep $((WAIT - 10))
DISPLAY=:78 import -window root $T/screen.png

echo "=== server"
grep -hE "connected\. SID|requested nickname|Adding|Authenticated|did not verify|kicked|Reason|Ranked matches need" $T/server.out | sed 's/^\[[0-9:]*\] //' | head -10
echo "=== browser console (game)"
grep -E "identity cert|Connecting as|client connected|Received initial|WebRTC: DataChannel|last_disconnect_reason" $T/firefox.out | head -10

if [ -n "$TAB_AWAY" ]; then
	DISPLAY=:78 python3 "$HERE/xinput.py" key Control_L+t
	echo "=== the game's tab in the background for $TAB_AWAY s"
	sleep $TAB_AWAY
	DISPLAY=:78 python3 "$HERE/xinput.py" key Control_L+w
	sleep 8
	DISPLAY=:78 import -window root $T/screen_back.png
	echo "=== back: $(grep -cE 'disconnect|Disconnected|data channel closed' $T/firefox.out) disconnect lines"
	grep -E "WebRTC: State|disconnect" $T/firefox.out | tail -4
fi

# KILL_SERVER=1: the server SIGKILLed in the game - the browser must say the connection is lost (screen_killed.png)
if [ -n "$KILL_SERVER" ]; then
	for p in $(pgrep -f "^$BIN --dedicated-server --appdata-dir $T/server "); do kill -9 $p; done
	echo "=== server killed"
	sleep 25
	DISPLAY=:78 import -window root $T/screen_killed.png
	grep -E "disconnect|Disconnected|closed|State: " $T/firefox.out | tail -5
fi

sleep 12
for p in $(pgrep -f "^$BIN .*--appdata-dir $S/web_test"); do kill $p; done
for p in $(pgrep -f "webserve.py $WEB"); do kill $p; done
for p in $(pgrep -f "^Xvfb :78"); do kill $p; done
echo "=== screenshot: $T/screen.png"

#!/bin/bash
# The server browser's tournament rows: a local master with an official tournament ("Official Cup", 4 teams - 2 matches),
# a community one (an open round 1 for 8 - 4 matches, "Community Cup") and a casual server; a client opens Browse servers
# (Settings-sized window, imgui_big.ini) and a screenshot is taken. KEEP_RUNNING=1 leaves everything up for clicking
# (DISPLAY=:$DISP, xinput.py) - stop it with "scenario_browser_tournaments.sh stop".
#   ONLY_COMMUNITY=listed|open: only the community tournament (the generator's screenshots) - 8 teams by nicknames, or
#   an open round 1 for 8 (any pair).
#   FAVORITES='[...]': the client's favorite_servers.json; KEY_OF_OFFICIAL / KEY_OF_COMMUNITY in it are replaced
#   by the servers' keys.
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. $HERE/test_env.sh
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
S=${SCRATCH:-/tmp/hypersomnia-nettest}/browser_tournaments
DISP=${DISP:-:997}
# SCREEN=1600x900: a bigger window (the generator's screenshots - long match names whole)
SCREEN=${SCREEN:-1280x720}
P_OFFICIAL=19400; P_COMMUNITY=19420; P_CASUAL=19440

if [ "$1" = "stop" ]; then
	for pid in $(pgrep -f -- "--appdata-dir $S/"); do kill $pid 2>/dev/null; done
	for pid in $(pgrep -f -- "Xvfb $DISP"); do kill $pid 2>/dev/null; done
	exit 0
fi

for port in 18000 18410 18420 18430 $P_OFFICIAL $((P_OFFICIAL + 1)) $P_COMMUNITY $((P_COMMUNITY + 3)) $P_CASUAL; do
	if ss -tuln | grep -q ":$port "; then echo "port $port busy - abort"; exit 1; fi
done

S=$S/$(date +%s); mkdir -p $S/official_servers.d
mkconf() { mkdir -p $S/$1/user/conf.d; cat > $S/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$S/official_servers.d" } }
EOF

server_conf() {
mkconf $1 <<EOF
{ "server_start": { "port": $2 },
  "server": { "server_name": "$3", "notified_server_list": "127.0.0.1:18430", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": $(($2 + 300)) },
  "server_private": { "report_ranked_match_url": "http://127.0.0.1:1/report_match" } }
EOF
}

server_conf official $P_OFFICIAL "Official"
server_conf community $P_COMMUNITY "Community"
server_conf casual $P_CASUAL "Casual server"

cd $REPO/hypersomnia
key_of() { $BIN --dedicated-server --print-server-key --appdata-dir $S/$1 $GAME_TEST_FLAGS 2>/dev/null | grep -E '^[0-9a-f]{64}$'; }
OFFICIAL_KEY=$(key_of official); COMMUNITY_KEY=$(key_of community)
echo "$OFFICIAL_KEY" > $S/official_servers.d/official

SECS=${KEEP_RUNNING:+1800}; SECS=${SECS:-240}
timeout $SECS $BIN --masterserver --appdata-dir $S/ms $GAME_TEST_FLAGS > $S/ms.out 2>&1 &
sleep 3
if [ -z "$ONLY_COMMUNITY" ]; then
	timeout $SECS $BIN --appdata-dir $S/official $GAME_TEST_FLAGS --tournament-teams "alice; bob; carol; dave" --tournament-name "Official Cup" > $S/official.out 2>&1 &
	timeout $SECS $BIN --dedicated-server --appdata-dir $S/casual $GAME_TEST_FLAGS > $S/casual.out 2>&1 &
fi

if [ "$ONLY_COMMUNITY" = listed ]; then
	timeout $SECS $BIN --appdata-dir $S/community $GAME_TEST_FLAGS --tournament-name "Community Cup" \
		--tournament-teams "Billan; Saper; Fajfer; Janusz; Kartezjan; Lopa; Litwin; Shuncio" > $S/community.out 2>&1 &
else
	timeout $SECS env HYPERSOMNIA_TOURNAMENT=8 $BIN --appdata-dir $S/community $GAME_TEST_FLAGS --tournament-name "Community Cup" > $S/community.out 2>&1 &
fi
sleep 12

mkconf eve <<EOF
{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:18420/server_list",
  "window": { "fullscreen": false, "size": [${SCREEN%x*}, ${SCREEN#*x}] }, "client": { "nickname": "eve" } }
EOF
cp $HERE/identity_ui/imgui_big.ini $S/eve/user/imgui.ini

if [ -n "$FAVORITES" ]; then
	echo "$FAVORITES" | sed "s/KEY_OF_OFFICIAL/$OFFICIAL_KEY/g; s/KEY_OF_COMMUNITY/$COMMUNITY_KEY/g" > $S/eve/user/favorite_servers.json
fi

Xvfb $DISP -screen 0 ${SCREEN}x24 > /dev/null 2>&1 &
sleep 2
env DISPLAY=$DISP LP_NUM_THREADS=2 timeout $SECS $CLIENT_BIN --appdata-dir $S/eve $GAME_TEST_FLAGS > $S/eve.out 2>&1 &
sleep 40
DISPLAY=$DISP python3 $HERE/xinput.py key Escape; sleep 3
# Browse servers: 223 px above the bottom, whatever the window's height
DISPLAY=$DISP python3 $HERE/xinput.py click 190 $((${SCREEN#*x} - 223)); sleep 12
DISPLAY=$DISP import -window root $S/browse.png

curl -s http://127.0.0.1:18420/server_list_json > $S/list.json
python3 -c "
import json,sys
l=json.load(open('$S/list.json'))
ok = any(e['name']=='Casual server' for e in l) and sum(e['tournament_name']=='Official Cup' for e in l)==2 and sum(e['tournament_name']=='Community Cup' for e in l)==4
print('list: casual + 2 official + 4 community matches -', 'PASS' if ok else 'FAIL')"
echo "=== $S - screenshot $S/browse.png, list $S/list.json, DISPLAY=$DISP"

if [ -z "$KEEP_RUNNING" ]; then
	$0 stop
fi

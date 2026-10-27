#!/bin/bash
# A favorite added by hand (key + address) of a server that is also on the server list: one row, the list's.
#   a local master and one community server on it; a client with that server as a manual favorite under its own name -
#   opens Browse servers: the listed row shows the typed name and a lit star, Unlisted (0) - so no ping of its own;
#   read it from the screenshot. KEEP_RUNNING=1 leaves everything up for clicking (kill by the printed dir).
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. $HERE/test_env.sh
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
S=${SCRATCH:-/tmp/hypersomnia-nettest}/manual_favorite_listed_$(date +%s); mkdir -p $S
DISP=${DISP:-:998}
RESULTS=()
check() { local what=$1; shift; if "$@"; then RESULTS+=("PASS  $what"); else RESULTS+=("FAIL  $what"); fi; echo "${RESULTS[-1]}"; }

for port in 18000 18410 18420 18430 10004; do
	if ss -tuln | grep -q ":$port "; then echo "port $port busy - abort"; exit 1; fi
done

mkconf() { mkdir -p $S/$1/user/conf.d; cat > $S/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false } }
EOF

mkconf community <<EOF
{ "server_start": { "ip": "0.0.0.0", "port": 10004, "slots": 16 },
  "server": { "server_name": "Name from the list", "map_cycle": [ "de_cyberaqua" ], "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true } }
EOF

mkconf eve <<EOF
{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:18420/server_list",
  "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "eve" } }
EOF

cd $REPO/hypersomnia
KEY=$($BIN --dedicated-server --print-server-key --appdata-dir $S/community $GAME_TEST_FLAGS 2>/dev/null | grep -E '^[0-9a-f]{64}$')

cat > $S/eve/user/favorite_servers.json <<EOF
[ { "key": "$KEY", "label": "", "address": "127.0.0.1:10004", "name": "My typed name", "manual": true, "official": false } ]
EOF

SECS=${KEEP_RUNNING:+1200}; SECS=${SECS:-200}
timeout $SECS $BIN --masterserver --appdata-dir $S/ms $GAME_TEST_FLAGS > $S/ms.out 2>&1 &
sleep 3
timeout $SECS $BIN --dedicated-server --appdata-dir $S/community $GAME_TEST_FLAGS > $S/community.out 2>&1 &
sleep 8
check "heartbeats only to the local master" grep -q "Sending heartbeat through UDP to 127.0.0.1:18430" $S/community.out

Xvfb $DISP -screen 0 1280x720x24 > /dev/null 2>&1 &
XVFB=$!
sleep 2
env DISPLAY=$DISP LP_NUM_THREADS=2 timeout $SECS $CLIENT_BIN --appdata-dir $S/eve $GAME_TEST_FLAGS > $S/eve.out 2>&1 &
sleep 40
DISPLAY=$DISP python3 $HERE/xinput.py key Escape; sleep 3
DISPLAY=$DISP python3 $HERE/xinput.py click 190 497; sleep 15
DISPLAY=$DISP import -window root $S/browse.png

if [ -z "$KEEP_RUNNING" ]; then
	for pid in $(pgrep -f -- "--appdata-dir $S/"); do kill $pid 2>/dev/null; done
	kill $XVFB 2>/dev/null
fi

echo; echo "=== $S (screenshot: $S/browse.png, DISPLAY=$DISP)"
printf '%s\n' "${RESULTS[@]}"

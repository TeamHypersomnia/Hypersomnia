#!/bin/bash
# A class joining together: N native clients over WebRTC from one address start at the same moment
# against one community server. All must get in.
# Usage: scenario_webrtc_crowd.sh [N=10]
. "$(dirname "$0")/test_env.sh"
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/webrtc_crowd
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
N=${1:-10}

for port in 18000 18410 18420 18430 10070; do
	if ss -tuln | grep -q ":$port "; then echo "port $port busy, abort"; exit 1; fi
done

rm -rf $T
mk() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }
mk ms <<J
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false } }
J
mk srv <<J
{ "webrtc_signalling_server_url": "ws://127.0.0.1:18000", "server_start": { "ip": "0.0.0.0", "port": 10070, "slots": 32 },
  "server": { "server_name": "WebRTC crowd", "notified_server_list": "127.0.0.1:18430", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19700, "webrtc_port_range_end": 19760 } }
J

cd $REPO/hypersomnia
timeout 300 $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1 &
sleep 3
timeout 290 $BIN --dedicated-server --appdata-dir $T/srv $GAME_TEST_FLAGS > $T/srv.out 2>&1 &

ID=""
for attempt in $(seq 1 30); do
	ID=$(curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin):
    if e['name']=='WebRTC crowd' and e.get('browser_connect_string'): print(e['browser_connect_string'])" 2>/dev/null)
	[ -n "$ID" ] && break
	sleep 2
done
[ -n "$ID" ] || { echo "the server never showed up"; exit 1; }

for i in $(seq 1 $N); do
	mk c$i <<J
{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:18420", "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "client": { "nickname": "crowd$i" } }
J
	timeout 90 xvfb-run -a $BIN --connect rtc:$ID --appdata-dir $T/c$i $GAME_TEST_FLAGS --mute > $T/c$i.out 2>&1 &
done
wait $(jobs -p | tail -n $N) 2>/dev/null
sleep 2

echo "=== $N clients started together: in game"
grep -l "Received initial state" $T/c*.out | wc -l
grep -hcE "connected no game client in time|Max clients" $T/srv.out
for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done

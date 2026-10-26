#!/bin/bash
# WebRTC soak: native clients join a community server over WebRTC and are killed at random moments
# (before ICE, mid-handshake, right after joining) - more cycles than the server's 64 WebRTC ids.
# Then a full join must still work and the server must not hold zombie clients.
# Usage: soak_webrtc.sh [cycles]   (default 150, ~20 min)
. "$(dirname "$0")/../test_env.sh"
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
CYCLES=${1:-150}
T=$S/soak/webrtc; rm -rf $T; mkdir -p $T
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}

for port in 18000 18410 18420 18430 10030 19300; do
	if ss -tulpn 2>/dev/null | grep -q ":$port\b"; then echo "port $port busy, abort"; exit 1; fi
done

mk() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }
mk ms <<J
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false } }
J
mk srv <<J
{ "webrtc_signalling_server_url": "ws://127.0.0.1:18000", "server_start": { "ip": "0.0.0.0", "port": 10030, "slots": 8 },
  "server": { "server_name": "WebRTC soak", "notified_server_list": "127.0.0.1:18430", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19300, "webrtc_port_range_end": 19310 } }
J
mk client <<J
{ "server_list_provider": "http://127.0.0.1:18420/server_list", "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "identity_broker_url": "http://127.0.0.1:1", "client": { "nickname": "rtc_soak" } }
J

cd $REPO/hypersomnia
timeout 3600 $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1 & MS=$!
sleep 3
timeout 3600 $BIN --dedicated-server --appdata-dir $T/srv $GAME_TEST_FLAGS > $T/srv.out 2>&1 & SRV=$!

# A native community server's WebRTC id is its address - the list's browser_connect_string
ID=""
for attempt in $(seq 1 30); do
	ID=$(curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin):
    if e['name']=='WebRTC soak' and e.get('browser_connect_string'): print(e['browser_connect_string'])" 2>/dev/null)
	[ -n "$ID" ] && break
	sleep 2
done
if [ -z "$ID" ]; then echo "the server never showed up on the list"; kill $SRV $MS; exit 1; fi
echo "server WebRTC id: $ID"

rss() { ps -o rss= -p $(pgrep -f "^$BIN --dedicated-server --appdata-dir $T/srv" | head -1) | tr -d ' '; }
echo "RSS before: $(rss) kB"
Xvfb :94 -screen 0 640x480x24 > /dev/null 2>&1 & XV=$!
sleep 1

for i in $(seq 1 $CYCLES); do
	DISPLAY=:94 $BIN --connect rtc:$ID --appdata-dir $T/client $GAME_TEST_FLAGS --mute > $T/c.out 2>&1 &
	C=$!
	sleep $(python3 -c "import random; print(round(random.uniform(0.5, 8.0), 2))")
	kill -9 $C 2>/dev/null
	wait $C 2>/dev/null
	if (( i % 25 == 0 )); then echo "  $i killed clients, server RSS $(rss) kB, peers mapped so far: $(grep -c 'New peer' $T/srv.out)"; fi
done

sleep 70
echo "=== after $CYCLES killed clients: a full join over WebRTC"
DISPLAY=:94 timeout 60 $BIN --connect rtc:$ID --appdata-dir $T/client $GAME_TEST_FLAGS --mute > $T/final.out 2>&1
grep -hE "Received initial state" $T/final.out | head -1
echo "RSS after: $(rss) kB"
echo "=== server: clients still connected at the end (should be none but the final one, which was killed by timeout)"
grep -hiE "Max clients|Invalid offer|is full" $T/srv.out | sort | uniq -c | head
echo "closed-before-set kicks: $(grep -c 'channel closed before the client was set' $T/srv.out)"
kill $SRV $MS $XV 2>/dev/null

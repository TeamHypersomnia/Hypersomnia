#!/bin/bash
# The per-address handshake limit (server_setup::allow_handshake, 20/s, bursts of 40): 4 soak clients at once from this
# machine's LAN address (loopback is not limited) must hit it - some cycles fail, the server logs refusals - and a moment
# later the same address gets in again. Usage: scenario_handshake_limit.sh (build_soak.sh first)
. "$(dirname "$0")/../test_env.sh"
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/handshake_limit; rm -rf $T; mkdir -p $T/srv/user/conf.d
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
SOAK=$S/soak/soak
LAN=$(ip -4 -o addr show scope global | awk '{print $4}' | cut -d/ -f1 | head -1)
[ -n "$LAN" ] || { echo "no LAN address"; exit 1; }
if ss -tuln | grep -q ":10080 "; then echo "port 10080 busy, abort"; exit 1; fi
cat > $T/srv/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": 10080, "slots": 8 },
  "server": { "server_name": "handshake limit", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19800, "webrtc_port_range_end": 19805 } }
J
cd $REPO/hypersomnia
timeout 120 $BIN --dedicated-server --appdata-dir $T/srv $GAME_TEST_FLAGS > $T/srv.out 2>&1 &
sleep 6
echo "=== 4 x 60 cycles at once from $LAN"
for i in 1 2 3 4; do $SOAK $LAN:10080 60 0 > $T/flood$i.out 2>&1 & done
wait $(jobs -p | tail -n 4)
cat $T/flood*.out | grep done
echo "refusals logged: $(grep -c 'a handshake was refused' $T/srv.out)"
sleep 3
echo "=== the same address a moment later"
$SOAK $LAN:10080 10 0
echo "=== loopback is not limited"
$SOAK 127.0.0.1:10080 200 0
for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done

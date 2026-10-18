#!/bin/bash
# Thousands of connect/disconnect cycles, half of them abandoned mid-handshake, against two live dedicated servers:
# one joined by address (any key) and one joined with its key, like from the server list. The servers must never refuse anyone,
# then a real game client must still get in. Catches anything that fills up after N connections.
# Usage: soak_netcode.sh [scale]   (scale 1 = ~9000 cycles, ~35 min on a Debug build; build_soak.sh first)
export ALSOFT_DRIVERS=null VISUAL=true
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
SCALE=${1:-1}
T=$S/soak/netcode; rm -rf $T; mkdir -p $T
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
SOAK=$S/soak/soak

for port in 10020 10021 19020 19021; do
	if ss -tulpn 2>/dev/null | grep -q ":$port\b"; then echo "port $port busy, abort"; exit 1; fi
done

mk() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }
for n in byaddress keyed; do
port=$([ $n = byaddress ] && echo 10020 || echo 10021)
mk $n <<J
{ "server_start": { "ip": "0.0.0.0", "port": $port, "slots": 8 },
  "server": { "server_name": "Soak $n", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": $((port+9000)), "webrtc_port_range_end": $((port+9005)) } }
J
done

cd $REPO/hypersomnia
KEY=$($BIN --print-server-key --appdata-dir $T/keyed 2>/dev/null | grep -E '^[0-9a-f]{64}$')
[ -n "$KEY" ] || { echo "no server key printed"; exit 1; }
timeout 6000 $BIN --dedicated-server --appdata-dir $T/byaddress --no-update-on-launch > $T/byaddress.out 2>&1 & C=$!
timeout 6000 $BIN --dedicated-server --appdata-dir $T/keyed --no-update-on-launch > $T/keyed.out 2>&1 & O=$!
sleep 10
rss() { ps -o rss= -p $(pgrep -f "^$BIN --dedicated-server --appdata-dir $T/$1" | head -1) | tr -d ' '; }
echo "RSS before: byaddress $(rss byaddress) kB, keyed $(rss keyed) kB"
echo "=== by address, every 2nd abandoned"; $SOAK 127.0.0.1:10020 $((4000*SCALE)) 2
echo "=== by address to the keyed one, every 2nd abandoned"; $SOAK 127.0.0.1:10021 $((3000*SCALE)) 2
echo "=== with the key, every 2nd abandoned"; $SOAK 127.0.0.1:10021 $((1500*SCALE)) 2 $KEY
echo "=== a wrong key must not connect"; $SOAK 127.0.0.1:10020 1 0 $KEY
echo "=== both again"; $SOAK 127.0.0.1:10020 200 0; $SOAK 127.0.0.1:10021 200 0 $KEY
sleep 35
echo "=== after the abandoned handshakes expired"; $SOAK 127.0.0.1:10020 100 0; $SOAK 127.0.0.1:10021 100 0 $KEY
echo "RSS after: byaddress $(rss byaddress) kB, keyed $(rss keyed) kB"
echo "=== a real game client joins the by-address server"
mk client <<J
{ "identity_broker_url": "http://127.0.0.1:1", "client": { "nickname": "after_soak" } }
J
timeout 40 xvfb-run -n 93 $BIN --connect 127.0.0.1:10020 --appdata-dir $T/client --no-update-on-launch --mute > $T/client.out 2>&1
grep -hE "Received initial state" $T/client.out | head -1
echo "=== full refusals logged by the servers"
grep -hc "is full" $T/byaddress.out $T/keyed.out
kill $C $O 2>/dev/null

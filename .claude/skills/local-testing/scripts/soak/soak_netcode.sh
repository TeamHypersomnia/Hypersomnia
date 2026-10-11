#!/bin/bash
# Thousands of connect/disconnect cycles, half of them abandoned mid-handshake, against two live dedicated servers:
# community (insecure only) and official (secure + insecure tokens). The servers must never refuse anyone,
# then a real game client must still get in. Catches anything that fills up after N connections.
# Usage: soak_netcode.sh [scale]   (scale 1 = ~9000 cycles, ~35 min on a Debug build; build_soak.sh first)
export ALSOFT_DRIVERS=null VISUAL=true
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
SCALE=${1:-1}
T=$S/soak/netcode; rm -rf $T; mkdir -p $T
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
SOAK=$S/soak/soak
SECRET=soak-test-secret-$(openssl rand -hex 8)
KEY=$(printf 'hypersomnia-netcode-key-v1' | openssl dgst -sha256 -hmac "$SECRET" -hex | awk '{print $2}')

for port in 10020 10021 19020 19021; do
	if ss -tulpn 2>/dev/null | grep -q ":$port\b"; then echo "port $port busy, abort"; exit 1; fi
done

mk() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }
for n in community official; do
port=$([ $n = community ] && echo 10020 || echo 10021)
secret=$([ $n = official ] && echo "$SECRET" || echo "")
mk $n <<J
{ "server_start": { "ip": "0.0.0.0", "port": $port, "slots": 8 },
  "server": { "server_name": "Soak $n", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": $((port+9000)), "webrtc_port_range_end": $((port+9005)) },
  "server_private": { "official_server_secret": "$secret" } }
J
done

cd $REPO/hypersomnia
timeout 6000 $BIN --dedicated-server --appdata-dir $T/community --no-update-on-launch > $T/community.out 2>&1 & C=$!
timeout 6000 $BIN --dedicated-server --appdata-dir $T/official --no-update-on-launch > $T/official.out 2>&1 & O=$!
sleep 10
rss() { ps -o rss= -p $(pgrep -f "^$BIN --dedicated-server --appdata-dir $T/$1" | head -1) | tr -d ' '; }
echo "RSS before: community $(rss community) kB, official $(rss official) kB"
echo "=== community, insecure, every 2nd abandoned"; $SOAK 127.0.0.1:10020 insecure - $((4000*SCALE)) 2
echo "=== official, insecure, every 2nd abandoned"; $SOAK 127.0.0.1:10021 insecure - $((3000*SCALE)) 2
echo "=== official, secure, every 2nd abandoned"; $SOAK 127.0.0.1:10021 $KEY soak $((1500*SCALE)) 2
echo "=== both again"; $SOAK 127.0.0.1:10020 insecure - 200 0; $SOAK 127.0.0.1:10021 $KEY soak 200 0
sleep 35
echo "=== after the abandoned handshakes expired"; $SOAK 127.0.0.1:10020 insecure - 100 0; $SOAK 127.0.0.1:10021 $KEY soak 100 0
echo "RSS after: community $(rss community) kB, official $(rss official) kB"
echo "=== a real game client joins the community server"
mk client <<J
{ "client": { "nickname": "after_soak" } }
J
timeout 40 xvfb-run -n 93 $BIN --connect 127.0.0.1:10020 --appdata-dir $T/client --no-update-on-launch > $T/client.out 2>&1
grep -hE "Received initial state" $T/client.out | head -1
echo "=== table full refusals logged by the servers"
grep -hc "is full" $T/community.out $T/official.out
kill $C $O 2>/dev/null

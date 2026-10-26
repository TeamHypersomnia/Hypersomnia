#!/bin/bash
# A LAN party whose router blocks broadcast: a favorite on the local network is asked directly too, and shows all its
# host's instances in Local - a tournament's matches in Ongoing Tournament. Simulated by ports outside the broadcast's
# (8412-8415): broadcast never finds these servers, only the favorites can.
#   a tournament of 4 (2 matches: ports P, P+1) and a casual server with 2 instances (Q, Q+1); a client with one
#   favorite of each (P and Q), no server list (unreachable) - opens Browse servers:
#   both asked directly, both found, each with all its ports; the hint under the server list's error; a screenshot.
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. $HERE/test_env.sh
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
S=${SCRATCH:-/tmp/hypersomnia-nettest}/lan_favorite_$(date +%s); mkdir -p $S
P=${P:-19300}; Q=$((P + 20)); DISP=${DISP:-:999}
RESULTS=()
check() { local what=$1; shift; if "$@"; then RESULTS+=("PASS  $what"); else RESULTS+=("FAIL  $what"); fi; echo "${RESULTS[-1]}"; }

for port in $P $((P + 1)) $Q $((Q + 1)); do
	if ss -tulpn 2>/dev/null | grep -q ":$port "; then echo "port $port busy - abort"; exit 1; fi
done

# never the production server list
server_conf() {
	mkdir -p $1/user/conf.d
	echo "{ \"server_start\": { \"port\": $2 }, \"server\": { \"notified_server_list\": \"127.0.0.1:1\", \"suppress_new_community_server_webhook\": true, \"webrtc_port_range_begin\": $(($2 + 300)) } }" > $1/user/conf.d/test.json
}

server_conf $S/tournament $P
server_conf $S/casual $Q

(cd $REPO/hypersomnia && exec timeout 300 $BIN --appdata-dir $S/tournament $GAME_TEST_FLAGS --tournament-teams "alice; bob; carol; dave" > $S/tournament.out 2>&1) &
(cd $REPO/hypersomnia && exec env HYPERSOMNIA_SERVER_INSTANCES=2 timeout 300 $BIN --appdata-dir $S/casual $GAME_TEST_FLAGS --dedicated-server > $S/casual.out 2>&1) &

for i in $(seq 60); do
	[ "$(grep -c 'server listening on' $S/tournament.out $S/casual.out 2>/dev/null | awk -F: '{s+=$2} END {print s}')" -ge 4 ] && break
	sleep 1
done

key_of() { (cd $REPO/hypersomnia && $BIN --print-server-key --appdata-dir $1 $GAME_TEST_FLAGS 2>/dev/null | tail -1); }
T_KEY=$(key_of $S/tournament); C_KEY=$(key_of $S/casual)

mkdir -p $S/eve/user/conf.d
echo '{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:1", "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "eve" } }' > $S/eve/user/conf.d/test.json
cat > $S/eve/user/favorite_servers.json <<EOF
[
  { "key": "$T_KEY", "label": "", "address": "127.0.0.1:$P", "name": "LAN tournament", "manual": true, "official": false },
  { "key": "$C_KEY", "label": "", "address": "127.0.0.1:$Q", "name": "LAN casual", "manual": true, "official": false }
]
EOF

Xvfb $DISP -screen 0 1280x720x24 > /dev/null 2>&1 &
XVFB=$!
sleep 2
(cd $REPO/hypersomnia && exec env DISPLAY=$DISP LP_NUM_THREADS=2 timeout 200 $CLIENT_BIN --appdata-dir $S/eve $GAME_TEST_FLAGS > $S/eve.out 2>&1) &
sleep 40
# the menu (a fresh game starts in the tutorial), then Browse servers
DISPLAY=$DISP python3 $HERE/xinput.py key Escape; sleep 3
DISPLAY=$DISP python3 $HERE/xinput.py click 190 497; sleep 15
DISPLAY=$DISP import -window root $S/browse.png

check "both favorites asked directly" grep -q "Asked 2 favorites on the local network directly" $S/eve.out
check "the tournament found" grep -q "Found a server on the local network at: 127.0.0.1:$P" $S/eve.out
check "... with both its matches" grep -q "127.0.0.1:$P on the local network has 2 ports: $P-$((P + 1))" $S/eve.out
check "the casual host found" grep -q "Found a server on the local network at: 127.0.0.1:$Q" $S/eve.out
check "... with both its instances" grep -q "127.0.0.1:$Q on the local network has 2 ports: $Q-$((Q + 1))" $S/eve.out

for pid in $(pgrep -f -- "--appdata-dir $S/"); do kill $pid 2>/dev/null; done
kill $XVFB 2>/dev/null
echo; echo "=== $S (screenshot: $S/browse.png)"
printf '%s\n' "${RESULTS[@]}"
echo "$(printf '%s\n' "${RESULTS[@]}" | grep -c ^PASS) / ${#RESULTS[@]}"

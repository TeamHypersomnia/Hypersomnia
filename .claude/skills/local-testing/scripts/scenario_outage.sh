#!/bin/bash
# Joining by address while the masterserver and the broker are both gone: a casual and a ranked server heartbeating
# to nowhere, clients whose broker is unreachable (a self-signed cert). Expected: they play casual and ranked - every
# valid cert is taken. A broker's cert with no masterserver: scenario_ranked_corners.sh (alice linked, no master).
. "$(dirname "$0")/test_env.sh"
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. "$HERE/lib.sh"
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/outage_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}

for port in 10040 10042; do
	if ss -tuln | grep -q ":$port "; then echo "port $port is busy - aborting"; exit 1; fi
done

# a fresh folder each run (never rm -rf)
T=$T.$(date +%s%N)
cleanup_on_exit $T
mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

server_conf() {
mkconf $1 <<J
{ "instances": { "ranked": $([ "$3" = NEVER ] && echo 0 || echo 1) }, "server_start": { "ip": "0.0.0.0", "port": $2, "slots": 16 },
  "server": { "server_name": "$1", "map_cycle": [ "de_cyberaqua" ], "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "ranked": { },
              "webrtc_port_range_begin": $(($2 + 9400)), "webrtc_port_range_end": $(($2 + 9405)) },
  "server_private": { "report_ranked_match_url": "http://127.0.0.1:1/report_match" } }
J
}

server_conf casual 10040 NEVER
server_conf ranked 10042 TEAMS_EQUAL

client_conf() {
mkconf $1 <<J
{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:1", "client": { "nickname": "$1" } }
J
}

cd $REPO/hypersomnia
for s in casual ranked; do
	timeout 120 nice -n 10 $BIN --dedicated-server --appdata-dir $T/$s $GAME_TEST_FLAGS > $T/$s.out 2>&1 &
done
for s in casual ranked; do wait_listening $T/$s.out; done

# run <name> <port> - until it plays or is refused (at most 40 s), then gone
run() {
	client_conf $1
	timeout 40 xvfb-run -d nice -n 10 $BIN --connect 127.0.0.1:$2 --appdata-dir $T/$1 $GAME_TEST_FLAGS --mute > $T/$1.out 2>&1 &
	wait_until 40 grep -q -E "Received initial state|Reason|need an account|connect failed" $T/$1.out
	for p in $(pgrep -f -- "^$BIN --connect .*--appdata-dir $T/$1 "); do kill $p; done
	wait_until 10 gone "^$BIN --connect .*--appdata-dir $T/$1 "
	echo "=== $1 -> $2"
	grep -hE "identity cert|Connecting as|Received initial state|Reason|need an account" $T/$1.out | sed 's/^\[[0-9:]*\] //' | head -5
}

run casual 10040 &
C1=$!
run ranked 10042 &
C2=$!
# only the clients - the servers are background jobs too
wait $C1 $C2

for s in casual ranked; do
	echo "=== server $s"
	grep -hE "Authenticated|kicked" $T/$s.out | sed 's/^\[[0-9:]*\] //' | head -8
done

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done

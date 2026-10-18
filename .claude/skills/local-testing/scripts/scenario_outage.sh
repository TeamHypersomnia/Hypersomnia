#!/bin/bash
# Joining by address while the masterserver is gone and the broker is down or up:
# a casual and a ranked server heartbeating to nowhere; a client whose broker is unreachable (self-signed cert)
# and one with the live broker (BROKER_URL, BROKER_PUBLIC_KEY - see broker.sh).
# Expected: the self-signed client plays casual and is told ranked needs an account; the broker client plays both.
export ALSOFT_DRIVERS=null VISUAL=true
umask 077
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/outage_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] || { echo "start broker.sh and export BROKER_URL, BROKER_PUBLIC_KEY"; exit 1; }

for port in 10040 10042; do
	if ss -tuln | grep -q ":$port "; then echo "port $port is busy - aborting"; exit 1; fi
done

rm -rf $T
mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

server_conf() {
mkconf $1 <<J
{ "instances": { "ranked": $([ "$3" = NEVER ] && echo 0 || echo 1) }, "server_start": { "ip": "0.0.0.0", "port": $2, "slots": 16 },
  "server": { "server_name": "$1", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "ranked": { },
              "webrtc_port_range_begin": $(($2 + 9400)), "webrtc_port_range_end": $(($2 + 9405)) },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "http://127.0.0.1:1/report_match" } }
J
}

server_conf casual 10040 NEVER
server_conf ranked 10042 TEAMS_EQUAL

client_conf() {
mkconf $1 <<J
{ "identity_broker_url": "$2", "server_list_provider": "http://127.0.0.1:1", "client": { "nickname": "$1" } }
J
}

cd $REPO/hypersomnia
for s in casual ranked; do
	timeout 120 nice -n 10 $BIN --dedicated-server --appdata-dir $T/$s --no-update-on-launch > $T/$s.out 2>&1 &
done
sleep 8

run() {
	client_conf $1 $2
	timeout 25 xvfb-run -a nice -n 10 $BIN --connect 127.0.0.1:$3 --appdata-dir $T/$1 --no-update-on-launch --mute > $T/$1.out 2>&1
	echo "=== $1 -> $3"
	grep -hE "identity cert|Connecting as|Received initial state|Reason|need an account" $T/$1.out | sed 's/^\[[0-9:]*\] //' | head -5
}

run nobroker_casual http://127.0.0.1:1 10040 &
run nobroker_ranked http://127.0.0.1:1 10042 &
run broker_casual $BROKER_URL 10040 &
run broker_ranked $BROKER_URL 10042 &
wait %3 %4 %5 %6 2>/dev/null
wait

for s in casual ranked; do
	echo "=== server $s"
	grep -hE "Authenticated|Ranked matches need|kicked" $T/$s.out | sed 's/^\[[0-9:]*\] //' | head -8
done

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done

#!/bin/bash
# A tournament as the config generator writes it (README_SERVER.md "Tournaments") - three key clients, 1v1:
# CASE=teams: --tournament-teams "A; B; C" --tournament-arenas ... - A has the bye, B plays C, then A plays the winner.
# CASE=open:  HYPERSOMNIA_TOURNAMENT=3 (the variable, as Docker) - two "Match #N (any pair)" servers; A and B take #1,
#             C waits alone on #2 - all 3 came, so round 1 closes by itself and C advances; then the winner of A-B plays C.
# A match ends by its loser's game being killed: max_rejoins 0, so leaving is an abandon.
# Then the same tournament is started once more - it must say it was played to the end already and start nothing.
# Usage: CASE=teams|open scenario_tournament.sh   (BIN / CLIENT_BIN: the headless server / the client)
. "$(dirname "$0")/test_env.sh"
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
CASE=${CASE:-teams}
# A fresh directory every run - never removed recursively
T=$S/tournament_${CASE}_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
PORT=18500
for p in 18500 18501 19500 19501; do
	if ss -tuln | grep -q ":$p "; then echo "port $p busy, abort"; exit 1; fi
done

mkdir -p $T/server/user/conf.d

# Never the production server list, nor its match reports
cat > $T/server/user/conf.d/test.json <<EOF
{
  "server_start": { "port": $PORT },
  "server": {
    "notified_server_list": "127.0.0.1:18430",
    "suppress_new_community_server_webhook": true,
    "webrtc_port_range_begin": 19500,
    "ranked": { "countdown_time": 3, "max_rejoins": 0 },
    "move_to_spectators_if_afk_for_secs": 999999,
    "kick_if_afk_for_secs": 999999
  },
  "server_private": {
    "report_ranked_match_url": "http://127.0.0.1:1/report_match"
  }
}
EOF

account_id() {
	mkdir -p $T/$1/user/.keys
	echo "hypersomnia-key-v1:$(openssl rand -hex 32)" > $T/$1/user/.keys/account.key
	python3 - $T/$1/user/.keys/account.key <<'PY'
import sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
print('key_' + Ed25519PrivateKey.from_private_bytes(seed).public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex())
PY
	cat > $T/$1/user/conf.d/test.json <<EOF
{
  "identity_broker_url": "http://127.0.0.1:1",
  "server_list_provider": "http://127.0.0.1:1",
  "window": { "fullscreen": false, "size": [1280, 720] },
  "client": { "nickname": "$1" }
}
EOF
}

for c in alice bob carol; do
	mkdir -p $T/$c/user/conf.d
done

ID_A=$(account_id alice)
ID_B=$(account_id bob)
ID_C=$(account_id carol)

start_server() {
	cd $REPO/hypersomnia
	if [ "$CASE" = open ]; then
		HYPERSOMNIA_TOURNAMENT=3 HYPERSOMNIA_TOURNAMENT_ARENAS="de_cyberaqua,de_silo" \
			timeout 400 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS > $T/$1.out 2>&1 &
	else
		timeout 400 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS \
			--tournament-teams "$ID_A; $ID_B; $ID_C" --tournament-arenas "de_cyberaqua,de_silo" > $T/$1.out 2>&1 &
	fi
	SERVER_PID=$!
	cd - > /dev/null
}

# client <name> <port> - its pid in CLIENT_PIDS[name]
declare -A CLIENT_PIDS
client() {
	# The game reads its content relative to the working directory
	(cd $REPO/hypersomnia && exec timeout 300 xvfb-run -a nice -n 10 $CLIENT_BIN --connect 127.0.0.1:$2 --appdata-dir $T/$1 $GAME_TEST_FLAGS > $T/$1.$2.$(date +%s).out 2>&1) &
	CLIENT_PIDS[$1]=$!
}

# The game's process under xvfb-run and timeout - by its appdata dir
kill_client() {
	for pid in $(pgrep -f -- "--appdata-dir $T/$1 "); do
		kill -9 $pid 2>/dev/null
	done
}

wait_log() {
	for i in $(seq 1 ${3:-60}); do
		grep -q "$2" $T/$1.out && return 0
		sleep 1
	done
	echo "TIMEOUT waiting for: $2"
	return 1
}

start_server server
wait_log server "server listening on" 30

if [ "$CASE" = open ]; then
	grep -hE "round 1 open|Match #" $T/server.out | head -3
	client alice $PORT; sleep 4
	client bob $PORT; sleep 4
	client carol $((PORT + 1))
	wait_log server "round 1 closes by itself" 60
	wait_log server "Open match is full" 60
	grep -hE "takes a place|Open match is full|closes by itself|Round 1 closed" $T/server.out
	# The match of alice and bob: bob loses
	wait_log server "Starting ranked" 60
	sleep 12
	kill_client bob
	wait_log server "Tournament stage 1: running" 120
	kill_client alice; kill_client carol
	sleep 3
	client alice $PORT; sleep 2; client carol $PORT
	sleep 30
	kill_client carol
	WINNER=alice
else
	# Round 1: alice has the bye, bob plays carol - carol loses
	grep -hE "byes|Tournament stage 0" $T/server.out | head -3
	client bob $PORT; sleep 2; client carol $PORT
	sleep 30
	kill_client carol
	wait_log server "Tournament stage 1: running" 120
	kill_client bob
	sleep 3
	client alice $PORT; sleep 2; client bob $PORT
	sleep 30
	kill_client alice
	WINNER=bob
fi

wait_log server "Tournament WINNER" 120
echo "=== the server"
grep -hE "Tournament|Open match|round 1|Round 1|WINNER|archived" $T/server.out | sed 's/^\[[0-9:]*\] //' | head -40

for c in alice bob carol; do kill_client $c; done
sleep 2
kill $SERVER_PID 2>/dev/null
sleep 2

echo "=== the same tournament again"
start_server again
wait_log again "played to the end already" 30 && echo "PASS: not started again"
kill $SERVER_PID 2>/dev/null

ID_W=$(eval echo \$ID_$(echo $WINNER | head -c1 | tr a-z A-Z))
if grep -q "Tournament WINNER: \[$ID_W\]" $T/server.out; then echo "PASS: $WINNER won"; else echo "FAIL: expected $WINNER ($ID_W) to win"; fi
echo "logs in $T"

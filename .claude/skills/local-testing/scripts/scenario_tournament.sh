#!/bin/bash
# Tournaments end to end, as the config generator starts them - flags or HYPERSOMNIA_TOURNAMENT* variables, key clients
# (the game under Xvfb), a match ended by its loser's game killed. Each case in its own fresh directory, a PASS/FAIL
# table at the end. CASES="..." runs some only (default: all). BIN / CLIENT_BIN: the headless server / the client.
#
#   teams_bye           --tournament-teams "A; B; C": A's bye, B-C, then A-B; the live page on the way; then the same
#                       flags again: "played to the end already", the page showing the winner
#   open_auto_close     HYPERSOMNIA_TOURNAMENT=3: two "Match #N (any pair)" servers, A+B on #1 ([alice] vs [bob]),
#                       C alone on #2 - all came, round 1 closes by itself, C's walkover, then the final
#   open_leave          TOURNAMENT=4: C leaves #2 before D comes - the place is free, nothing closes (3 of 4 here);
#                       C comes back, #2 fills, both matches play - no walkover
#   teams_2v2           --tournament-teams "alice, bob; carol, dave": 2v2 by nicknames
#   teams_accounts      --tournament-teams "alice=<id>; bob=<id>; carol=<id>" (Listed accounts): mallory, playing as "bob",
#                       is turned away; the real bob gets in under another name in the game; the servers, the page and
#                       the next round named by the listed nicknames; a list with an id missing doesn't start
#   crash_mid_match     A vs B live, the server killed (SIGKILL) and started again with the same flags: resumed,
#                       the match goes on, one result
#   crash_in_freeze     A vs B live, B leaves (the match freezes, max_rejoins 2), the server killed in the freeze;
#                       started again, B never comes back - A wins by B's abandon after the rejoin time
#   crash_between       A; B; C: right as round 2 starts the server is killed - resumed at round 2, round 1 not replayed
#   crash_open          TOURNAMENT=4: #1 decided, #2 live when the server is killed - #1 stays decided, #2's pair
#                       plays again on a fresh open server, 4 teams in the end, no duplicates
#   page_heartbeat      the masterserver's list gives the tournament's page; HYPERSOMNIA_TOURNAMENT_PAGE=off: no page;
#                       =18600: the page on its own port; =abc: the server doesn't start; a taken port: no page in the list;
#                       flags that make no tournament (4 players 5v5, players and teams both, teams of other sizes) don't start
#
# Usage: [CASES="crash_mid_match open_leave"] scenario_tournament.sh
. "$(dirname "$0")/test_env.sh"
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
ALL="teams_bye open_auto_close open_leave teams_2v2 teams_accounts crash_mid_match crash_in_freeze crash_between crash_open page_heartbeat"
CASES=${CASES:-$ALL}
PORT=18500
SUITE=$S/tournament_suite_$(date +%s)
mkdir -p $SUITE
RESULTS=$SUITE/results
touch $RESULTS

for p in 18410 18420 18430 18500 18501 18502 18503 18600 19500 19501 19502 19503; do
	if ss -tuln | grep -q ":$p "; then echo "port $p busy, abort"; exit 1; fi
done

# ---------- the helpers ----------

check() {
	# check <what> <command...> - PASS when the command succeeds
	local what=$1; shift

	if "$@" > /dev/null 2>&1; then
		echo "PASS  $CASE: $what" | tee -a $RESULTS
	else
		echo "FAIL  $CASE: $what" | tee -a $RESULTS
	fi
}

new_case() {
	CASE=$1
	T=$SUITE/$CASE
	mkdir -p $T/server/user/conf.d
	SERVER_RUN=0

	# Never the production server list, nor its match reports
	cat > $T/server/user/conf.d/test.json <<EOF
{
  "server_start": { "port": $PORT },
  "server": {
    "notified_server_list": "127.0.0.1:18430",
    "suppress_new_community_server_webhook": true,
    "webrtc_port_range_begin": 19500,
    "ranked": { "countdown_time": 3, "max_rejoins": ${MAX_REJOINS:-0}, "rejoin_time_limit": ${REJOIN_TIME:-150} },
    "move_to_spectators_if_afk_for_secs": 999999,
    "kick_if_afk_for_secs": 999999
  },
  "server_private": { "report_ranked_match_url": "http://127.0.0.1:1/report_match" }
}
EOF
	echo "=== $CASE ($T)"
}

declare -A ID

# player <name> [the nickname in the game - the name by default]
player() {
	mkdir -p $T/$1/user/conf.d $T/$1/user/.keys
	echo "hypersomnia-key-v1:$(openssl rand -hex 32)" > $T/$1/user/.keys/account.key
	cat > $T/$1/user/conf.d/test.json <<EOF
{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:1", "window": { "fullscreen": false, "size": [480, 270] }, "client": { "nickname": "${2:-$1}" } }
EOF
	ID[$1]=$(python3 - $T/$1/user/.keys/account.key <<'PY'
import sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
print('key_' + Ed25519PrivateKey.from_private_bytes(seed).public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex())
PY
)
}

# start_server [VAR=value ...] -- [flags ...]: each start its own log, server<N>.out; SRV is the latest
start_server() {
	local envs=()

	while [ $# -gt 0 ] && [ "$1" != "--" ]; do envs+=("$1"); shift; done
	[ "$1" = "--" ] && shift

	SERVER_RUN=$((SERVER_RUN + 1))
	SRV=$T/server$SERVER_RUN.out
	(cd $REPO/hypersomnia && exec env "${envs[@]}" timeout 600 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS "$@" > $SRV 2>&1) &
	wait_log "server listening on\|played to the end already\|ERROR:" 30
}

# The server's own process (under the subshell and timeout) - by its appdata dir
crash_server() {
	for pid in $(pgrep -f -- "--appdata-dir $T/server "); do kill -9 $pid 2>/dev/null; done
	sleep 2
}

stop_server() {
	for pid in $(pgrep -f -- "--appdata-dir $T/server "); do kill $pid 2>/dev/null; done
	sleep 3
}

client() {
	# client <name> <port> - a small window and few render threads: four of them at once must not starve each other
	# (with full size, all four time out together - "Connection timed out")
	(cd $REPO/hypersomnia && exec env LP_NUM_THREADS=2 timeout 300 xvfb-run -a -s "-screen 0 640x360x24" nice -n 10 $CLIENT_BIN --connect 127.0.0.1:$2 --appdata-dir $T/$1 $GAME_TEST_FLAGS > $T/$1.$2.$(date +%s%N).out 2>&1) &
}

kill_client() {
	for pid in $(pgrep -f -- "--appdata-dir $T/$1 "); do kill -9 $pid 2>/dev/null; done
}

kill_all_clients() {
	for c in alice bob carol dave mallory; do kill_client $c; done
	sleep 2
}

# wait_log <pattern> [secs] - in the latest server log
wait_log() {
	for i in $(seq 1 ${2:-60}); do
		grep -q "$1" $SRV 2>/dev/null && return 0
		sleep 1
	done

	echo "  (timeout waiting for: $1)"
	return 1
}

# wait_count <pattern> <n> [secs]
wait_count() {
	for i in $(seq 1 ${3:-60}); do
		[ "$(grep -c "$1" $SRV 2>/dev/null)" -ge "$2" ] && return 0
		sleep 1
	done

	echo "  (timeout waiting for $2x: $1)"
	return 1
}

in_log() { grep -q "$1" $SRV; }
not_in_log() { ! grep -q "$1" $SRV; }

winner_is() { grep -q "Tournament WINNER: \[${ID[$1]}\]" $SRV; }

# Every player on the page by their nickname - never a shortened account id, also of a loser who quit
all_named='all("..." not in n for r in t["rounds"] for m in r["matches"] for n in m["team_a"]+m["team_b"])'

# page '<python expression on t, the page's JSON>' - e.g. page 't["finished"]'
page() {
	curl -s --max-time 3 http://127.0.0.1:$PORT/tournament.json > $T/page.json || return 1
	python3 -c "import json,sys; t=json.load(open('$T/page.json')); sys.exit(0 if ($1) else 1)"
}

archived() {
	python3 -c "import json,glob; s=json.load(open(sorted(glob.glob('$T/server/user/tournament.completed.*.state.json'))[-1])); print(len(s['$1']))"
}

finish_case() {
	kill_all_clients
	stop_server
}

# ---------- the cases ----------

case_teams_bye() {
	new_case teams_bye
	player alice; player bob; player carol
	local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}; ${ID[carol]}" --tournament-arenas "de_cyberaqua,de_silo")
	start_server -- "${flags[@]}"
	check "1 bye" in_log "1 byes"
	client bob $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	sleep 3
	check "page: round 1 live, one bye" page 't["rounds"][0]["matches"][0]["status"]=="live" and len(t["rounds"][0]["byes"])==1'
	kill_client carol
	wait_log "Tournament stage 1: running" 120
	kill_client bob; sleep 2
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client alice
	wait_log "Tournament WINNER" 120
	check "bob won" winner_is bob
	check "round 2 on de_silo" in_log "Choosing arena: de_silo"
	sleep 2
	check "page: finished, one winner" page 't["finished"] and len(t["winner"])==1'
	check "page: everyone by nickname" page "$all_named"
	finish_case

	start_server -- "${flags[@]}"
	check "not started again" in_log "played to the end already"
	sleep 2
	check "page after the restart: finished" page 't["finished"] and len(t["winner"])==1'
	finish_case
}

case_open_auto_close() {
	new_case open_auto_close
	player alice; player bob; player carol
	start_server HYPERSOMNIA_TOURNAMENT=3 HYPERSOMNIA_TOURNAMENT_ARENAS="de_cyberaqua,de_silo" --
	check "2 servers" in_log "round 1 open - 2 servers"
	client alice $PORT; sleep 4; client bob $PORT; sleep 4
	client carol $((PORT + 1))
	wait_log "Round 1 closed" 60
	check "named [alice] vs [bob]" in_log "Open match is full: \[alice\] vs \[bob\]"
	check "closed by itself" in_log "closes by itself"
	wait_count "Starting ranked" 1 60
	sleep 3
	check "page: carol's walkover" page 'any(m["status"]=="walkover" and m["team_a"]==["carol"] for m in t["rounds"][0]["matches"])'
	kill_client bob
	wait_log "Tournament stage 1: running" 120
	kill_client alice; kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 2 90
	sleep 3
	check "page: the final, by nicknames" page 't["rounds"][1]["matches"][0]["status"]=="live" and sorted(t["rounds"][1]["matches"][0]["team_a"]+t["rounds"][1]["matches"][0]["team_b"])==["alice","carol"]'
	sleep 2
	kill_client carol
	wait_log "Tournament WINNER" 120
	check "alice won" winner_is alice
	finish_case
}

case_open_leave() {
	new_case open_leave
	player alice; player bob; player carol; player dave
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client alice $PORT; sleep 3; client bob $PORT; sleep 3
	client carol $((PORT + 1))
	wait_count "takes a place" 3 60
	kill_client carol
	wait_log "left before the match" 30
	check "carol's place freed" in_log "left before the match"
	client dave $((PORT + 1))
	wait_count "takes a place" 4 60
	sleep 6
	check "nothing closed with 3 of 4 here" not_in_log "Round 1 closed"
	client carol $((PORT + 1))
	wait_count "takes a place" 5 60
	wait_count "Open match is full" 2 30
	check "#2 filled by dave and carol" in_log "Open match is full: \[dave\] vs \[carol\]\|Open match is full: \[carol\] vs \[dave\]"
	wait_count "Starting ranked" 2 90
	sleep 3
	check "page: 2 live matches" page 'sum(m["status"]=="live" for m in t["rounds"][0]["matches"])==2'
	check "no walkover" not_in_log "Round 1 closed"
	kill_client bob; kill_client dave
	wait_log "Tournament stage 1: running" 120
	kill_client alice; kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 3 90
	sleep 5
	kill_client carol
	wait_log "Tournament WINNER" 120
	check "alice won" winner_is alice
	sleep 2
	check "page: everyone by nickname, the losers too" page "$all_named"
	finish_case
}

case_teams_2v2() {
	new_case teams_2v2
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice, bob; carol, dave" --tournament-team-size 2
	check "by nicknames" in_log "authenticates players by nickname"
	client alice $PORT; sleep 2; client bob $PORT; sleep 2; client carol $PORT; sleep 2; client dave $PORT
	wait_count "Starting ranked" 1 120
	sleep 5
	check "page: [alice, bob] vs [carol, dave]" page 'sorted(t["rounds"][0]["matches"][0]["team_a"])==["alice","bob"] and sorted(t["rounds"][0]["matches"][0]["team_b"])==["carol","dave"]'
	kill_client carol; kill_client dave
	wait_log "Tournament WINNER" 120
	check "alice and bob won" in_log "Tournament WINNER: \[alice, bob\]"
	finish_case
}

case_teams_accounts() {
	new_case teams_accounts
	player alice; player bob bobby_in_game; player carol; player mallory bob
	local flags=(--tournament-teams "alice=0x${ID[alice]#key_}; bob=${ID[bob]}; carol=${ID[carol]}")

	start_server -- --tournament-teams "alice=${ID[alice]}; bob"
	check "a list with an id missing doesn't start" in_log "no account id for bob"
	stop_server

	start_server -- "${flags[@]}"
	check "by account" in_log "authenticates players by account"
	check "round 1 named by the list: [bob] vs [carol]" in_log "Match: \[bob\] vs \[carol\]"
	client mallory $PORT
	wait_log "connected to a match that's not theirs" 60
	check "mallory as bob turned away" in_log "connected to a match that's not theirs"
	kill_client mallory
	client bob $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	sleep 3
	check "page: bob vs carol, as listed" page 'sorted(t["rounds"][0]["matches"][0]["team_a"]+t["rounds"][0]["matches"][0]["team_b"])==["bob","carol"]'
	kill_client carol
	wait_log "Tournament stage 1: running" 120
	check "round 2 named by the list: alice and bob" in_log "Match: \[alice\] vs \[bob\]\|Match: \[bob\] vs \[alice\]"
	kill_client bob; sleep 2
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client alice
	wait_log "Tournament WINNER" 120
	check "bob won" winner_is bob
	sleep 2
	check "page: everyone by the listed nickname" page "$all_named"' and "bobby_in_game" not in str(t)'
	finish_case
}

case_crash_mid_match() {
	new_case crash_mid_match
	player alice; player bob
	local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}")
	start_server -- "${flags[@]}"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 8
	crash_server
	kill_all_clients
	start_server -- "${flags[@]}"
	check "resumed at round 1" in_log "resumed from .* at stage 0"
	check "the match goes on" in_log "resuming stage 0 with 1 ongoing"
	check "not started fresh" not_in_log "hash mismatch"
	client alice $PORT; sleep 2; client bob $PORT
	sleep 20
	kill_client bob
	wait_log "Tournament WINNER" 150
	check "alice won" winner_is alice
	check "one result" [ "$(archived match_history)" = 1 ]
	finish_case
}

case_crash_in_freeze() {
	MAX_REJOINS=2 REJOIN_TIME=20 new_case crash_in_freeze
	player alice; player bob
	local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}")
	start_server -- "${flags[@]}"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 8
	kill_client bob
	sleep 4
	check "bob suspended, not abandoned" not_in_log "ABANDONED"
	crash_server
	kill_client alice
	start_server -- "${flags[@]}"
	check "resumed" in_log "resuming stage 0 with 1 ongoing"
	client alice $PORT
	wait_log "Tournament WINNER" 180
	check "alice won - bob never came back" winner_is alice
	check "one result" [ "$(archived match_history)" = 1 ]
	finish_case
}

case_crash_between() {
	new_case crash_between
	player alice; player bob; player carol
	local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}; ${ID[carol]}")
	start_server -- "${flags[@]}"
	client bob $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	sleep 3
	kill_client carol
	wait_log "Tournament stage 1: running" 120
	crash_server
	kill_all_clients
	start_server -- "${flags[@]}"
	check "resumed at round 2" in_log "resumed from .* at stage 1"
	check "round 1 not replayed" not_in_log "Tournament stage 0"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	kill_client alice
	wait_log "Tournament WINNER" 120
	check "bob won" winner_is bob
	check "two results" [ "$(archived match_history)" = 2 ]
	finish_case
}

case_crash_open() {
	new_case crash_open
	player alice; player bob; player carol; player dave
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client alice $PORT; sleep 3; client bob $PORT; sleep 3
	client carol $((PORT + 1)); sleep 3; client dave $((PORT + 1))
	wait_count "Starting ranked" 2 120
	sleep 3
	kill_client bob
	wait_log "Quitting tournament match #$PORT" 60
	crash_server
	kill_all_clients
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	check "resumed with #1 decided" in_log "resuming stage 0 with 2 ongoing"
	check "#2 open again" in_log "Match #2 (any pair)"
	client carol $((PORT + 1)); sleep 3; client dave $((PORT + 1))
	wait_log "Open match is full" 60
	wait_count "Starting ranked" 1 90
	sleep 3
	kill_client dave
	wait_log "Tournament stage 1: running" 120
	kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client carol
	wait_log "Tournament WINNER" 120
	check "alice won" winner_is alice
	check "4 teams, no duplicates" [ "$(archived teams)" = 4 ]
	check "3 results" [ "$(archived match_history)" = 3 ]
	finish_case
}

case_page_heartbeat() {
	new_case page_heartbeat
	mkdir -p $T/ms/user/conf.d $T/official_servers.d
	cat > $T/ms/user/conf.d/test.json <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF
	(cd $REPO/hypersomnia && exec timeout 120 $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1) &
	sleep 3
	start_server HYPERSOMNIA_TOURNAMENT=2 --
	sleep 15
	curl -s http://127.0.0.1:18420/server_list_json > $T/list.json
	check "the list gives the page" python3 -c "import json,sys; sys.exit(0 if any(e.get('tournament_page')=='http://127.0.0.1:$PORT/' for e in json.load(open('$T/list.json'))) else 1)"
	check "the page answers" curl -sf --max-time 3 http://127.0.0.1:$PORT/
	stop_server

	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=off --
	sleep 15
	curl -s http://127.0.0.1:18420/server_list_json > $T/list.json
	check "off: no page in the list" python3 -c "import json,sys; sys.exit(0 if all(not e.get('tournament_page') for e in json.load(open('$T/list.json'))) else 1)"
	check "off: no page served" bash -c "! curl -sf --max-time 3 http://127.0.0.1:$PORT/"
	stop_server

	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=18600 --
	sleep 15
	curl -s http://127.0.0.1:18420/server_list_json > $T/list.json
	check "its own port: in the list" python3 -c "import json,sys; sys.exit(0 if any(e.get('tournament_page')=='http://127.0.0.1:18600/' for e in json.load(open('$T/list.json'))) else 1)"
	check "its own port: served" curl -sf --max-time 3 http://127.0.0.1:18600/tournament.json
	stop_server

	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=abc --
	sleep 3
	check "a wrong port stops the start" in_log "expected off or a port"
	stop_server

	# the page's port taken by something else: the tournament goes on, the list gives no page
	python3 -m http.server 18601 --bind 0.0.0.0 > /dev/null 2>&1 &
	local blocker=$!
	sleep 1
	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=18601 --
	sleep 15
	curl -s http://127.0.0.1:18420/server_list_json > $T/list.json
	check "a taken port: not served, said so" in_log "could not take TCP port 18601"
	check "a taken port: no page in the list" python3 -c "import json,sys; l=json.load(open('$T/list.json')); sys.exit(0 if l and all(not e.get('tournament_page') for e in l) else 1)"
	stop_server
	kill $blocker 2>/dev/null

	# flags that make no tournament don't start
	start_server HYPERSOMNIA_TOURNAMENT=4 HYPERSOMNIA_TOURNAMENT_TEAM_SIZE=5 --
	check "4 players for 5v5 don't start" in_log "can't make two teams"
	stop_server
	start_server HYPERSOMNIA_TOURNAMENT=4 -- --tournament-teams "alice; bob"
	check "players and teams both don't start" in_log "are given both"
	stop_server
	start_server -- --tournament-teams "alice, bob; carol"
	check "teams of other sizes don't start" in_log "a team of 1 players, expected 2"
	stop_server
	for pid in $(pgrep -f -- "--appdata-dir $T/ms "); do kill $pid 2>/dev/null; done
}

for c in $CASES; do
	case_$c
	sleep 3
done

echo
echo "=== results ($SUITE)"
cat $RESULTS
echo "$(grep -c '^PASS' $RESULTS) passed, $(grep -c '^FAIL' $RESULTS) failed"

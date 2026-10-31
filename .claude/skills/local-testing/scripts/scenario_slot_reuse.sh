#!/bin/bash
# Servers must never quietly stop taking players: real games joining a server of 1 slot one after another, each leaving
# another way - the next one must always get in, and the slot must be free again within its time.
#
#   leave       one slot, over UDP: a new player each time, gone by SIGTERM (quits - the slot free at once), SIGKILL (the
#               server's own 3 s timeout) and SIGSTOP (the network gone without a word - the same timeout). Then someone
#               joining the full server is refused ("The server is full") - the slot really is one
#   same_key    one slot, alice in: her game killed and started again at once, 5 times - always back in (her old
#               connection replaced, "connected again from elsewhere"); then she quits and bob, new, gets in - nothing eaten
#   webrtc      one slot, UDP and WebRTC (rtc:<id> through the local signalling server) in turns, gone by SIGTERM and SIGKILL
#   master_restart  the masterserver killed and started again under a running server: the server back on its list
#               within a minute - then a WebRTC player (through the restarted signalling) and a UDP one get in
#   ranked      a ranked server (1v1, de_cyberaqua with 2 rounds of 5 s): alice and bob play a whole match to its end and
#               quit; once the map starts again (~15 s of the match's summary) carol, new, must get in - a finished match
#               never leaves the server refusing
#
# A PASS/FAIL table at the end (and the exit code). CASES="leave webrtc" runs some only.
# BIN: the game (default build/Debug-x64-clang-claude). Ports 17000-17260 (own, apart from the other scenarios').
# Usage: [CASES="..."] scenario_slot_reuse.sh
# bash reads a script as it runs it - an edit meanwhile would break a run: it runs a copy of itself
if [ -z "$SLOT_REUSE_HERE" ]; then
	copy=$(mktemp /tmp/scenario_slot_reuse.XXXXXX.sh)
	cp "$0" $copy
	SLOT_REUSE_HERE=$(cd "$(dirname "$0")" && pwd) exec bash $copy "$@"
fi

HERE=$SLOT_REUSE_HERE
. "$HERE/test_env.sh"
. "$HERE/lib.sh"
umask 077
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
CASES=${CASES:-leave same_key webrtc master_restart ranked}
P_SIGNAL=17000
P_LIST_FALLBACK=17010
P_LIST=17020
P_MASTER=17030
PORT=17100
P_RANKED=17110
P_RTC=17200

# A fresh directory every run - never removed recursively
SUITE=$S/slot_reuse_$(date +%s)
mkdir -p $SUITE
RESULTS=$SUITE/results
touch $RESULTS

for p in $P_SIGNAL $P_LIST_FALLBACK $P_LIST $P_MASTER $PORT $P_RANKED $P_RTC $((P_RTC + 30)); do
	if ss -tuln | grep -q ":$p "; then echo "port $p busy, abort"; exit 1; fi
done

# Ctrl+C, a kill or the end: nothing of this run left running
cleanup_on_exit $SUITE

check() {
	local what=$1; shift

	if "$@" > /dev/null 2>&1; then
		echo "PASS  $CASE: $what" | tee -a $RESULTS
	else
		echo "FAIL  $CASE: $what" | tee -a $RESULTS
	fi
}

conf() { mkdir -p $1/user/conf.d; cat > $1/user/conf.d/test.json; }

new_case() {
	CASE=$1
	T=$SUITE/$CASE
	mkdir -p $T
	echo "=== $CASE ($T)"
}

# ---------- the master and the servers ----------

start_master() {
	conf $T/ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": $P_SIGNAL, "first_udp_command_port": $P_MASTER, "num_udp_command_ports": 1,
  "server_list_port": $P_LIST, "fallback_http_server_list_port": $P_LIST_FALLBACK, "report_rtc_errors_to_webhook": false } }
EOF
	(cd $REPO/hypersomnia && exec timeout 900 $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1) &
	wait_until 20 curl -sf --max-time 1 http://127.0.0.1:$P_LIST/server_list_json || echo "  (the masterserver's list doesn't answer)"
}

# start_server - a casual server of one slot, no bots, told to the local master only
start_server() {
	conf $T/server <<EOF
{ "webrtc_signalling_server_url": "ws://127.0.0.1:$P_SIGNAL",
  "server_start": { "ip": "0.0.0.0", "port": $PORT, "slots": 1 },
  "server": { "server_name": "Slot reuse", "notified_server_list": "127.0.0.1:$P_MASTER", "suppress_new_community_server_webhook": true,
              "bot_quota": [0], "map_cycle": [ "fy_minilab" ], "webrtc_port_range_begin": $P_RTC, "webrtc_port_range_end": $((P_RTC + 30)),
              "move_to_spectators_if_afk_for_secs": 999999, "kick_if_afk_for_secs": 999999 },
  "server_private": { "report_ranked_match_url": "http://127.0.0.1:1/report_match" } }
EOF
	SRV=$T/server.out
	(cd $REPO/hypersomnia && exec timeout 900 $BIN --dedicated-server --appdata-dir $T/server $GAME_TEST_FLAGS > $SRV 2>&1) &
	wait_until 30 grep -q "server listening on" $SRV || echo "  (the server never said it listens)"
}

kill_master() { kill_by -TERM "--appdata-dir $T/ms "; }

stop_all() {
	for pid in $(pgrep -f -- "^$BIN --connect .*$T/"); do kill -CONT $pid 2>/dev/null; kill -9 $pid 2>/dev/null; done
	wait_until 10 gone "^$BIN --connect .*$T/"

	for pid in $(pgrep -f -- "--appdata-dir $T/"); do kill $pid 2>/dev/null; done
	wait_until 15 gone "--appdata-dir $T/"
}

# ---------- the players ----------

DISP=600

# client <name> <connect string> [big] - its own Xvfb display, few threads: a small window, or big - fullscreen 1280x720 (the
# faction window's Auto-assign at 585,430). LOG_OF[name]: its log
declare -A LOG_OF

client() {
	DISP=$((DISP + 1))
	while [ -e /tmp/.X$DISP-lock ]; do DISP=$((DISP + 1)); done

	local window='"window": { "fullscreen": false, "size": [480, 270] }, ' screen=640x360
	[ "$3" = big ] && window="" && screen=1280x720

	[ -f $T/$1/user/conf.d/test.json ] || conf $T/$1 <<EOF
{ "identity_broker_url": "http://127.0.0.1:1", "server_list_provider": "http://127.0.0.1:$P_LIST/server_list",
  "webrtc_signalling_server_url": "ws://127.0.0.1:$P_SIGNAL", $window"client": { "nickname": "$1" } }
EOF
	LOG_OF[$1]=$T/$1.$(date +%s%N).out
	(cd $REPO/hypersomnia && exec env LP_NUM_THREADS=2 timeout 300 xvfb-run -n $DISP -s "-screen 0 ${screen}x24" nice -n 10 \
		$BIN --connect "$2" --appdata-dir $T/$1 $GAME_TEST_FLAGS > ${LOG_OF[$1]} 2>&1) &
	DISPLAY_OF[$1]=$DISP
}

declare -A DISPLAY_OF

game_of() { echo "^$BIN --connect .*--appdata-dir $T/$1 "; }

# entered <name> [secs] - its latest game got the whole state: it plays
entered() { wait_until ${2:-40} grep -q "Received initial state" ${LOG_OF[$1]}; }

refused_full() { wait_until ${2:-30} grep -q "The server is full" ${LOG_OF[$1]}; }

disconnects() { grep -c "Client disconnected. Details" $SRV; }

# leave <name> <term|kill|stop> - and how many seconds until the server freed its slot (999: never within 30 s)
leave() {
	local before=$(disconnects) t0=$(date +%s%N)

	case $2 in
		term) for pid in $(pgrep -f -- "$(game_of $1)"); do kill $pid; done ;;
		kill) for pid in $(pgrep -f -- "$(game_of $1)"); do kill -9 $pid; done ;;
		stop) for pid in $(pgrep -f -- "$(game_of $1)"); do kill -STOP $pid; done ;;
	esac

	if wait_until 30 bash -c "[ \$(grep -c 'Client disconnected. Details' $SRV) -gt $before ]"; then
		FREED_IN=$(( ($(date +%s%N) - t0) / 1000000 ))
	else
		FREED_IN=999000
	fi

	for pid in $(pgrep -f -- "$(game_of $1)"); do kill -CONT $pid 2>/dev/null; kill -9 $pid 2>/dev/null; done
	wait_until 10 gone "$(game_of $1)"
	echo "  $1 left by $2: the slot free after $((FREED_IN / 1000)).$(( (FREED_IN % 1000) / 100 )) s"
}

# freed_within <secs> - the last leave freed the slot that soon
freed_within() { [ $FREED_IN -le $(($1 * 1000)) ]; }

rtc_id() {
	curl -s --max-time 2 http://127.0.0.1:$P_LIST/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin):
    if e['name']=='Slot reuse' and e.get('browser_connect_string'): print(e['browser_connect_string'])" 2>/dev/null
}

has_rtc_id() { [ -n "$(rtc_id)" ]; }

# ---------- the cases ----------

case_leave() {
	new_case leave
	start_master
	start_server
	local n=0 prev=start

	for how in term kill stop term kill stop; do
		n=$((n + 1))
		client p$n 127.0.0.1:$PORT
		check "p$n (#$n, after a $prev) gets in" entered p$n
		leave p$n $how
		# free at once on a quit; when the game vanished: the 3 s timeout, then the kick's own linger (~5.6 s in all)
		local limit=$([ $how = term ] && echo 2 || echo 8)
		check "p$n left by $how: the slot free within $limit s" freed_within $limit
		prev=$how
	done

	client full_a 127.0.0.1:$PORT
	check "full_a gets in" entered full_a
	client full_b 127.0.0.1:$PORT
	check "one slot taken: full_b refused, told the server is full" refused_full full_b
	check "only full_b refused" [ "$(grep -c '(reason=The server is full' $SRV)" = 1 ]
	stop_all
}

case_same_key() {
	new_case same_key
	start_master
	start_server
	client alice 127.0.0.1:$PORT
	check "alice gets in" entered alice

	for i in 1 2 3 4 5; do
		for pid in $(pgrep -f -- "$(game_of alice)"); do kill -9 $pid; done
		wait_until 10 gone "$(game_of alice)"
		client alice 127.0.0.1:$PORT
		check "alice killed and started again at once (#$i): back in" entered alice
	done

	# back before her old connection timed out (3 s): it is replaced, not waited out - 5 restarts, 5 replacements
	check "her old connection replaced each time" [ $(grep -c 'connected again from elsewhere' $SRV) -ge 5 ]
	check "nobody told the server is full" bash -c "! grep -q 'The server is full' $SRV"
	leave alice term
	client bob 127.0.0.1:$PORT
	check "alice quits, bob (new) gets in - the replaced connections took no slot for good" entered bob
	stop_all
}

case_webrtc() {
	new_case webrtc
	start_master
	start_server
	wait_until 30 has_rtc_id
	local id=$(rtc_id)
	check "the server's WebRTC id on the list" [ -n "$id" ]

	# rtc - udp - rtc - udp ..., each gone by quitting or killed
	local n=0

	for step in rtc:term udp:kill rtc:kill udp:term rtc:term rtc:kill udp:term; do
		n=$((n + 1))
		local how=${step#*:} connect=127.0.0.1:$PORT
		[ ${step%%:*} = rtc ] && connect=rtc:$id
		client w$n $connect
		check "w$n over ${step%%:*} (#$n) gets in" entered w$n 60
		leave w$n $how
		# killed over WebRTC: the 1.5 s web timeout only pauses it (a tab may come back), its DataChannel found dead
		# ~8 s later frees the slot (~13 s in all); over UDP as in leave
		local limit=$([ $how = term ] && echo 2 || { [ ${step%%:*} = rtc ] && echo 20 || echo 8; })
		check "w$n over ${step%%:*} left by $how: the slot free within $limit s" freed_within $limit
	done

	stop_all
}

case_ranked() {
	new_case ranked
	# a 1v1 of 2 rounds of 5 s - idle players end it 1:1 in about a minute
	short_ranked_map $T/server/user/downloads/maps
	conf $T/server <<EOF
{ "instances": { "ranked": 1 }, "server_start": { "ip": "0.0.0.0", "port": $P_RANKED, "slots": 2 },
  "server": { "server_name": "Slot reuse ranked", "map_cycle": [ "de_e2e_short" ], "game_mode": "bomb_defusal", "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "sync_all_external_arenas_on_startup": false, "ranked": { "countdown_time": 2 },
              "max_ranked_players_per_ip": 3,
              "move_to_spectators_if_afk_for_secs": 999999, "kick_if_afk_for_secs": 999999,
              "webrtc_port_range_begin": $((P_RTC + 40)), "webrtc_port_range_end": $((P_RTC + 45)) },
  "server_private": { "report_ranked_match_url": "http://127.0.0.1:1/report_match" } }
EOF
	SRV=$T/server.out
	(cd $REPO/hypersomnia && exec timeout 600 $BIN --dedicated-server --appdata-dir $T/server $GAME_TEST_FLAGS > $SRV 2>&1) &
	wait_until 30 grep -q "server listening on" $SRV

	# 1280x720: Auto-assign in the faction window at 585,430, clicked until both play
	client alice 127.0.0.1:$P_RANKED big
	client bob 127.0.0.1:$P_RANKED big

	for i in $(seq 1 24); do
		grep -q "Starting ranked\." $SRV && break
		sleep 5
		for c in alice bob; do DISPLAY=:${DISPLAY_OF[$c]} python3 "$HERE/xinput.py" click 585 430 > /dev/null 2>&1; done
	done

	check "the ranked match starts" grep -q "Starting ranked\." $SRV
	check "... and ends by itself" wait_until 180 grep -q "Match report JSON" $SRV
	# the report goes nowhere (no site) - the server must take players all the same once its players go. The ~15 s after
	# the match (its summary) still answer "A match is being played there"; then the map starts again
	leave alice term
	leave bob term
	check "the map starts again after the match" wait_until 60 bash -c "sed -n '/Match report JSON/,\$p' $SRV | grep -q 'Reinferring all entities at step: 1\$'"
	echo "  (after the match: $(grep 'Match report JSON' $SRV | cut -c2-9) the report, $(sed -n '/Match report JSON/,$p' $SRV | grep 'Reinferring all entities at step: 1$' | head -1 | cut -c2-9) the map again)"
	client carol 127.0.0.1:$P_RANKED
	check "carol (new) gets in after the match" entered carol 60
	check "not told a match is being played" bash -c "! grep -q 'A match is being played\|The server is full' ${LOG_OF[carol]}"
	stop_all
}

case_master_restart() {
	new_case master_restart
	start_master
	start_server
	check "the server on the list" wait_until 30 has_rtc_id
	kill_master
	# else the new one can't bind and the old one answers for it - no restart tested
	check "the old master gone" gone "--appdata-dir $T/ms "
	start_master
	# the server opens its WebSocket to the signalling server again (backoff up to 30 s - not in its log) and heartbeats
	# to the new master: back on the list, and joined over WebRTC through it
	check "the server back on the restarted master's list within 60 s" wait_until 60 has_rtc_id
	client r1 rtc:$(rtc_id)
	check "r1 over WebRTC gets in through the restarted master" entered r1 60
	leave r1 term
	client r2 127.0.0.1:$PORT
	check "then r2 over UDP" entered r2
	stop_all
}

for c in $CASES; do
	case_$c
done

echo
echo "=== results ($SUITE)"
cat $RESULTS
echo "$(grep -c '^PASS' $RESULTS) passed, $(grep -c '^FAIL' $RESULTS) failed"
[ "$(grep -c '^FAIL' $RESULTS)" = 0 ]

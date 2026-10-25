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
#   teams_2v2           --tournament-teams "alice, bob; carol, dave": 2v2 by nicknames - each claimed by its account
#   nick_claim          "alice; bob": alice comes - her account takes the nickname; mallory as alice later: taken;
#                       alice back under another nickname - in by her key, still alice on the page; she wins by her key
#   open_nick_taken     TOURNAMENT=4: alice as "Player" on #1 - bob as "Player" on #2 is told it's taken, no place
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
#   join_timeout_listed "alice; bob; carol; dave", a 1 minute join timeout: only alice comes - she wins hers by walkover,
#                       nobody comes to carol-dave - both out; alice the winner without a match
#   join_timeout_equal  2v2, only alice and carol come: equal numbers - the match starts with them after the timeout
#   join_timeout_open   TOURNAMENT=4, alice alone on #1, bob alone on #2: the timeout closes round 1, they play the final
#   open_one_place      TOURNAMENT=4: alice has a place on #1 - the same account on #2 is sent back to #1
#   open_admin_elsewhere carol, an admin with a place on #1, joins #2 too: there she watches - not sent back, no second place
#   admin_live          alice vs bob live, carol (an admin) joins: a spectator - not "too late", not "not theirs"
#   spectator_live      alice vs bob live, dave (HYPERSOMNIA_SPECTATOR) joins, leaves and comes back: a spectator each time,
#                       the match never freezes; mallory (nobody's) is refused while it's played
#   watch_others        "alice; bob; carol; dave": carol-dave live, then alice beats bob - alice and bob (decided) watch
#                       carol-dave while it's played
#   watch_limit         "alice; bob; carol; dave", --tournament-spectators 1: carol-dave's server has 3 slots; alice
#                       (decided) watches it, bob is refused - no more places; carol and dave play on
#   quick_rejoin        alice vs bob by accounts, live: bob's game killed and started again at once - back in his
#                       match before the server noticed the old connection gone, never "joined too late"
#   walkover_b          "alice; bob; carol; dave", 1 minute: only bob of the first match comes, carol-dave play - the page
#                       has bob's walkover (team b) while the round goes on
#   open_after_played   TOURNAMENT=4: alice beats bob on #1 - both only watch #2, neither takes a place there
#   avatars             alice joins with an avatar, bob without - listed nicknames: by her account (claimed), bob 404,
#                       ?nickname= isn't served; listed accounts: ?account=<her key_ id> and 0x... are hers
#   page_json           --tournament-name, a results webhook (a local listener): the page's JSON (name, config, round states),
#                       429 past 10 requests at once, one POST of the FINISHED results
#   watch_limit_admins  "alice; bob", --tournament-spectators 1 (3 slots): carol (an admin) and dave (the spectator
#                       account) come first - alice and bob still both get in, never "The server is full"
#   claim_twice         "alice; bob; carol; dave": alice's key takes "alice", then the same key comes as "carol" to
#                       carol-dave's server - refused (one account in two teams), "carol" stays unclaimed
#   mixed_teams         a team per line with an empty line among them ("alice, bob" / "" / "carol, dave"): doesn't start
#   admission_allowed   a persistent server with allowed_accounts [alice]: bob is refused, alice and carol (an admin) get in
#   admission_full      a persistent server of 2 slots, full: dave is refused, carol (an admin) gets in
#
# Usage: [CASES="crash_mid_match open_leave"] scenario_tournament.sh
. "$(dirname "$0")/test_env.sh"
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
ALL="watch_limit nick_claim open_nick_taken spectator_live watch_others quick_rejoin walkover_b open_after_played avatars page_json teams_bye open_auto_close open_leave teams_2v2 teams_accounts crash_mid_match crash_in_freeze crash_between crash_open page_heartbeat join_timeout_listed join_timeout_equal join_timeout_open open_one_place open_admin_elsewhere admin_live admission_allowed admission_full watch_limit_admins claim_twice mixed_teams"
CASES=${CASES:-$ALL}
PORT=18500
SUITE=$S/tournament_suite_$(date +%s)
mkdir -p $SUITE
RESULTS=$SUITE/results
touch $RESULTS

for p in 18410 18420 18430 18500 18501 18502 18503 18600 18602 19500 19501 19502 19503; do
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

# Only the game itself (as a crash) - xvfb-run then closes its Xvfb; killed with it, the Xvfb would stay behind
kill_client() {
	for pid in $(pgrep -f -- "^$CLIENT_BIN --connect .*--appdata-dir $T/$1 "); do kill -9 $pid 2>/dev/null; done
}

kill_all_clients() {
	for c in alice bob carol dave mallory alice2 carol2; do kill_client $c; done
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
all_named='all("..." not in p["nickname"] and not p["nickname"].startswith("key_") for r in t["rounds"] for m in r["matches"] for p in m["team_a"]+m["team_b"])'

# page '<python expression on t, the page's JSON>' - e.g. page 'finished'. R(i): round i+1 (the JSON lists the final first),
# N(team): its nicknames, finished: its state is FINISHED. The page is made at most once a second - a fresh one is waited for.
page() {
	sleep 1.2
	curl -s --max-time 3 http://127.0.0.1:${PAGE_PORT:-$PORT}/tournament.json > $T/page.json || return 1
	python3 -c "
import json,sys
t=json.load(open('$T/page.json'))
R=lambda i: t['rounds'][::-1][i]
N=lambda team: [p['nickname'] for p in team]
finished=t['state']=='FINISHED'
sys.exit(0 if ($1) else 1)"
}

archived() {
	python3 -c "import json,glob; s=json.load(open(sorted(glob.glob('$T/server/user/tournament.completed.*.state.json'))[-1])); print(len(s['$1']))"
}

finish_case() {
	kill_all_clients
	stop_server
}

# server_extra '<json>' - one more config file for the case's server, applied after the test's own
server_extra() {
	echo "$1" > $T/server/user/conf.d/zz_case.json
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
	check "page: round 1 live, one bye" page 'R(0)["matches"][0]["status"]=="live" and len(R(0)["byes"])==1 and R(0)["state"]=="PLAYING"'
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
	check "page: finished, one winner, unnamed: My Tournament" page 'finished and len(t["winner_team"])==1 and all(r["state"]=="PLAYED" for r in t["rounds"]) and t["name"]=="My Tournament"'
	check "page: 2 rounds - the final on de_silo, alice's bye kept in round 1" page 'len(t["rounds"])==2 and t["rounds"][0]["name"]=="Final" and len(t["rounds"][0]["matches"])==1 and t["rounds"][0]["arena"]=="de_silo" and R(0)["name"]=="Semifinals" and R(0)["arena"]=="de_cyberaqua" and [N(b) for b in R(0)["byes"]]==[["alice"]]'
	check "page: everyone by nickname" page "$all_named"
	finish_case

	start_server -- "${flags[@]}"
	check "not started again" in_log "played to the end already"
	sleep 2
	check "page after the restart: finished, 2 rounds" page 'finished and len(t["winner_team"])==1 and len(t["rounds"])==2'
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
	check "page: carol's walkover" page 'any(m["status"]=="walkover" and N(m["team_a"])==["carol"] for m in R(0)["matches"])'
	kill_client bob
	wait_log "Tournament stage 1: running" 120
	kill_client alice; kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 2 90
	sleep 3
	check "page: the final, by nicknames" page 'R(1)["matches"][0]["status"]=="live" and sorted(N(R(1)["matches"][0]["team_a"]+R(1)["matches"][0]["team_b"]))==["alice","carol"]'
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
	check "page: 2 live matches" page 'sum(m["status"]=="live" for m in R(0)["matches"])==2'
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
	client alice $PORT; sleep 2; client bob $PORT; sleep 2; client carol $PORT; sleep 2; client dave $PORT
	wait_count "Starting ranked" 1 120
	check "each nickname claimed by its account" bash -c "[ \$(grep -c 'takes the listed nickname' $SRV) = 4 ] && grep -q '${ID[dave]} takes the listed nickname dave' $SRV"
	sleep 5
	check "page: [alice, bob] vs [carol, dave]" page 'sorted(N(R(0)["matches"][0]["team_a"]))==["alice","bob"] and sorted(N(R(0)["matches"][0]["team_b"]))==["carol","dave"] and t["config"]["who_gets_in"]=="listed nicknames" and t["config"]["team_size"]==2'
	kill_client carol; kill_client dave
	wait_log "Tournament WINNER" 120
	check "alice and bob won" in_log "Tournament WINNER: \[${ID[alice]}, ${ID[bob]}\]"
	finish_case
}

case_nick_claim() {
	new_case nick_claim
	player alice; player bob; player mallory alice
	start_server -- --tournament-teams "alice; bob"
	client alice $PORT
	wait_log "takes the listed nickname alice" 60
	check "alice's nickname taken by her account" in_log "${ID[alice]} takes the listed nickname alice"
	kill_client alice; sleep 3
	client mallory $PORT
	wait_log "already taken in this tournament" 60
	check "mallory as alice later: taken" in_log "already taken in this tournament"
	check "mallory took no place" not_in_log "${ID[mallory]} takes the listed nickname"
	kill_client mallory
	sed -i 's/"nickname": "alice"/"nickname": "alice_renamed"/' $T/alice/user/conf.d/test.json
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	check "alice back under another nickname - in by her key" in_log "alice_renamed"
	check "page: alice and bob as listed, by their accounts" page 'sorted((p["nickname"], p["account"]) for p in R(0)["matches"][0]["team_a"]+R(0)["matches"][0]["team_b"])==[("alice","'${ID[alice]}'"),("bob","'${ID[bob]}'")] and "alice_renamed" not in str(t)'
	kill_client bob
	wait_log "Tournament WINNER" 120
	check "alice won - her account" winner_is alice
	finish_case
}

case_open_nick_taken() {
	new_case open_nick_taken
	player alice Player; player bob Player
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client alice $PORT
	wait_count "takes a place" 1 60
	client bob $((PORT + 1))
	wait_log "taken by another player" 60
	check "bob as Player on #2: told it's taken" in_log "already taken in this tournament"
	check "no second place" bash -c "[ \$(grep -c 'takes a place' $SRV) = 1 ]"
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
	check "round 1 named by the list: [bob] vs [carol]" in_log "Match: \[bob\] vs \[carol\]"
	client mallory $PORT
	wait_log "connected to a match that's not theirs" 60
	check "mallory as bob turned away" in_log "connected to a match that's not theirs"
	kill_client mallory
	client bob $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	sleep 3
	check "page: bob vs carol, as listed" page 'sorted(N(R(0)["matches"][0]["team_a"]+R(0)["matches"][0]["team_b"]))==["bob","carol"] and t["config"]["who_gets_in"]=="listed accounts"'
	kill_client carol
	wait_log "Tournament stage 1: running" 120
	check "round 2 named by the list: alice and bob" in_log "Match: \[alice\] vs \[bob\]\|Match: \[bob\] vs \[alice\]"
	kill_client bob; sleep 2
	# bob comes to round 2 under yet another nickname - still let in by his account, still shown as bob
	sed -i 's/"bobby_in_game"/"bob_renamed"/' $T/bob/user/conf.d/test.json
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client alice
	wait_log "Tournament WINNER" 120
	check "bob won" winner_is bob
	sleep 2
	check "renamed between rounds: let in" in_log "bob_renamed"
	check "page: everyone by the listed nickname" page "$all_named"' and "bobby_in_game" not in str(t) and "bob_renamed" not in str(t) and all(p["account"].startswith("key_") for r in t["rounds"] for m in r["matches"] for p in m["team_a"]+m["team_b"])'
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
	wait_count "comes back to the recovered match" 2 90
	sleep 3
	kill_client dave
	wait_log "Tournament stage 1: running" 120
	kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
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

case_join_timeout_listed() {
	new_case join_timeout_listed
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-join-timeout 1

	# nobody here yet: the minute doesn't run - it counts from the round's first player
	sleep 75
	check "nobody yet: nothing timed out after the minute" not_in_log "Join timeout"
	client alice $PORT
	wait_log "Join timeout" 100
	wait_count "Join timeout: " 4 30
	check "alice wins hers by walkover" in_log "the team that came wins by walkover"
	check "nobody came to carol-dave" in_log "nobody came - both teams are out"
	wait_log "Tournament WINNER\|Tournament over" 60
	check "alice the winner" winner_is alice
	sleep 2
	check "page: alice's walkover" page 'any(m["status"]=="walkover" and N(m["team_a"])==["alice"] for m in R(0)["matches"]) and t["config"]["join_timeout_minutes"]==1'
	finish_case
}

case_join_timeout_equal() {
	new_case join_timeout_equal
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice, bob; carol, dave" --tournament-team-size 2 --tournament-join-timeout 1
	client alice $PORT; sleep 2; client carol $PORT
	wait_log "equal numbers - the match starts" 100
	check "equal numbers start the match" in_log "equal numbers - the match starts"
	wait_count "Starting ranked" 1 90
	check "the match started 1v1" in_log "Starting ranked"
	sleep 5
	kill_client carol
	wait_log "Tournament WINNER" 200
	check "alice and bob won - bob never came: still his listed nickname" in_log "Tournament WINNER: \[${ID[alice]}, nick_bob\]"
	finish_case
}

case_join_timeout_open() {
	new_case join_timeout_open
	player alice; player bob
	start_server HYPERSOMNIA_TOURNAMENT=4 HYPERSOMNIA_TOURNAMENT_JOIN_TIMEOUT=1 --
	client alice $PORT; sleep 3; client bob $((PORT + 1))
	wait_count "takes a place" 2 60
	wait_count "Join timeout: round 1's match here closes" 2 100
	check "round 1 closed by the timeout" in_log "Join timeout: round 1's match here closes"
	wait_log "Tournament stage 1: running" 60
	check "the final: alice vs bob" in_log "Tournament stage 1: running"
	kill_client alice; kill_client bob; sleep 2
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	kill_client bob
	wait_log "Tournament WINNER" 200
	check "alice won" winner_is alice
	finish_case
}

case_open_one_place() {
	new_case open_one_place
	player alice
	mkdir -p $T/alice2/user
	cp -r $T/alice/user/conf.d $T/alice/user/.keys $T/alice2/user/
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client alice $PORT
	wait_count "takes a place" 1 60
	client alice2 $((PORT + 1))
	wait_log "has a place in Match #1" 60
	check "the same account sent back to Match #1" in_log "has a place in Match #1"
	check "no second place" bash -c "[ \$(grep -c 'takes a place' $SRV) = 1 ]"
	finish_case
}

case_open_admin_elsewhere() {
	new_case open_admin_elsewhere
	player carol
	mkdir -p $T/carol2/user
	cp -r $T/carol/user/conf.d $T/carol/user/.keys $T/carol2/user/
	server_extra "{ \"server_private\": { \"admin_profiles\": [ { \"account\": \"${ID[carol]}\" } ] } }"
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client carol $PORT
	wait_count "takes a place" 1 60
	client carol2 $((PORT + 1))
	wait_log "watches the tournament's match as a spectator" 60
	check "the admin with a place elsewhere watches" in_log "watches the tournament's match as a spectator"
	check "not sent back, no second place" bash -c "! grep -q 'has a place in' $SRV && [ \$(grep -c 'takes a place' $SRV) = 1 ]"
	finish_case
}

case_admin_live() {
	new_case admin_live
	player alice; player bob; player carol
	server_extra "{ \"server_private\": { \"admin_profiles\": [ { \"account\": \"${ID[carol]}\", \"rcon\": \"MASTER_RCON\" } ] } }"
	start_server -- --tournament-teams "alice; bob"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 8
	client carol $PORT
	wait_log "watches the tournament's match as a spectator" 60
	check "carol watches as a spectator" in_log "watches the tournament's match as a spectator"
	wait_log "Adding carol to game state" 30
	check "carol: never a team problem" not_in_log "pre-assigning your team"
	check "carol in the game, not timed out" bash -c "grep -q 'Adding carol to game state' $SRV && ! grep -q 'carol.*timed out' $SRV"
	sleep 5
	check "not too late, not theirs" not_in_log "carol has connected to a match that's not theirs"
	check "the match goes on" not_in_log "Tournament WINNER"
	kill_client bob
	wait_log "Tournament WINNER" 200
	check "alice won with carol watching" winner_is alice
	finish_case
}

case_spectator_live() {
	new_case spectator_live
	player alice; player bob; player dave; player mallory
	start_server HYPERSOMNIA_SPECTATOR=${ID[dave]} -- --tournament-teams "alice; bob"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 8
	client dave $PORT
	wait_log "watches the tournament's match as a spectator" 60
	check "the spectator account watches" in_log "watches the tournament's match as a spectator"
	client mallory $PORT
	sleep 15
	check "nobody's mallory not let in" not_in_log "Adding mallory to game state"
	kill_client mallory
	kill_client dave
	sleep 5
	client dave $PORT
	wait_count "Adding dave to game state" 2 60
	check "the spectator back again" bash -c "[ \$(grep -c 'Adding dave to game state' $SRV) -ge 2 ]"
	check "never a team problem" not_in_log "pre-assigning your team"
	check "dave never froze the match" bash -c "! grep -i 'dave' $SRV | grep -qi 'suspend\|abandon'"
	check "the match goes on" not_in_log "Tournament WINNER"
	kill_client bob
	wait_log "Tournament WINNER" 200
	check "alice won with dave watching" winner_is alice
	finish_case
}

case_watch_others() {
	new_case watch_others
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave"
	client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 1 90
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client bob
	wait_log "abandoned by faction" 60
	sleep 3
	kill_client alice; sleep 2
	client alice $((PORT + 1)); sleep 2; client bob $((PORT + 1))
	wait_count "watches the tournament's match as a spectator" 2 60
	check "alice (won) and bob (out) watch carol-dave" bash -c "[ \$(grep -c \"watches the tournament's match as a spectator\" $SRV) -ge 2 ]"
	check "not turned away" not_in_log "connected to a match that's not theirs"
	sleep 5
	check "alice and bob in carol-dave's game, no team problem" bash -c "grep -q \"T$((PORT + 1))] Adding alice to game state\" $SRV && grep -q \"T$((PORT + 1))] Adding bob to game state\" $SRV && ! grep -q 'pre-assigning your team' $SRV"
	finish_case
}

case_watch_limit() {
	new_case watch_limit
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-spectators 1
	check "carol-dave's server: 2 players + 1 to watch" in_log "T$((PORT + 1))] Starting yojimbo::Server with [0-9]* connections for 3 slots"
	client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 1 90
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client bob
	wait_log "abandoned by faction" 60
	sleep 3
	kill_client alice; sleep 2
	client alice $((PORT + 1))
	wait_count "watches the tournament's match as a spectator" 1 60
	client bob $((PORT + 1))
	wait_log "No more places to watch this match" 60
	check "alice watches carol-dave" bash -c "grep -q \"T$((PORT + 1))] Adding alice to game state\" $SRV"
	check "bob refused - no more places to watch" in_log "No more places to watch this match"
	check "carol and dave still in their match" bash -c "! grep -E -q \"T$((PORT + 1))] .*(carol|dave) was kicked\" $SRV"
	finish_case
}

case_quick_rejoin() {
	new_case quick_rejoin
	player alice; player bob
	start_server -- --tournament-teams "alice=${ID[alice]}; bob=${ID[bob]}"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 8
	kill_client bob
	client bob $PORT
	wait_count "Adding bob to game state" 2 60
	check "bob back in his match" bash -c "[ \$(grep -c 'Adding bob to game state' $SRV) -ge 2 ]"
	check "never joined too late" not_in_log "Joined too late"
	check "his game never told \"only its players can rejoin\"" bash -c "! grep -q 'only its players can rejoin' $T/bob.*.out"
	finish_case
}

case_walkover_b() {
	new_case walkover_b
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-join-timeout 1
	client bob $PORT; sleep 2
	client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 1 90
	wait_log "the team that came wins by walkover" 100
	check "page: bob's walkover, as team b, while carol-dave play" page 'any(m["status"]=="walkover" and m["winner"]=="b" and N(m["team_b"])==["bob"] for m in R(0)["matches"]) and R(0)["state"]=="PLAYING"'
	finish_case
}

case_open_after_played() {
	new_case open_after_played
	player alice; player bob
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	kill_client bob
	wait_log "abandoned by faction" 60
	sleep 3
	kill_client alice; sleep 2
	client alice $((PORT + 1)); sleep 2; client bob $((PORT + 1))
	wait_count "watches the tournament's match as a spectator" 2 60
	check "alice (won) and bob (out) only watch #2" bash -c "[ \$(grep -c \"watches the tournament's match as a spectator\" $SRV) -ge 2 ]"
	check "neither takes a place there" bash -c "[ \$(grep -c 'takes a place' $SRV) = 2 ]"
	finish_case
}

case_avatars() {
	new_case avatars
	player alice; player bob
	cp $REPO/hypersomnia/content/gfx/necessary/blank_avatar.png $T/alice_avatar.png
	echo "{ \"client\": { \"avatar_image_path\": \"$T/alice_avatar.png\" } }" > $T/alice/user/conf.d/zz_avatar.json
	local size=$(identify -format '%wx%h' $T/alice_avatar.png)

	start_server -- --tournament-teams "alice; bob"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	curl -s -o $T/a.png -w '%{http_code}' "http://127.0.0.1:$PORT/avatar?account=${ID[alice]}" > $T/a.code
	check "listed nicknames: alice's avatar by her account, $size" bash -c "[ \$(cat $T/a.code) = 200 ] && [ \$(identify -format '%wx%h' $T/a.png) = $size ]"
	check "listed nicknames: kept in tournament_avatars" bash -c "[ \$(ls $T/server/user/tournament_avatars/*.png | wc -l) = 1 ]"
	check "listed nicknames: bob has none - 404" bash -c "[ \$(curl -s -o /dev/null -w '%{http_code}' 'http://127.0.0.1:$PORT/avatar?account=${ID[bob]}') = 404 ]"
	check "?nickname= isn't served - 404" bash -c "[ \$(curl -s -o /dev/null -w '%{http_code}' 'http://127.0.0.1:$PORT/avatar?nickname=alice') = 404 ]"
	kill_all_clients
	stop_server

	start_server -- --tournament-teams "alice=${ID[alice]}; bob=${ID[bob]}" --tournament-name "Avatar Cup"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	check "by accounts: alice's avatar by her key_ id" bash -c "[ \$(curl -s -o /dev/null -w '%{http_code}' 'http://127.0.0.1:$PORT/avatar?account=${ID[alice]}') = 200 ]"
	check "by accounts: the same by 0x..." bash -c "[ \$(curl -s -o /dev/null -w '%{http_code}' 'http://127.0.0.1:$PORT/avatar?account=0x${ID[alice]#key_}') = 200 ]"
	check "by accounts: the page names her account" page 'any(p["account"]=="'${ID[alice]}'" for r in t["rounds"] for m in r["matches"] for p in m["team_a"]+m["team_b"])'
	finish_case
}

case_page_json() {
	new_case page_json
	player alice; player bob; player carol
	python3 - $T/webhook.json > $T/webhook.out 2>&1 <<'PY' &
import sys, http.server
class H(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        body = self.rfile.read(int(self.headers['Content-Length']))
        open(sys.argv[1], 'ab').write(body + b'\n')
        open(sys.argv[1] + '.body', 'wb').write(body)
        open(sys.argv[1] + '.key', 'w').write(self.headers.get('X-Hypersomnia-Server-Key', ''))
        open(sys.argv[1] + '.sig', 'w').write(self.headers.get('X-Hypersomnia-Signature', ''))
        self.send_response(200); self.end_headers()
http.server.HTTPServer(('127.0.0.1', 18602), H).serve_forever()
PY
	local listener=$!
	sleep 1
	start_server HYPERSOMNIA_TOURNAMENT_NAME="Test Cup" HYPERSOMNIA_TOURNAMENT_RESULTS_WEBHOOK=http://127.0.0.1:18602/results -- --tournament-teams "alice; bob; carol"
	check "page: name, ONGOING, config" page 't["name"]=="Test Cup" and t["state"]=="ONGOING" and t["winner_team"]==[] and {k: v for k, v in t["config"].items() if k!="arenas"}=={"who_gets_in":"listed nicknames","team_size":1,"teams":3,"join_timeout_minutes":10}'
	check "page: the final first, COMING; round 1 PLAYING" page 't["rounds"][0]["name"]=="Final" and t["rounds"][0]["state"]=="COMING" and R(0)["state"]=="PLAYING" and "score_a" not in R(0)["matches"][0]'
	check "page: a waiting match has its server" page 'R(0)["matches"][0]["status"]=="waiting" and R(0)["matches"][0]["port"]>0'
	check "the page itself served" curl -sf --max-time 3 http://127.0.0.1:$PORT/
	sleep 3
	local codes=$(for i in $(seq 1 25); do curl -s -o /dev/null -w '%{http_code}\n' http://127.0.0.1:$PORT/tournament.json; done | sort | uniq -c | tr '\n' ' ')
	echo "  (25 requests at once: $codes)"
	check "429 past the burst" bash -c "echo '$codes' | grep -q ' 429'"
	check "the first 10 answered" bash -c "echo '$codes' | grep -qE '(1[0-9]|2[0-9]) 200'"
	sleep 3
	check "answered again after a while" curl -sf --max-time 3 http://127.0.0.1:$PORT/tournament.json
	client bob $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	sleep 5
	kill_client carol
	wait_log "Tournament stage 1: running" 120
	kill_client bob; sleep 2
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	sleep 5
	kill_client bob
	wait_log "results were sent to the webhook\|results webhook" 200
	check "the webhook answered" in_log "results were sent to the webhook (200)"
	check "one POST, FINISHED, the winner, name" python3 -c "import json,sys; l=open('$T/webhook.json').read().splitlines(); t=json.loads(l[0]); sys.exit(0 if len(l)==1 and t['state']=='FINISHED' and t['name']=='Test Cup' and [p['nickname'] for p in t['winner_team']]==['alice'] and all(r['state']=='PLAYED' for r in t['rounds']) else 1)"
	check "page: FINISHED as the webhook's" page 'finished and t["name"]=="Test Cup"'
	local key=$(cd $REPO/hypersomnia && $BIN --print-server-key --appdata-dir $T/server $GAME_TEST_FLAGS 2>/dev/null | tail -1)
	check "the webhook signed by the server's key" verify_signed $T/webhook.json.body $T/webhook.json.key $T/webhook.json.sig $key
	curl -s -D $T/endpoint.headers -o $T/endpoint.body http://127.0.0.1:$PORT/tournament.json
	grep -i '^x-hypersomnia-server-key' $T/endpoint.headers | cut -d' ' -f2 | tr -d '\r\n' > $T/endpoint.key
	grep -i '^x-hypersomnia-signature' $T/endpoint.headers | cut -d' ' -f2 | tr -d '\r\n' > $T/endpoint.sig
	check "the endpoint signed by the server's key" verify_signed $T/endpoint.body $T/endpoint.key $T/endpoint.sig $key
	check "a changed body fails the check" bash -c "echo ' ' >> $T/endpoint.body; ! verify_signed $T/endpoint.body $T/endpoint.key $T/endpoint.sig $key"
	check "tournament_results.json: the webhook's very bytes" cmp $T/server/user/tournament_results.json $T/webhook.json.body
	local codes=$(for i in $(seq 1 25); do curl -s -o /dev/null -w '%{http_code}\n' "http://127.0.0.1:$PORT/avatar?account=player$i"; done | sort -u | tr '\n' ' ')
	check "25 avatars at once: none limited ($codes)" bash -c "! echo '$codes' | grep -q 429"
	kill_all_clients
	stop_server

	# started again, over already: the results saved anew (a stale file overwritten) and posted again
	echo stale > $T/server/user/tournament_results.json
	start_server HYPERSOMNIA_TOURNAMENT_NAME="Test Cup" HYPERSOMNIA_TOURNAMENT_RESULTS_WEBHOOK=http://127.0.0.1:18602/results -- --tournament-teams "alice; bob; carol"
	check "the same name again: played already" in_log "played to the end already"
	wait_log "results were sent to the webhook" 30
	check "over already: posted again" bash -c "[ \$(wc -l < $T/webhook.json) = 2 ]"
	check "over already: tournament_results.json saved anew" cmp $T/server/user/tournament_results.json $T/webhook.json.body
	kill $listener 2>/dev/null
	stop_server
	start_server HYPERSOMNIA_TOURNAMENT_NAME="Test Cup 2" -- --tournament-teams "alice; bob; carol"
	check "a new name: a new tournament" bash -c "grep -q 'server listening on' $SRV && ! grep -q 'played to the end already' $SRV"
	finish_case
}

# verify_signed <body file> <key hex file> <signature hex file> <the server's key> - Ed25519 of the body by that key
verify_signed() {
	python3 - "$@" <<'PY'
import sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
body = open(sys.argv[1], 'rb').read()
key = open(sys.argv[2]).read().strip()
sig = open(sys.argv[3]).read().strip()
if key != sys.argv[4]:
    sys.exit(1)
Ed25519PublicKey.from_public_bytes(bytes.fromhex(key)).verify(bytes.fromhex(sig), body)
PY
}
export -f verify_signed

# a local masterserver on 18420 (its list) / 18430 (heartbeats) - the test servers tell it, never the real one
start_masterserver() {
	mkdir -p $T/ms/user/conf.d $T/official_servers.d
	cat > $T/ms/user/conf.d/test.json <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF
	(cd $REPO/hypersomnia && exec timeout 300 $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1) &
	sleep 3
}

# a persistent server for the admission cases - no tournament, no bots
start_persistent() {
	server_extra "$1"
	start_server -- 
}

case_admission_allowed() {
	new_case admission_allowed
	player alice; player bob; player carol
	start_masterserver
	start_persistent "{ \"server\": { \"bot_quota\": [0] }, \"server_private\": { \"allowed_accounts\": [ \"${ID[alice]}\" ], \"admin_profiles\": [ { \"account\": \"${ID[carol]}\" } ] } }"
	client bob $PORT
	wait_log "is not among the listed players" 60
	check "bob refused" in_log "${ID[bob]} is not among the listed players"
	client alice $PORT; sleep 2; client carol $PORT
	sleep 12
	check "alice in" in_log "Adding alice to game state"
	check "carol (an admin) in" in_log "Adding carol to game state"
	curl -s http://127.0.0.1:18420/server_list_json > $T/list.json
	check "a padlock on the list, no password" python3 -c "import json,sys; l=json.load(open('$T/list.json')); sys.exit(0 if l and all(e.get('listed_players_only') and not e.get('require_password') for e in l) else 1)"
	for pid in $(pgrep -f -- "--appdata-dir $T/ms "); do kill $pid 2>/dev/null; done
	finish_case

	# listed accounts where only nicknames are known: the list could never be checked - the server doesn't start
	start_persistent "{ \"server\": { \"bot_quota\": [0], \"authenticate_with_nicknames\": true }, \"server_private\": { \"allowed_accounts\": [ \"${ID[alice]}\" ] } }"
	check "allowed_accounts with nicknames: not started" in_log "allowed_accounts with authenticate_with_nicknames"
	stop_server
}

case_admission_full() {
	new_case admission_full
	player alice; player bob; player carol; player dave
	start_persistent "{ \"server_start\": { \"slots\": 2 }, \"server\": { \"bot_quota\": [0] }, \"server_private\": { \"admin_profiles\": [ { \"account\": \"${ID[carol]}\" } ] } }"
	client alice $PORT; sleep 3; client bob $PORT; sleep 8
	client dave $PORT
	sleep 10
	check "dave refused - full" in_log "The server is full"
	client carol $PORT
	sleep 12
	check "carol (an admin) in on the full server" in_log "Adding carol to game state"
	check "only dave refused" bash -c "[ \$(grep -c '(reason=The server is full' $SRV) = 1 ]"
	finish_case
}

case_watch_limit_admins() {
	new_case watch_limit_admins
	player alice; player bob; player carol; player dave
	server_extra "{ \"server_private\": { \"admin_profiles\": [ { \"account\": \"${ID[carol]}\", \"rcon\": \"MASTER_RCON\" } ] } }"
	start_server HYPERSOMNIA_SPECTATOR=${ID[dave]} -- --tournament-teams "alice; bob" --tournament-spectators 1
	check "the match's server: 2 players + 1 to watch" in_log "Starting yojimbo::Server with [0-9]* connections for 3 slots"
	client carol $PORT; sleep 2; client dave $PORT
	wait_count "Adding dave to game state" 1 60
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	check "alice and bob both in" bash -c "grep -q 'Adding alice to game state' $SRV && grep -q 'Adding bob to game state' $SRV"
	check "nobody told the server is full" not_in_log "The server is full"
	finish_case
}

case_claim_twice() {
	new_case claim_twice
	player alice; player bob; player carol; player dave
	# alice's key under the nickname "carol" - a shared computer
	player alice2 carol
	cp $T/alice/user/.keys/account.key $T/alice2/user/.keys/account.key
	start_server -- --tournament-teams "alice; bob; carol; dave"
	client alice $PORT
	wait_log "takes the listed nickname alice" 60
	client alice2 $((PORT + 1))
	wait_log "already plays in a team" 60
	check "the same key refused as carol" in_log "already plays in a team - not taking the listed nickname carol"
	check "carol not claimed" not_in_log "takes the listed nickname carol"
	finish_case
}

case_mixed_teams() {
	new_case mixed_teams
	start_server -- --tournament-teams "$(printf 'alice, bob\n\ncarol, dave')"
	check "a team per line and an empty line: doesn't start" in_log "not both"
	stop_server
}

for c in $CASES; do
	case_$c
	sleep 3
done

echo
echo "=== results ($SUITE)"
cat $RESULTS
echo "$(grep -c '^PASS' $RESULTS) passed, $(grep -c '^FAIL' $RESULTS) failed"

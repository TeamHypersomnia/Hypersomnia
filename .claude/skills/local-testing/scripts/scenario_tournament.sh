#!/bin/bash
# Tournaments end to end, as the config generator starts them - flags or HYPERSOMNIA_TOURNAMENT* variables, key clients
# (the game under Xvfb), a match ended by its loser's game killed. Each case in its own fresh directory, a PASS/FAIL
# table at the end. CASES="..." runs some only (default: all). BIN / CLIENT_BIN: the headless server / the client.
#
#   budget_nick_case    "alice; Bob; carol; dave", --tournament-spectator-slots 4: 2 on each of the 2 matches; Alice and bob
#                       take their listed nicknames whatever the case; an unlisted nickname is told the match's list
#   finished_servers    "alice; bob; carol; dave", a local master: a decided match's server stays up empty (finished on the
#                       list, a newcomer refused with "Match finished..."), the round waits for the other match; the final
#                       stays finished on the list after the end and after a restart ("The tournament has finished.")
#   teams_bye           --tournament-teams "A; B; C": A's bye, B-C, then A-B; the live page on the way; then the same
#                       flags again: "played to the end already", the page showing the winner
#   open_auto_close     HYPERSOMNIA_TOURNAMENT=3: two "Match #N (any pair)" servers, A+B on #1 ([alice] vs [bob]),
#                       C alone on #2 - all came, round 1 closes by itself, C's walkover, then the final
#   open_choice         TOURNAMENT=2, a 300 s countdown: bob can't join alice's full team; he takes the other one;
#                       /go from both starts the match at once - carol, still spectating, is sent to another match;
#                       bob, killed and back, is in his team again
#   open_same_nick      TOURNAMENT=4: alice and bob both as "Player" spectate #1 and #2 (one server renames a second
#                       Player itself) - alice takes a team, bob choosing one is told it's taken, no second place
#   watch_limit_final   "alice; bob; carol; dave", --tournament-spectator-slots 1: the final's server has 3 slots - the teams out
#                       only watch, they take no players' slots
#   crash_nobody_back_listed / _open  a match killed mid-way and started again, nobody comes back: decided as if
#                       nobody came - the tournament doesn't fail
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
#   custom_live         map_cycle_add_custom_maps, an empty server changing maps every 3 s: a custom map put into
#                       custom/maps while it runs gets played, no restart
#   custom_star         ["*"] with custom maps: the custom one played, the custom de_silo never (the official one)
#   custom_broken       a custom map with a broken JSON in the cycle: skipped ("Failed to load"), the server goes on
#   maps_folder_move    downloads/arenas with a map and an empty downloads/maps: moved at the start, the old one gone
#   tournament_custom_missing --tournament-maps fy_not_uploaded: "Missing map: ..." (bold yellow), then no tournament
#   custom_maps         map_cycle_add_custom_maps on, 2 maps in custom/maps and one named de_silo, no map and no
#                       cycle: de_silo's custom copy ignored, the server starts on fy_custom_a, a player downloads it;
#                       off (the default): the custom map on the server but not in rotation (de_cyberaqua), named in
#                       the cycle - played from custom/maps, nothing downloaded
#   kept_map_not_in_cycle a run on de_silo, then the cycle changed to fy_minilab: the kept de_silo dropped, fy_minilab played
#   tournament_custom_map --tournament-maps fy_cup_final, only in custom/maps: no download wait, the match plays it
#   map_download_retry  a tournament whose map isn't on disk, its provider refusing the catalogue twice and cutting off
#                       the first map file: it asks again until the map is here, then runs
#   map_catalogue_deadline a provider that sends a byte every 2 s: the server gives the catalogue up after 5 s and
#                       starts within 10 s; --download-maps-and-quit keeps waiting (20 s), and with nothing listening
#                       asks again every 3 s
#   map_download_and_quit HYPERSOMNIA_MAP_CYCLE=de_cyberaqua,de_hotline --download-maps-and-quit: only de_hotline, then quits
#                       HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT=1 with a misspelled map: exit 1, the map named
#   map_download_cycle  a server (no tournament) with map_cycle ["de_hotline"], clean appdata: only it downloaded at the
#                       start; started again - up to date, nothing downloaded
#   map_download        --tournament-maps de_hotline with clean appdata, no --sync: only de_hotline downloaded from
#                       hypersomnia.io, the match plays on it
#   bomb_always         an arena whose default mode is quick_test, listed as "fy_mode_test quick_test": played as bomb defusal
#   open_long_names     TOURNAMENT=4 2v2, 25-character nicknames: the full match is named "[a.. +1] vs [c.. +1]", within 60 bytes
#   round1_off          "alice; bob", round 1 max wait off, only alice comes: no deadline on a fresh page, her match waiting
#   five_teams_walkovers 5 listed teams, both max waits 1 minute, only alice ever comes: byes, walkovers and both-out
#                       matches through 3 rounds - alice the winner, nothing stuck
#   crash_renews_deadline "alice; bob", round 1 max wait 1 minute: the server killed 15 s in and started again - the
#                       page's deadline 1 + 2 minutes from the restart (a restart gives the round its whole time again)
#   crash_in_break      round 1 played, the server killed 15 s into the break and started again: the final's deadline
#                       on the page counted again from the restart
#   join_timeout_equal  2v2, only alice and carol come: both teams here - the match starts with them after the timeout
#   join_timeout_open   TOURNAMENT=4, alice alone on #1, bob alone on #2, carol only spectating #1: round 1's timeout
#                       closes it, alice and bob play the final, carol in no team
#   break_timeout       "alice; bob; carol; dave", round 1 off, the break 1 minute: after round 1 nothing is decided for
#                       30 s; then only alice comes to the final - carol out after the break, alice the winner
#   open_one_place      TOURNAMENT=4: alice has a place on #1 - the same account on #2 is sent back to #1
#   open_admin_elsewhere carol, an admin with a place on #1, joins #2 too: there she watches - not sent back, no second place
#   admin_live          alice vs bob live, carol (an admin) joins: a spectator - not "too late", not "not theirs"
#   spectator_live      alice vs bob live, dave (HYPERSOMNIA_SPECTATOR) joins, leaves and comes back: a spectator each time,
#                       the match never freezes; mallory (nobody's) is refused while it's played
#   watch_others        "alice; bob; carol; dave": carol-dave live, then alice beats bob - alice and bob (decided) watch
#                       carol-dave while it's played
#   watch_limit         "alice; bob; carol; dave", --tournament-spectator-slots 1: carol-dave's server has 3 slots; alice
#                       (decided) watches it, bob is refused - no more places; carol and dave play on
#   quick_rejoin        alice vs bob by accounts, live: bob's game killed and started again at once - back in his
#                       match before the server noticed the old connection gone, never "joined too late"
#   walkover_b          "alice; bob; carol; dave", 1 minute: only bob of the first match comes, carol-dave play - the page
#                       has bob's walkover (team b) while the round goes on, the timeouts in its config
#   open_after_played   TOURNAMENT=4: alice beats bob on #1 - both only watch #2, neither takes a place there
#   avatars             alice joins with an avatar, bob without - listed nicknames: by her account (claimed), bob 404,
#                       ?nickname= isn't served; listed accounts: ?account=<her key_ id> and 0x... are hers
#   page_json           --tournament-name, a results webhook (a local listener): the page's JSON (name, config, round states),
#                       429 past 10 requests at once, one POST of the FINISHED results
#   watch_limit_admins  "alice; bob", --tournament-spectator-slots 1 (3 slots): carol (an admin) and dave (the spectator
#                       account) come first - alice and bob still both get in, never "The server is full"
#   claim_twice         "alice; bob; carol; dave": alice's key takes "alice", then the same key comes as "carol" to
#                       carol-dave's server - refused (one account in two teams), "carol" stays unclaimed
#   password_known      a join password, "alice=<id>; bob=<id>; carol=<id>; dave=<id>": mallory (nobody's) is told it's
#                       wrong; the players get in without it (their accounts are on the roster), bob - out after his
#                       match - watches carol-dave without it too
#   mixed_teams         a team per line with an empty line among them ("alice, bob" / "" / "carol, dave"): doesn't start
#   admission_allowed   a persistent server with allowed_accounts [alice]: bob is refused, alice and carol (an admin) get in
#   admission_full      a persistent server of 2 slots, full: dave is refused, carol (an admin) gets in
#
# Usage: [CASES="crash_mid_match open_leave"] scenario_tournament.sh
# bash reads a script as it runs it - an edit meanwhile breaks a running suite: it runs a copy of itself
if [ -z "$TOURNAMENT_SUITE_HERE" ]; then
	copy=$(mktemp /tmp/scenario_tournament.XXXXXX.sh)
	cp "$0" $copy
	TOURNAMENT_SUITE_HERE=$(cd "$(dirname "$0")" && pwd) exec bash $copy "$@"
fi

HERE=$TOURNAMENT_SUITE_HERE
. "$HERE/test_env.sh"
. "$HERE/lib.sh"
umask 077
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude-headless/Hypersomnia}
CLIENT_BIN=${CLIENT_BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
ALL="finished_servers budget_nick_case map_download_retry_typo tournament_custom_late custom_live custom_star custom_broken maps_folder_move tournament_custom_missing custom_maps tournament_custom_map kept_map_not_in_cycle map_download_retry map_catalogue_deadline map_download_and_quit map_download_cycle map_download bomb_always open_long_names round1_off five_teams_walkovers crash_renews_deadline crash_in_break break_timeout open_choice open_same_nick watch_limit_final crash_nobody_back_listed crash_nobody_back_open watch_limit nick_claim open_nick_taken spectator_live watch_others quick_rejoin walkover_b open_after_played avatars page_json teams_bye open_auto_close open_leave teams_2v2 teams_accounts crash_mid_match crash_in_freeze crash_between crash_open page_heartbeat join_timeout_equal join_timeout_open open_one_place open_admin_elsewhere admin_live admission_allowed admission_full watch_limit_admins claim_twice mixed_teams password_known"
# SHARD=0, 1, ...: run several suites at once - each on its own ports and X displays. SHARDS=N without CASES: this one
# runs every N-th case of ALL (SHARDS=2: shards 0 and 1 run it all between them)
SHARD=${SHARD:-0}
SHARDS=${SHARDS:-1}
CASES=${CASES:-$(echo $ALL | tr ' ' '\n' | awk -v n=$SHARDS -v k=$SHARD '(NR - 1) % n == k' | tr '\n' ' ')}
# one shard spans ports 18000-19550 + OFF: shards 0-3 by 0, 50, 400, 450 never overlap (by SHARD*50 shard 2's game
# port would be shard 0's page)
SHARD_OFFSETS=(0 50 400 450)
OFF=${OFF:-${SHARD_OFFSETS[$SHARD]}}
PORT=$((18500 + OFF))
P_SIGNAL=$((18000 + OFF))
P_LIST_FALLBACK=$((18410 + OFF))
P_LIST=$((18420 + OFF))
P_MASTER=$((18430 + OFF))
P_PAGE=$((18600 + OFF))
P_HOOK=$((18602 + OFF))
P_DRIP=$((18604 + OFF))
P_FLAKY=$((18605 + OFF))
P_RTC=$((19500 + OFF))
SUITE=$S/tournament_suite_$(date +%s)_$SHARD
mkdir -p $SUITE
RESULTS=$SUITE/results
touch $RESULTS

# Ctrl+C, a kill or the end: nothing of this suite left running
cleanup_on_exit $SUITE

for p in $P_SIGNAL $P_DRIP $P_FLAKY $((P_PAGE + 1)) $P_LIST_FALLBACK $P_LIST $P_MASTER $PORT $((PORT + 1)) $((PORT + 2)) $((PORT + 3)) $P_PAGE $P_HOOK $P_RTC $((P_RTC + 1)) $((P_RTC + 2)) $((P_RTC + 3)); do
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
    "notified_server_list": "127.0.0.1:$P_MASTER",
    "suppress_new_community_server_webhook": true,
    "webrtc_port_range_begin": $P_RTC,
    "ranked": { "countdown_time": ${COUNTDOWN:-3}, "max_rejoins": ${MAX_REJOINS:-0}, "rejoin_time_limit": ${REJOIN_TIME:-150} },
    "max_ranked_players_per_ip": ${MAX_PER_IP:-2},
    "move_to_spectators_if_afk_for_secs": 999999,
    "kick_if_afk_for_secs": 999999
  },
  "server_private": { "report_ranked_match_url": "http://127.0.0.1:1/report_match" }
}
EOF
	echo "=== $CASE ($T)"
}

declare -A ID
declare -A DISPLAY_OF
declare -A SPECTATING_AT

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
	QUIET=1 wait_log "server listening on\|played to the end already\|ERROR:" 30
}

# The server's own process (under the subshell and timeout) - by its appdata dir
crash_server() { kill_by -9 "--appdata-dir $T/server "; }
stop_server() { kill_by -TERM "--appdata-dir $T/server "; }

# The next free X display for a client - its own, so its faction window can be clicked. A shard counts up through
# the whole suite (~150 clients): 500 apart, never into the next shard's
CLIENT_DISPLAY=$((140 + SHARD * 500))

next_display() {
	CLIENT_DISPLAY=$((CLIENT_DISPLAY + 1))

	while [ -e /tmp/.X$CLIENT_DISPLAY-lock ]; do
		CLIENT_DISPLAY=$((CLIENT_DISPLAY + 1))
	done
}

# The faction window of the 480x270 window (opened by M): Metropolis on the left, Resistance on the right
FACTION_X=(225 320)
FACTION_Y=183

client() {
	# client <name> <port> - a small window and few render threads: four of them at once must not starve each other
	# (with full size, all four time out together - "Connection timed out")
	# An open match lets players in as spectators: each chooses a team as a player would (choose_team). NO_TEAM=1: never.
	next_display
	DISPLAY_OF[$1]=$CLIENT_DISPLAY
	SPECTATING_AT[$1]=$(cat $T/server*.out 2>/dev/null | grep -c "${ID[$1]} comes in as a spectator, to choose a team")
	(cd $REPO/hypersomnia && exec env LP_NUM_THREADS=2 timeout 300 xvfb-run -n $CLIENT_DISPLAY -s "-screen 0 640x360x24" nice -n 10 $CLIENT_BIN --connect 127.0.0.1:$2 --appdata-dir $T/$1 $GAME_TEST_FLAGS > $T/$1.$2.$(date +%s%N).out 2>&1) &

	if [ -z "$NO_TEAM" ]; then
		choose_team $1
	fi
}

# choose_team <name> [faction: 0 Metropolis, 1 Resistance] - M, then a faction, every few seconds until the server gives
# the player a place. Only once the server let it in to choose (an open match) - elsewhere a click would only shoot. Without a faction: each one twice in a row - a full team leaves the window open, so the next M
# closes it, and both get a click with the window open. Another choose_team for the same player stops this one.
choose_team() {
	local name=$1 only=$2
	local d=${DISPLAY_OF[$name]}
	local id=${ID[$name]}
	local placed_before=$(cat $T/server*.out 2>/dev/null | grep -c "$id takes a place")
	# only as a spectator since this game started (client) - an earlier game's (round 1) is no reason to click in a
	# later, listed match
	local spectating_before=${SPECTATING_AT[$name]:-0}
	local run=$(date +%s%N)
	echo $run > $T/$name.choosing
	echo 0 > $T/$name.clicks

	# checked every second, 3 s after a click for the server to answer
	(clicks=0
	for i in $(seq 1 240); do
		sleep 1
		[ "$(cat $T/$name.choosing 2>/dev/null)" = "$run" ] || break
		[ "$(cat $T/server*.out 2>/dev/null | grep -c "$id takes a place")" -gt "$placed_before" ] && break
		[ -e /tmp/.X$d-lock ] || { [ $i -gt 12 ] && break; continue; }
		[ "$(cat $T/server*.out 2>/dev/null | grep -c "$id comes in as a spectator, to choose a team")" -gt "$spectating_before" ] || continue
		DISPLAY=:$d python3 "$HERE/xinput.py" scroll 280 300 0 > /dev/null 2>&1
		DISPLAY=:$d python3 "$HERE/xinput.py" key m > /dev/null 2>&1
		sleep 1
		DISPLAY=:$d python3 "$HERE/xinput.py" click ${FACTION_X[${only:-$(((clicks / 2) % 2))}]} $FACTION_Y > /dev/null 2>&1
		clicks=$((clicks + 1))
		echo $clicks > $T/$name.clicks
		sleep 3
	done) &
}

# clicked <name> <n> - choose_team clicked a faction n times
clicked() { [ "$(cat $T/$1.clicks 2>/dev/null)" -ge "$2" ]; }

# say <name> <text> - in the game's chat (Y), as a player types it
say() {
	local d=${DISPLAY_OF[$1]}
	DISPLAY=:$d python3 "$HERE/xinput.py" scroll 280 300 0 > /dev/null 2>&1
	DISPLAY=:$d python3 "$HERE/xinput.py" key y > /dev/null 2>&1
	sleep 1
	DISPLAY=:$d python3 "$HERE/xinput.py" type "$2" > /dev/null 2>&1
	DISPLAY=:$d python3 "$HERE/xinput.py" key Return > /dev/null 2>&1
}

# say_heard <name> <text> - said until the server logs it ("Server: <name>: <text>"), at most 3 times: a key pressed
# through XTest is sometimes lost on a loaded machine
say_heard() {
	for i in 1 2 3; do
		say $1 "$2"
		wait_until 5 grep -q "Server: $1: $2" $SRV && return 0
	done
}

kill_client() { kill_by -9 "^$CLIENT_BIN --connect .*--appdata-dir $T/$1 "; }
kill_all_clients() { kill_by -9 "^$CLIENT_BIN --connect .*--appdata-dir $T/"; }

# wait_log <pattern> [secs] - in the latest server log
wait_log() {
	for i in $(seq 1 ${2:-60}); do
		grep -q "$1" $SRV 2>/dev/null && return 0
		sleep 1
	done

	# QUIET=1: only a wait, no FAIL of its own (start_server - a server waiting for a map may print none of them)
	if [ -n "$QUIET" ]; then
		echo "  (timeout waiting for: $1)"
	else
		echo "FAIL  $CASE: timeout waiting for: $1" | tee -a $RESULTS
	fi

	return 1
}

# wait_count <pattern> <n> [secs]
wait_count() {
	for i in $(seq 1 ${3:-60}); do
		[ "$(grep -c "$1" $SRV 2>/dev/null)" -ge "$2" ] && return 0
		sleep 1
	done

	echo "FAIL  $CASE: timeout waiting for $2x: $1" | tee -a $RESULTS
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
	page_now "$1"
}

# page_now - the same without waiting for a fresh page
page_now() {
	curl -s --max-time 3 http://127.0.0.1:$PORT/tournament.json > $T/page.json || return 1
	python3 -c "
import json,sys,time
t=json.load(open('$T/page.json'))
R=lambda i: t['rounds'][::-1][i]
N=lambda team: [p['nickname'] for p in team]
finished=t['state']=='FINISHED'
sys.exit(0 if ($1) else 1)"
}

# wait_page '<expression>' [secs]
wait_page() { wait_until ${2:-30} page_now "$1"; }

# wait_live - every match that started is past its 5 s of STARTING (leaving then would start it again): on the page
# none "starting", one "live"
all_matches='[m for r in t["rounds"] for m in r["matches"]]'

wait_live() {
	wait_page "not any(m['status']=='starting' for m in $all_matches) and any(m['status']=='live' for m in $all_matches)" ${1:-30} ||
		echo "  (timeout waiting for a live match)"
}

# round_deadline <i> - round i+1's deadline on the page (unix seconds)
round_deadline() {
	page_now 'True' && python3 -c "import json; t=json.load(open('$T/page.json')); print(t['rounds'][::-1][$1]['deadline'])"
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

# The local masterserver for a case [for <secs>] - its server_list_json read by list '<python on l, the entries>'
start_master() {
	mkdir -p $T/ms/user/conf.d $T/official_servers.d
	cat > $T/ms/user/conf.d/test.json <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": $P_SIGNAL, "first_udp_command_port": $P_MASTER, "num_udp_command_ports": 1,
  "server_list_port": $P_LIST, "fallback_http_server_list_port": $P_LIST_FALLBACK, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF
	(cd $REPO/hypersomnia && exec timeout ${1:-900} $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1) &
	wait_until 20 curl -sf --max-time 1 http://127.0.0.1:$P_LIST/server_list_json || echo "  (the masterserver's list doesn't answer)"
}

stop_master() { kill_by -TERM "--appdata-dir $T/ms "; }

# list '<python on l, the list's entries>' - P(port): the entries at that port. Asked every half second until true
# (LIST_SECS, 20 by default) - a heartbeat goes every 10 s
list() { wait_until ${LIST_SECS:-20} list_now "$1"; }

list_now() {
	curl -s --max-time 3 http://127.0.0.1:$P_LIST/server_list_json > $T/list.json || return 1
	python3 -c "
import json,sys
l=json.load(open('$T/list.json'))
P=lambda port: [e for e in l if e['ip'].endswith(':%d' % port)]
sys.exit(0 if ($1) else 1)"
}

# in_client_log <name> <pattern> - any of its logs
in_client_log() { cat $T/$1.*.out 2>/dev/null | grep -q "$2"; }

case_finished_servers() {
	new_case finished_servers
	start_master
	player alice; player bob; player carol; player dave; player mallory
	local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}; ${ID[carol]}; ${ID[dave]}" --tournament-name "Cup")
	local A=$PORT B=$((PORT + 1))
	start_server -- "${flags[@]}"
	client alice $A; sleep 2; client bob $A
	client carol $B; sleep 2; client dave $B
	wait_count "Starting ranked" 2 120
	wait_live
	check "list: both matches, the round's range and stage" list "len(P($A))==1 and len(P($B))==1 and all(e['tournament_name']=='Cup' and e['tournament_stage']==1 and e['instance_first_port']==$A and e['instance_num_ports']==2 and not e['finished'] for e in P($A)+P($B))"
	kill_client bob
	wait_log "\[T$A\] Tournament match finished" 120
	check "#1 finished, still up, empty" list "P($A) and P($A)[0]['finished'] and P($A)[0]['num_playing']==0"
	check "alice left by herself, told why" in_client_log alice "Match finished. You can watch the other matches."
	NO_TEAM=1 client mallory $A
	check "a newcomer refused, told why" wait_until 30 in_client_log mallory "Match finished. You can watch the other matches."
	check "the round goes on while #2 is played" not_in_log "Tournament round 2: running"
	kill_client dave
	wait_log "Tournament round 2: running" 120
	check "round 2: the final on the first port, stage 0" list "len(P($A))==1 and P($A)[0]['tournament_stage']==0 and not P($A)[0]['finished'] and P($A)[0]['instance_num_ports']==1"
	kill_all_clients
	client alice $A; sleep 2; client carol $A
	wait_count "Starting ranked" 3 90
	wait_live
	kill_client carol
	wait_log "Tournament WINNER" 120
	check "alice won" winner_is alice
	check "over: the final stays on the list, finished" list "len(P($A))==1 and P($A)[0]['finished'] and P($A)[0]['tournament_stage']==0 and not P($B)"
	kill_client mallory
	NO_TEAM=1 client mallory $A
	check "over: a newcomer told the tournament finished" wait_until 30 in_client_log mallory "The tournament has finished."
	finish_case
	# its goodbye: the entry gone - so the next check sees the restarted server's own heartbeat
	check "stopped: gone from the list" list "not P($A)"

	start_server -- "${flags[@]}"
	check "not started again" in_log "played to the end already"
	check "restarted over: the final finished on the list" list "len(P($A))==1 and P($A)[0]['finished'] and P($A)[0]['tournament_stage']==0"
	finish_case
	stop_master
}

case_budget_nick_case() {
	new_case budget_nick_case
	# alice listed as "alice", joins as "Alice"; bob listed as "Bob", joins as "bob"; mallory isn't listed
	player alice Alice; player bob bob; player mallory Zed
	start_server -- --tournament-teams "alice; Bob; carol; dave" --tournament-spectator-slots 4
	wait_count "server listening on" 2 30
	check "a round of 2 matches, 4 to watch: 2 players + 2 on each" bash -c "[ \$(grep -c 'connections for 4 slots' $SRV) -ge 2 ]"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "takes the listed nickname" 2 90
	check "Alice takes alice, bob takes Bob - the case doesn't matter" bash -c "grep -q 'takes the listed nickname Alice' $SRV && grep -q 'takes the listed nickname bob' $SRV"
	wait_count "Starting ranked" 1 90
	NO_TEAM=1 client mallory $((PORT + 1))
	check "a nickname not on the list: told which ones the match waits for" wait_until 30 in_client_log mallory "isn't on this match's list"
	finish_case
}

# ---------- the cases ----------

case_open_choice() {
	# three from this machine: the network's limit of 2 raised
	COUNTDOWN=300 MAX_REJOINS=2 MAX_PER_IP=3 new_case open_choice
	player alice; player bob; player carol
	start_server HYPERSOMNIA_TOURNAMENT=2 --
	NO_TEAM=1 client alice $PORT; NO_TEAM=1 client bob $PORT; NO_TEAM=1 client carol $PORT
	choose_team alice 1
	wait_count "takes a place" 1 60
	choose_team bob 1
	# two clicks on alice's full team (the window stays open, the next M closes it - both with it open)
	check "bob clicked alice's team twice" wait_until 60 clicked bob 2
	sleep 3
	check "bob can't join alice's full team" bash -c "[ \$(grep -c 'takes a place' $SRV) = 1 ]"
	check "no match with one team" not_in_log "Starting ranked"
	choose_team bob 0
	wait_count "takes a place" 2 60
	check "bob takes the other team" in_log "${ID[bob]} takes a place in"
	sleep 5
	check "a 300 s countdown: not started yet" not_in_log "Starting ranked"
	say_heard alice "/go"; say_heard bob "/go"
	wait_log "All players are ready for ranked" 30
	check "/go from both starts it at once" in_log "All players are ready for ranked"
	wait_count "Starting ranked" 1 30
	wait_log "still spectating when it started" 20
	check "carol, still spectating, sent to another match" in_log "${ID[carol]} still spectating when it started"
	local faction=$(grep "${ID[bob]} takes a place in" $SRV | tail -1 | sed 's/.* in \([0-9]*\)\.$/\1/')
	# live - past the 5 s of STARTING, when a leave would restart it
	wait_live
	kill_client bob
	NO_TEAM=1 client bob $PORT
	wait_count "Adding bob to game state" 2 60
	check "bob back in his team ($faction)" bash -c "grep 'Assigned team for bob' $SRV | tail -1 | grep -q ': $faction\$'"
	check "no place taken again" bash -c "[ \$(grep -c '${ID[bob]} takes a place' $SRV) = 1 ]"
	finish_case
}

case_open_same_nick() {
	new_case open_same_nick
	player alice Player; player bob Player
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	NO_TEAM=1 client alice $PORT; NO_TEAM=1 client bob $((PORT + 1))
	choose_team alice 1
	wait_count "takes a place" 1 60
	choose_team bob 0
	wait_log "chose a team with the nickname" 60
	check "the second Player told to change it" in_log "${ID[bob]} chose a team with the nickname Player"
	check "no second place" bash -c "[ \$(grep -c 'takes a place' $SRV) = 1 ]"
	finish_case
}

case_watch_limit_final() {
	new_case watch_limit_final
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-spectator-slots 1
	client alice $PORT; sleep 2; client bob $PORT; sleep 2; client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 2 120
	wait_live
	kill_client bob; kill_client dave
	wait_log "Tournament round 2: running" 150
	check "the final's server: 2 players + 1 to watch (the teams out watch, they aren't players)" wait_until 15 bash -c "sed -n '/round 2: running/,\$p' $SRV | grep -q 'T$PORT\] Starting yojimbo::Server with [0-9]* connections for 3 slots'"
	finish_case
}

# crash_nobody_back <listed|open> - the server killed mid-match, started again, nobody comes back
crash_nobody_back() {
	MAX_REJOINS=2 REJOIN_TIME=20 new_case crash_nobody_back_$1
	player alice; player bob

	if [ $1 = listed ]; then
		local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}")
		start_server -- "${flags[@]}"
	else
		local flags=()
		start_server HYPERSOMNIA_TOURNAMENT=2 --
	fi

	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 120
	wait_live
	crash_server
	kill_all_clients

	if [ $1 = listed ]; then
		start_server -- "${flags[@]}"
	else
		start_server HYPERSOMNIA_TOURNAMENT=2 --
	fi

	wait_log "to its end - as if nobody came" 120
	check "decided as if nobody came" in_log "nobody played the match on port"
	check "the tournament doesn't fail" not_in_log "ended without a result\\|exited without resolving"
	finish_case
}

case_crash_nobody_back_listed() { crash_nobody_back listed; }
case_crash_nobody_back_open() { crash_nobody_back open; }

case_crash_renews_deadline() {
	new_case crash_renews_deadline
	player alice; player bob
	local flags=(--tournament-teams "alice; bob" --tournament-round-1-max-wait 1)
	start_server -- "${flags[@]}"
	client alice $PORT
	# the round's deadline as the page shows it - the one its servers keep (a real timeout: join_timeout_*)
	wait_page '"deadline" in R(0)'
	local first=$(round_deadline 0)
	sleep 15
	crash_server
	kill_all_clients
	local before=$(date +%s)
	start_server -- "${flags[@]}"
	local restarted=$(date +%s)
	wait_page '"deadline" in R(0)'
	local again=$(round_deadline 0)
	# a restart gives round 1 its whole time again: 1 minute + 2 of grace from the restart, not from the start
	check "round 1's deadline counted again from the restart ($first -> $again, restarted $before..$restarted)" [ $((again - first)) -ge 15 -a $again -ge $((before + 180 - 1)) -a $again -le $((restarted + 180 + 1)) ]
	finish_case
}

case_crash_in_break() {
	new_case crash_in_break
	player alice; player bob; player carol; player dave
	local flags=(--tournament-teams "alice; bob; carol; dave" --tournament-round-1-max-wait off --tournament-next-rounds-max-wait 1)
	start_server -- "${flags[@]}"
	client alice $PORT; sleep 2; client bob $PORT; sleep 2; client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 2 120
	wait_live
	kill_client bob; kill_client dave
	wait_log "Tournament round 2: running" 150
	kill_client alice; kill_client carol
	wait_page '"deadline" in R(1)'
	local first=$(round_deadline 1)
	sleep 15
	crash_server
	local before=$(date +%s)
	start_server -- "${flags[@]}"
	local restarted=$(date +%s)
	check "resumed at round 2" in_log "resumed from .* at round 2"
	wait_page '"deadline" in R(1)'
	local again=$(round_deadline 1)
	# a restart gives the break its whole time again: 1 min + 2 min of grace from the restart (a real one: break_timeout)
	check "the break's deadline counted again from the restart ($first -> $again, restarted $before..$restarted)" [ $((again - first)) -ge 15 -a $again -ge $((before + 180 - 1)) -a $again -le $((restarted + 180 + 1)) ]
	finish_case
}

case_open_long_names() {
	# four from this machine: the network's limit of 2 raised
	MAX_PER_IP=4 new_case open_long_names
	player alice aaaaaaaaaaaaaaaaaaaaaaaaa; player bob bbbbbbbbbbbbbbbbbbbbbbbbb; player carol ccccccccccccccccccccccccc; player dave ddddddddddddddddddddddddd
	start_server HYPERSOMNIA_TOURNAMENT=4 HYPERSOMNIA_TOURNAMENT_TEAM_SIZE=2 --
	NO_TEAM=1 client alice $PORT; NO_TEAM=1 client bob $PORT; NO_TEAM=1 client carol $PORT; NO_TEAM=1 client dave $PORT
	choose_team alice 1
	wait_count "takes a place" 1 60
	choose_team bob 1
	wait_count "takes a place" 2 60
	choose_team carol 0
	wait_count "takes a place" 3 60
	choose_team dave 0
	wait_log "Open match is full" 90
	local name=$(grep "Open match is full: " $SRV | tail -1 | sed 's/.*Open match is full: //')
	check "the name shortened to each team's first player: $name" bash -c "echo '$name' | grep -q '+1\] vs \[.*+1\]'"
	check "the name fits the server list (60 bytes)" [ $(printf '%s' "$name" | wc -c) -le 60 ]
	finish_case
}

case_round1_off() {
	new_case round1_off
	player alice; player bob
	start_server -- --tournament-teams "alice; bob" --tournament-round-1-max-wait off
	client alice $PORT
	wait_log "Adding alice to game state" 60
	# the round's deadline is what its servers decide by - off: none on the page, its match waiting there
	# a fresh page (page waits for one), not the first answer - absence must not pass on a page made before alice came
	check "off: no deadline, alice's match waiting" page 't["config"]["round_1_max_wait_minutes"]==0 and "deadline" not in R(0) and R(0)["matches"][0]["status"]=="waiting"'
	finish_case
}

case_five_teams_walkovers() {
	new_case five_teams_walkovers
	player alice; player bob; player carol; player dave; player erin
	# alice last: the byes go to the first lines - she has to win her matches by walkovers
	start_server -- --tournament-teams "bob; carol; dave; erin; alice" --tournament-round-1-max-wait 1 --tournament-next-rounds-max-wait 1
	# round 1: erin vs alice on the first port; round 2: bob vs carol there, dave vs alice on the next - she goes to hers
	client alice $PORT
	wait_log "Tournament round 2: running" 400
	kill_client alice
	client alice $((PORT + 1))
	wait_log "Tournament WINNER\|Tournament over" 700
	check "alice the winner of 5 teams nobody else came to" winner_is alice
	check "a match nobody came to: both out" in_log "nobody came - both teams are out"
	check "alice's opponent out: a walkover" in_log "a team with nobody here is out"
	check "not stuck: never aborted" not_in_log "exited without resolving"
	finish_case
}

case_bomb_always() {
	new_case bomb_always
	player alice; player bob
	# an arena whose own default is another mode: fy_minilab copied, its default quick_test
	local a=$T/server/user/downloads/maps/fy_mode_test
	mkdir -p $a && cp -r $REPO/hypersomnia/content/arenas/fy_minilab/. $a/
	mv $a/fy_minilab.json $a/fy_mode_test.json
	python3 -c "import json; p='$a/fy_mode_test.json'; d=json.load(open(p)); d['settings']['default_server_mode']='quick_test'; json.dump(d, open(p, 'w'))"
	start_server -- --tournament-teams "alice; bob" --tournament-maps "fy_mode_test quick_test"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 150
	wait_live
	kill_client bob
	wait_log "Match report JSON" 120
	check "bomb defusal, whatever the arena's default and the list say" bash -c "grep 'Match report JSON' $SRV | grep -q '\"game_mode\":\"Bomb Defusal\"'"
	finish_case
}

case_map_download() {
	new_case map_download
	player alice; player bob
	# a community map: the test server's own appdata ($T/server) has none - downloaded from hypersomnia.io, the players'
	# games ($T/alice, $T/bob) get it from the server
	start_server -- --tournament-teams "alice; bob" --tournament-maps de_hotline
	wait_log "Tournament round 1: running" 180
	check "not on disk: downloaded from hypersomnia.io" in_log "Community maps to play: de_hotline - downloading"
	check "the map in the test server's own appdata" [ -f $T/server/user/downloads/maps/de_hotline/de_hotline.json ]
	check "only the maps it needs - no --sync, nothing else downloaded" [ "$(command ls $T/server/user/downloads/maps)" = de_hotline ]
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 240
	check "the match on it started - the players downloaded it from the server" in_log "Starting ranked"
	finish_case
}

case_map_catalogue_deadline() {
	new_case map_catalogue_deadline
	# a provider that never finishes answering - a byte every 2 s: no socket timeout ends it, only the catalogue's 5 s
	timeout 60 python3 -c "
import socket, threading, time
s = socket.socket(); s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1); s.bind(('127.0.0.1', $P_DRIP)); s.listen()
def drip(c):
    try:
        while True: c.send(b'H'); time.sleep(2)
    except OSError: pass
while True: threading.Thread(target=drip, args=(s.accept()[0],), daemon=True).start()
" &
	local drip=$!
	sleep 1
	server_extra '{ "server": { "map_cycle": ["de_hotline"], "external_arena_files_provider": "http://127.0.0.1:'$P_DRIP'" } }'
	local t0=$(date +%s)
	start_server --
	check "the server starts within 10 s" [ $(($(date +%s) - t0)) -le 10 ]
	check "the catalogue given up after 5 s" in_log "No map catalogue from http://127.0.0.1:$P_DRIP"
	stop_server
	# the setup's step has nothing else to do: it keeps waiting for the answer - the admin sees it, Ctrl+C stops it
	(cd $REPO/hypersomnia && exec env HYPERSOMNIA_MAP_CYCLE=de_hotline timeout 20 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS --download-maps-and-quit > $T/server/dl.out 2>&1)
	check "--download-maps-and-quit still waiting after 20 s (killed by timeout)" [ $? = 124 ]
	kill $drip 2>/dev/null
	# no provider at all: asks again every 3 s
	server_extra '{ "server": { "map_cycle": ["de_hotline"], "external_arena_files_provider": "http://127.0.0.1:1" } }'
	(cd $REPO/hypersomnia && exec env HYPERSOMNIA_MAP_CYCLE=de_hotline timeout 12 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS --download-maps-and-quit > $T/server/dl2.out 2>&1)
	check "... and with nothing listening: asks again" [ "$(grep -c 'asking again in 3 seconds' $T/server/dl2.out)" -ge 2 ]
	finish_case
}

# a custom map: fy_minilab copied as <name> into the case's server's custom/maps
add_custom_map() {
	local c=$T/server/user/custom/maps/$1
	mkdir -p $c && cp -r $REPO/hypersomnia/content/arenas/fy_minilab/. $c/ && mv $c/fy_minilab.json $c/$1.json
}

case_custom_live() {
	new_case custom_live
	# an empty server changes maps every 3 s: a custom map put in while it runs is in the cycle by itself
	add_custom_map fy_custom_a
	server_extra '{ "server": { "map": "", "map_cycle": ["fy_minilab"], "map_cycle_random": false, "map_cycle_add_custom_maps": true, "when_idle_change_maps_once_every_mins": 0.05 } }'
	start_server --
	wait_log "Loading arena from: .*custom/maps/fy_custom_a/" 60
	check "the custom map in the cycle" in_log "Loading arena from: .*custom/maps/fy_custom_a/"
	add_custom_map fy_custom_new
	wait_log "Loading arena from: .*custom/maps/fy_custom_new/" 60
	check "one put in while it runs: in the cycle, no restart" in_log "Loading arena from: .*custom/maps/fy_custom_new/"
	finish_case
}

case_custom_star() {
	new_case custom_star
	# "*" (every map on disk) with custom maps on: the custom ones in once, the custom de_silo never
	add_custom_map fy_custom_s; add_custom_map de_silo
	server_extra '{ "server": { "map": "", "map_cycle": ["*"], "map_cycle_random": false, "map_cycle_add_custom_maps": true, "when_idle_change_maps_once_every_mins": 0.05 } }'
	start_server --
	wait_log "Loading arena from: .*custom/maps/fy_custom_s/" 90
	check "*: the custom map played" in_log "Loading arena from: .*custom/maps/fy_custom_s/"
	check "*: the custom de_silo never - the official one" not_in_log "Loading arena from: .*custom/maps/de_silo/"
	finish_case
}

case_custom_broken() {
	new_case custom_broken
	# a custom map that doesn't read: skipped, the server goes on
	add_custom_map fy_broken
	echo "{ not json" > $T/server/user/custom/maps/fy_broken/fy_broken.json
	server_extra '{ "server": { "map": "", "map_cycle": ["fy_minilab", "fy_broken"], "map_cycle_random": false, "when_idle_change_maps_once_every_mins": 0.05 } }'
	start_server --
	wait_log 'Failed to load "fy_broken"' 60
	check "a broken custom map: skipped" in_log 'Failed to load "fy_broken"'
	local idle_before=$(grep -ac "Idle for" $SRV)
	check "... and the server goes on" wait_until 15 bash -c "[ \$(grep -ac 'Idle for' $SRV) -gt $idle_before ]"
	finish_case
}

case_maps_folder_move() {
	new_case maps_folder_move
	# a server from before 4.3.0: its downloaded maps in downloads/arenas - and an empty downloads/maps (as the site's
	# deployment makes it): moved at the start
	local old=$T/server/user/downloads/arenas/de_hotline
	mkdir -p $old $T/server/user/downloads/maps && cp -r $REPO/hypersomnia/content/arenas/fy_minilab/. $old/ && mv $old/fy_minilab.json $old/de_hotline.json
	server_extra '{ "server": { "map": "", "map_cycle": ["fy_minilab"] } }'
	start_server --
	check "downloads/arenas moved to downloads/maps" in_log "Moved the downloaded maps"
	check "... the map there" [ -f $T/server/user/downloads/maps/de_hotline/de_hotline.json ]
	check "... the old folder gone" [ ! -e $T/server/user/downloads/arenas ]
	finish_case
}

case_tournament_custom_missing() {
	new_case tournament_custom_missing
	# a custom map not uploaded before the start (the setup skipped): said in bold yellow, then the tournament doesn't start
	start_server -- --tournament-teams "alice; bob" --tournament-maps fy_not_uploaded
	wait_log "nor in custom/maps" 60
	check "said: Missing map, is it a custom one" in_log "Missing map: fy_not_uploaded. Is this a custom map?"
	check "the tournament doesn't start without it" in_log 'no map "fy_not_uploaded" - neither in the game, nor among the community maps (hypersomnia.io/arenas), nor in custom/maps'
	check "... no round running" not_in_log "Tournament round 1: running"
	finish_case
}

case_custom_maps() {
	new_case custom_maps
	player alice
	# two maps of the admin's own in custom/maps (fy_minilab copied under new names) - and one named as an official
	# (de_silo): the official is played, the custom one ignored
	custom_map() {
		local c=$T/server/user/custom/maps
		mkdir -p $c/$1 && cp -r $REPO/hypersomnia/content/arenas/fy_minilab/. $c/$1/ && mv $c/$1/fy_minilab.json $c/$1/$1.json
	}
	custom_map fy_custom_a; custom_map fy_custom_b; custom_map de_silo
	# added to the cycle, no map, no maps listed: the server starts on a custom one (in order: the first)
	server_extra '{ "server": { "map": "", "map_cycle": [], "map_cycle_random": false, "map_cycle_add_custom_maps": true } }'
	start_server --
	check "custom maps found, the custom de_silo ignored" in_log 'Custom map "de_silo" ignored'
	check "both custom maps on the server" in_log "Rescanned arenas on disk: .* (custom: 2)"
	check "no map, no cycle: it starts on the first custom map" in_log "Loading arena from: .*custom/maps/fy_custom_a/"
	client alice $PORT
	check "a player downloads the custom map from the server" wait_until 90 [ -f $T/alice/user/downloads/maps/fy_custom_a/fy_custom_a.json ]
	check "... straight from it - hypersomnia.io never asked for a custom map" bash -c "! cat $T/alice.*.out | grep -q 'HTTP downloader\|External downloading session'"
	check "the player's game makes no custom/ (a dedicated server's only)" [ ! -e $T/alice/user/custom ]
	stop_server
	kill_all_clients
	# not added to the cycle (the default): there all the same - by name, not in rotation
	new_case custom_maps_off
	custom_map fy_custom_c
	server_extra '{ "server": { "map": "", "map_cycle": [] } }'
	start_server --
	check "off: custom/maps there, its map on the server" in_log "Rescanned arenas on disk: .* (custom: 1)"
	check "off, nothing listed: de_cyberaqua, not the custom map" in_log "No map and no map_cycle set - de_cyberaqua"
	stop_server
	# named in the cycle - without the setting (a fresh server: the map played is kept for a restart)
	new_case custom_maps_named
	custom_map fy_custom_c
	server_extra '{ "server": { "map": "", "map_cycle": ["fy_custom_c"] } }'
	start_server --
	check "named in the cycle: played from custom/maps" in_log "Loading arena from: .*custom/maps/fy_custom_c/"
	check "nothing downloaded for it" not_in_log "Community maps to play: fy_custom_c"
	finish_case
}

case_kept_map_not_in_cycle() {
	new_case kept_map_not_in_cycle
	# a server played de_silo; the admin changes the cycle to fy_minilab and restarts: not de_silo again
	server_extra '{ "server": { "map": "", "map_cycle": ["de_silo"] } }'
	start_server --
	check "the first run plays de_silo" in_log "Loading arena from: content/arenas/de_silo/"
	stop_server
	server_extra '{ "server": { "map": "", "map_cycle": ["fy_minilab"] } }'
	start_server --
	check "the kept map not in the new cycle: dropped" in_log "The map of the last run (de_silo) isn't in the map cycle any more"
	check "... the cycle's map played" in_log "Loading arena from: content/arenas/fy_minilab/"
	finish_case
}

case_tournament_custom_map() {
	new_case tournament_custom_map
	player alice; player bob
	local c=$T/server/user/custom/maps/fy_cup_final
	mkdir -p $c && cp -r $REPO/hypersomnia/content/arenas/fy_minilab/. $c/ && mv $c/fy_minilab.json $c/fy_cup_final.json
	start_server -- --tournament-teams "alice; bob" --tournament-maps fy_cup_final
	wait_log "Tournament round 1: running" 120
	check "a tournament on a custom map: nothing to download, no wait" not_in_log "Missing map:"
	check "... hypersomnia.io never asked" not_in_log "Community maps to play"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 150
	check "its match on the custom map" in_log "Loading arena from: .*custom/maps/fy_cup_final/"
	finish_case
}

case_map_download_retry() {
	new_case map_download_retry
	# a provider in front of hypersomnia.io that refuses the catalogue twice, then cuts off the first map file
	timeout 300 python3 -c "
import http.server, urllib.request
state = {'catalogue': 0, 'file': 0}
class H(http.server.BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def do_GET(self):
        if '?' in self.path and state['catalogue'] < 2:
            state['catalogue'] += 1; self.send_response(503); self.end_headers(); return
        if '?' not in self.path and state['file'] < 1:
            state['file'] += 1; self.wfile.flush(); self.connection.close(); return
        body = urllib.request.urlopen('https://hypersomnia.io' + self.path, timeout=30).read()
        self.send_response(200); self.send_header('Content-Length', str(len(body))); self.end_headers(); self.wfile.write(body)
http.server.ThreadingHTTPServer(('127.0.0.1', $P_FLAKY), H).serve_forever()
" &
	local flaky=$!
	sleep 1
	server_extra '{ "server": { "external_arena_files_provider": "http://127.0.0.1:'$P_FLAKY'/arenas" } }'
	start_server -- --tournament-teams "alice; bob" --tournament-maps de_hotline
	wait_log "Tournament round 1: running" 240
	check "a tournament map not on disk: it waits for it" in_log "Missing map: de_hotline. Is this a custom map?"
	check "the catalogue refused: asked again" in_log "No map catalogue from .* yet"
	check "a map file cut off: asked again" in_log "Some maps weren't downloaded - asking again"
	check "then downloaded" [ -f $T/server/user/downloads/maps/de_hotline/de_hotline.json ]
	check "... and the tournament runs" in_log "Tournament round 1: running"
	kill $flaky 2>/dev/null
	finish_case
}

case_map_download_retry_typo() {
	new_case map_download_retry_typo
	# a tournament map hypersomnia.io doesn't have (a typo) next to one whose file is cut off once: asked again only for
	# the one that's there - then the start names the typo, never an endless wait
	timeout 300 python3 -c "
import http.server, urllib.request
state = {'file': 0}
class H(http.server.BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def do_GET(self):
        if '?' not in self.path and state['file'] < 1:
            state['file'] += 1; self.wfile.flush(); self.connection.close(); return
        body = urllib.request.urlopen('https://hypersomnia.io' + self.path, timeout=30).read()
        self.send_response(200); self.send_header('Content-Length', str(len(body))); self.end_headers(); self.wfile.write(body)
http.server.ThreadingHTTPServer(('127.0.0.1', $P_FLAKY), H).serve_forever()
" &
	local flaky=$!
	sleep 1
	server_extra '{ "server": { "external_arena_files_provider": "http://127.0.0.1:'$P_FLAKY'/arenas" } }'
	start_server -- --tournament-teams "alice; bob" --tournament-maps "de_hotline, fy_nowhere"
	wait_log 'no map "fy_nowhere"' 120
	check "the cut-off map asked again" in_log "Some maps weren't downloaded - asking again"
	check "the typo named - not an endless wait" in_log 'no map "fy_nowhere"'
	check "... no round running" not_in_log "Tournament round 1: running"
	kill $flaky 2>/dev/null
	finish_case
}

case_tournament_custom_late() {
	new_case tournament_custom_late
	# a LAN without internet: the yellow line says the map is missing, the admin copies it into custom/maps - the
	# wait notices it, no restart
	server_extra '{ "server": { "external_arena_files_provider": "http://127.0.0.1:1/arenas" } }'
	start_server -- --tournament-teams "alice; bob" --tournament-maps fy_late
	wait_log "Missing map: fy_late" 15
	check "waits for the missing map" in_log "Missing map: fy_late. Is this a custom map? Trying to fetch it from 127.0.0.1"
	add_custom_map fy_late
	wait_log "Tournament round 1: running" 30
	check "copied in while it waits: the tournament starts" in_log "Tournament round 1: running"
	finish_case
}

case_map_download_cycle() {
	new_case map_download_cycle
	# not a tournament: a server whose map cycle names a community map - downloaded at the start, nothing else
	python3 -c "import json; p='$T/server/user/conf.d/test.json'; d=json.load(open(p)); d['server']['map_cycle']=['de_hotline']; json.dump(d, open(p, 'w'))"
	start_server --
	wait_log "server listening on" 120
	check "the cycle's community map downloaded at the start" in_log "Community maps to play: de_hotline - downloading"
	check "only the map it needs - no --sync, nothing else" [ "$(command ls $T/server/user/downloads/maps)" = de_hotline ]
	stop_server
	# started again: up to date - nothing downloaded
	start_server --
	wait_log "server listening on" 120
	# up to date - or hypersomnia.io stalled past the 5 s: either way nothing downloaded, the map on disk played
	check "a second start: nothing downloaded" not_in_log "Syncing [0-9]* arena"
	finish_case
}

case_map_download_and_quit() {
	new_case map_download_and_quit
	# the setup's step: the maps named by the variable downloaded, then it quits - nothing else
	(cd $REPO/hypersomnia && exec env HYPERSOMNIA_MAP_CYCLE=de_cyberaqua,de_hotline timeout 300 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS --download-maps-and-quit > $T/server/dl.out 2>&1)
	check "it quit by itself" [ $? = 0 ]
	check "only the community map named - nothing else" [ "$(command ls $T/server/user/downloads/maps)" = de_hotline ]
	check "no server started" bash -c "! grep -q 'server listening on' $T/server/dl.out"
	# the variable's own: HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT=1 (Docker), a map hypersomnia.io doesn't have (a typo) - fails
	(cd $REPO/hypersomnia && exec env HYPERSOMNIA_MAP_CYCLE=de_hotlinee HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT=1 timeout 300 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS > $T/server/dl2.out 2>&1)
	check "a misspelled map: the step fails" [ $? = 1 ]
	check "... and names it" grep -q 'no map "de_hotlinee"' $T/server/dl2.out
	check "... the variable quits too - no server started" bash -c "! grep -q 'server listening on' $T/server/dl2.out"
	# a value that's neither on nor off: an error at once, never a server that starts instead
	(cd $REPO/hypersomnia && exec env HYPERSOMNIA_MAP_CYCLE=de_hotline HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT=y timeout 30 $BIN --appdata-dir $T/server $GAME_TEST_FLAGS > $T/server/dl3.out 2>&1)
	check "HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT=y: exit 1" [ $? = 1 ]
	check "... named" grep -q 'HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT="y"' $T/server/dl3.out
}

case_admin_live() {
	new_case admin_live
	player alice; player bob; player carol
	server_extra "{ \"server_private\": { \"admin_profiles\": [ { \"account\": \"${ID[carol]}\", \"rcon\": \"MASTER_RCON\" } ] } }"
	start_server -- --tournament-teams "alice; bob"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
	client carol $PORT
	wait_log "watches the tournament's match as a spectator" 60
	check "carol watches as a spectator" in_log "watches the tournament's match as a spectator"
	wait_log "Adding carol to game state" 30
	check "carol: never a team problem" not_in_log "pre-assigning your team"
	check "carol in the game, not timed out" bash -c "grep -q 'Adding carol to game state' $SRV && ! grep -q 'Connection timed out' $T/carol.*.out"
	sleep 5
	check "not too late, not theirs" not_in_log "carol has connected to a match that's not theirs"
	check "the match goes on" not_in_log "Tournament WINNER"
	kill_client bob
	wait_log "Tournament WINNER" 200
	check "alice won with carol watching" winner_is alice
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

case_quick_rejoin() {
	new_case quick_rejoin
	player alice; player bob
	start_server -- --tournament-teams "alice=${ID[alice]}; bob=${ID[bob]}"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
	kill_client bob
	client bob $PORT
	wait_count "Adding bob to game state" 2 60
	check "bob back in his match" bash -c "[ \$(grep -c 'Adding bob to game state' $SRV) -ge 2 ]"
	check "never joined too late" not_in_log "Joined too late"
	check "his game never told \"wrong match\" or \"too late\"" bash -c "! grep -q 'Connected to a wrong match\|Joined too late' $T/bob.*.out"
	finish_case
}

case_spectator_live() {
	new_case spectator_live
	player alice; player bob; player dave; player mallory
	start_server HYPERSOMNIA_SPECTATOR=${ID[dave]} -- --tournament-teams "alice; bob"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
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
	check "dave never froze the match" bash -c "! grep -q 'Arena: suspending\|abandoned by faction' $SRV"
	check "the match goes on" not_in_log "Tournament WINNER"
	kill_client bob
	wait_log "Tournament WINNER" 200
	check "alice won with dave watching" winner_is alice
	finish_case
}

case_watch_limit() {
	new_case watch_limit
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-spectator-slots 1
	check "carol-dave's server: 2 players + 1 to watch" in_log "T$((PORT + 1))] Starting yojimbo::Server with [0-9]* connections for 3 slots"
	client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 1 90
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	wait_live
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
	check "carol and dave still in their match" bash -c "! grep -q 'was kicked' $T/carol.*.out $T/dave.*.out"
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
	wait_live
	kill_client bob
	wait_log "abandoned by faction" 60
	sleep 3
	kill_client alice; sleep 2
	client alice $((PORT + 1)); sleep 2; client bob $((PORT + 1))
	wait_count "watches the tournament's match as a spectator" 2 60
	check "alice (won) and bob (out) watch carol-dave" bash -c "[ \$(grep -c \"watches the tournament's match as a spectator\" $SRV) -ge 2 ]"
	check "not turned away" not_in_log "connected to a match that's not theirs"
	wait_log "T$((PORT + 1))] Adding bob to game state" 30
	check "alice and bob in carol-dave's game, no team problem" bash -c "grep -q \"T$((PORT + 1))] Adding alice to game state\" $SRV && grep -q \"T$((PORT + 1))] Adding bob to game state\" $SRV && ! grep -q 'pre-assigning your team' $SRV"
	finish_case
}

case_teams_bye() {
	new_case teams_bye
	player alice; player bob; player carol
	local flags=(--tournament-teams "${ID[alice]}; ${ID[bob]}; ${ID[carol]}" --tournament-maps "de_cyberaqua,de_silo")
	start_server -- "${flags[@]}"
	check "1 bye" in_log "1 byes"
	client bob $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	wait_live
	check "page: round 1 live, one bye" page 'R(0)["matches"][0]["status"] in ("starting","live") and len(R(0)["byes"])==1 and R(0)["state"]=="PLAYING"'
	kill_client carol
	wait_log "Tournament round 2: running" 120
	kill_client bob; sleep 2
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	wait_live
	kill_client alice
	wait_log "Tournament WINNER" 120
	check "bob won" winner_is bob
	check "round 2 on de_silo" in_log "Choosing arena: de_silo"
	sleep 2
	check "page: finished, one winner, unnamed: My Tournament" page 'finished and len(t["winner_team"])==1 and all(r["state"]=="PLAYED" for r in t["rounds"]) and t["name"]=="My Tournament"'
	check "page: 2 rounds - the final on de_silo, alice's bye kept in round 1" page 'len(t["rounds"])==2 and t["rounds"][0]["name"]=="Final" and len(t["rounds"][0]["matches"])==1 and t["rounds"][0]["map"]=="de_silo" and R(0)["name"]=="Semifinals" and R(0)["map"]=="de_cyberaqua" and [N(b) for b in R(0)["byes"]]==[["alice"]]'
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
	start_server HYPERSOMNIA_TOURNAMENT=3 HYPERSOMNIA_TOURNAMENT_MAPS="de_cyberaqua,de_silo" --
	check "2 servers" in_log "round 1 open - 2 servers"
	client alice $PORT; sleep 4; client bob $PORT; sleep 4
	client carol $((PORT + 1))
	wait_log "Round 1 closed" 60
	check "named by its players" in_log "Open match is full: \[alice\] vs \[bob\]\|Open match is full: \[bob\] vs \[alice\]"
	check "closed by itself" in_log "closes by itself"
	wait_count "Starting ranked" 1 60
	wait_live
	check "page: carol's walkover" page 'any(m["status"]=="walkover" and N(m["team_a"])==["carol"] for m in R(0)["matches"])'
	kill_client bob
	wait_log "Tournament round 2: running" 120
	kill_client alice; kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 2 90
	wait_live
	check "page: the final, by nicknames" page 'R(1)["matches"][0]["status"] in ("starting","live") and sorted(N(R(1)["matches"][0]["team_a"]+R(1)["matches"][0]["team_b"]))==["alice","carol"]'
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
	wait_live
	# right after "Starting ranked": the 5 s start - STARTING on the page, live after it
	check "page: 2 matches starting or live" page 'sum(m["status"] in ("starting","live") for m in R(0)["matches"])==2'
	check "no walkover" not_in_log "Round 1 closed"
	kill_client bob; kill_client dave
	wait_log "Tournament round 2: running" 120
	kill_client alice; kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 3 90
	wait_live
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
	check "page: [alice, bob] vs [carol, dave]" page 'sorted(N(R(0)["matches"][0]["team_a"]))==["alice","bob"] and sorted(N(R(0)["matches"][0]["team_b"]))==["carol","dave"] and t["config"]["who_participates"]=="rosters by nicknames" and t["config"]["team_size"]==2'
	wait_live
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
	# until the server notices the killed game (its connection times out - longer under load) a second "alice"
	# would be renamed alice1
	local timeouts=$(grep -ac "Connection timed out" $SRV)
	kill_client alice
	wait_count "Connection timed out" $((timeouts + 1)) 60
	client mallory $PORT
	wait_log "already taken in this tournament" 60
	check "mallory as alice later: taken" in_log "already taken in this tournament"
	check "mallory took no place" not_in_log "${ID[mallory]} takes the listed nickname"
	kill_client mallory
	sed -i 's/"nickname": "alice"/"nickname": "alice_renamed"/' $T/alice/user/conf.d/test.json
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
	check "alice back under another nickname - in by her key" in_log "Adding alice_renamed to game state"
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
	wait_live
	check "page: bob vs carol, as listed" page 'sorted(N(R(0)["matches"][0]["team_a"]+R(0)["matches"][0]["team_b"]))==["bob","carol"] and t["config"]["who_participates"]=="rosters by accounts"'
	kill_client carol
	wait_log "Tournament round 2: running" 120
	check "round 2 named by the list: alice and bob" in_log "Match: \[alice\] vs \[bob\]\|Match: \[bob\] vs \[alice\]"
	kill_client bob; sleep 2
	# bob comes to round 2 under yet another nickname - still let in by his account, still shown as bob
	sed -i 's/"bobby_in_game"/"bob_renamed"/' $T/bob/user/conf.d/test.json
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	wait_live
	kill_client alice
	wait_log "Tournament WINNER" 120
	check "bob won" winner_is bob
	sleep 2
	check "renamed between rounds: let in" in_log "Adding bob_renamed to game state"
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
	wait_live
	crash_server
	kill_all_clients
	start_server -- "${flags[@]}"
	check "resumed at round 1" in_log "resumed from .* at round 1"
	check "the match goes on" in_log "resuming round 1 with 1 ongoing"
	check "not started fresh" not_in_log "hash mismatch"
	client alice $PORT; sleep 2; client bob $PORT
	# the restarted server's own log
	wait_log "Adding bob to game state" 60
	wait_live
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
	wait_live
	kill_client bob
	sleep 4
	check "bob suspended, not abandoned" bash -c "grep -q 'Arena: suspending' $SRV && ! grep -q 'abandoned by faction' $SRV"
	crash_server
	kill_client alice
	start_server -- "${flags[@]}"
	check "resumed" in_log "resuming round 1 with 1 ongoing"
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
	wait_live
	kill_client carol
	wait_log "Tournament round 2: running" 120
	crash_server
	kill_all_clients
	start_server -- "${flags[@]}"
	check "resumed at round 2" in_log "resumed from .* at round 2"
	check "round 1 not replayed" not_in_log "Tournament round 1: running"
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
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
	wait_live
	kill_client bob
	wait_log "\[T$PORT\] Tournament match finished" 60
	crash_server
	kill_all_clients
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	check "resumed with #1 decided" in_log "resuming round 1 with 2 ongoing"
	wait_log "\[T$PORT\] Tournament match finished" 15
	check "#1 started again finished, not resumed" in_log "\[T$PORT\] Tournament match finished"
	check "#2 open again" in_log "Match #2 (any pair)"
	client carol $((PORT + 1)); sleep 3; client dave $((PORT + 1))
	wait_count "comes back to the recovered match" 2 90
	sleep 3
	kill_client dave
	wait_log "Tournament round 2: running" 120
	kill_client carol; sleep 2
	client alice $PORT; sleep 2; client carol $PORT
	wait_count "Starting ranked" 1 90
	wait_live
	kill_client carol
	wait_log "Tournament WINNER" 120
	check "alice won" winner_is alice
	check "4 teams, no duplicates" [ "$(archived teams)" = 4 ]
	check "3 results" [ "$(archived match_history)" = 3 ]
	finish_case
}

case_page_heartbeat() {
	new_case page_heartbeat
	start_master
	start_server HYPERSOMNIA_TOURNAMENT=2 --
	check "the list gives the page" list "any(e.get('tournament_page')=='http://127.0.0.1:$PORT/' for e in l)"
	check "the page answers" curl -sf --max-time 3 http://127.0.0.1:$PORT/
	stop_server

	# a stopped server says goodbye: the list is empty until the next one's heartbeat
	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=off --
	check "off: no page in the list" list "P($PORT) and all(not e.get('tournament_page') for e in l)"
	check "off: no page served" bash -c "! curl -sf --max-time 3 http://127.0.0.1:$PORT/"
	stop_server

	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=$P_PAGE --
	check "its own port: in the list" list "any(e.get('tournament_page')=='http://127.0.0.1:$P_PAGE/' for e in l)"
	check "its own port: served" curl -sf --max-time 3 http://127.0.0.1:$P_PAGE/tournament.json
	stop_server

	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=abc --
	check "a wrong port stops the start" in_log "expected off or a port"
	stop_server

	# the page's port taken by something else: the tournament goes on, the list gives no page
	timeout 300 python3 -m http.server $((P_PAGE + 1)) --bind 0.0.0.0 > /dev/null 2>&1 &
	local blocker=$!
	wait_until 10 curl -s --max-time 1 http://127.0.0.1:$((P_PAGE + 1))/
	start_server HYPERSOMNIA_TOURNAMENT=2 HYPERSOMNIA_TOURNAMENT_PAGE=$((P_PAGE + 1)) --
	check "a taken port: not served, said so" in_log "could not take TCP port $((P_PAGE + 1))"
	check "a taken port: no page in the list" list "P($PORT) and all(not e.get('tournament_page') for e in l)"
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
	start_server -- --tournament-teams "alice; abcdefghijabcdefghijabcdefghijabcdefghijX"
	check "a listed nickname nobody can join as (41 characters) doesn't start" in_log "no player can join as"
	stop_server
	start_server -- --tournament-teams "alice; bob" --tournament-arenas de_silo
	check "an old flag doesn't start - and says its new name" in_log "tournament-arenas is no longer read - it's --tournament-maps now"
	stop_server
	stop_master
}

case_join_timeout_equal() {
	new_case join_timeout_equal
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice, bob; carol, dave" --tournament-team-size 2 --tournament-round-1-max-wait 1
	client alice $PORT; sleep 2; client carol $PORT
	wait_log "the match starts with them" 220
	check "both teams here: the match starts with whoever came" in_log "the match starts with them"
	wait_count "Starting ranked" 1 90
	check "the match started 1v1" in_log "Starting ranked"
	wait_live
	kill_client carol
	wait_log "Tournament WINNER" 200
	check "alice and bob won - bob never came: still his listed nickname" in_log "Tournament WINNER: \[${ID[alice]}, nick_bob\]"
	finish_case
}

case_join_timeout_open() {
	# three from this machine: the network's limit of 2 raised
	MAX_PER_IP=3 new_case join_timeout_open
	player alice; player bob; player carol
	start_server HYPERSOMNIA_TOURNAMENT=4 HYPERSOMNIA_TOURNAMENT_ROUND_1_MAX_WAIT=1 --
	client alice $PORT; sleep 3; client bob $((PORT + 1))
	wait_count "takes a place" 2 60
	# carol only watches alice's match - never chooses a team
	NO_TEAM=1 client carol $PORT
	check "carol watches #1" wait_until 60 grep -q "${ID[carol]} comes in as a spectator" $SRV
	wait_count "Round timeout: round 1's match here closes" 2 220
	check "round 1 closed by the timeout" in_log "Round timeout: round 1's match here closes"
	wait_log "Tournament round 2: running" 60
	# logged by the final's instance a moment after "round 2: running"
	check "the final: alice vs bob" wait_until 10 in_log "Match: \[alice\] vs \[bob\]\|Match: \[bob\] vs \[alice\]"
	check "carol, only spectating, in no team" bash -c "! grep -q '${ID[carol]} takes a place' $SRV"
	kill_all_clients
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
	kill_client bob
	wait_log "Tournament WINNER" 200
	check "alice won" winner_is alice
	check "his game never told \"wrong match\" or \"too late\"" bash -c "! grep -q 'Connected to a wrong match\|Joined too late' $T/bob.*.out"
	finish_case
}

case_break_timeout() {
	new_case break_timeout
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-round-1-max-wait off --tournament-next-rounds-max-wait 1
	client alice $PORT; sleep 2; client bob $PORT; sleep 2; client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 2 120
	wait_live
	kill_client bob; kill_client dave
	wait_log "Tournament round 2: running" 150
	kill_client alice; kill_client carol
	# the break: nothing decided right away - the final waits its minute (+ 2 of grace) after round 1's end
	sleep 90
	check "the break: the final not decided after 90 s (1 minute + grace)" not_in_log "Round timeout: "
	client alice $PORT
	wait_log "Round timeout: " 220
	check "after the break, carol (not here) is out" in_log "a team with nobody here is out - the other wins by walkover"
	wait_log "Tournament WINNER" 60
	check "alice the winner" winner_is alice
	finish_case
}

case_walkover_b() {
	new_case walkover_b
	player alice; player bob; player carol; player dave
	start_server -- --tournament-teams "alice; bob; carol; dave" --tournament-round-1-max-wait 1
	client bob $PORT; sleep 2
	client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 1 90
	wait_log "the other wins by walkover" 220
	check "page: bob's walkover, as team b, while carol-dave play" page 'any(m["status"]=="walkover" and m["winner"]=="b" and N(m["team_b"])==["bob"] for m in R(0)["matches"]) and R(0)["state"]=="PLAYING"'
	check "page: the timeouts in the config" page 't["config"]["round_1_max_wait_minutes"]==1 and t["config"]["next_rounds_max_wait_minutes"]==15'
	finish_case
}

case_open_after_played() {
	new_case open_after_played
	player alice; player bob
	start_server HYPERSOMNIA_TOURNAMENT=4 --
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 1 90
	wait_live
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
	wait_live
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
	wait_live
	check "by accounts: alice's avatar by her key_ id" bash -c "[ \$(curl -s -o /dev/null -w '%{http_code}' 'http://127.0.0.1:$PORT/avatar?account=${ID[alice]}') = 200 ]"
	check "by accounts: the same by 0x..." bash -c "[ \$(curl -s -o /dev/null -w '%{http_code}' 'http://127.0.0.1:$PORT/avatar?account=0x${ID[alice]#key_}') = 200 ]"
	check "by accounts: the page names her account" page 'any(p["account"]=="'${ID[alice]}'" for r in t["rounds"] for m in r["matches"] for p in m["team_a"]+m["team_b"])'
	finish_case
}

case_page_json() {
	new_case page_json
	player alice; player bob; player carol
	python3 - $T/webhook.json $P_HOOK > $T/webhook.out 2>&1 <<'PY' &
import sys, http.server
class H(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        body = self.rfile.read(int(self.headers['Content-Length']))
        open(sys.argv[1], 'ab').write(body + b'\n')
        open(sys.argv[1] + '.body', 'wb').write(body)
        open(sys.argv[1] + '.key', 'w').write(self.headers.get('X-Hypersomnia-Server-Key', ''))
        open(sys.argv[1] + '.sig', 'w').write(self.headers.get('X-Hypersomnia-Signature', ''))
        self.send_response(200); self.end_headers()
http.server.HTTPServer(('127.0.0.1', int(sys.argv[2])), H).serve_forever()
PY
	local listener=$!
	sleep 1
	start_server HYPERSOMNIA_TOURNAMENT_NAME="Test Cup" HYPERSOMNIA_TOURNAMENT_RESULTS_WEBHOOK=http://127.0.0.1:$P_HOOK/results -- --tournament-teams "alice; bob; carol"
	check "page: name, ONGOING, config" page 't["name"]=="Test Cup" and t["state"]=="ONGOING" and t["winner_team"]==[] and {k: v for k, v in t["config"].items() if k!="maps"}=={"who_participates":"rosters by nicknames","team_size":1,"teams":3,"round_1_max_wait_minutes":30,"next_rounds_max_wait_minutes":15}'
	check "page: the final first, COMING; round 1 PLAYING" page 't["rounds"][0]["name"]=="Final" and t["rounds"][0]["state"]=="COMING" and R(0)["state"]=="PLAYING" and "score_a" not in R(0)["matches"][0]'
	check "page: a waiting match has its server" page 'R(0)["matches"][0]["status"]=="waiting" and R(0)["matches"][0]["port"]>0'
	check "page: the round's deadline (30 min + 2 of grace from now) for its countdown" page 'R(0)["grace_secs"]==120 and abs(R(0)["deadline"]-time.time()-32*60)<60'
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
	check "page: bob and carol both in, the match being played or starting" page 'R(0)["matches"][0]["status"] in ("starting","live")'
	wait_live
	kill_client carol
	wait_log "Tournament round 2: running" 120
	kill_client bob; sleep 2
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	wait_live
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
	start_server HYPERSOMNIA_TOURNAMENT_NAME="Test Cup" HYPERSOMNIA_TOURNAMENT_RESULTS_WEBHOOK=http://127.0.0.1:$P_HOOK/results -- --tournament-teams "alice; bob; carol"
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

# a persistent server for the admission cases - no tournament, no bots
start_persistent() {
	server_extra "$1"
	start_server -- 
}

case_admission_allowed() {
	new_case admission_allowed
	player alice; player bob; player carol
	start_master
	start_persistent "{ \"server\": { \"bot_quota\": [0] }, \"server_private\": { \"allowed_accounts\": [ \"${ID[alice]}\" ], \"admin_profiles\": [ { \"account\": \"${ID[carol]}\" } ] } }"
	client bob $PORT
	wait_log "is not among the listed players" 60
	check "bob refused" in_log "${ID[bob]} is not among the listed players"
	client alice $PORT; sleep 2; client carol $PORT
	wait_log "Adding alice to game state" 60
	wait_log "Adding carol to game state" 60
	check "alice in" in_log "Adding alice to game state"
	check "carol (an admin) in" in_log "Adding carol to game state"
	check "a padlock on the list, no password" list "l and all(e.get('listed_players_only') and not e.get('require_password') for e in l)"
	stop_master
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
	client alice $PORT; sleep 3; client bob $PORT
	wait_count "Adding .* to game state" 2 60
	client dave $PORT
	wait_log "The server is full" 60
	check "dave refused - full" in_log "The server is full"
	client carol $PORT
	wait_log "Adding carol to game state" 60
	check "carol (an admin) in on the full server" in_log "Adding carol to game state"
	check "only dave refused" bash -c "[ \$(grep -c '(reason=The server is full' $SRV) = 1 ]"
	finish_case
}

case_watch_limit_admins() {
	new_case watch_limit_admins
	player alice; player bob; player carol; player dave
	server_extra "{ \"server_private\": { \"admin_profiles\": [ { \"account\": \"${ID[carol]}\", \"rcon\": \"MASTER_RCON\" } ] } }"
	start_server HYPERSOMNIA_SPECTATOR=${ID[dave]} -- --tournament-teams "alice; bob" --tournament-spectator-slots 1
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
	wait_log "may not take the listed nickname" 60
	check "the same key refused as carol" in_log "may not take the listed nickname carol"
	check "carol not claimed" not_in_log "takes the listed nickname carol"
	finish_case
}

case_password_known() {
	new_case password_known
	player alice; player bob; player carol; player dave; player mallory
	start_server HYPERSOMNIA_PASSWORD=secret -- --tournament-teams "alice=${ID[alice]}; bob=${ID[bob]}; carol=${ID[carol]}; dave=${ID[dave]}"
	# before any match runs - a running one refuses outsiders already at the handshake, before the password
	client mallory $((PORT + 1))
	wait_log "Wrong password" 60
	check "mallory (nobody's) told the password is wrong" in_log "Wrong password"
	kill_client mallory
	client carol $((PORT + 1)); sleep 2; client dave $((PORT + 1))
	wait_count "Starting ranked" 1 90
	client alice $PORT; sleep 2; client bob $PORT
	wait_count "Starting ranked" 2 90
	check "the players in without the password" bash -c "[ \$(grep -c 'Wrong password' $SRV) = 1 ]"
	wait_live
	kill_client bob
	wait_log "abandoned by faction" 60
	sleep 3
	client bob $((PORT + 1))
	wait_count "watches the tournament's match as a spectator" 1 60
	check "bob (out) watches carol-dave without the password" bash -c "grep -q \"T$((PORT + 1))] Adding bob to game state\" $SRV && [ \$(grep -c 'Wrong password' $SRV) = 1 ]"
	finish_case
}

case_mixed_teams() {
	new_case mixed_teams
	start_server -- --tournament-teams "$(printf 'alice, bob\n\ncarol, dave')"
	check "a team per line and an empty line: doesn't start" in_log "not both"
	stop_server
}

[ -n "$(echo $CASES)" ] || { echo "no cases to run (SHARD=$SHARD of SHARDS=$SHARDS, CASES=\"$CASES\")"; exit 1; }

for c in $CASES; do
	declare -F case_$c > /dev/null || { CASE=$c; echo "FAIL  $c: no such case" | tee -a $RESULTS; continue; }
	case_$c
done

echo
echo "=== results ($SUITE)"
cat $RESULTS
echo "$(grep -c '^PASS' $RESULTS) passed, $(grep -c '^FAIL' $RESULTS) failed"
[ "$(grep -c '^FAIL' $RESULTS)" = 0 ]

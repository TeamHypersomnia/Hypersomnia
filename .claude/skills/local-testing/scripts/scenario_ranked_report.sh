#!/bin/bash
# A whole ranked match to its natural end, in about a minute: the test map de_e2e_short (de_cyberaqua with a max team score
# of 2 - the least there is, so 2 rounds - a 5 s round, no warmup) on a ranked server, two key clients under Xvfb that click
# Auto-assign in the faction window, the report signed and sent to the local broker (broker.sh), which knows the server
# as official "e2e". Then the site's database: the match and both players' rows.
# Not a tournament server: tournaments force overtime (win by two), and idle players only ever win their defending rounds
# (time out, no bomb), so the match would never end. Without overtime 1:1 after both halves ends it.
# STEAM_BOB=1: bob plays on the Steam build with the user's running Steam (the broker needs STEAM_API_KEY_FILE).
# ALICE_FROM_MENU=1: alice starts in the main menu with the site's leaderboards, joins through Connect to server,
# and goes back to the menu after the match - the leaderboards must be downloaded again (2 downloads), showing her new MMR.
# REJOIN=1: alice is killed (SIGKILL) during the match and her game is started again after REJOIN_AFTER (5) seconds, without
# --connect - user/ranked_match.json must bring her back into the same match, and the report must still have her
# (use a longer ROUND_TIME, e.g. 15).
# AVATAR=<png>: both play with this avatar - the report carries it, the site keeps it for the accounts.
# UNOFFICIAL=1: the site doesn't know the server - the report is refused (401) once and kept in user/unsent_reports
# (.json + .headers); then the key goes into official_servers.d and the kept report is posted with curl - it must count.
# Usage: [MAX_TEAM_SCORE=2] [ROUND_TIME=5] [STEAM_BOB=1] [ALICE_FROM_MENU=1] [REJOIN=1] [AVATAR=<png>] scenario_ranked_report.sh
. "$(dirname "$0")/test_env.sh"
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. "$HERE/lib.sh"
S=${SCRATCH:-/tmp/hypersomnia-nettest}
# A fresh directory every run - never removed recursively
T=$S/ranked_report_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] || { echo "start broker.sh and export BROKER_URL, BROKER_PUBLIC_KEY"; exit 1; }
if ss -tuln | grep -q ":10060 "; then echo "port 10060 busy, abort"; exit 1; fi

mkdir -p $T/server/user/conf.d $T/server/user/downloads/maps
cleanup_on_exit $T

# The test map: a copy of de_cyberaqua with tiny rules
short_ranked_map $T/server/user/downloads/maps ${MAX_TEAM_SCORE:-2} ${ROUND_TIME:-5}

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
}

ID_A=$(account_id alice)
ID_B=$(account_id bob)
[ -n "$STEAM_BOB" ] && ID_B=steam

cat > $T/server/user/conf.d/test.json <<J
{ "instances": { "ranked": 1 }, "server_start": { "ip": "0.0.0.0", "port": 10060, "slots": 16 },
  "server": { "server_name": "[PL] Report test", "map_cycle": [ "de_e2e_short" ], "game_mode": "bomb_defusal", "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "sync_all_external_arenas_on_startup": false,
              "ranked": { "countdown_time": 2 },
              "move_to_spectators_if_afk_for_secs": 999999, "kick_if_afk_for_secs": 999999,
              "webrtc_port_range_begin": 19600, "webrtc_port_range_end": 19605 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "$BROKER_URL/report_match" } }
J

cd $REPO/hypersomnia

# Official on the site by its key
KEY=$($BIN --dedicated-server --print-server-key --appdata-dir $T/server 2>/dev/null | grep -E '^[0-9a-f]{64}$')
[ -z "$UNOFFICIAL" ] && echo "$KEY" > $S/broker_site/private/official_servers.d/e2e

timeout 180 $BIN --dedicated-server --appdata-dir $T/server $GAME_TEST_FLAGS > $T/server.out 2>&1 &
wait_listening $T/server.out

AVATAR_JSON=""
[ -n "$AVATAR" ] && AVATAR_JSON=", \"avatar_image_path\": \"$AVATAR\""

for c in alice bob; do
	mkdir -p $T/$c/user/conf.d
	cat > $T/$c/user/conf.d/test.json <<J
{ "identity_broker_url": "$BROKER_URL", "server_list_provider": "http://127.0.0.1:1", "client": { "nickname": "$c"$AVATAR_JSON } }
J
done

# Each client in its own Xvfb, clicking Auto-assign (the faction window's button at 585,430 in 1280x720) a few times as it loads
DISPLAYS=""
D_A=$(free_display 86)
D_B=$(free_display $((D_A + 1)))
for c in alice:$D_A bob:$D_B; do
	name=${c%%:*}; d=${c##*:}
	timeout 300 Xvfb :$d -screen 0 1280x720x24 > /dev/null 2>&1 &
	DISPLAYS="$DISPLAYS $!"
	cleanup_also $!
	wait_until 5 test -e /tmp/.X$d-lock
	CBIN=$BIN
	[ "$name" = bob ] && [ -n "$STEAM_BOB" ] && CBIN=$REPO/build/Debug-x64-clang-claude-steam/Hypersomnia

	if [ "$name" = alice ] && [ -n "$ALICE_FROM_MENU" ]; then
		cat > $T/alice/user/conf.d/menu.json <<J
{ "launch_at_startup": "MAIN_MENU", "window": { "fullscreen": false, "size": [1280, 720] }, "main_menu": { "leaderboards_provider_url": "$BROKER_URL/leaderboards" } }
J
		(cd $REPO/hypersomnia && DISPLAY=:$d timeout 200 $CBIN --appdata-dir $T/$name $GAME_TEST_FLAGS --mute > $T/$name.out 2>&1 &)
		sleep 12
		DISPLAY=:$d import -window root $T/alice_menu_before.png
		DISPLAY=:$d python3 "$HERE/xinput.py" click 199 323; sleep 2
		DISPLAY=:$d python3 "$HERE/xinput.py" click 455 340
		DISPLAY=:$d python3 "$HERE/xinput.py" type "127.0.0.1:10060"
		DISPLAY=:$d python3 "$HERE/xinput.py" click 380 524
	else
		(cd $REPO/hypersomnia && DISPLAY=:$d timeout 150 $CBIN --connect 127.0.0.1:10060 --appdata-dir $T/$name $GAME_TEST_FLAGS --mute > $T/$name.out 2>&1 &)
	fi
done

# Auto-assign until the match starts (at most 6 tries, 5 s apart)
for t in 1 2 3 4 5 6; do
	wait_until 5 grep -q "Starting ranked\.$" $T/server.out && break
	for d in $D_A $D_B; do DISPLAY=:$d python3 "$HERE/xinput.py" click 585 430; done
done

for d in $D_A $D_B; do DISPLAY=:$d import -window root $T/joined_$d.png; done

if [ -n "$REJOIN" ]; then
	kill -9 $(pgrep -f "^$BIN --connect 127.0.0.1:10060 --appdata-dir $T/alice ")
	echo "alice killed at $(date +%T)"
	sleep ${REJOIN_AFTER:-5}
	# No --connect: user/ranked_match.json brings the game back to the match by itself, on the account it was played on
	echo "alice's ranked_match.json: $(cat $T/alice/user/ranked_match.json 2>/dev/null | sed 's/key_[0-9a-f]\{56\}/key_.../')"
	(cd $REPO/hypersomnia && DISPLAY=:$D_A timeout 150 $BIN --appdata-dir $T/alice $GAME_TEST_FLAGS --mute > $T/alice_rejoin.out 2>&1 &)
	echo "alice rejoins at $(date +%T)"
	for t in 4 8 12; do sleep 4; DISPLAY=:$D_A import -window root $T/rejoin_$t.png; done
fi

for i in $(seq 1 140); do
	grep -q "delivered (HTTP\|not delivered\|refused the match report\|Not an official server" $T/server.out && break
	sleep 1
done

if [ -n "$ALICE_FROM_MENU" ]; then
	sleep 3
	DISPLAY=:$D_A python3 "$HERE/xinput.py" key Escape; sleep 2
	DISPLAY=:$D_A python3 "$HERE/xinput.py" click 179 613; sleep 6
	DISPLAY=:$D_A import -window root $T/alice_menu_after.png
	echo "alice's leaderboards downloads (2 expected - at start and after the ranked match): $(grep -c 'Launching future_response' $T/alice.out)"
fi

echo "=== server ($i s)"
grep -hE "Authenticated|Starting ranked|Match report JSON|delivered|refused|ERROR" $T/server.out | sed 's/^\[[0-9:]*\] //; s/key_[0-9a-f]\{56\}/key_…/g' | cut -c1-160 | head -12
if [ -n "$UNOFFICIAL" ]; then
	echo "=== unsent (refused once, kept): $(ls $T/server/user/unsent_reports/ 2>/dev/null | tr '\n' ' ')"
	sleep 20
	echo "    retries after the 401 (0 expected): $(grep -c 'Report .*: HTTP 401' $T/server.out) request(s) in all"
	echo "$KEY" > $S/broker_site/private/official_servers.d/e2e
	sleep 2
	for f in $T/server/user/unsent_reports/*.json; do
		echo "    posted by hand: HTTP $(curl -s -o /dev/null -w '%{http_code}' -H @${f%.json}.headers -H 'Content-Type: application/json' --data-binary @$f $BROKER_URL/report_match)"
	done
fi

echo "=== site"
"$HERE/broker.sh" sql "SELECT server_id, win_score, lose_score, report_id FROM matches ORDER BY match_id DESC LIMIT 1"
"$HERE/broker.sh" sql "SELECT account_id, round(mmr,2) AS mmr, matches_won, matches_lost FROM mmr_team WHERE account_id IN ('$ID_A', '$ID_B') OR ('$ID_B' = 'steam' AND account_id LIKE 'steam_%')"

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
kill $DISPLAYS 2>/dev/null
rm -f $S/broker_site/private/official_servers.d/e2e

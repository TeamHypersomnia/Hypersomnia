#!/bin/bash
# A whole ranked match to its natural end, in about a minute: the test map de_e2e_short (de_cyberaqua with a max team score
# of 2 - the least there is, so 2 rounds - a 5 s round, no warmup) on a ranked server, two key clients under Xvfb that click
# Auto-assign in the faction window, the report signed and sent to the local broker (broker.sh), which knows the server
# as official "e2e". Then the site's database: the match and both players' rows.
# Not a tournament server: tournaments force overtime (win by two), and idle players only ever win their defending rounds
# (time out, no bomb), so the match would never end. Without overtime 1:1 after both halves ends it.
# Usage: [MAX_TEAM_SCORE=2] [ROUND_TIME=5] scenario_ranked_report.sh
export ALSOFT_DRIVERS=null VISUAL=true
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/ranked_report
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] || { echo "start broker.sh and export BROKER_URL, BROKER_PUBLIC_KEY"; exit 1; }
if ss -tuln | grep -q ":10060 "; then echo "port 10060 busy, abort"; exit 1; fi

rm -rf $T
mkdir -p $T/server/user/conf.d $T/server/user/downloads/arenas

# The test map: a copy of de_cyberaqua with tiny rules
A=$T/server/user/downloads/arenas/de_e2e_short
cp -r $REPO/hypersomnia/content/arenas/de_cyberaqua $A
mv $A/de_cyberaqua.json $A/de_e2e_short.json
rm -f $A/last_saved.json $A/editor_view.json
python3 - $A/de_e2e_short.json ${MAX_TEAM_SCORE:-2} ${ROUND_TIME:-5} <<'PY'
import json, sys
p = sys.argv[1]
d = json.load(open(p))
d['meta']['name'] = 'de_e2e_short'
d['game_modes']['bomb_defusal'].update({
	'max_team_score': int(sys.argv[2]), 'warmup_time': 0, 'freeze_time': 1, 'buy_time': 1,
	'round_time': int(sys.argv[3]), 'round_end_time': 1.0, 'default_bot_quota': 0
})
json.dump(d, open(p, 'w'), indent=1)
PY

account_id() {
	mkdir -p $T/$1/user
	echo "hypersomnia-key-v1:$(openssl rand -hex 32)" > $T/$1/user/account.key
	python3 - $T/$1/user/account.key <<'PY'
import sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
print('key_' + Ed25519PrivateKey.from_private_bytes(seed).public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex())
PY
}

ID_A=$(account_id alice)
ID_B=$(account_id bob)

cat > $T/server/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": 10060, "slots": 16 },
  "server": { "server_name": "[PL] Report test", "arena": "de_e2e_short", "game_mode": "bomb_defusal", "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "sync_all_external_arenas_on_startup": false,
              "ranked": { "autostart_when": "TEAMS_EQUAL", "countdown_time": 2 },
              "move_to_spectators_if_afk_for_secs": 999999, "kick_if_afk_for_secs": 999999,
              "webrtc_port_range_begin": 19600, "webrtc_port_range_end": 19605 },
  "server_private": { "identity_broker_public_key": "$BROKER_PUBLIC_KEY", "report_ranked_match_url": "$BROKER_URL/report_match" } }
J

cd $REPO/hypersomnia

# Official on the site by its key
KEY=$($BIN --print-server-key --appdata-dir $T/server 2>/dev/null | grep -E '^[0-9a-f]{64}$')
echo "$KEY" > $S/broker_site/private/official_servers.d/e2e

timeout 180 $BIN --dedicated-server --appdata-dir $T/server --no-update-on-launch > $T/server.out 2>&1 &
sleep 8

for c in alice bob; do
	mkdir -p $T/$c/user/conf.d
	cat > $T/$c/user/conf.d/test.json <<J
{ "identity_broker_url": "$BROKER_URL", "server_list_provider": "http://127.0.0.1:1", "client": { "nickname": "$c" } }
J
done

# Each client in its own Xvfb, clicking Auto-assign (the faction window's button at 585,430 in 1280x720) a few times as it loads
DISPLAYS=""
for c in alice:86 bob:87; do
	name=${c%%:*}; d=${c##*:}
	Xvfb :$d -screen 0 1280x720x24 > /dev/null 2>&1 &
	DISPLAYS="$DISPLAYS $!"
	sleep 1
	(cd $REPO/hypersomnia && DISPLAY=:$d timeout 150 $BIN --connect 127.0.0.1:10060 --appdata-dir $T/$name --no-update-on-launch > $T/$name.out 2>&1 &)
done

for t in 1 2 3 4 5 6; do
	sleep 5
	for d in 86 87; do DISPLAY=:$d python3 "$HERE/xinput.py" click 585 430; done
done

for i in $(seq 1 140); do
	grep -q "delivered (HTTP\|not delivered\|refused the match report" $T/server.out && break
	sleep 1
done

echo "=== server ($i s)"
grep -hE "Authenticated|Starting ranked|Match report JSON|delivered|refused|ERROR" $T/server.out | sed 's/^\[[0-9:]*\] //; s/key_[0-9a-f]\{56\}/key_…/g' | cut -c1-160 | head -12
echo "=== site"
"$HERE/broker.sh" sql "SELECT server_id, win_score, lose_score, report_id FROM matches ORDER BY match_id DESC LIMIT 1"
"$HERE/broker.sh" sql "SELECT account_id, round(mmr,2) AS mmr, matches_won, matches_lost FROM mmr_team WHERE account_id IN ('$ID_A', '$ID_B')"

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
kill $DISPLAYS 2>/dev/null
rm -f $S/broker_site/private/official_servers.d/e2e

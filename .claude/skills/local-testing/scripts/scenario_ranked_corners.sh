#!/bin/bash
# Ranked corners of a linked account, mid-match (CASE=unlink or CASE=broker_down):
#   alice: a key linked to a fake Steam account (straight in the site's DB), signed in through a broker proxy (slow_broker.py,
#          no delay) that can be stopped; bob: a key account. A ranked server on the 2-round test map (as scenario_ranked_report.sh).
#   Once the match is live:
#     unlink      - alice's key is unlinked on the site (as /profile would), then alice is killed and started again;
#     broker_down - the proxy is stopped (hypersomnia.io down), then alice is killed and started again.
#   Expected: broker_down - alice rejoins as the same Steam account (her saved steam.cert), the match ends and the report names
#   that account. unlink - nothing holds the Steam cert for the match any more: alice rejoins with it only if she connects
#   before the broker says the key is unlinked; otherwise the server refuses her key account (an accepted loss, two rare events).
# Needs broker.sh running (BROKER_URL, BROKER_PUBLIC_KEY exported, SCRATCH as for broker.sh).
# Resources: port 18710 (+ WebRTC 19710-19715), broker proxy 13710, displays :96 :97.
# Usage: CASE=unlink|broker_down [ROUND_TIME=15] scenario_ranked_corners.sh
export ALSOFT_DRIVERS=null VISUAL=true
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/ranked_corners_${CASE:-unlink}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
PORT=18710
PROXY=13710
STEAM_ID=steam_7656119000000$(printf '%04d' $((RANDOM % 10000)))
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] || { echo "start broker.sh and export BROKER_URL, BROKER_PUBLIC_KEY"; exit 1; }
for p in $PORT $PROXY; do if ss -tuln | grep -q ":$p "; then echo "port $p busy, abort"; exit 1; fi; done

rm -rf $T
mkdir -p $T/server/user/conf.d $T/server/user/downloads/arenas

A=$T/server/user/downloads/arenas/de_e2e_short
cp -r $REPO/hypersomnia/content/arenas/de_cyberaqua $A
mv $A/de_cyberaqua.json $A/de_e2e_short.json
rm -f $A/last_saved.json $A/editor_view.json
python3 - $A/de_e2e_short.json 2 ${ROUND_TIME:-15} <<'PY'
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

# alice's key linked to the fake Steam account, as a link in the browser would
(cd $S/broker_site && node -e '
const db = require("./src/db");
db.prepare("INSERT OR IGNORE INTO associations (child_id, parent_id) VALUES (?, ?)").run(process.argv[1], process.argv[2]);
db.prepare("INSERT OR REPLACE INTO key_devices (child_id, device, os, last_cert_at) VALUES (?, ?, ?, ?)").run(process.argv[1], "corners", "linux", 1);
process.exit(0);' $ID_A $STEAM_ID)

cat > $T/server/user/conf.d/test.json <<J
{ "instances": { "ranked": 1 }, "server_start": { "ip": "0.0.0.0", "port": $PORT, "slots": 16 },
  "server": { "server_name": "[PL] Corners test", "arena": "de_e2e_short", "game_mode": "bomb_defusal", "notified_server_list": "127.0.0.1:1",
              "suppress_new_community_server_webhook": true, "sync_all_external_arenas_on_startup": false,
              "ranked": { "start_when": "TEAMS_EQUAL", "countdown_time": 2 },
              "move_to_spectators_if_afk_for_secs": 999999, "kick_if_afk_for_secs": 999999,
              "webrtc_port_range_begin": 19710, "webrtc_port_range_end": 19715 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "$BROKER_URL/report_match" } }
J

cd $REPO/hypersomnia

KEY=$($BIN --print-server-key --appdata-dir $T/server 2>/dev/null | grep -E '^[0-9a-f]{64}$')
echo "$KEY" > $S/broker_site/private/official_servers.d/corners

# The broker as alice sees it: a proxy that can be stopped
(setsid python3 "$HERE/identity_ui/slow_broker.py" $PROXY 0 > $T/proxy.out 2>&1 < /dev/null &)
sleep 1

timeout 300 $BIN --dedicated-server --appdata-dir $T/server --no-update-on-launch > $T/server.out 2>&1 &
sleep 8

mkdir -p $T/alice/user/conf.d $T/bob/user/conf.d
echo "{ \"identity_broker_url\": \"http://127.0.0.1:$PROXY\", \"server_list_provider\": \"http://127.0.0.1:1\", \"client\": { \"nickname\": \"alice\" } }" > $T/alice/user/conf.d/test.json
echo "{ \"identity_broker_url\": \"$BROKER_URL\", \"server_list_provider\": \"http://127.0.0.1:1\", \"client\": { \"nickname\": \"bob\" } }" > $T/bob/user/conf.d/test.json

start_client() {
	local name=$1 d=$2 out=$3; shift 3
	(cd $REPO/hypersomnia && DISPLAY=:$d setsid timeout 280 $BIN "$@" --appdata-dir $T/$name --no-update-on-launch > $T/$out 2>&1 < /dev/null &)
}

for d in 96 97; do (setsid Xvfb :$d -screen 0 1280x720x24 > /dev/null 2>&1 < /dev/null &); done
sleep 1

# alice first gets her Steam cert (the proxy is up) - a key put in by hand is asked about once (as an import) -
# and only then joins, as the Steam account
start_client alice 96 alice_first.out
for i in $(seq 1 40); do grep -q "Saved the Steam cert\|identity cert for steam_\|Linked" $T/alice_first.out 2>/dev/null && [ -f $T/alice/user/.keys/steam.cert ] && break; sleep 1; done
echo "alice's first start: $(grep -o 'identity cert for [a-z_0-9]*' $T/alice_first.out | tail -1), steam.cert: $([ -f $T/alice/user/.keys/steam.cert ] && echo yes || echo no)"
for p in $(pgrep -f -- "Hypersomnia --appdata-dir $T/alice "); do kill $p; done
sleep 2
start_client alice 96 alice.out --connect 127.0.0.1:$PORT
start_client bob 97 bob.out --connect 127.0.0.1:$PORT

for t in 1 2 3 4 5 6; do
	sleep 5
	for d in 96 97; do DISPLAY=:$d python3 "$HERE/xinput.py" click 585 430; done
done

for i in $(seq 1 30); do grep -q "Starting ranked\|ranked match started\|Match started" $T/server.out && break; sleep 1; done
echo "alice's cert: $(grep -o 'identity cert for [a-z_0-9]*' $T/alice.out | tail -1)"
echo "alice's ranked_match.json: $(cat $T/alice/user/ranked_match.json 2>/dev/null)"
DISPLAY=:96 import -window root $T/live.png

case ${CASE:-unlink} in
unlink)
	(cd $S/broker_site && node -e 'require("./src/link_key").unlink(process.argv[1], process.argv[2]); console.log("unlinked on the site"); process.exit(0)' $ID_A $STEAM_ID)
	;;
broker_down)
	for p in $(lsof -ti tcp:$PROXY -sTCP:LISTEN); do kill $p; done
	echo "the broker proxy is down"
	;;
esac

sleep 3
for p in $(pgrep -f -- "Hypersomnia --connect 127.0.0.1:$PORT --appdata-dir $T/alice "); do kill -9 $p; done
echo "alice killed at $(date +%T)"
sleep 5
start_client alice 96 alice_rejoin.out
echo "alice started again at $(date +%T)"
for t in 4 8 12 16; do sleep 4; DISPLAY=:96 import -window root $T/rejoin_$t.png; done

for i in $(seq 1 180); do
	grep -q "delivered (HTTP\|not delivered\|refused the match report" $T/server.out && break
	sleep 1
done

echo "=== alice after the restart"
grep -hE "Rejoin|rejoin|identity cert|saved cert|Not rejoining|kept until|no longer linked|Connecting as" $T/alice_rejoin.out | sed 's/^\[[0-9:]*\] //' | head -8
echo "=== server"
grep -hE "Authenticated|Starting ranked|abandon|delivered|refused|ERROR" $T/server.out | sed 's/^\[[0-9:]*\] //; s/key_[0-9a-f]\{56\}/key_…/g' | cut -c1-160 | head -12
echo "=== site (alice's Steam account $STEAM_ID, alice's key ${ID_A:0:12}…, bob ${ID_B:0:12}…)"
"$HERE/broker.sh" sql "SELECT account_id, round(mmr,2) AS mmr, matches_won, matches_lost FROM mmr_team WHERE account_id IN ('$ID_A', '$ID_B', '$STEAM_ID')"

for p in $(pgrep -f -- "--appdata-dir $T/"); do kill $p; done
for p in $(lsof -ti tcp:$PROXY -sTCP:LISTEN); do kill $p; done
for d in 96 97; do for p in $(pgrep -f "^Xvfb :$d "); do kill $p; done; done
rm -f $S/broker_site/private/official_servers.d/corners

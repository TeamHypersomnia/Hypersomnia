#!/bin/bash
# One end-to-end ranked match: dedicated server on arena (match.sh), clients here, backend on arenaus.
# The second client is killed once the match is live, so it abandons and the match ends with a report at once.
# The clients play with account keys made here (or ones already in their folders); the test broker on arenaus gives them certs.
# Usage: run_match.sh <client A> <client B> [live wait secs]
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
export ALSOFT_DRIVERS=null
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
E=${SCRATCH:-/tmp/hypersomnia-nettest}/e2e
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
ARENA_IP=${ARENA_IP:-57.128.242.21}
ARENAUS_IP=${ARENAUS_IP:-154.12.226.211}
A=$1
B=$2
LIVE_WAIT=${3:-35}

for p in $(pgrep -f "^$BIN --connect $ARENA_IP:9005"); do kill $p; done

# An account key per client, its account id from the public key
account_id() {
	local key=$E/client_$1/user/account.key
	mkdir -p $(dirname $key)
	[ -f $key ] || (umask 077; echo "hypersomnia-key-v1:$(openssl rand -hex 32)" > $key)
	python3 - $key <<'PY'
import sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
print('key_' + Ed25519PrivateKey.from_private_bytes(seed).public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex())
PY
}

ID_A=${ID_A:-$(account_id $A)}
ID_B=${ID_B:-$(account_id $B)}

# The ids go into a remote shell command and a JSON file there
for id in "$ID_A" "$ID_B"; do
	[[ "$id" =~ ^[A-Za-z0-9_]+$ ]] || { echo "invalid account id: $id"; exit 1; }
done

# The clients ask the test backend for the list and the certs, never production
for c in $A $B; do
	mkdir -p $E/client_$c/user/conf.d
	cat > $E/client_$c/user/conf.d/test.json <<J
{ "server_list_provider": "http://$ARENAUS_IP:18420", "identity_broker_url": "http://$ARENAUS_IP:13000",
  "webrtc_signalling_server_url": "ws://$ARENAUS_IP:18000", "website_url": "http://$ARENAUS_IP:13000", "client": { "nickname": "$c" } }
J
done

BROKER_KEY=$(timeout 60 ssh arenaus "bash ~/hs-test-backend/backend.sh broker_key")
[[ "$BROKER_KEY" =~ ^[0-9a-f]{64}$ ]] || { echo "no broker key from arenaus: $BROKER_KEY"; exit 1; }

# Official on the test masterserver by its key
SERVER_KEY=$(timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh key")
[[ "$SERVER_KEY" =~ ^[0-9a-f]{64}$ ]] || { echo "no server key from arena: $SERVER_KEY"; exit 1; }
timeout 60 ssh arenaus "mkdir -p ~/hs-test-backend/official_servers.d && echo $SERVER_KEY > ~/hs-test-backend/official_servers.d/pl"

echo "=== server: $A ($ID_A) vs $B ($ID_B)"
timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh start $ID_A $ID_B $BROKER_KEY"
sleep 6

echo "=== test masterserver list"
curl -s --max-time 5 http://$ARENAUS_IP:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], e['ip'], 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'), 'domain=', e.get('domain'), 'shown=', e.get('site_displayed_address'))"

cd $REPO/hypersomnia
xvfb-run -a $BIN --connect $ARENA_IP:9005 --appdata-dir $E/client_$A --no-update-on-launch > $E/$A.out 2>&1 &
xvfb-run -a $BIN --connect $ARENA_IP:9005 --appdata-dir $E/client_$B --no-update-on-launch > $E/$B.out 2>&1 &

sleep $LIVE_WAIT

echo "=== killing $B (abandon)"
for p in $(pgrep -f "^$BIN --connect $ARENA_IP:9005 --appdata-dir $E/client_$B"); do kill -9 $p; done
sleep 15

echo "=== dedicated server log"
timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh log"

for p in $(pgrep -f "^$BIN --connect $ARENA_IP:9005"); do kill $p; done
timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh stop" > /dev/null
sleep 2

#!/bin/bash
# Admin rights of a key linked to a Steam account (its cert from the broker says the key proved it, not a Steam ticket),
# on a server with a password. Expected:
#   A: admin_profiles = the steam_ account only -> "Wrong password" (not an admin outside the Steam build)
#   B: admin_profiles = steam_ + the key_ -> joins without the password (an admin through its key)
# The client's first start answers [Link] to "is it yours?" (centered modal), which saves steam_cert.json.
# Prints "== case A: <n> wrong password, ..." lines. Needs broker.sh started with this SCRATCH.
export ALSOFT_DRIVERS=null VISUAL=true
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
# A fresh directory every run - never removed recursively
T=$S/admin_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
PORT=18720
STEAM_ID=steam_7656119000000$(printf '%04d' $((RANDOM % 10000)))
[ -n "$BROKER_URL" ] && [ -n "$BROKER_PUBLIC_KEY" ] || { echo "start broker.sh and export BROKER_URL, BROKER_PUBLIC_KEY"; exit 1; }
mkdir -p $T/alice/user/.keys $T/alice/user/conf.d
echo "hypersomnia-key-v1:$(openssl rand -hex 32)" > $T/alice/user/.keys/account.key
ID_A=$(python3 - $T/alice/user/.keys/account.key <<'PY'
import sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
print('key_' + Ed25519PrivateKey.from_private_bytes(seed).public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex())
PY
)
echo "$ID_A" > $T/alice_id
(cd $S/broker_site && node -e '
const db = require("./src/db");
db.prepare("INSERT OR IGNORE INTO associations (child_id, parent_id) VALUES (?, ?)").run(process.argv[1], process.argv[2]);
db.prepare("INSERT OR REPLACE INTO key_devices (child_id, device, os, last_cert_at) VALUES (?, ?, ?, ?)").run(process.argv[1], "admin", "linux", 1);
process.exit(0);' $ID_A $STEAM_ID)

echo "{ \"identity_broker_url\": \"$BROKER_URL\", \"server_list_provider\": \"http://127.0.0.1:1\", \"client\": { \"nickname\": \"alice\" },
  \"launch_at_startup\": \"MAIN_MENU\", \"skip_tutorial\": true, \"window\": { \"fullscreen\": false, \"size\": [1280, 720] } }" > $T/alice/user/conf.d/test.json
cp "$HERE/identity_ui/imgui_big.ini" $T/alice/user/imgui.ini

start_client() {
	local out=$1; shift
	(cd $REPO/hypersomnia && DISPLAY=:96 setsid timeout 120 $BIN "$@" --appdata-dir $T/alice --no-update-on-launch --mute > $T/$out 2>&1 < /dev/null &)
}
stop_client() {
	for p in $(pgrep -f -- "Hypersomnia --appdata-dir $T/alice"); do kill $p; done
	sleep 2
}

(setsid Xvfb :96 -screen 0 1280x720x24 > /dev/null 2>&1 < /dev/null &)
sleep 1
start_client first.out
for i in $(seq 1 40); do grep -q "identity cert for steam_" $T/first.out 2>/dev/null && break; sleep 1; done
sleep 2
for i in $(seq 1 10); do
	[ -f $T/alice/user/.keys/steam_cert.json ] && break
	DISPLAY=:96 python3 "$HERE/xinput.py" click 493 472
	sleep 1
done
echo "first start: steam_cert.json: $([ -f $T/alice/user/.keys/steam_cert.json ] && echo yes || echo no)"
stop_client

run_case() {
	local name=$1 profiles=$2
	local D=$T/srv_$name
	mkdir -p $D/user/conf.d
	cat > $D/user/conf.d/test.json <<J
{ "instances": { "ranked": 0 }, "server_start": { "ip": "0.0.0.0", "port": $PORT, "slots": 8 },
  "server": { "server_name": "admin $name", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19720, "webrtc_port_range_end": 19725 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "server_password": "secretpw", "admin_profiles": $profiles } }
J
	(cd $REPO/hypersomnia && setsid timeout 60 $BIN --dedicated-server --appdata-dir $D --no-update-on-launch > $D.out 2>&1 < /dev/null &)
	sleep 6
	start_client $name.out --connect 127.0.0.1:$PORT
	sleep 15
	echo "== case $name: $(grep -a -c 'Wrong password' $D.out) wrong password, $(grep -a -c 'Authenticated' $D.out) authenticated"
	stop_client
	for p in $(pgrep -f -- "--appdata-dir $D "); do kill $p; done
	sleep 2
}

run_case A "[ { \"account\": \"$STEAM_ID\", \"rcon\": \"MASTER_RCON\" } ]"
run_case B "[ { \"account\": \"$STEAM_ID\", \"rcon\": \"MASTER_RCON\" }, { \"account\": \"$ID_A\", \"rcon\": \"MASTER_RCON\" } ]"
for p in $(pgrep -f -- "Xvfb :96"); do kill $p; done

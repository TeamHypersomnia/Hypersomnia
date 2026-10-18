#!/bin/bash
# Local end-to-end test of the server keys, the handshake with certs and the identity broker.
# Everything on localhost, heartbeats only to the local test master, the broker is a scratch copy of the site.
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
. "$(dirname "$0")/test_env.sh"
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
umask 077
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
# A fresh directory every run - never removed recursively
T=$S/keys_test_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
GAME=$REPO/hypersomnia
# The broker: a site copy running on this port (see broker.sh), or empty for none
BROKER_URL=${BROKER_URL:-}
BROKER_PUBLIC_KEY=${BROKER_PUBLIC_KEY:-}

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
mkdir -p $T/official_servers.d

for port in 18000 18410 18420 18430 10000 10002 10004; do
	if ss -tuln | grep -q ":$port "; then echo "port $port is busy - aborting"; exit 1; fi
done

mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF

server_conf() {
mkconf $1 <<EOF
{
  "instances": { "ranked": $([ "$3" = NEVER ] && echo 0 || echo 1) }, "server_start": { "ip": "0.0.0.0", "port": $2, "slots": 16 },
  "server": { "server_name": "$1", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { } },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "server_password": "$4",
                      "report_ranked_match_url": "${BROKER_URL:-http://127.0.0.1:1}/report_match" }
}
EOF
}

server_conf casual_de 10000 NEVER ""
server_conf ranked_pl 10002 TEAMS_EQUAL ""
server_conf community 10004 NEVER "${COMMUNITY_PASSWORD:-}"

cd $GAME

# The official servers are known by their public keys
for s in casual_de ranked_pl; do
	key=$($BIN --print-server-key --appdata-dir $T/$s 2>/dev/null | grep -E '^[0-9a-f]{64}$')
	echo "$key" > $T/official_servers.d/$s
	echo "official $s: $key"
done

client_conf() {
mkconf $1 <<EOF
{
  "identity_broker_url": "${BROKER_URL:-http://127.0.0.1:1}",
  "server_list_provider": "http://127.0.0.1:18420",
  "window": { "fullscreen": false, "size": [1280, 720] },
  "client": { "nickname": "$1" }
}
EOF
}

timeout ${MS_SECS:-200} $BIN --masterserver --appdata-dir $T/ms $GAME_TEST_FLAGS > $T/ms.out 2>&1 &
sleep 3
for s in casual_de ranked_pl community; do
	timeout ${SERVER_SECS:-190} nice -n 10 $BIN --dedicated-server --appdata-dir $T/$s $GAME_TEST_FLAGS > $T/$s.out 2>&1 &
done
sleep 12

echo "=== heartbeat targets"
grep -h "Sending heartbeat through UDP to" $T/casual_de.out $T/ranked_pl.out $T/community.out | awk '{print $7}' | sort | uniq -c

echo "=== list"
curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], e['ip'], 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'), 'key=', e.get('server_public_key','')[:16])"

client_conf c_casual
client_conf c_ranked
client_conf c_community

run_client() {
	timeout $3 xvfb-run -a nice -n 10 $BIN --connect $2 --appdata-dir $T/$1 $GAME_TEST_FLAGS --mute > $T/$1.out 2>&1
}

run_client c_casual 127.0.0.1:10000 ${CLIENT_SECS:-20} &
P1=$!
run_client c_ranked 127.0.0.1:10002 ${CLIENT_SECS:-20} &
P2=$!
run_client c_community 127.0.0.1:10004 ${CLIENT_SECS:-20} &
P3=$!
wait $P1 $P2 $P3

for c in c_casual c_ranked c_community; do
	echo "=== client $c"
	grep -hE "Connecting as|identity cert|client connected to server|connect failed|Received initial public vars|last_disconnect_reason|Reason" $T/$c.out | sed 's/^\[[0-9:]*\] //' | head -8
done

for s in casual_de ranked_pl community; do
	echo "=== server $s"
	grep -hE "connected\. SID|Authenticated|did not verify|kicked|Kicking|Ranked matches need|banned" $T/$s.out | sed 's/^\[[0-9:]*\] //' | head -12
done

echo "=== account.key files"
for c in c_casual c_ranked c_community; do
	F=$T/$c/user/.keys/account.key
	if [ -f "$F" ]; then echo "  $c: $(stat -c %a $F)"; else echo "  $c: none"; fi
done

if [ -z "$KEEP_RUNNING" ]; then
	for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
	sleep 1
	echo "=== leftover: $(pgrep -fc "^$BIN")"
fi

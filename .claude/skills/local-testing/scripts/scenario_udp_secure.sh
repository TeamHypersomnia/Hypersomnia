#!/bin/bash
# Local end-to-end test of secure connect tokens. Everything on localhost, heartbeats only to the local test master.
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
export ALSOFT_DRIVERS=null
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
# Test secrets in the configs - owner only
umask 077
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
T=$S/secure_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
GAME=$REPO/hypersomnia

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
rm -rf $T
mkdir -p $T/official_servers.d

SECRET_DE=$(openssl rand -hex 24)
SECRET_PL=$(openssl rand -hex 24)
echo "$SECRET_DE" > $T/official_servers.d/de
echo "$SECRET_PL" > $T/official_servers.d/pl
chmod 600 $T/official_servers.d/*

mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF

server_conf() {
mkconf $1 <<EOF
{
  "server_start": { "ip": "0.0.0.0", "port": $2, "slots": 16 },
  "server": { "server_name": "$1", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { "autostart_when": "$4" } },
  "server_private": { "official_server_secret": "$3" }
}
EOF
}

server_conf casual_de 10000 "$SECRET_DE" NEVER
server_conf ranked_pl 10002 "$SECRET_PL" TEAMS_EQUAL
server_conf community 10004 "" NEVER

client_conf() {
mkconf $1 <<EOF
{
  "connect_token_provider": "$2",
  "server_list_provider": "http://127.0.0.1:18420",
  "window": { "fullscreen": false, "size": [1280, 720] },
  "client": { "nickname": "$1", "secure_connection_to_official_casual_servers": true }
}
EOF
}

client_conf c_casual "http://127.0.0.1:18420/connect_token"
client_conf c_ranked "http://127.0.0.1:18420/connect_token"
client_conf c_ranked_noprov "http://127.0.0.1:1/connect_token"
client_conf c_community "http://127.0.0.1:18420/connect_token"

cd $GAME
timeout 200 $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
for s in casual_de ranked_pl community; do
	timeout 190 nice -n 10 $BIN --dedicated-server --appdata-dir $T/$s --no-update-on-launch > $T/$s.out 2>&1 &
done
sleep 12

echo "=== heartbeat targets"
grep -h "Sending heartbeat through UDP to" $T/casual_de.out $T/ranked_pl.out $T/community.out | awk '{print $7}' | sort | uniq -c

echo "=== list"
curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], e['ip'], 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'))"

echo "=== endpoint"
for target in 127.0.0.1:10000 127.0.0.1:10002 127.0.0.1:10004 1.2.3.4:5; do
	R=$(curl -s -w ' HTTP%{http_code}' "http://127.0.0.1:18420/connect_token?server=$target")
	echo "  $target -> $(echo "$R" | python3 -c "
import sys,json
raw=sys.stdin.read(); body,code=raw.rsplit(' ',1)
try:
    d=json.loads(body); print(code, 'server_id=', d.get('server_id'), 'token_hex_len=', len(d.get('token','')), d.get('error',''))
except Exception: print(code, body[:60])")"
done

run_client() {
	timeout $3 xvfb-run -a nice -n 10 $BIN --connect $2 --appdata-dir $T/$1 --no-update-on-launch > $T/$1.out 2>&1
}

run_client c_casual 127.0.0.1:10000 20 &
P1=$!
run_client c_ranked 127.0.0.1:10002 20 &
P2=$!
run_client c_ranked_noprov 127.0.0.1:10002 20 &
P3=$!
run_client c_community 127.0.0.1:10004 20 &
P4=$!
wait $P1 $P2 $P3 $P4

for c in c_casual c_ranked c_ranked_noprov c_community; do
	echo "=== client $c"
	grep -hE "secure connect token|Connecting securely|client connected to server|secure connection|Received an invalid|last_disconnect_reason" $T/$c.out | sed 's/^\[[0-9:]*\] //' | head -6
done

for s in casual_de ranked_pl community; do
	echo "=== server $s"
	grep -hE "connected securely|connected\. SID|Ignoring auth_request|kicked|Reason|key_|identity key|uthenticat" $T/$s.out | sed 's/^\[[0-9:]*\] //' | head -12
done

echo "=== identity.key files (only where a secure connection asked for authentication)"
for c in c_casual c_ranked c_ranked_noprov c_community; do
	F=$T/$c/user/identity.key
	if [ -f "$F" ]; then echo "  $c: $(stat -c %a $F) $(cut -c1-24 $F)..."; else echo "  $c: none"; fi
done

echo "=== reconnect c_ranked: same account?"
cp $T/ranked_pl.out $T/ranked_pl_first.out
run_client c_ranked 127.0.0.1:10002 15
grep -ohE "key_[0-9a-f]{64}" $T/ranked_pl.out | sort | uniq -c

echo "=== rate limit (40 requests from one IP)"
for i in $(seq 1 40); do curl -s -o /dev/null -w '%{http_code}\n' "http://127.0.0.1:18420/connect_token?server=127.0.0.1:10000"; done | sort | uniq -c

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
echo "=== leftover: $(pgrep -fc "^$BIN")"

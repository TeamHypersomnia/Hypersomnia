#!/bin/bash
# Native client over WebRTC to a local official ranked server: the token is requested by the WebRTC alias,
# the same path a browser takes. Everything on localhost.
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
export ALSOFT_DRIVERS=null
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
# Test secrets in the configs - owner only
umask 077
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
T=$S/rtc_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
GAME=$REPO/hypersomnia

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
rm -rf $T
mkdir -p $T/official_servers.d
SECRET_PL=$(openssl rand -hex 24)
echo "$SECRET_PL" > $T/official_servers.d/pl
chmod 600 $T/official_servers.d/*

mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF

mkconf ranked <<EOF
{
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "server_start": { "ip": "0.0.0.0", "port": 10002, "slots": 16 },
  "server": { "server_name": "[PL] Ranked test", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { "autostart_when": "TEAMS_EQUAL" },
              "webrtc_port_range_begin": 19000, "webrtc_port_range_end": 19050 },
  "server_private": { "official_server_secret": "$SECRET_PL" }
}
EOF

mkconf c_rtc <<EOF
{
  "connect_token_provider": "http://127.0.0.1:18420/connect_token",
  "server_list_provider": "http://127.0.0.1:18420",
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "window": { "fullscreen": false, "size": [1280, 720] },
  "client": { "nickname": "c_rtc" }
}
EOF

cd $GAME
timeout 120 $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
timeout 110 nice -n 10 $BIN --dedicated-server --appdata-dir $T/ranked --no-update-on-launch > $T/ranked.out 2>&1 &
sleep 12

echo "=== list (webrtc ids)"
curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], e['ip'], 'webrtc_id=', e.get('webrtc_id'), 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'))"

echo "=== token by alias"
curl -s "http://127.0.0.1:18420/connect_token?server=ranked-pl" | python3 -c "import json,sys; d=json.load(sys.stdin); print(' server_id=', d.get('server_id'), 'token_hex_len=', len(d.get('token','')))"

timeout 30 xvfb-run -a nice -n 10 $BIN --connect "rtc:ranked-pl" --appdata-dir $T/c_rtc --no-update-on-launch > $T/c_rtc.out 2>&1

echo "=== client"
grep -hE "WebRTC|secure connect token|Connecting securely|client connected to server|identity key|last_disconnect_reason" $T/c_rtc.out | sed 's/^\[[0-9:]*\] //' | head -10
echo "=== server"
grep -hE "connected securely|connected\. SID|Authenticated|kicked|Reason|Ignoring auth" $T/ranked.out | sed 's/^\[[0-9:]*\] //' | head -10

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
echo "=== leftover: $(pgrep -fc "^$BIN")"

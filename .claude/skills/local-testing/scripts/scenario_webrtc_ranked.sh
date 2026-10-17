#!/bin/bash
# Native client over WebRTC to a local official ranked server joined by its WebRTC alias, the same path a browser takes.
# The client's cert comes from the broker (BROKER_URL, BROKER_PUBLIC_KEY - see broker.sh); without one, ranked refuses it.
# Everything on localhost.
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
export ALSOFT_DRIVERS=null
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
umask 077
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
# A fresh directory every run - never removed recursively
T=$S/rtc_test_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
GAME=$REPO/hypersomnia
BROKER_URL=${BROKER_URL:-}
BROKER_PUBLIC_KEY=${BROKER_PUBLIC_KEY:-}

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
mkdir -p $T/official_servers.d

mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF

mkconf ranked <<EOF
{
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "instances": { "ranked": 1 }, "server_start": { "ip": "0.0.0.0", "port": 10002, "slots": 16 },
  "server": { "server_name": "[PL] Ranked test", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { },
              "webrtc_port_range_begin": 19000, "webrtc_port_range_end": 19050 },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"],
                      "report_ranked_match_url": "${BROKER_URL:-http://127.0.0.1:1}/report_match" }
}
EOF

mkconf c_rtc <<EOF
{
  "identity_broker_url": "${BROKER_URL:-http://127.0.0.1:1}",
  "server_list_provider": "http://127.0.0.1:18420",
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "window": { "fullscreen": false, "size": [1280, 720] },
  "client": { "nickname": "c_rtc" }
}
EOF

cd $GAME
# Official by its public key
$BIN --print-server-key --appdata-dir $T/ranked 2>/dev/null | grep -E '^[0-9a-f]{64}$' > $T/official_servers.d/pl

timeout 120 $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
timeout 110 nice -n 10 $BIN --dedicated-server --appdata-dir $T/ranked --no-update-on-launch > $T/ranked.out 2>&1 &
sleep 12

echo "=== list (webrtc ids)"
curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], e['ip'], 'webrtc_id=', e.get('webrtc_id'), 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'))"

timeout 30 xvfb-run -a nice -n 10 $BIN --connect "rtc:ranked-pl:1" --appdata-dir $T/c_rtc --no-update-on-launch > $T/c_rtc.out 2>&1

echo "=== client"
grep -hE "WebRTC|Connecting as|identity cert|client connected to server|Received initial|last_disconnect_reason" $T/c_rtc.out | sed 's/^\[[0-9:]*\] //' | head -10
echo "=== server"
grep -hE "connected\. SID|Authenticated|did not verify|kicked|Reason|Ranked matches need" $T/ranked.out | sed 's/^\[[0-9:]*\] //' | head -10

for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done
sleep 1
echo "=== leftover: $(pgrep -fc "^$BIN")"

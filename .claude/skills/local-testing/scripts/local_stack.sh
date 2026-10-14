#!/bin/bash
# A local stack for manual testing, left running: test masterserver, an official ranked server [PL] and an official
# casual server [DE] (both with WebRTC, official by their keys), the broker (broker.sh, port 13500), and the web build
# pointed at both on http://localhost:8765.
# Usage: local_stack.sh [web build dir] [hours]     Stop: local_stack.sh stop
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/local_stack
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
WEB_BUILD=${1:-$REPO/build/Release-Web-clang-claude-web}
HOURS=${2:-2}

stop() {
	for p in $(pgrep -f "^(timeout [0-9]+ )?(nice -n 10 )?$BIN --.* --appdata-dir $T/"); do kill $p; done
	for p in $(pgrep -f "webserve.py $T/web"); do kill $p; done
	"$HERE/broker.sh" stop
}

if [ "$1" == "stop" ]; then
	stop
	echo "stopped"
	exit 0
fi

stop
sleep 1

for port in 18000 18410 18420 18430 10002 10004 8765 13500; do
	if ss -tulpn 2>/dev/null | grep -q ":$port\b"; then echo "port $port busy, abort"; exit 1; fi
done

rm -rf $T
mkdir -p $T/official_servers.d
eval "$(OFFICIAL_SERVERS_DIR=$T/official_servers.d "$HERE/broker.sh" start | grep -E '^BROKER_(URL|PUBLIC_KEY)=')"

mkconf() { mkdir -p $T/$1/user/conf.d; cat > $T/$1/user/conf.d/test.json; }

mkconf ms <<EOF
{ "masterserver": { "ip": "127.0.0.1", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$T/official_servers.d" } }
EOF

server_conf() {
mkconf $1 <<EOF
{
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "server_start": { "ip": "0.0.0.0", "port": $2, "slots": 16 },
  "server": { "server_name": "$3", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { "autostart_when": "$4" },
              "webrtc_port_range_begin": $5, "webrtc_port_range_end": $(($5 + 50)) },
  "server_private": { "identity_broker_public_keys": ["$BROKER_PUBLIC_KEY"], "report_ranked_match_url": "$BROKER_URL/report_match" }
}
EOF
}

server_conf ranked 10002 "[PL] Local ranked" TEAMS_EQUAL 19000 pl
server_conf casual 10004 "[DE] Local casual" NEVER 19100 de

"$HERE/prepare_web.sh" "$WEB_BUILD" $T/web > /dev/null

SECS=$((HOURS * 3600))
cd $REPO/hypersomnia
for s in ranked:pl casual:de; do
	$BIN --print-server-key --appdata-dir $T/${s%%:*} 2>/dev/null | grep -E '^[0-9a-f]{64}$' > $T/official_servers.d/${s##*:}
done

timeout $SECS $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
timeout $SECS nice -n 10 $BIN --dedicated-server --appdata-dir $T/ranked --no-update-on-launch > $T/ranked.out 2>&1 &
timeout $SECS nice -n 10 $BIN --dedicated-server --appdata-dir $T/casual --no-update-on-launch > $T/casual.out 2>&1 &
timeout $SECS python3 "$HERE/webserve.py" $T/web 8765 > $T/web.out 2>&1 &
sleep 10

curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], 'webrtc_id=', e.get('webrtc_id'), 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'))"

cat <<EOF

Running for $HOURS h. Logs: $T/{ms,ranked,casual}.out
  Web, ranked:   http://localhost:8765/game/ranked-pl
  Web, casual:   http://localhost:8765/game/de
  Web, list:     http://localhost:8765/
  Broker:        $BROKER_URL (key $BROKER_PUBLIC_KEY)
  Native:        add to a conf.d:  "server_list_provider": "http://127.0.0.1:18420",
                 "identity_broker_url": "$BROKER_URL", "website_url": "$BROKER_URL",
                 "webrtc_signalling_server_url": "ws://127.0.0.1:18000"
                 then connect to 127.0.0.1:10002 (ranked) or 127.0.0.1:10004 (casual)
  Stop:          $0 stop
EOF

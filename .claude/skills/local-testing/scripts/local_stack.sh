#!/bin/bash
# A local stack for manual testing, left running: test masterserver, an official ranked server [PL] and an official
# casual server [DE] (both with WebRTC), and the web build pointed at this master on http://localhost:8765.
# Usage: local_stack.sh [web build dir] [hours]     Stop: local_stack.sh stop
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/local_stack
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
WEB_BUILD=${1:-$REPO/build/Release-Web-clang-claude-web}
HOURS=${2:-2}

stop() {
	for p in $(pgrep -f "^(timeout [0-9]+ )?(nice -n 10 )?$BIN --.* --appdata-dir $T/"); do kill $p; done
	for p in $(pgrep -f "webserve.py $T/web"); do kill $p; done
}

if [ "$1" == "stop" ]; then
	stop
	echo "stopped"
	exit 0
fi

stop
sleep 1

for port in 18000 18410 18420 18430 10002 10004 8765; do
	if ss -tulpn 2>/dev/null | grep -q ":$port\b"; then echo "port $port busy, abort"; exit 1; fi
done

rm -rf $T
mkdir -p $T/official_servers.d
openssl rand -hex 24 > $T/official_servers.d/pl
openssl rand -hex 24 > $T/official_servers.d/de
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
  "webrtc_signalling_server_url": "ws://127.0.0.1:18000",
  "server_start": { "ip": "0.0.0.0", "port": $2, "slots": 16 },
  "server": { "server_name": "$3", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:18430",
              "suppress_new_community_server_webhook": true, "ranked": { "autostart_when": "$4" },
              "webrtc_port_range_begin": $5, "webrtc_port_range_end": $(($5 + 50)) },
  "server_private": { "official_server_secret": "$(cat $T/official_servers.d/$6)" }
}
EOF
}

server_conf ranked 10002 "[PL] Local ranked" TEAMS_EQUAL 19000 pl
server_conf casual 10004 "[DE] Local casual" NEVER 19100 de

"$(dirname "$0")/prepare_web.sh" "$WEB_BUILD" $T/web > /dev/null

SECS=$((HOURS * 3600))
cd $REPO/hypersomnia
timeout $SECS $BIN --masterserver --appdata-dir $T/ms --no-update-on-launch > $T/ms.out 2>&1 &
sleep 3
timeout $SECS nice -n 10 $BIN --dedicated-server --appdata-dir $T/ranked --no-update-on-launch > $T/ranked.out 2>&1 &
timeout $SECS nice -n 10 $BIN --dedicated-server --appdata-dir $T/casual --no-update-on-launch > $T/casual.out 2>&1 &
timeout $SECS python3 "$(dirname "$0")/webserve.py" $T/web 8765 > $T/web.out 2>&1 &
sleep 10

curl -s http://127.0.0.1:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], 'webrtc_id=', e.get('webrtc_id'), 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'))"

cat <<EOF

Running for $HOURS h. Logs: $T/{ms,ranked,casual}.out
  Web, ranked:   http://localhost:8765/game/ranked-pl   (needs Discord sign-in - only if localhost is an allowed redirect)
  Web, casual:   http://localhost:8765/game/de          (secure: joined by URL the server is not on the list yet, so the token is asked for anyway)
  Web, list:     http://localhost:8765/
  Native:        add to a conf.d:  "server_list_provider": "http://127.0.0.1:18420",
                 "connect_token_provider": "http://127.0.0.1:18420/connect_token",
                 "webrtc_signalling_server_url": "ws://127.0.0.1:18000"
                 then connect to 127.0.0.1:10002 (ranked) or 127.0.0.1:10004 (casual)
  Stop:          $0 stop
EOF

#!/bin/bash
# One ranked match on arena, all inside ~/yojimbo-test/e2e: a dedicated server with its own official_server_secret,
# heartbeating to the test masterserver on arenaus and reporting to the test site there.
# Tournament mode puts the two given accounts on the teams, so that idle CLI clients get to play.
# A real Steam Web API key in ~/yojimbo-test/e2e/steam_web_api_key (0600) makes Steam tickets verifiable;
# delete it after the test.
# Usage: match.sh start <account A> <account B> | stop | log
E=$HOME/yojimbo-test/e2e
export LD_LIBRARY_PATH=$E/lib
# The server config holds its official_server_secret (and a real Steam Web API key, if given) - owner only
umask 077

stop() {
	if [ -f $E/server.pid ]; then
		# Only if that pid is still our server - after a reboot it could be anything
		pid=$(cat $E/server.pid)
		if ps -o args= -p $pid 2>/dev/null | grep -q -- "--appdata-dir $E/server"; then
			kill $pid 2>/dev/null
		fi
		rm -f $E/server.pid
	fi

	rm -f $E/server/user/conf.d/test.json
}

case "$1" in
start)
	for id in "$2" "$3"; do
		[[ "$id" =~ ^[A-Za-z0-9_]+$ ]] || { echo "invalid account id: $id"; exit 1; }
	done

	stop
	sleep 1

	cat > $E/tournament.json <<EOF
{
  "skill_level_source": "ORDER",
  "teams": [ [ "$2" ], [ "$3" ] ],
  "additional_spectators": [],
  "arenas": [ "de_cyberaqua bomb_defusal" ]
}
EOF

	mkdir -p $E/server/user/conf.d
	cat > $E/server/user/conf.d/test.json <<EOF
{
  "tournament_file": "$E/tournament.json",
  "server_start": { "ip": "0.0.0.0", "port": 9005, "slots": 16 },
  "webrtc_signalling_server_url": "ws://${ARENAUS_IP:-154.12.226.211}:18000",
  "server": {
    "server_name": "[PL] E2E ranked",
    "domain": "e2e-test.hypersomnia.io",
    "arena": "de_cyberaqua",
    "notified_server_list": "${ARENAUS_IP:-154.12.226.211}:18430",
    "suppress_new_community_server_webhook": true,
    "ranked": { "autostart_when": "ALWAYS", "countdown_time": 5, "max_rejoins": 0 },
    "move_to_spectators_if_afk_for_secs": 999999,
    "kick_if_afk_for_secs": 999999,
    "freeze_time": false,
    "game_speed": 1.0,
    "webrtc_port_range_begin": 9006,
    "webrtc_port_range_end": 9010
  },
  "server_private": {
    "official_server_secret": "$(cat $E/secret)",
    "report_ranked_match_url": "http://${ARENAUS_IP:-154.12.226.211}:13000/report_match",
    "steam_web_api_key": "$(cat $E/steam_web_api_key 2>/dev/null || echo e2e-dummy)"
  }
}
EOF

	cd $E/hypersomnia
	setsid nohup timeout 600 $E/Hypersomnia --dedicated-server --appdata-dir $E/server --no-update-on-launch > $E/server.out 2>&1 < /dev/null &
	echo $! > $E/server.pid
	sleep 6
	grep -hE "Tournament|listening|heartbeat through UDP" $E/server.out | head -4
	;;
stop)
	stop
	echo stopped
	;;
log)
	grep -hE "connected securely|Authenticated|wrong match|kicked|Reason|ABANDON|report|Report|webhook|RANKED|ranked" $E/server.out | sed 's/^\[[0-9:]*\] //' | head -40
	;;
esac

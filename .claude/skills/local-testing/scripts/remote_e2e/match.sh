#!/bin/bash
# One ranked match on arena, all inside ~/yojimbo-test/e2e: a dedicated server with its own official_server_secret,
# heartbeating to the test masterserver on arenaus and reporting to the test site there.
# Tournament mode puts the two given accounts on the teams, so that idle CLI clients get to play.
# Usage: match.sh start <account A> <account B> | stop | log
E=$HOME/yojimbo-test/e2e
export LD_LIBRARY_PATH=$E/lib

stop() {
	if [ -f $E/server.pid ]; then
		kill $(cat $E/server.pid) 2>/dev/null
		rm -f $E/server.pid
	fi
}

case "$1" in
start)
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
    "game_speed": 1.0
  },
  "server_private": {
    "official_server_secret": "$(cat $E/secret)",
    "report_ranked_match_url": "http://${ARENAUS_IP:-154.12.226.211}:13000/report_match",
    "steam_web_api_key": "e2e-dummy"
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

#!/bin/bash
# Test backend on arenaus, all inside ~/hs-test-backend: a test masterserver (UDP 18430, HTTP 18410/18420, signalling 18000)
# and a copy of the site (port 13000) with its own clean database, both reading official_servers.d.
# Usage: backend.sh start|stop|status
B=$HOME/hs-test-backend
export LD_LIBRARY_PATH=$B/lib

stop() {
	for f in $B/*.pid; do
		[ -f "$f" ] || continue
		kill $(cat $f) 2>/dev/null
		rm -f $f
	done
}

case "$1" in
start)
	stop
	sleep 1

	mkdir -p $B/ms/user/conf.d
	cat > $B/ms/user/conf.d/test.json <<EOF
{ "masterserver": { "ip": "0.0.0.0", "signalling_server_port": 18000, "first_udp_command_port": 18430, "num_udp_command_ports": 1,
  "server_list_port": 18420, "fallback_http_server_list_port": 18410, "report_rtc_errors_to_webhook": false,
  "official_servers_dir": "$B/official_servers.d" } }
EOF

	cd $B/hypersomnia
	setsid nohup timeout 7200 $B/Hypersomnia --masterserver --appdata-dir $B/ms --no-update-on-launch > $B/ms.out 2>&1 < /dev/null &
	echo $! > $B/ms.pid

	# A new session secret every start, readable only here - fake_steam_session.sh signs cookies with it
	(umask 077; openssl rand -hex 32 > $B/session_secret)

	cd $B/page
	PORT=13000 NODE_ENV=development SESSION_SECRET=$(cat $B/session_secret) OFFICIAL_SERVERS_DIR=$B/official_servers.d SERVER_LIST_URL=http://127.0.0.1:18420/server_list_json \
		setsid nohup timeout 7200 $B/node/bin/node app.js > $B/site.out 2>&1 < /dev/null &
	echo $! > $B/site.pid

	sleep 5
	ss -tulpn 2>/dev/null | grep -E ":(18000|18410|18420|18430|13000)\b" | awk '{print "  listening", $1, $5}'
	;;
stop)
	stop
	echo stopped
	;;
status)
	for f in $B/*.pid; do [ -f "$f" ] && echo "$(basename $f): $(ps -o etime=,args= -p $(cat $f) | cut -c1-80)"; done
	;;
esac

#!/bin/bash
# An official host on arena for playing by hand, all inside ~/yojimbo-test/e2e/official: one process with casual c1, c2
# (ports 9005, 9006) and ranked r1 (9007), WebRTC on 9008-9010 - one server.key, listed on the test masterserver on arenaus
# (official there by that key - put `official.sh key` into its official_servers.d) and trusting its test broker.
# Usage: official.sh start <test broker public key> | key | stop | log      (SECS=<s> - how long it runs, 2 h by default)
E=$HOME/yojimbo-test/e2e
O=$E/official
export LD_LIBRARY_PATH=$E/lib
umask 077

stop() {
	if [ -f $O.pid ]; then
		# Only if that pid is still our server - after a reboot it could be anything
		pid=$(cat $O.pid)
		if ps -o args= -p $pid 2>/dev/null | grep -q -- "--appdata-dir $O"; then
			kill $pid 2>/dev/null
		fi
		rm -f $O.pid
	fi
}

case "$1" in
start)
	[[ "$2" =~ ^[0-9a-f]{64}$ ]] || { echo "invalid broker key: $2"; exit 1; }
	stop
	sleep 1

	mkdir -p $O/user/conf.d
	cat > $O/user/conf.d/test.json <<EOF
{
  "instances": { "casual": 2, "ranked": 1 }, "server_start": { "ip": "0.0.0.0", "port": 9005, "slots": 16 },
  "webrtc_signalling_server_url": "ws://${ARENAUS_IP:-154.12.226.211}:18000",
  "server": {
    "server_name": "[PL] Test official",
    "arena": "de_cyberaqua",
    "notified_server_list": "${ARENAUS_IP:-154.12.226.211}:18430",
    "suppress_new_community_server_webhook": true,
    "time_zone": "Europe/Warsaw",
    "webrtc_port_range_begin": 9008,
    "webrtc_port_range_end": 9010
  },
  "server_private": {
    "identity_broker_public_keys": ["$2"],
    "report_ranked_match_url": "http://${ARENAUS_IP:-154.12.226.211}:13000/report_match"
  }
}
EOF

	cd $E/hypersomnia
	setsid nohup timeout ${SECS:-7200} $E/Hypersomnia --dedicated-server --appdata-dir $O --no-update-on-launch > $O.out 2>&1 < /dev/null &
	echo $! > $O.pid
	sleep 8
	grep -hE "Starting .* server instance|heartbeat through UDP" $O.out | sort -u | head -6
	;;
key)
	mkdir -p $O/user
	cd $E/hypersomnia
	$E/Hypersomnia --print-server-key --appdata-dir $O 2>/dev/null | grep -E '^[0-9a-f]{64}$'
	;;
stop)
	stop
	echo stopped
	;;
log)
	tail -40 $O.out
	;;
esac

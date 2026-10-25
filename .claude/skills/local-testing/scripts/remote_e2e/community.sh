#!/bin/bash
# A community server on arena (port 9005), all inside ~/yojimbo-test/e2e/community: listed on the test masterserver on arenaus
# by its public address - so a client joining from the list expects the list's key (locally every server is a LAN entry, no key).
# Uses the e2e dir's binary, content and libs (match.sh's upload).
#   community.sh start [silent]   starts it with a new key each time (its server.key removed); "silent" never tells the masterserver,
#                                 so the list keeps the previous key (within the master's 60 s entry timeout) - a misconfigured server
#   community.sh crash            kills it without a goodbye to the masterserver (its entry, with the key, stays for 60 s)
#   community.sh stop | key | log                (PASSWORD=<pw> start - a server with a password; SECS=<s> - how long it runs, 900 by default)
E=$HOME/yojimbo-test/e2e
C=$E/community
export LD_LIBRARY_PATH=$E/lib
umask 077

stop() {
	if [ -f $C.pid ]; then
		# Only if that pid is still our server - after a reboot it could be anything
		pid=$(cat $C.pid)
		if ps -o args= -p $pid 2>/dev/null | grep -q -- "--appdata-dir $C"; then
			kill $pid 2>/dev/null
		fi
		rm -f $C.pid
	fi
}

case "$1" in
start)
	stop
	sleep 1
	# A new key: only the key file goes, nothing else
	rm -f $C/user/.keys/server.key
	mkdir -p $C/user/conf.d

	NOTIFY="\"notified_server_list\": \"${ARENAUS_IP:-154.12.226.211}:18430\","

	if [ "$2" = "silent" ]; then
		NOTIFY="\"notified_server_list\": \"127.0.0.1:1\","
	fi

	cat > $C/user/conf.d/test.json <<EOF
{
  "server_start": { "ip": "0.0.0.0", "port": 9005, "slots": 8 },
  "webrtc_signalling_server_url": "ws://${ARENAUS_IP:-154.12.226.211}:18000",
  "server": {
    "server_name": "[PL] E2E community",
    "map_cycle": [ "de_cyberaqua" ],
    $NOTIFY
    "suppress_new_community_server_webhook": true,
    "webrtc_port_range_begin": 9006,
    "webrtc_port_range_end": 9010
  },
  "server_private": { "server_password": "${PASSWORD:-}" }
}
EOF

	cd $E/hypersomnia
	setsid nohup timeout ${SECS:-900} $E/Hypersomnia --dedicated-server --appdata-dir $C --no-update-on-launch --no-open-logs > $C.out 2>&1 < /dev/null &
	echo $! > $C.pid
	sleep 5
	$E/Hypersomnia --dedicated-server --print-server-key --appdata-dir $C 2>/dev/null | grep -E '^[0-9a-f]{64}$'
	;;
key)
	cd $E/hypersomnia
	$E/Hypersomnia --dedicated-server --print-server-key --appdata-dir $C 2>/dev/null | grep -E '^[0-9a-f]{64}$'
	;;
stop)
	stop
	echo stopped
	;;
crash)
	# SIGKILL to the server itself (the pid file has timeout's): no goodbye to the masterserver, so its entry stays until the timeout
	if [ -f $C.pid ]; then
		for p in $(ps -o pid= --ppid $(cat $C.pid)); do kill -9 $p; done
	fi
	stop
	echo crashed
	;;
log)
	grep -hE "connected\. SID|listening|heartbeat|kicked|Reason|disconnect" $C.out | sed 's/^\[[0-9:]*\] //' | tail -20
	;;
esac

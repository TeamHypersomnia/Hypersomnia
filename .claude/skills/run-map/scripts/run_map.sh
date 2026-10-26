#!/bin/bash
# A dedicated server on any map, optionally with bots and a client - the base for every in-game test.
#
#   MAP          arena name (content/arenas, user/downloads/maps) - default de_cyberaqua
#   MAP_DIR      any arena folder instead (e.g. made by map_copy.py) - its folder name must be MAP
#   BOTS=1       fill with bots (they play only while a human client is connected)
#   CLIENT       spectator (default: joins under xvfb-run, stays a spectator) | display (on $DISPLAY, e.g. Xvfb :78 for
#                screenshots and xinput.py clicks) | none
#   SECS         how long (default 180); KEEP=1 returns at once and leaves everything running (pids in <dir>/*.pid)
#   SERVER_EXTRA more JSON fields of "server", e.g. ', "bot_difficulty": "HARD"'
#   CONF_EXTRA   more top-level JSON fields for both the server and the client, e.g. ', "debug": { "log_solvable_hashes": true }'
#   BIN, PORT, SCRATCH, LOG_GREP (printed from the server log at the end), LOG_TAIL
#
# Prints the run dir (server log <dir>/srv.out, client log <dir>/c.out). Heartbeats go to 127.0.0.1:1, never production.
. "$(dirname "$0")/../../local-testing/scripts/test_env.sh"
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
MAP=${MAP:-de_cyberaqua}
SECS=${SECS:-180}
PORT=${PORT:-10091}
CLIENT=${CLIENT:-spectator}
BOTS_JSON=$([ "$BOTS" = 1 ] && echo true || echo false)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/map_${MAP}_$(date +%s)

if ss -tuln | grep -q ":$PORT "; then echo "port $PORT busy, abort"; exit 1; fi

mkdir -p $T/srv/user/conf.d $T/srv/user/downloads/maps $T/c/user/conf.d $T/c/user/downloads/maps

if [ -z "$MAP_DIR" ] && [ -d "$REPO/hypersomnia/user/downloads/maps/$MAP" ]; then
	MAP_DIR="$REPO/hypersomnia/user/downloads/maps/$MAP"
fi

if [ -n "$MAP_DIR" ]; then
	# Copied, not symlinked - the arena scan skips symlinks ("Failed to load arena").
	cp -r "$MAP_DIR" $T/srv/user/downloads/maps/$MAP
	cp -r "$MAP_DIR" $T/c/user/downloads/maps/$MAP
fi

cat > $T/srv/user/conf.d/test.json <<J
{ "server_start": { "ip": "127.0.0.1", "port": $PORT, "slots": 16 },
  "server": { "server_name": "map test", "map_cycle": [ "$MAP" ], "bots": $BOTS_JSON, "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19930, "webrtc_port_range_end": 19935 $SERVER_EXTRA } $CONF_EXTRA }
J
cat > $T/c/user/conf.d/test.json <<J
{ "identity_broker_url": "http://127.0.0.1:1", "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "maptest" } $CONF_EXTRA }
J

cd $REPO/hypersomnia
timeout $((SECS + 20)) $BIN --dedicated-server --appdata-dir $T/srv $GAME_TEST_FLAGS > $T/srv.out 2>&1 &
echo $! > $T/srv.pid
sleep 6

case "$CLIENT" in
	spectator) CLIENT_RUN="xvfb-run -a $BIN" ;;
	display) CLIENT_RUN="$BIN" ;;
	*) CLIENT_RUN="" ;;
esac

if [ -n "$CLIENT_RUN" ]; then
	if [ "$KEEP" = 1 ]; then
		timeout $SECS $CLIENT_RUN --connect 127.0.0.1:$PORT --appdata-dir $T/c $GAME_TEST_FLAGS --mute > $T/c.out 2>&1 &
		echo $! > $T/c.pid
	else
		timeout $SECS $CLIENT_RUN --connect 127.0.0.1:$PORT --appdata-dir $T/c $GAME_TEST_FLAGS --mute > $T/c.out 2>&1
	fi
elif [ "$KEEP" != 1 ]; then
	sleep $SECS
fi

echo "DIR $T"

if [ "$KEEP" = 1 ]; then
	exit 0
fi

sleep 22
grep -m1 "Sending heartbeat through UDP to" $T/srv.out
grep -m1 -E "Failed to load arena|Choosing arena" $T/srv.out
grep -ciE "assert|segfault|terminate called" $T/srv.out $T/c.out 2>/dev/null
echo "client desyncs: $(grep -c 'Client desynchronized' $T/c.out 2>/dev/null)"
[ -n "$LOG_GREP" ] && grep -E "$LOG_GREP" $T/srv.out | tail -${LOG_TAIL:-60}
exit 0

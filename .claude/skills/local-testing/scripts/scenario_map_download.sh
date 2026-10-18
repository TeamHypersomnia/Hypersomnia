#!/bin/bash
# A client without the server's map downloads it - directly from the server (UDP), or from an HTTP provider (EXT=<url>).
# LATENCY/LOSS: the client's network simulator. KILL_AT=<s>: the server is SIGKILLed that long after the client starts.
# Prints the client's download progress, its disconnect reason and whether it got into the game.
# Usage: MAP=<arena in hypersomnia/user/downloads/arenas> [EXT=..] [LATENCY=..] [LOSS=..] [KILL_AT=..] scenario_map_download.sh <out dir>
# A long download: EXT=http://127.0.0.1:12180/arenas with slow_http.py <dir with arenas/> 12180 50000 (the provider is the arenas folder).
export ALSOFT_DRIVERS=null VISUAL=true
REPO=$HOME/Hypersomnia
BIN=$REPO/build/Debug-x64-clang-claude/Hypersomnia
T=$1
MAP=${MAP:-hyperactive_space_station}
mkdir -p $T/srv/user/conf.d $T/srv/user/downloads/arenas $T/cli/user/conf.d
cp -r $REPO/hypersomnia/user/downloads/arenas/$MAP $T/srv/user/downloads/arenas/

cat > $T/srv/user/conf.d/test.json <<EOF
{ "server_start": { "ip": "127.0.0.1", "port": 12101, "slots": 8 },
  "server": { "server_name": "dl test", "arena": "$MAP", "notified_server_list": "127.0.0.1:1",
    "suppress_new_community_server_webhook": true, "allow_direct_arena_file_downloads": true,
    "external_arena_files_provider": "${EXT:-}" } }
EOF

cat > $T/cli/user/conf.d/test.json <<EOF
{ "identity_broker_url": "", "server_list_provider": "http://127.0.0.1:1", "skip_tutorial": true,
  "window": { "fullscreen": false, "size": [1280, 720] },
  "client": { "nickname": "downloader", "network_simulator": { "is_enabled": true,
    "value": { "latency_ms": ${LATENCY:-0}, "jitter_ms": 0, "loss_percent": ${LOSS:-0}, "duplicates_percent": 0 } } } }
EOF

cd $REPO/hypersomnia
setsid timeout 300 $BIN --dedicated-server --appdata-dir $T/srv --no-update-on-launch > $T/srv.out 2>&1 < /dev/null &
SRV=$!
sleep 4
setsid timeout 200 Xvfb :81 -screen 0 1280x720x24 > /dev/null 2>&1 < /dev/null &
XV=$!
sleep 1
START=$(date +%s)
DISPLAY=:81 setsid timeout 180 $BIN --appdata-dir $T/cli --no-update-on-launch --mute --connect 127.0.0.1:12101 > $T/cli.out 2>&1 < /dev/null &
CLI=$!

if [ -n "$KILL_AT" ]; then
	sleep $KILL_AT
	# the server's own process, not timeout's
	pkill -9 -P $SRV
	echo "server killed at +$(( $(date +%s) - START )) s"
fi

for i in $(seq 1 170); do
	if grep -aq "Received initial state\|last_disconnect_reason" $T/cli.out; then break; fi
	sleep 1
done
sleep 2
DISPLAY=:81 import -window root $T/end.png
echo "client after +$(( $(date +%s) - START )) s:"
grep -aE "Downloading|download|Download complete|Received initial state|last_disconnect_reason|Nothing came" $T/cli.out | grep -v "keepalive" | sed 's/^\[[0-9:]*\] //' | uniq | tail -6
kill $CLI $XV 2>/dev/null
pkill -P $SRV 2>/dev/null
kill $SRV 2>/dev/null

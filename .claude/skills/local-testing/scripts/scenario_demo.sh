#!/bin/bash
# Demos: a client records one while it plays on a local server (recording is on by default), then another start
# replays that file (the connect string is demo://<the demo's path>). The replay must load and run without errors.
. "$(dirname "$0")/test_env.sh"
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/demo_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
if ss -tuln | grep -q ":10090 "; then echo "port 10090 busy, abort"; exit 1; fi
rm -rf $T; mkdir -p $T/srv/user/conf.d $T/c/user/conf.d
cat > $T/srv/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": 10090, "slots": 8 },
  "server": { "server_name": "demo test", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19900, "webrtc_port_range_end": 19905 } }
J
cat > $T/c/user/conf.d/test.json <<J
{ "identity_broker_url": "http://127.0.0.1:1", "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "demotest" } }
J
cd $REPO/hypersomnia
timeout 90 $BIN --dedicated-server --appdata-dir $T/srv $GAME_TEST_FLAGS > $T/srv.out 2>&1 &
sleep 6
timeout 30 xvfb-run -a $BIN --connect 127.0.0.1:10090 --appdata-dir $T/c $GAME_TEST_FLAGS --mute > $T/record.out 2>&1
for p in $(pgrep -f "^$BIN .*--appdata-dir $T/srv"); do kill $p; done
# Demos are compressed when the game comes to the main menu - a client killed in game leaves it raw until then
cat > $T/c/user/conf.d/menu.json <<J
{ "launch_at_startup": "MAIN_MENU" }
J
timeout 20 xvfb-run -a $BIN --appdata-dir $T/c $GAME_TEST_FLAGS --mute > $T/menu.out 2>&1
rm $T/c/user/conf.d/menu.json
DEMO=$(ls -t $T/c/user/demos/*.demc 2>/dev/null | head -1)
echo "=== recorded: ${DEMO:-none} ($( [ -n "$DEMO" ] && stat -c %s "$DEMO") bytes)"
[ -n "$DEMO" ] || exit 1
Xvfb :88 -screen 0 1280x720x24 > /dev/null 2>&1 &
XV=$!
sleep 1
DISPLAY=:88 timeout 25 $BIN --connect "demo://$DEMO" --appdata-dir $T/c $GAME_TEST_FLAGS --mute > $T/replay.out 2>&1 &
G=$!
sleep 20
DISPLAY=:88 import -window root $T/replay.png
wait $G
kill $XV
echo "=== replay"
grep -hiE "demo|replay|error|failed|exception" $T/replay.out | grep -viE "record demo|demos dir" | sed 's/^\[[0-9:]*\] //' | head -10
echo "=== screenshot: $T/replay.png"

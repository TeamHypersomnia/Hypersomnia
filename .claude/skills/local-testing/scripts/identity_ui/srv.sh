#!/bin/bash
# srv.sh start|stop <name> <port> [ranked: NEVER|TEAMS_EQUAL] - a local server trusting the scratch broker
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
BIN=${BIN:-$(cd "$(dirname "$0")/../../../../.." && pwd)/build/Debug-x64-clang-claude/Hypersomnia}
eval "$(cat $S/nettest/broker_env)"
cmd=$1; name=$2; port=$3; when=${4:-NEVER}
T=$S/srv/$name
if [ "$cmd" = stop ]; then for p in $(pgrep -f "^$BIN --dedicated-server --appdata-dir $T "); do kill $p; done; exit; fi
rm -rf $T; mkdir -p $T/user/conf.d
cat > $T/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": $port, "slots": 8 },
  "server": { "server_name": "ui $name", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "ranked": { "autostart_when": "$when" }, "webrtc_port_range_begin": $((port + 9000)), "webrtc_port_range_end": $((port + 9005)) },
  "server_private": { "identity_broker_public_key": "$BROKER_PUBLIC_KEY" } }
J
(cd $(cd "$(dirname "$0")/../../../../.." && pwd)/hypersomnia && setsid timeout ${SECS:-3000} $BIN --dedicated-server --appdata-dir $T --no-update-on-launch > $T.out 2>&1 < /dev/null &)
sleep 5

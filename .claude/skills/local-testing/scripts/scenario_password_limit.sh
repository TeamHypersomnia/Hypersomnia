#!/bin/bash
# Wrong server passwords: a password-protected server (localhost not auto-authorized), 7 joins without the password.
# Expected: 5 times "Wrong password", then "Too many wrong passwords" for the rest of the minute.
export ALSOFT_DRIVERS=null VISUAL=true
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
T=$S/pw_test
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
if ss -tuln | grep -q ":10040 "; then echo "port 10040 busy, abort"; exit 1; fi
rm -rf $T; mkdir -p $T/srv/user/conf.d $T/c/user/conf.d
cat > $T/srv/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": 10040, "slots": 8 },
  "server": { "server_name": "pw test", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": 19440, "webrtc_port_range_end": 19445 },
  "server_private": { "server_password": "sekret" } }
J
cat > $T/c/user/conf.d/test.json <<J
{ "identity_broker_url": "http://127.0.0.1:1", "client": { "nickname": "pwtest" } }
J
cd $REPO/hypersomnia
timeout 200 $BIN --dedicated-server --appdata-dir $T/srv --no-update-on-launch > $T/srv.out 2>&1 &
sleep 6
for i in 1 2 3 4 5 6 7; do
	timeout 7 xvfb-run -a $BIN --connect 127.0.0.1:10040 --appdata-dir $T/c --no-update-on-launch > $T/c$i.out 2>&1
done
grep -hE "\(reason=" $T/srv.out | sed 's/^\[[0-9:]*\] //' | uniq -c
for p in $(pgrep -f "^$BIN .*--appdata-dir $T/"); do kill $p; done

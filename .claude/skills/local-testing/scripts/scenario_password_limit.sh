#!/bin/bash
# Wrong server passwords: a password-protected server (localhost not auto-authorized), 7 joins without the password.
# Expected: 5 times "Wrong password", then "Too many wrong passwords" for the rest of the minute. The limit is per
# address: an 8th join from this machine's LAN address is told "Wrong password" again, never locked out with the rest.
# Prints the reasons counted ("5 (reason=Wrong password" ...) and "lan: <reason>" (or "lan: SKIP" without a LAN address).
. "$(dirname "$0")/test_env.sh"
umask 077
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. "$HERE/lib.sh"
S=${SCRATCH:-/tmp/hypersomnia-nettest}
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
if ss -tuln | grep -q ":10040 "; then echo "port 10040 busy, abort"; exit 1; fi
# A fresh directory every run - never removed recursively
T=$S/pw_test.$(date +%s%N); mkdir -p $T/srv/user/conf.d $T/c/user/conf.d
cleanup_on_exit $T
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
timeout 200 $BIN --dedicated-server --appdata-dir $T/srv $GAME_TEST_FLAGS > $T/srv.out 2>&1 &
wait_listening $T/srv.out

reasons() { grep -c "(reason=" $T/srv.out; }

# join <out> <address> - one start of the game until the server kicked it (a reason in its log), then gone
join() {
	local before=$(reasons)
	timeout 40 xvfb-run -d $BIN --connect $2:10040 --appdata-dir $T/c $GAME_TEST_FLAGS --mute > $T/$1.out 2>&1 &
	wait_until 30 bash -c "[ \$(grep -c '(reason=' $T/srv.out) -gt $before ]" || echo "  ($1: no reason from the server in 30 s)"
	for p in $(pgrep -f -- "^$BIN --connect .*--appdata-dir $T/c "); do kill $p; done
	wait_until 10 gone "^$BIN --connect .*--appdata-dir $T/c "
}

for i in 1 2 3 4 5 6 7; do
	join c$i 127.0.0.1
done

grep -hE "\(reason=" $T/srv.out | sed 's/^\[[0-9:]*\] //' | uniq -c
LAN=$(lan_address)

if [ -n "$LAN" ]; then
	join lan $LAN
	echo "lan: $(grep -h "(reason=" $T/srv.out | tail -1 | sed 's/.*(reason=//')"
else
	echo "lan: SKIP (no LAN address)"
fi

#!/bin/bash
# The regression suite after any change to networking, accounts, servers or the site: every check below, one after
# another, in a fresh scratch directory, then a PASS/FAIL table. Exit code 0 only if all passed. About 13 minutes.
#
#   regression.sh                 the local checks
#   SOAK=1 regression.sh          + the netcode soak (~9000 connections, ~35 min)
#   REMOTE=1 regression.sh        + the remote checks (arena, arenaus): the site's MMR rules on the remote test backend,
#                                   one real ranked match through real NAT, a web client over TURN only
#   ONLY="mmr keys" regression.sh only the named checks (names as in the table)
#
# Builds first: the Debug build in build/Debug-x64-clang-claude (BIN=...). REMOTE also needs the Ubuntu 22.04 headless
# build and the web build (build_headless_ubuntu2204.sh, build_web.sh) - it uploads them.
# Uses ports 13500 (the scratch site), 17000-17260, 18000-18760 and 19000-19760; checks they are free.
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. "$HERE/test_env.sh"
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
# Short: the clients' downloaded maps lie deep inside, and the game refuses too long paths ("Absolute resolved file path
# is too long") - a scratchpad under /tmp/claude-.../ is already too long for the ranked corners.
ROOT=${SCRATCH_ROOT:-/tmp/hsreg}
# A fresh directory every run - never removed recursively
export SCRATCH=$ROOT/$(date +%m%d%H%M%S)
mkdir -p $SCRATCH
LOG=$SCRATCH/logs
mkdir -p $LOG

declare -a NAMES RESULTS TIMES

wanted() {
	[ -z "$ONLY" ] || [[ " $ONLY " == *" $1 "* ]]
}

# run <name> <timeout secs> <command...> - the command's output goes to $LOG/<name>.log;
# the check that follows reads it and says PASS or FAIL
run() {
	local name=$1 secs=$2; shift 2
	echo "=== $name"
	local start=$(date +%s)
	# SIGTERM at the time (the scenario's own cleanup runs then), SIGKILL 20 s later if it still hangs
	( cd $HERE && timeout --kill-after=20 $secs "$@" ) > $LOG/$name.log 2>&1
	LAST_EXIT=$?
	LAST_SECS=$(( $(date +%s) - start ))
}

record() {
	NAMES+=("$1"); RESULTS+=("$2"); TIMES+=("$LAST_SECS")
	echo "    $2 ($LAST_SECS s) - $LOG/$1.log"
}

# check <name> <condition as a command> - after run
check() {
	local name=$1; shift
	if "$@"; then record $name PASS; else record $name FAIL; fi
}

has() { grep -a -q -E "$2" "$LOG/$1.log"; }
count_is() { [ "$(grep -a -c -E "$2" "$LOG/$1.log")" = "$3" ]; }

for port in 13500 18000 18410 18420 18430 18710 18720 10002 10040 10042 10070 10080 17000 17100 17110; do
	if ss -tuln | grep -q ":$port "; then echo "port $port is busy - stop what uses it first"; exit 1; fi
done

# --- the scratch site (broker, match reports) ---

# Ctrl+C or a kill: the scratch site stopped too - left on 13500 it fails the next run's port check
trap 'timeout 60 $HERE/broker.sh stop > /dev/null 2>&1' EXIT
trap 'exit 130' INT TERM

openssl rand -hex 32 > $SCRATCH/session_secret
export SESSION_SECRET=$(cat $SCRATCH/session_secret)
timeout 120 $HERE/broker.sh start > $LOG/broker.log 2>&1
eval "$(grep -E '^BROKER_(URL|PUBLIC_KEY)=' $LOG/broker.log)"
export BROKER_URL BROKER_PUBLIC_KEY
printf 'BROKER_URL=%s\nBROKER_PUBLIC_KEY=%s\n' "$BROKER_URL" "$BROKER_PUBLIC_KEY" > $SCRATCH/broker_env
[ -n "$BROKER_URL" ] || { echo "the scratch site did not start - see $LOG/broker.log"; exit 1; }
SITE=$SCRATCH/broker_site

# --- the checks ---

if wanted unit_tests; then
	run unit_tests 300 bash -c "cd $REPO/hypersomnia && xvfb-run -d $BIN --unit-tests-only $GAME_TEST_FLAGS"
	check unit_tests [ $LAST_EXIT = 0 ]
fi

if wanted netcode; then
	run netcode 600 bash -c "Y=$REPO/src/3rdparty/yojimbo; cd $SCRATCH && printf 'void netcode_test(void);int netcode_init(void);void netcode_term(void);int main(void){netcode_init();netcode_test();netcode_term();return 0;}' > nct.c && clang -O1 -DNETCODE_ENABLE_TESTS=1 -DNETCODE_DEBUG=1 -I\$Y/netcode -I\$Y/include nct.c \$Y/netcode/netcode.c -lsodium -lm -o nctest && stdbuf -oL ./nctest"
	check netcode [ $LAST_EXIT = 0 ]
fi

if wanted cert_vectors; then
	run cert_vectors 60 node $SITE/tools/identity_cert_vectors_test.js
	check cert_vectors has cert_vectors "vectors: OK"
fi

if wanted mmr; then
	run mmr 300 bash -c "cd $SITE && python3 run_mmr_tests.py 13500"
	check mmr has mmr "All [0-9]+ assertions passed"
fi

if wanted link_flow; then
	run link_flow 300 python3 link_flow_test.py $SITE $BROKER_URL
	check link_flow has link_flow "^0 failed"
fi

if wanted password_limit; then
	run password_limit 400 ./scenario_password_limit.sh
	check password_limit bash -c "grep -a -q -E '5 \(reason=Wrong password' $LOG/password_limit.log && grep -a -q -E '2 \(reason=Too many wrong passwords' $LOG/password_limit.log && grep -a -q -E '^lan: (Wrong password|SKIP)' $LOG/password_limit.log"
fi

if wanted keys; then
	run keys 400 ./scenario_keys.sh
	# in, not only authenticated (a server could still refuse after it): the public vars came to all three; the official
	# two listed as such; account.key private
	check keys bash -c "[ \$(grep -a -c 'Authenticated' $LOG/keys.log) = 3 ] && [ \$(grep -a -c 'Received initial public vars' $LOG/keys.log) = 3 ] && [ \$(grep -a -c 'official= True' $LOG/keys.log) = 2 ] && [ \$(grep -a -c ': 600\$' $LOG/keys.log) = 3 ]"
fi

if wanted admin; then
	run admin 300 ./scenario_admin.sh
	# case B: in without the password - authenticated, not only "never asked"
	check admin bash -c "grep -a -q 'case A: 1 wrong password' $LOG/admin.log && grep -a -q -E 'case B: 0 wrong password, [1-9][0-9]* authenticated' $LOG/admin.log"
fi

if wanted ranked_report; then
	run ranked_report 400 ./scenario_ranked_report.sh
	check ranked_report has ranked_report "delivered \(HTTP 200\)"
fi

if wanted outage; then
	run outage 200 ./scenario_outage.sh
	# no masterserver, no broker: a self-signed cert plays casual and ranked
	check outage bash -c "[ \$(grep -a -c 'Received initial state' $LOG/outage.log) = 2 ]"
fi

if wanted webrtc_ranked; then
	run webrtc_ranked 200 ./scenario_webrtc_ranked.sh
	check webrtc_ranked has webrtc_ranked "=== result: in over WebRTC"
fi

if wanted webrtc_crowd; then
	run webrtc_crowd 240 ./scenario_webrtc_crowd.sh 6
	check webrtc_crowd bash -c "grep -a -A1 'started together: in game' $LOG/webrtc_crowd.log | tail -1 | grep -q '^6\$' && grep -a -A1 'refused for too many' $LOG/webrtc_crowd.log | tail -1 | grep -q '^0\$'"
fi

if wanted handshake_limit; then
	run handshake_limit 200 bash -c "./soak/build_soak.sh && ./soak/scenario_handshake_limit.sh"
	# hit from the LAN address, the same address in again a moment later, loopback never limited (or no LAN address)
	check handshake_limit bash -c "grep -a -q '^SKIP: no LAN address' $LOG/handshake_limit.log || { grep -a -q -E 'refusals logged: [1-9]' $LOG/handshake_limit.log && grep -a -q 'done: 10 cycles, 10 connected, 0 abandoned mid-handshake, 0 failed' $LOG/handshake_limit.log && grep -a -q 'done: 200 cycles, 200 connected, 0 abandoned mid-handshake, 0 failed' $LOG/handshake_limit.log; }"
fi

# servers never quietly stop taking players: real games on a server of 1 slot, every way of leaving
if wanted slot_reuse; then
	run slot_reuse 1500 ./scenario_slot_reuse.sh
	check slot_reuse has slot_reuse "^[1-9][0-9]* passed, 0 failed$"
fi

for c in broker_down unlink; do
	if wanted corners_$c; then
		run corners_$c 420 env CASE=$c STRANGER=1 ./scenario_ranked_corners.sh
		# unlink: alice may come back as steam_ or be refused (an accepted loss, see the scenario) - only broker_down must
		# bring her back as steam_
		steam_back="[ \$(grep -a -c 'Authenticated.*steam_' $LOG/corners_$c.log) -ge 2 ]"
		[ $c = unlink ] && steam_back=true
		check corners_$c bash -c "grep -a -q 'A match is being played there' $LOG/corners_$c.log && grep -a -q 'delivered (HTTP 200)' $LOG/corners_$c.log && $steam_back"
	fi
done

if [ -n "$SOAK" ] && wanted soak; then
	run soak 3000 bash -c "./soak/build_soak.sh && ./soak/soak_netcode.sh 1"
	# All 8 soak runs ended (a crashed one prints no "done:"), every one "0 failed" except the one wrong key that must
	# fail; then a real client gets in
	check soak bash -c "[ \"\$(grep -a -c 'done:' $LOG/soak.log)\" = 8 ] && [ -z \"\$(grep -a 'done:' $LOG/soak.log | grep -v ' 0 failed' | grep -v '0 connected, 0 abandoned mid-handshake, 1 failed')\" ] && grep -a -q '0 connected, 0 abandoned mid-handshake, 1 failed' $LOG/soak.log && grep -a -q 'Received initial state' $LOG/soak.log"
fi

if [ -n "$REMOTE" ] && wanted remote; then
	run remote 1800 ./remote_e2e/regression_remote.sh
	check remote has remote "^REMOTE: all passed"
fi

# --- the table ---

echo
printf '%-16s %-6s %6s\n' CHECK RESULT SECS
failed=0
for i in "${!NAMES[@]}"; do
	printf '%-16s %-6s %6s\n' "${NAMES[$i]}" "${RESULTS[$i]}" "${TIMES[$i]}"
	[ "${RESULTS[$i]}" = PASS ] || failed=$((failed + 1))
done
echo
echo "logs: $LOG"
[ $failed = 0 ] && echo "ALL PASSED" || echo "$failed FAILED"
exit $failed

# Helpers for the scenarios and builds - sourced after test_env.sh: . "$HERE/lib.sh"

# wait_until <secs> <command...> - every quarter second until the command succeeds; 1 when the time runs out
wait_until() {
	local secs=$1; shift
	local end=$(($(date +%s) + secs))

	until "$@" > /dev/null 2>&1; do
		[ $(date +%s) -ge $end ] && return 1
		sleep 0.25
	done
}

gone() { ! pgrep -f -- "$1" > /dev/null; }

# kill_by <signal> <pattern> - and wait until they are gone; whatever ignores the signal for 15 s gets SIGKILL
kill_by() {
	for pid in $(pgrep -f -- "$2"); do kill $1 $pid 2>/dev/null; done

	if ! wait_until 15 gone "$2"; then
		echo "  (still running after 15 s: $2 - SIGKILL)"
		for pid in $(pgrep -f -- "$2"); do kill -9 $pid 2>/dev/null; done
		wait_until 5 gone "$2"
	fi
}

# wait_listening <log> [secs] - a server (or a masterserver's list) started: netcode says it listens
wait_listening() {
	wait_until ${2:-30} grep -q "server listening on" $1 || echo "  (no \"server listening on\" in $1 after ${2:-30} s)"
}

# free_display [from] - the first X display number from <from> (default 96) with no lock
free_display() {
	local d=${1:-96}
	while [ -e /tmp/.X$d-lock ]; do d=$((d + 1)); done
	echo $d
}

# lan_address - this machine's address on its network (empty without a default route)
lan_address() {
	ip -4 route get 1.1.1.1 2>/dev/null | awk '{ for (i = 1; i < NF; i++) if ($i == "src") print $(i + 1) }'
}

# short_ranked_map <maps dir> [max team score] [round time] - de_cyberaqua as de_e2e_short in a server's
# downloads/maps: 2 rounds of 5 s by default, no warmup - idle players end a ranked match 1:1 in about a minute
short_ranked_map() {
	local a=$1/de_e2e_short
	mkdir -p $1
	cp -r $REPO/hypersomnia/content/arenas/de_cyberaqua $a
	mv $a/de_cyberaqua.json $a/de_e2e_short.json
	rm -f $a/last_saved.json $a/editor_view.json
	python3 - $a/de_e2e_short.json ${2:-2} ${3:-5} <<'PY'
import json, sys
p = sys.argv[1]
d = json.load(open(p))
d['meta']['name'] = 'de_e2e_short'
d['game_modes']['bomb_defusal'].update({
	'max_team_score': int(sys.argv[2]), 'warmup_time': 0, 'freeze_time': 1, 'buy_time': 1,
	'round_time': int(sys.argv[3]), 'round_end_time': 1.0, 'default_bot_quota': 0
})
json.dump(d, open(p, 'w'), indent=1)
PY
}

# wait_build_slot - one build on the machine at a time (31 GB, no swap - two full ones freeze the desktop): takes a slot
# of the global lock (MAX_BUILDS, 1 by default) on fd 9 and waits while any other ninja runs (the user's, the build
# MCP's). The caller keeps fd 9 open for the whole build - its ninja inherits it, the lock goes with the last process.
wait_build_slot() {
	local locks=${XDG_RUNTIME_DIR:-/tmp}/hypersomnia-builds max=${MAX_BUILDS:-1} said=""
	mkdir -p $locks

	while true; do
		for slot in $(seq 1 $max); do
			exec 9> $locks/slot$slot

			if flock -n 9; then
				[ "$(pgrep -x ninja | wc -l)" -lt $max ] && return 0
				flock -u 9
			fi
		done

		[ -z "$said" ] && echo "waiting for another build to end ($(pgrep -ax ninja | head -3 | tr '\n' ' '))" && said=1
		sleep 10
	done
}

# cleanup_on_exit <dir> [extra pids...] - the end, Ctrl+C, a kill or regression.sh's timeout: the games of <dir> first
# (SIGKILL, anchored on the game's own binary - never its xvfb-run or timeout, which then take their Xvfb down), then
# SIGTERM to everything else with <dir>/ in its command line (servers, timeout, xvfb-run, setsid ones), the extra pids
# (an Xvfb, a proxy) and the background jobs.
cleanup_on_exit() {
	CLEANUP_DIR=$1; shift
	CLEANUP_PIDS="$*"
	trap __cleanup EXIT
	trap 'exit 130' INT TERM
}

# cleanup_also <pids...> - more to stop at the end (an Xvfb started later)
cleanup_also() { CLEANUP_PIDS="$CLEANUP_PIDS $*"; }

__cleanup() {
	for pid in $(pgrep -f -- "^[^ ]*/Hypersomnia .*$CLEANUP_DIR/"); do kill -CONT $pid 2>/dev/null; kill -9 $pid 2>/dev/null; done
	sleep 1
	for pid in $(pgrep -f -- "$CLEANUP_DIR/") $CLEANUP_PIDS; do kill -CONT $pid 2>/dev/null; kill $pid 2>/dev/null; done
	kill $(jobs -p) 2>/dev/null
}

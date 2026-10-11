#!/bin/bash
# One end-to-end ranked match: dedicated server on arena (match.sh), clients here, backend on arenaus.
# The second client is killed once the match is live, so it abandons and the match ends with a report at once.
# Usage: run_match.sh <client A> <client B> [live wait secs]
# Silent: OpenAL plays to its null backend, so the test clients never sound on the user's speakers.
export ALSOFT_DRIVERS=null
# A failing game opens its failure log with $VISUAL - never in the user's editor
export VISUAL=true
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
E=${SCRATCH:-/tmp/hypersomnia-nettest}/e2e
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
ARENA_IP=${ARENA_IP:-57.128.242.21}
ARENAUS_IP=${ARENAUS_IP:-154.12.226.211}
A=$1
B=$2
LIVE_WAIT=${3:-35}

for p in $(pgrep -f "^$BIN --connect $ARENA_IP:9005"); do kill $p; done

echo "=== server: $A vs $B"
timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh start $(cat $E/id_$A) $(cat $E/id_$B)"
sleep 6

echo "=== test masterserver list"
curl -s --max-time 5 http://$ARENAUS_IP:18420/server_list_json | python3 -c "
import json,sys
for e in json.load(sys.stdin): print(' ', e['name'], e['ip'], 'official=', e.get('is_official'), 'ranked=', e.get('is_ranked'), 'domain=', e.get('domain'), 'shown=', e.get('site_displayed_address'))"

cd $REPO/hypersomnia
xvfb-run -a $BIN --connect $ARENA_IP:9005 --appdata-dir $E/client_$A --no-update-on-launch > $E/$A.out 2>&1 &
xvfb-run -a $BIN --connect $ARENA_IP:9005 --appdata-dir $E/client_$B --no-update-on-launch > $E/$B.out 2>&1 &

sleep $LIVE_WAIT

echo "=== killing $B (abandon)"
for p in $(pgrep -f "^$BIN --connect $ARENA_IP:9005 --appdata-dir $E/client_$B"); do kill -9 $p; done
sleep 15

echo "=== dedicated server log"
timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh log"

for p in $(pgrep -f "^$BIN --connect $ARENA_IP:9005"); do kill $p; done
timeout 60 ssh arena "bash ~/yojimbo-test/e2e/match.sh stop" > /dev/null
sleep 2

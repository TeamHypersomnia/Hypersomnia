#!/bin/bash
# A bot match on any map with a spectator client - see the run-map skill (MAP, MAP_DIR, SECS, LOG_GREP, SERVER_EXTRA...).
HERE=$(cd "$(dirname "$0")" && pwd)
BOTS=1 exec "$HERE/../../run-map/scripts/run_map.sh" "$@"

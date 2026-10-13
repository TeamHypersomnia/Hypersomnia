#!/bin/bash
# Prints a fresh link code for a Steam account on the scratch site: link_code.sh [steam id64]
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
STEAM=${1:-76561198051900812}
export SESSION_SECRET=$(cat $S/nettest/session_secret)
COOKIE=$(python3 $(dirname "$0")/../fake_steam_session.py $S/nettest/broker_site $STEAM "Pythagoras of Samos")
# The confirm page's [Link] - a POST makes the code
curl -s -o /dev/null -D - -H "Cookie: $COOKIE" -d "state=0123456789abcdef0123456789abcdef&port=" "http://127.0.0.1:13500/link_key/start" | grep -i "^location" | sed 's/.*#code=\([A-Z0-9]*\).*/\1/'

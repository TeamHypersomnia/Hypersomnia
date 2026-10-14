#!/bin/bash
# Prints a fresh link code for a Steam account on the scratch site, made for a game key: link_code.sh <key hex> [steam id64]
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
KEY=$1
STEAM=${2:-76561198051900812}
[ -n "$KEY" ] || { echo "usage: link_code.sh <key hex> [steam id64]"; exit 1; }
export SESSION_SECRET=$(cat $S/nettest/session_secret)
COOKIE=$(python3 $(dirname "$0")/../fake_steam_session.py $S/nettest/broker_site $STEAM "Pythagoras of Samos")
STATE=$(openssl rand -hex 16)
# The confirm page's [Link] makes the code under the state; the game would fetch it from /link_key/result
curl -s -o /dev/null -H "Cookie: $COOKIE" -d "state=$STATE&key=$KEY" "http://127.0.0.1:13500/link_key/start"
curl -s "http://127.0.0.1:13500/link_key/result?state=$STATE" | sed 's/.*"code":"\([A-Z0-9]*\)".*/\1/'

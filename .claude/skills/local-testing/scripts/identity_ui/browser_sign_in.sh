#!/bin/bash
# Plays the player's browser for "Link with Steam": takes the last /link_key/start URL the game opened
# (see fake_bin/xdg-open), opens it signed in to Steam anew for it on the scratch site and clicks the page's [Link].
# The game, asking for its cert every 3 s, then learns by itself that it is linked.
# Usage: OPENED_URLS=<file> browser_sign_in.sh [steam id64]   - prints the confirm page's and the result page's HTTP status
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
STEAM=${1:-76561198051900812}
URL=$(grep -E "/link_key/start" "${OPENED_URLS:-/tmp/hypersomnia-opened-urls}" | tail -1)
[ -n "$URL" ] || { echo "no /link_key/start URL opened"; exit 1; }
STATE=$(echo "$URL" | sed -E 's/.*[?&]state=([0-9a-f]+).*/\1/')
export SESSION_SECRET=$(cat ${SESSION_SECRET_FILE:-$S/nettest/session_secret})
COOKIE=$(python3 $(dirname "$0")/../fake_steam_session.py ${SITE_DIR:-$S/nettest/broker_site} $STEAM "Pythagoras of Samos" $STATE)
PAGE=$(curl -s -o /dev/null -w "%{http_code}" -H "Cookie: $COOKIE" "$URL")
echo "confirm page: HTTP $PAGE"
# [Link] posts what the page got: the state, the game's key, the device
QUERY=${URL#*\?}
curl -s -o /dev/null -w "result page: HTTP %{http_code}\n" -H "Cookie: $COOKIE" --data "$QUERY" "${URL%%\?*}"

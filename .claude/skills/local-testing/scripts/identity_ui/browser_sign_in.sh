#!/bin/bash
# Plays the player's browser for "Link with Steam": takes the last /link_key/start URL the game opened (see fake_bin/xdg-open),
# opens it signed in to Steam on the scratch site, and follows the redirect back to the game's listener on 127.0.0.1.
# Usage: OPENED_URLS=<file> browser_sign_in.sh [steam id64]   - prints what the listener answered (HTTP status)
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
STEAM=${1:-76561198051900812}
URL=$(grep "/link_key/start" "${OPENED_URLS:-/tmp/hypersomnia-opened-urls}" | tail -1)
[ -n "$URL" ] || { echo "no /link_key/start URL opened"; exit 1; }
export SESSION_SECRET=$(cat $S/nettest/session_secret)
COOKIE=$(python3 $(dirname "$0")/../fake_steam_session.py $S/nettest/broker_site $STEAM "Pythagoras of Samos")
# The page asks which Steam account to link to; [Link] posts the same state and port
PAGE=$(curl -s -o /dev/null -w "%{http_code}" -H "Cookie: $COOKIE" "$URL")
echo "confirm page: HTTP $PAGE"
QUERY=${URL#*\?}
STATE=$(echo "$QUERY" | sed -n 's/.*state=\([0-9a-f]*\).*/\1/p')
PORT=$(echo "$QUERY" | sed -n 's/.*port=\([0-9]*\).*/\1/p')
BACK=$(curl -s -o /dev/null -D - -H "Cookie: $COOKIE" -d "state=$STATE&port=$PORT" "${URL%%\?*}" | grep -i "^location" | tr -d '\r' | sed 's/^[Ll]ocation: //')
echo "site redirects to: $(echo "$BACK" | sed 's/code=[A-Z0-9]*/code=.../')"
curl -s -o /dev/null -w "listener: HTTP %{http_code}\n" "$BACK"

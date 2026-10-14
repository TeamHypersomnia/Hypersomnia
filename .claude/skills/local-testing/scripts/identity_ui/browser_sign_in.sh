#!/bin/bash
# Plays the player's browser for "Link with Steam" or "Unlink from Steam...": takes the last /link_key/start or /link_key/unlink
# URL the game opened (see fake_bin/xdg-open), opens it signed in to Steam on the scratch site and clicks the page's button.
# The game, asking /link_key/result with its state every second, then learns how it went by itself.
# Usage: OPENED_URLS=<file> browser_sign_in.sh [steam id64]   - prints the confirm page's and the result page's HTTP status
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
STEAM=${1:-76561198051900812}
URL=$(grep -E "/link_key/(start|unlink)" "${OPENED_URLS:-/tmp/hypersomnia-opened-urls}" | tail -1)
[ -n "$URL" ] || { echo "no /link_key/start or /link_key/unlink URL opened"; exit 1; }
export SESSION_SECRET=$(cat $S/nettest/session_secret)
COOKIE=$(python3 $(dirname "$0")/../fake_steam_session.py $S/nettest/broker_site $STEAM "Pythagoras of Samos")
PAGE=$(curl -s -o /dev/null -w "%{http_code}" -H "Cookie: $COOKIE" "$URL")
echo "confirm page: HTTP $PAGE"
# [Link] (or [Unlink this game key]) posts what the page got: the state, the game's key, the device
QUERY=${URL#*\?}
curl -s -o /dev/null -w "result page: HTTP %{http_code}\n" -H "Cookie: $COOKIE" --data "$QUERY" "${URL%%\?*}"

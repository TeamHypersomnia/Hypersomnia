#!/bin/bash
# Signs in to the test site on arenaus as a Steam user without Steam: inserts a passport session into its database and
# prints the matching connect.sid cookie, signed with the session secret backend.sh generated at its start.
# Usage: fake_steam_session.sh <steam id64> [display name]
STEAM_ID=$1
NAME=${2:-E2E Steam}

[[ "$STEAM_ID" =~ ^[0-9]+$ ]] || { echo "steam id64: digits only"; exit 1; }
NAME_HEX=$(printf '%s' "$NAME" | xxd -p | tr -d '\n')

timeout 60 ssh arenaus "python3 - $STEAM_ID $NAME_HEX" <<'EOF'
import base64, hashlib, hmac, json, secrets, sqlite3, sys, urllib.parse
from datetime import datetime, timedelta, timezone

steam_id, name = sys.argv[1], bytes.fromhex(sys.argv[2]).decode()
secret = open('/home/ubuntu/hs-test-backend/session_secret').read().strip().encode()
sid = secrets.token_urlsafe(24)
expires = datetime.now(timezone.utc) + timedelta(days=1)

sess = {
	'cookie': { 'originalMaxAge': 604800000, 'expires': expires.isoformat(), 'httpOnly': True, 'path': '/', 'sameSite': 'lax' },
	'passport': { 'user': { 'id': steam_id, 'displayName': name, 'provider': 'steam' } }
}

c = sqlite3.connect('/home/ubuntu/hs-test-backend/page/private/mmr.db')
with c:
	c.execute('INSERT INTO sessions (sid, sess, expire) VALUES (?, ?, ?)', (sid, json.dumps(sess), expires.isoformat()))

signature = base64.b64encode(hmac.new(secret, sid.encode(), hashlib.sha256).digest()).decode().rstrip('=')
print('connect.sid=' + urllib.parse.quote('s:' + sid + '.' + signature, safe=''))
EOF

#!/usr/bin/env python3
"""
Signs in to a local test site (broker.sh) as a Steam user without Steam: inserts a passport session into its DB and prints
the connect.sid cookie, signed with the SESSION_SECRET the site was started with.
Usage: SESSION_SECRET=<the site's> fake_steam_session.py <site dir> [steam id64] [name]
"""
import base64, hashlib, hmac, json, os, secrets, sqlite3, sys, urllib.parse
from datetime import datetime, timedelta, timezone

site_dir = sys.argv[1]
steam_id = sys.argv[2] if len(sys.argv) > 2 else '76561198000000001'
name = sys.argv[3] if len(sys.argv) > 3 else 'Test Steam'
secret = os.environ['SESSION_SECRET'].encode()
sid = secrets.token_urlsafe(24)
expires = datetime.now(timezone.utc) + timedelta(days=1)
sess = { 'cookie': { 'originalMaxAge': 604800000, 'expires': expires.isoformat(), 'httpOnly': True, 'path': '/' },
         'passport': { 'user': { 'id': steam_id, 'displayName': name, 'provider': 'steam' } } }
db = sqlite3.connect(os.path.join(site_dir, 'private/mmr.db'))
with db:
    db.execute('INSERT INTO sessions (sid, sess, expire) VALUES (?, ?, ?)', (sid, json.dumps(sess), expires.isoformat()))
sig = base64.b64encode(hmac.new(secret, sid.encode(), hashlib.sha256).digest()).decode().rstrip('=')
print('connect.sid=' + urllib.parse.quote('s:' + sid + '.' + sig, safe=''))

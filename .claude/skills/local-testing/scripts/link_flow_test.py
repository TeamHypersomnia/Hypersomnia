#!/usr/bin/env python3
"""
The whole key <-> Steam link flow against a test site (broker.sh), as the game does it - without Steam:
a fake Steam session in the site's DB, /link_key/start -> the code in the redirect, /link_key/info, the signed GET /link_key/link,
a cert for the key (now the Steam account), the signed GET /link_key/unlink, a cert again (the key's own account).
Usage: SESSION_SECRET=<the site's> link_flow_test.py <site dir> <site url>
"""
import base64, hashlib, hmac, json, os, secrets, sqlite3, sys, time, urllib.parse, urllib.request
from datetime import datetime, timedelta, timezone
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat

site_dir, url = sys.argv[1], sys.argv[2].rstrip('/')
secret = os.environ['SESSION_SECRET'].encode()
steam_id = '7656119' + str(secrets.randbelow(10**10)).zfill(10)
failed = []

def check(name, ok, detail=''):
    print(('PASS ' if ok else 'FAIL ') + name + (': ' + detail if detail else ''))
    if not ok: failed.append(name)

class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, *a): return None

opener = urllib.request.build_opener(NoRedirect)

def post(path, fields, cookie=None):
    req = urllib.request.Request(url + path, data=urllib.parse.urlencode(fields).encode())
    if cookie: req.add_header('Cookie', cookie)
    try:
        r = opener.open(req)
        return r.status, r.read().decode(), dict(r.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode(), dict(e.headers)

def start(state, cookie, port=''):
    # The confirm page's [Link]: the code only comes from the POST
    return post('/link_key/start', { 'state': state, 'port': port }, cookie)

def get(path, cookie=None):
    req = urllib.request.Request(url + path)
    if cookie: req.add_header('Cookie', cookie)
    try:
        r = opener.open(req)
        return r.status, r.read().decode(), dict(r.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode(), dict(e.headers)

# A Steam session, like fake_steam_session.sh
sid = secrets.token_urlsafe(24)
expires = datetime.now(timezone.utc) + timedelta(days=1)
sess = { 'cookie': { 'originalMaxAge': 604800000, 'expires': expires.isoformat(), 'httpOnly': True, 'path': '/' },
         'passport': { 'user': { 'id': steam_id, 'displayName': 'Link Test', 'provider': 'steam' } } }
db = sqlite3.connect(os.path.join(site_dir, 'private/mmr.db'))
with db:
    db.execute('INSERT INTO sessions (sid, sess, expire) VALUES (?, ?, ?)', (sid, json.dumps(sess), expires.isoformat()))
sig = base64.b64encode(hmac.new(secret, sid.encode(), hashlib.sha256).digest()).decode().rstrip('=')
cookie = 'connect.sid=' + urllib.parse.quote('s:' + sid + '.' + sig, safe='')

state = secrets.token_hex(16)
status, body, _ = get('/link_key/start?state=' + state, cookie)
check('start asks which Steam account first (no code yet)', status == 200 and 'Link Test' in body and '#code=' not in body)
status, _, headers = start(state, cookie)
location = headers.get('Location', '')
check('Link on that page goes to the return url with code and state', status == 303 and '#code=' in location and ('state=' + state) in location, location)
status, _, _ = start(secrets.token_hex(16), None)
check('Link without a session is refused', status == 400)
fragment = urllib.parse.parse_qs(location.split('#', 1)[1]) if '#' in location else {}
code = fragment.get('code', [''])[0]

status, _, headers = get('/link_key/start?state=' + state)
check('start without a session goes to Steam sign in', status == 302 and '/auth/steam' in headers.get('Location', ''), headers.get('Location', ''))

status, _, _ = get('/link_key/start?state=xyz', cookie)
check('start refuses a malformed state', status == 400)

status, body, headers = get('/link_key/info?code=' + code)
check('info names the Steam account', status == 200 and json.loads(body).get('steam_id') == 'steam_' + steam_id, body)
check('info allows the game origin (CORS)', headers.get('Access-Control-Allow-Origin') == '*')

key = Ed25519PrivateKey.generate()
pub = key.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()

def cert_account():
    session = Ed25519PrivateKey.generate().public_key().public_bytes(Encoding.Raw, PublicFormat.Raw)
    s = key.sign(b'hs-cert-req-v1' + session).hex()
    status, body, _ = get('/cert?session=%s&key=%s&sig=%s' % (session.hex(), pub, s))
    if status != 200: return 'HTTP %d %s' % (status, body)
    raw = bytes.fromhex(json.loads(body)['cert'])
    n = raw[1]
    return raw[2:2 + n].decode()

check('cert before linking is the key', cert_account() == 'key_' + pub)

# Ranked progress of the key: moves to a Steam account without matches on linking, comes back on the key's own unlink
with db:
    db.execute("INSERT INTO mmr_team (account_id, nickname, mmr, mu, sigma, matches_won, matches_lost) VALUES (?, 'k', 7.5, 30, 5, 3, 2)", ('key_' + pub,))

def mmr_owner():
    # Only this run's two accounts - the database may hold earlier runs
    rows = db.execute("SELECT account_id FROM mmr_team WHERE mmr = 7.5 AND matches_won = 3 AND account_id IN (?, ?)", ('key_' + pub, 'steam_' + steam_id)).fetchall()
    return [r[0] for r in rows]

link_sig = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code, steam_id)).encode()).hex()
status, body, headers = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, '0' * 128))
check('link refuses a bad signature', status == 401, body)
status, body, headers = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, link_sig))
check('link by GET', status == 200 and json.loads(body).get('to') == 'steam_' + steam_id, body)
check('link allows the game origin (CORS)', headers.get('Access-Control-Allow-Origin') == '*')
status, body, _ = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, link_sig))
check('a code links once', status == 401, body)

check('cert after linking is the Steam account', cert_account() == 'steam_' + steam_id)
check('progress moved to the Steam account', mmr_owner() == ['steam_' + steam_id], str(mmr_owner()))

ts = int(time.time())
unlink_sig = key.sign(('hypersomnia-unlink-v1 %s %d' % (pub, ts)).encode()).hex()
status, body, _ = get('/link_key/unlink?public_key=%s&timestamp=%d&signature=%s' % (pub, ts - 3600, unlink_sig))
check('unlink refuses another timestamp', status == 401, body)
status, body, _ = get('/link_key/unlink?public_key=%s&timestamp=%d&signature=%s' % (pub, ts, unlink_sig))
check('unlink by GET', status == 200 and json.loads(body).get('from') == 'steam_' + steam_id, body)

check('cert after unlinking is the key again', cert_account() == 'key_' + pub)
check('progress moved back to the key', mmr_owner() == ['key_' + pub], str(mmr_owner()))

status, body, _ = get('/link_key/unlink?public_key=%s&timestamp=%d&signature=%s' % (pub, ts, unlink_sig))
check('a signed unlink can not be replayed', status == 401, body)

# The native game: &port sends the code to its listener on 127.0.0.1
state2 = secrets.token_hex(16)
status, _, headers = start(state2, cookie, '40123')
location = headers.get('Location', '')
check('start with a port goes back to the game on 127.0.0.1', status == 303 and location.startswith('http://127.0.0.1:40123/linked?code=') and ('state=' + state2) in location, location)
status, _, _ = get('/link_key/start?state=%s&port=80' % state2, cookie)
check('start refuses a port below 1024', status == 400)
code2 = urllib.parse.parse_qs(urllib.parse.urlparse(location).query).get('code', [''])[0]

# Bans: a key unlinked from a banned Steam account stays banned; a banned key can not be linked
link_sig2 = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code2, steam_id)).encode()).hex()
status, body, _ = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code2, link_sig2))
check('relink', status == 200, body)
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))
ts2 = ts + 1
unlink_sig2 = key.sign(('hypersomnia-unlink-v1 %s %d' % (pub, ts2)).encode()).hex()
status, body, _ = get('/link_key/unlink?public_key=%s&timestamp=%d&signature=%s' % (pub, ts2, unlink_sig2))
check('unlink from a banned account', status == 200, body)
check('the unlinked key is banned too', cert_account().startswith('HTTP 403'), cert_account())
state3 = secrets.token_hex(16)
_, _, headers = start(state3, cookie)
code3 = urllib.parse.parse_qs(headers.get('Location', '').split('#', 1)[-1]).get('code', [''])[0]
link_sig3 = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code3, steam_id)).encode()).hex()
status, body, _ = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code3, link_sig3))
check('a banned key can not be linked', status == 403, body)
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

# Unlinking on /profile works like the key's own unlink: the Steam account's ban goes with the key
state4 = secrets.token_hex(16)
_, _, headers = start(state4, cookie)
code4 = urllib.parse.parse_qs(headers.get('Location', '').split('#', 1)[-1]).get('code', [''])[0]
link_sig4 = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code4, steam_id)).encode()).hex()
status, body, _ = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code4, link_sig4))
check('link again', status == 200, body)
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))
status, _, _ = post('/profile/unlink_key', { 'key': 'key_' + pub }, cookie)
check('unlink on /profile', status in (302, 303) and cert_account().startswith('HTTP 403'), cert_account())
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

# A banned Steam account posting someone else's key to /profile/unlink_key: that key is not its, nothing happens to it
foreign = 'key_' + secrets.token_hex(32)
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))
post('/profile/unlink_key', { 'key': foreign }, cookie)
banned_foreign = db.execute('SELECT 1 FROM broker_bans WHERE account = ?', (foreign,)).fetchone()
check('a foreign key posted to /profile/unlink_key is not banned', banned_foreign is None, str(banned_foreign))

# No linking to a banned Steam account
state5 = secrets.token_hex(16)
_, _, headers = start(state5, cookie)
code5 = urllib.parse.parse_qs(headers.get('Location', '').split('#', 1)[-1]).get('code', [''])[0]
link_sig5 = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code5, steam_id)).encode()).hex()
status, body, _ = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code5, link_sig5))
check('no link to a banned Steam account', status == 403 and 'banned' in body, '%s %s' % (status, body))
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

print('%d failed' % len(failed))
sys.exit(1 if failed else 0)

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
status, _, headers = get('/link_key/start?state=' + state, cookie)
location = headers.get('Location', '')
check('start redirects to the return url with code and state', status == 302 and '#code=' in location and ('state=' + state) in location, location)
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
    raw = bytes.fromhex(body)
    n = raw[1]
    return raw[2:2 + n].decode()

check('cert before linking is the key', cert_account() == 'key_' + pub)

link_sig = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code, steam_id)).encode()).hex()
status, body, headers = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, '0' * 128))
check('link refuses a bad signature', status == 401, body)
status, body, headers = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, link_sig))
check('link by GET', status == 200 and json.loads(body).get('to') == 'steam_' + steam_id, body)
check('link allows the game origin (CORS)', headers.get('Access-Control-Allow-Origin') == '*')
status, body, _ = get('/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, link_sig))
check('a code links once', status == 401, body)

check('cert after linking is the Steam account', cert_account() == 'steam_' + steam_id)

ts = int(time.time())
unlink_sig = key.sign(('hypersomnia-unlink-v1 %s %d' % (pub, ts)).encode()).hex()
status, body, _ = get('/link_key/unlink?public_key=%s&timestamp=%d&signature=%s' % (pub, ts - 3600, unlink_sig))
check('unlink refuses another timestamp', status == 401, body)
status, body, _ = get('/link_key/unlink?public_key=%s&timestamp=%d&signature=%s' % (pub, ts, unlink_sig))
check('unlink by GET', status == 200 and json.loads(body).get('from') == 'steam_' + steam_id, body)

check('cert after unlinking is the key again', cert_account() == 'key_' + pub)

print('%d failed' % len(failed))
sys.exit(1 if failed else 0)

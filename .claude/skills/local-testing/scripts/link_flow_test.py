#!/usr/bin/env python3
"""
The whole key <-> Steam link flow against a test site (broker.sh), as the game does it - without Steam:
a fake Steam session in the site's DB (signed in anew for the page's state), the confirm page /link_key/start?state&t&key&sig,
its POST (links at once), the game's cert (now the Steam account), the game's signed unlink GET, a cert again (the key's
own account), replays refused by the last change's time, bans, the clock slack.
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
    print(('PASS ' if ok else 'FAIL ') + name + (': ' + detail if detail and not ok else ''))
    if not ok: failed.append(name)

class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, *a): return None

opener = urllib.request.build_opener(NoRedirect)

# Every request from its own address (X-Real-IP, taken from loopback): the site's per-address limits never hit the test
def own_address(req):
    req.add_header('X-Real-IP', '10.%d.%d.%d' % (secrets.randbelow(256), secrets.randbelow(256), secrets.randbelow(256)))

def post(path, fields, cookie=None):
    req = urllib.request.Request(url + path, data=urllib.parse.urlencode(fields).encode())
    own_address(req)
    if cookie: req.add_header('Cookie', cookie)
    try:
        r = opener.open(req)
        return r.status, r.read().decode(), dict(r.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode(), dict(e.headers)

def get(path, cookie=None):
    req = urllib.request.Request(url + path)
    own_address(req)
    if cookie: req.add_header('Cookie', cookie)
    try:
        r = opener.open(req)
        return r.status, r.read().decode(), dict(r.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode(), dict(e.headers)

def fields(state, signer=None, t=None):
    # What the game puts in the URL: its state signed with its key, the device
    signer = signer or key
    public = signer.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()
    t = t or str(int(time.time()))
    return { 'state': state, 't': t, 'key': public, 'sig': signer.sign(('hypersomnia-link-v1 %s %s' % (state, t)).encode()).hex(), 'device': 'test', 'os': 'linux' }

def signed_in_anew_for(state):
    # What the Steam sign-in started from the link page leaves in the session (auth.js): this state's page may link
    with db:
        row = db.execute('SELECT sess FROM sessions WHERE sid = ?', (sid,)).fetchone()
        sess = json.loads(row[0])
        sess['fresh_link_state'] = state
        db.execute('UPDATE sessions SET sess = ? WHERE sid = ?', (json.dumps(sess), sid))

def confirm(state, cookie, signer=None, t=None, fresh=True):
    # The confirm page's [Link]: links at once
    if fresh: signed_in_anew_for(state)
    return post('/link_key/start', fields(state, signer, t), cookie)

def unlink(t=None, signer=None):
    # "Unlink from Steam..." in the game: one GET signed by the key, answered at once
    signer = signer or key
    public = signer.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()
    t = t or str(int(time.time()))
    status, body, headers = get('/link_key/unlink?' + urllib.parse.urlencode({ 't': t, 'key': public, 'sig': signer.sign(('hypersomnia-unlink-v1 %s' % t).encode()).hex() }))
    try: return status, json.loads(body), headers
    except Exception: return status, { 'raw': body }, headers

def link(signer=None, t=None):
    status, body, _ = confirm(secrets.token_hex(16), cookie, signer, t)
    return status, body

# A Steam session, like fake_steam_session.py
sid = secrets.token_urlsafe(24)
expires = datetime.now(timezone.utc) + timedelta(days=1)
sess = { 'cookie': { 'originalMaxAge': 604800000, 'expires': expires.isoformat(), 'httpOnly': True, 'path': '/' },
         'passport': { 'user': { 'id': steam_id, 'displayName': 'Link Test', 'provider': 'steam' } } }
db = sqlite3.connect(os.path.join(site_dir, 'private/mmr.db'))
with db:
    db.execute('INSERT INTO sessions (sid, sess, expire) VALUES (?, ?, ?)', (sid, json.dumps(sess), expires.isoformat()))
sig = base64.b64encode(hmac.new(secret, sid.encode(), hashlib.sha256).digest()).decode().rstrip('=')
cookie = 'connect.sid=' + urllib.parse.quote('s:' + sid + '.' + sig, safe='')

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

state = secrets.token_hex(16)
start_query = urllib.parse.urlencode(fields(state))

status, _, headers = get('/link_key/start?' + start_query, cookie)
check('start signed in from before still goes to the Steam sign-in (a fresh one every link)', status == 302 and '/auth/steam' in headers.get('Location', ''), headers.get('Location', ''))
status, _, headers = get('/link_key/start?' + start_query)
check('start without a session goes to the Steam sign-in', status == 302 and '/auth/steam' in headers.get('Location', ''))
signed_in_anew_for(state)
status, body, _ = get('/link_key/start?' + start_query, cookie)
check('signed in anew: the page asks about the Steam account, no device name', status == 200 and 'Link Test' in body and 'Yes, I started it' in body and '>test<' not in body and 'device: test' not in body)
check('no /link_key/result any more', get('/link_key/result?state=' + state)[0] == 404)

old_t = str(int(time.time()) - 3600)
old = fields(state, t=old_t)
status, body, _ = get('/link_key/start?' + urllib.parse.urlencode(old), cookie)
check('start refuses a link page an hour old', status == 400 and 'too old' in body)
status, _, _ = get('/link_key/start?state=xyz&t=1&key=%s&sig=%s' % (pub, '0' * 128), cookie)
check('start refuses a malformed state', status == 400)
bad = fields(state)
bad['sig'] = fields(secrets.token_hex(16))['sig']
status, _, _ = get('/link_key/start?' + urllib.parse.urlencode(bad), cookie)
check('start refuses a signature of another state', status == 400)
status, _, _ = post('/link_key/start', bad, cookie)
check('Link refuses a signature of another state', status == 400)
status, _, _ = confirm(secrets.token_hex(16), None)
check('Link without a session is refused', status == 400)
status, body, _ = confirm(secrets.token_hex(16), cookie, fresh=False)
check('Link without signing in anew for this page is refused', status == 400 and 'Sign in through Steam' in body, body[:200])

check('cert before linking is the key', cert_account() == 'key_' + pub)

# Ranked progress of the key: moves to a Steam account without matches on linking, stays there on an unlink
with db:
    db.execute("INSERT INTO mmr_team (account_id, nickname, mmr, mu, sigma, matches_won, matches_lost) VALUES (?, 'k', 7.5, 30, 5, 3, 2)", ('key_' + pub,))

def mmr_owner():
    rows = db.execute("SELECT account_id FROM mmr_team WHERE mmr = 7.5 AND matches_won = 3 AND account_id IN (?, ?)", ('key_' + pub, 'steam_' + steam_id)).fetchall()
    return [r[0] for r in rows]

link_t = str(int(time.time()))
status, body, headers = confirm(state, cookie, t=link_t)
check('Link links and says to go back to the game (no redirect)', status == 200 and 'You can go back to Hypersomnia' in body and 'Location' not in headers, body[:200])
check('the game learns it from its cert: the Steam account', cert_account() == 'steam_' + steam_id)
check('progress moved to the Steam account', mmr_owner() == ['steam_' + steam_id], str(mmr_owner()))
status, body, _ = confirm(state, cookie, t=link_t)
check('the same page sent again does nothing (not newer than the last change)', status == 409 and 'used already' in body, '%s %s' % (status, body[:100]))

# Unlink from the game: one signed GET
status, result, headers = unlink(t=str(int(link_t) - 5))
check('an unlink older than the link is refused', status == 409, str(result))
status, result, headers = get('/link_key/unlink?t=%s&key=%s&sig=%s' % (int(time.time()) + 1, pub, '0' * 128))[0], None, None
check('an unsigned unlink is refused', status == 400)
time.sleep(1.1)
unlink_t = str(int(time.time()))
status, result, headers = unlink(t=unlink_t)
check('unlink from the game, answered at once', status == 200 and result == { 'unlinked': True }, str(result))
check('the unlink allows the game origin (CORS)', headers.get('Access-Control-Allow-Origin') == '*')
check('cert after unlinking is the key again', cert_account() == 'key_' + pub)
check('progress stays with the Steam account after the unlink', mmr_owner() == ['steam_' + steam_id], str(mmr_owner()))
status, result, _ = unlink(t=unlink_t)
check('the same unlink sent again does nothing', status == 409, str(result))

# The old link page (from a browser history), confirmed after the unlink: refused
status, body, _ = confirm(state, cookie, t=link_t)
check('the old link page links nothing after the unlink', status == 409 and cert_account() == 'key_' + pub, '%s %s' % (status, body[:100]))

# Bans: a key unlinked from a banned Steam account stays banned; a banned key can not be linked
time.sleep(1.1)
status, body = link()
check('relink', status == 200 and cert_account() == 'steam_' + steam_id, body[:200])
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))
time.sleep(1.1)
status, result, _ = unlink()
check('unlink from a banned account', status == 200, str(result))
check('the unlinked key is banned too', cert_account().startswith('HTTP 403'), cert_account())
time.sleep(1.1)
status, body = link()
check('a banned key can not be linked (said on the page)', status == 403 and 'banned' in body, body[:200])
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

# Unlinking on /profile works like the key's own unlink: the Steam account's ban goes with the key
time.sleep(1.1)
status, body = link()
check('link again', status == 200, body[:200])
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))
status, _, _ = post('/profile/unlink_key', { 'key': 'key_' + pub }, cookie)
check('unlink on /profile', status in (302, 303) and cert_account().startswith('HTTP 403'), cert_account())
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

# A game whose clock is 5 minutes behind links, is unlinked on /profile, and links again right after (the slack)
behind = Ed25519PrivateKey.generate()
behind_pub = behind.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()
status, body = link(behind, t=str(int(time.time()) - 300))
check('a game 5 min behind links', status == 200, body[:200])
post('/profile/unlink_key', { 'key': 'key_' + behind_pub }, cookie)
status, body = link(behind, t=str(int(time.time()) - 299))
check('and links again right after a /profile unlink', status == 200, '%s %s' % (status, body[-300:]))

# A banned Steam account posting someone else's key to /profile/unlink_key: that key is not its, nothing happens to it
foreign = 'key_' + secrets.token_hex(32)
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))
post('/profile/unlink_key', { 'key': foreign }, cookie)
banned_foreign = db.execute('SELECT 1 FROM broker_bans WHERE account = ?', (foreign,)).fetchone()
check('a foreign key posted to /profile/unlink_key is not banned', banned_foreign is None, str(banned_foreign))
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

print('%d failed' % len(failed))
sys.exit(1 if failed else 0)

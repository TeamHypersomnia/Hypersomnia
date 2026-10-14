#!/usr/bin/env python3
"""
The whole key <-> Steam link flow against a test site (broker.sh), as the game does it - without Steam:
a fake Steam session in the site's DB, the confirm page /link_key/start?state&key&sig, its POST (links at once),
/link_key/result for the game, a cert for the key (now the Steam account), the unlink page and its POST,
a cert again (the key's own account), bans, a page for another key.
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

def get(path, cookie=None):
    req = urllib.request.Request(url + path)
    if cookie: req.add_header('Cookie', cookie)
    try:
        r = opener.open(req)
        return r.status, r.read().decode(), dict(r.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode(), dict(e.headers)

def fields(state, signer=None):
    # What the game puts in the URL: its state signed with its key, the device
    signer = signer or key
    public = signer.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()
    t = str(int(time.time()))
    return { 'state': state, 't': t, 'key': public, 'sig': signer.sign(('hypersomnia-link-v1 %s %s' % (state, t)).encode()).hex(), 'device': 'test', 'os': 'linux' }

def confirm(state, cookie, signer=None):
    # The confirm page's [Link]: links at once
    return post('/link_key/start', fields(state, signer), cookie)

def result_for(state):
    # What the game asks every second: how the link for its state went, once
    status, body, _ = get('/link_key/result?state=' + state)
    return json.loads(body) if status == 200 else None

def unlink_fields(state, t=None):
    # What the game puts in the unlink URL: its state and the time signed with its key
    t = t or str(int(time.time()))
    return { 'state': state, 't': t, 'key': pub, 'sig': key.sign(('hypersomnia-unlink-v1 %s %s' % (state, t)).encode()).hex(), 'device': 'test', 'os': 'linux' }

def unlink(cookie_of=None, state=None):
    # "Unlink from Steam..." in the game: the page signed in as the account, its [Unlink this game key]
    state = state or secrets.token_hex(16)
    fields = unlink_fields(state)
    page_status, page, _ = get('/link_key/unlink?' + urllib.parse.urlencode(fields), cookie_of or cookie)
    status, body, _ = post('/link_key/unlink', fields, cookie_of or cookie)
    return page_status, page, status, result_for(state)

def link(signer=None):
    state = secrets.token_hex(16)
    status, body, _ = confirm(state, cookie, signer)
    return status, result_for(state)

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

key = Ed25519PrivateKey.generate()
pub = key.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()

state = secrets.token_hex(16)
start_query = urllib.parse.urlencode(fields(state))
status, body, _ = get('/link_key/start?' + start_query, cookie)
check('start asks about the device and the Steam account first', status == 200 and 'Link Test' in body and 'Yes, this is my device' in body and 'test' in body)
check('nothing for the game before the player confirms', result_for(state) is None)
_, _, headers = get('/link_key/result?state=' + state)
check('result allows the game origin (CORS) - the web build asks it from another origin', headers.get('Access-Control-Allow-Origin') == '*')

status, _, headers = get('/link_key/start?' + start_query)
check('start without a session goes to Steam sign in', status == 302 and '/auth/steam' in headers.get('Location', ''), headers.get('Location', ''))
old = fields(state)
old_t = str(int(time.time()) - 3600)
old.update({ 't': old_t, 'sig': key.sign(('hypersomnia-link-v1 %s %s' % (state, old_t)).encode()).hex() })
status, body, _ = get('/link_key/start?' + urllib.parse.urlencode(old), cookie)
check('start refuses a link page an hour old', status == 400 and 'too old' in body)
check('the game learns that the link page was too old', 'too old' in (result_for(state) or {}).get('error', ''), str(result_for(state)))
state = secrets.token_hex(16)
start_query = urllib.parse.urlencode(fields(state))
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


def cert_account():
    session = Ed25519PrivateKey.generate().public_key().public_bytes(Encoding.Raw, PublicFormat.Raw)
    s = key.sign(b'hs-cert-req-v1' + session).hex()
    status, body, _ = get('/cert?session=%s&key=%s&sig=%s' % (session.hex(), pub, s))
    if status != 200: return 'HTTP %d %s' % (status, body)
    raw = bytes.fromhex(json.loads(body)['cert'])
    n = raw[1]
    return raw[2:2 + n].decode()

check('cert before linking is the key', cert_account() == 'key_' + pub)

# Ranked progress of the key: moves to a Steam account without matches on linking, stays there on an unlink
with db:
    db.execute("INSERT INTO mmr_team (account_id, nickname, mmr, mu, sigma, matches_won, matches_lost) VALUES (?, 'k', 7.5, 30, 5, 3, 2)", ('key_' + pub,))

def mmr_owner():
    # Only this run's two accounts - the database may hold earlier runs
    rows = db.execute("SELECT account_id FROM mmr_team WHERE mmr = 7.5 AND matches_won = 3 AND account_id IN (?, ?)", ('key_' + pub, 'steam_' + steam_id)).fetchall()
    return [r[0] for r in rows]

status, body, headers = confirm(state, cookie)
check('Link on that page links and says to go back to the game (no redirect)', status == 200 and 'You can go back to Hypersomnia' in body and 'Location' not in headers, body[:200])
check('the game learns it is linked', result_for(state) == { 'linked': 'steam_' + steam_id })
check('the result is given again until it expires (one lost on the way)', result_for(state) == { 'linked': 'steam_' + steam_id })
status, body, _ = confirm(state, cookie)
check('a state links once', status == 409, '%s %s' % (status, body[:100]))

check('cert after linking is the Steam account', cert_account() == 'steam_' + steam_id)
check('progress moved to the Steam account', mmr_owner() == ['steam_' + steam_id], str(mmr_owner()))


status, _, headers = get('/link_key/unlink?' + urllib.parse.urlencode(unlink_fields(secrets.token_hex(16))))
check('the unlink page without a session goes to Steam sign in', status == 302 and '/auth/steam' in headers.get('Location', ''))
status, _, _ = get('/link_key/unlink?state=%s&key=%s' % (secrets.token_hex(16), pub), cookie)
check('an unsigned unlink page is refused', status == 400)
forged = unlink_fields(secrets.token_hex(16))
forged['sig'] = unlink_fields(secrets.token_hex(16))['sig']
status, _, _ = post('/link_key/unlink', forged, cookie)
check('an unlink signed for another state is refused', status == 400)
page_status, page, status, result = unlink()
check('the unlink page shows the device and one button', page_status == 200 and 'Unlink this game key' in page and 'test' in page)
check('unlink on that page, the game learns it', status == 200 and result == { 'unlinked': 'steam_' + steam_id }, str(result))

check('cert after unlinking is the key again', cert_account() == 'key_' + pub)
check('progress stays with the Steam account after the unlink', mmr_owner() == ['steam_' + steam_id], str(mmr_owner()))

page_status, page, status, result = unlink()
check('a key not linked: the page says so', page_status == 409 and 'not linked' in page and 'not linked' in (result or {}).get('error', ''), str(result))

# Bans: a key unlinked from a banned Steam account stays banned; a banned key can not be linked
status, result = link()
check('relink', status == 200 and result == { 'linked': 'steam_' + steam_id }, str(result))
with db:
    db.execute('INSERT OR IGNORE INTO broker_bans (account, reason) VALUES (?, ?)', ('steam_' + steam_id, 'Test ban.'))

page_status, page, status, result = unlink()
check('unlink from a banned account', status == 200, str(result))
check('the unlinked key is banned too', cert_account().startswith('HTTP 403'), cert_account())
status, result = link()
check('a banned key can not be linked (the game learns why)', status == 403 and 'banned' in (result or {}).get('error', ''), str(result))
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

# Unlinking on /profile works like the key's own unlink: the Steam account's ban goes with the key
status, result = link()
check('link again', status == 200, str(result))
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
status, result = link()
check('no link to a banned Steam account', status == 403 and 'banned' in (result or {}).get('error', ''), str(result))
with db:
    db.execute('DELETE FROM broker_bans WHERE account IN (?, ?)', ('steam_' + steam_id, 'key_' + pub))

print('%d failed' % len(failed))
sys.exit(1 if failed else 0)

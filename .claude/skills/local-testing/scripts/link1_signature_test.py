#!/usr/bin/env python3
# Usage: STEAM_CWD=<dir with steam_appid.txt containing 2660970> link1_signature_test.py <this scripts dir> <broker site.out>
# Needs the user's running Steam client and broker.sh started with STEAM_API_KEY_FILE.
# link=1 at the broker: the replayable cert request signature must not link, the ticket-bound link signature must.
import hashlib, subprocess, sys, urllib.request, urllib.error, os
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
scripts, site_out = sys.argv[1], sys.argv[2]

def ticket_for(pub):
    identity = hashlib.sha256(b'hs-steam-v1' + pub).digest()[:15].hex()
    out = subprocess.run(['python3', os.path.join(scripts, 'steam_tickets.py'), identity], capture_output=True, text=True, cwd=os.environ['STEAM_CWD']).stdout.split()
    return bytes.fromhex(out[2].lower())

def cert(pub, ticket, sig):
    url = 'http://127.0.0.1:13500/cert?session=%s&steam=%s&link=1&key=%s&sig=%s' % (pub.hex(), ticket.hex(), pub.hex(), sig.hex())
    try:
        return urllib.request.urlopen(url, timeout=20).status
    except urllib.error.HTTPError as e:
        return e.code

failed = 0
for label, make_sig, expect in [
    ('replayable cert request signature', lambda k, pub, t: k.sign(b'hs-cert-req-v1' + pub), 'Not linking'),
    ('ticket-bound link signature', lambda k, pub, t: k.sign(b'hs-cert-link-v1' + pub + hashlib.sha256(t).digest()), 'The Steam build linked'),
]:
    key = Ed25519PrivateKey.generate()
    pub = key.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw)
    t = ticket_for(pub)
    before = open(site_out).read()
    status = cert(pub, t, make_sig(key, pub, t))
    new = open(site_out).read()[len(before):]
    ok = expect in new and ('key_' + pub.hex()) in new
    print(('PASS ' if ok else 'FAIL ') + label + ' (HTTP %d): %s' % (status, new.strip().splitlines()[-1] if new.strip() else ''))
    failed += not ok
print('%d failed' % failed)

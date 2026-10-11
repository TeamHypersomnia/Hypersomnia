#!/usr/bin/env python3
"""
CrazyGames tokens at the broker with a test key pair (broker.sh started with CRAZYGAMES_PUBLIC_KEY_PEM=<its public key>
and CRAZYGAMES_GAME_ID): a valid token gets a crazygames_ cert; another game, an expired token, a bad signature,
another algorithm and a bad user id are refused.
Usage: crazygames_cert_test.py <private key pem> <game id> <site url>
"""
import base64, json, os, sys, time, urllib.request, urllib.error
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding

key = serialization.load_pem_private_key(open(sys.argv[1], 'rb').read(), None)
game_id, url = sys.argv[2], sys.argv[3].rstrip('/')
failed = []

def b64(b): return base64.urlsafe_b64encode(b).rstrip(b'=').decode()

def jwt(payload, alg='RS256', sign_key=key):
    head = b64(json.dumps({'alg': alg, 'typ': 'JWT'}).encode())
    body = b64(json.dumps(payload).encode())
    sig = sign_key.sign(f'{head}.{body}'.encode(), padding.PKCS1v15(), hashes.SHA256())
    return f'{head}.{body}.{b64(sig)}'

def cert(token):
    session = os.urandom(32).hex()
    try:
        r = urllib.request.urlopen(f'{url}/cert?session={session}&crazygames={urllib.parse.quote(token)}')
        raw = bytes.fromhex(r.read().decode())
        return 200, raw[2:2 + raw[1]].decode()
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode()

import urllib.parse
now = int(time.time())
good = { 'userId': 'cgUser_123', 'gameId': game_id, 'username': 'tester', 'exp': now + 3600 }

def check(name, got, want_status, want_body=None):
    ok = got[0] == want_status and (want_body is None or got[1] == want_body)
    print(('PASS ' if ok else 'FAIL ') + name + ': ' + str(got))
    if not ok: failed.append(name)

check('valid token', cert(jwt(good)), 200, 'crazygames_cgUser_123')
check('another game', cert(jwt({**good, 'gameId': 'other'})), 401)
check('expired', cert(jwt({**good, 'exp': now - 10})), 401)
other = __import__('cryptography.hazmat.primitives.asymmetric.rsa', fromlist=['rsa']).generate_private_key(65537, 2048)
check('signed by another key', cert(jwt(good, sign_key=other)), 401)
check('another algorithm in the header', cert(jwt(good, alg='HS256')), 401)
check('bad user id', cert(jwt({**good, 'userId': '../x'})), 401)
check('not a jwt', cert('abc'), 400)
check('garbage parts', cert('a.b.c'), 400)
print('%d failed' % len(failed))
sys.exit(1 if failed else 0)

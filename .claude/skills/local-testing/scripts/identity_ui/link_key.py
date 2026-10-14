#!/usr/bin/env python3
# Links an account key file to a Steam account on the scratch site, as confirming the game's link page would:
#   SCRATCH_ROOT=.. link_key.py <account.key> [steam id64]   - prints what the game would get from /link_key/result
import json, os, secrets, subprocess, sys, urllib.parse, urllib.request
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
S = os.environ.get('SCRATCH_ROOT', '/tmp/hypersomnia-ui')
steam = sys.argv[2] if len(sys.argv) > 2 else '76561198051900812'
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
key = Ed25519PrivateKey.from_private_bytes(seed)
pub = key.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()
state = secrets.token_hex(16)
env = dict(os.environ, SESSION_SECRET=open(S + '/nettest/session_secret').read().strip())
here = os.path.dirname(os.path.abspath(__file__))
cookie = subprocess.check_output(['python3', here + '/../fake_steam_session.py', S + '/nettest/broker_site', steam, 'Pythagoras of Samos'], env=env).decode().strip()
fields = { 'state': state, 'key': pub, 'sig': key.sign(('hypersomnia-link-v1 ' + state).encode()).hex(), 'device': 'test', 'os': 'linux' }
req = urllib.request.Request('http://127.0.0.1:13500/link_key/start', data=urllib.parse.urlencode(fields).encode(), headers={ 'Cookie': cookie })
try:
    urllib.request.urlopen(req).read()
except urllib.error.HTTPError as e:
    pass
print(urllib.request.urlopen('http://127.0.0.1:13500/link_key/result?state=' + state).read().decode())

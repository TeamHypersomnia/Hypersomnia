#!/usr/bin/env python3
# Links an account key file to a Steam account on the scratch site, like the game does: link_api.py <account.key> <code> <steam id64>
import sys, urllib.request
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
seed = bytes.fromhex(open(sys.argv[1]).read().strip().split(':')[1])
key = Ed25519PrivateKey.from_private_bytes(seed)
pub = key.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw).hex()
code, steam = sys.argv[2], sys.argv[3]
sig = key.sign(('hypersomnia-link-v1 %s steam_%s' % (code, steam)).encode()).hex()
r = urllib.request.urlopen('http://127.0.0.1:13500/link_key/link?public_key=%s&code=%s&signature=%s' % (pub, code, sig))
print(r.status, r.read().decode()[:200])

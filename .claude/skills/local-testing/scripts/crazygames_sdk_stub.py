#!/usr/bin/env python3
"""
Writes a stub of the CrazyGames SDK v3 for the CrazyGames web build in pw_driver.js (PW_ROUTES): a signed-in user whose
token is a JWT signed with a test key - the broker must run with that key's public part (CRAZYGAMES_PUBLIC_KEY_PEM).
Usage: crazygames_sdk_stub.py <private key pem> <game id> <user id> <out.js>
User id "-": a guest - no token, no user (the key is still read, any will do).
"""
import base64, json, sys, time
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding

key = serialization.load_pem_private_key(open(sys.argv[1], 'rb').read(), None)
game_id, user_id, out = sys.argv[2], sys.argv[3], sys.argv[4]

def b64(b): return base64.urlsafe_b64encode(b).rstrip(b'=').decode()

payload = { 'userId': user_id, 'gameId': game_id, 'username': 'CG ' + user_id, 'profilePictureUrl': '', 'exp': int(time.time()) + 3600, 'expiresIn': 3600 }
head = b64(json.dumps({ 'alg': 'RS256', 'typ': 'JWT' }).encode())
body = b64(json.dumps(payload).encode())
token = head + '.' + body + '.' + b64(key.sign((head + '.' + body).encode(), padding.PKCS1v15(), hashes.SHA256()))

guest = user_id == '-'
token_js = 'null' if guest else json.dumps(token)
user_js = 'null' if guest else '({ username: %s })' % json.dumps(payload['username'])

open(out, 'w').write("""
window.CrazyGames = { SDK: {
  environment: 'local',
  init: async () => {},
  data: { getItem: k => localStorage.getItem('cg_' + k), setItem: (k, v) => localStorage.setItem('cg_' + k, v), removeItem: k => localStorage.removeItem('cg_' + k) },
  game: { getInviteParam: () => null, hideInviteButton() {}, showInviteButton: () => '', inviteLink: () => '', happytime() {}, gameplayStart() {}, gameplayStop() {}, loadingStart() {}, loadingStop() {} },
  user: { isUserAccountAvailable: true, getUserToken: async () => { const t = %s; if (!t) throw new Error('userNotAuthenticated'); return t; }, showAuthPrompt: async () => ({}), getUser: async () => %s, addAuthListener() {} },
  ad: { requestAd: (type, callbacks) => { if (callbacks && callbacks.adFinished) callbacks.adFinished(); } },
  banner: { requestBanner: async () => {}, clearAllBanners() {} }
} };
console.log('CrazyGames SDK stub loaded');
""" % (token_js, user_js))

#!/bin/bash
# Copies a web build and points its packaged default_config.json at a local masterserver:
# the three URLs are replaced in place in Hypersomnia.data with text of the same length (padded with spaces),
# so the package offsets stay valid without repackaging.
# Usage: prepare_web.sh <web build dir> <output dir>
set -e
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
B=$1
W=$2
rm -rf $W
mkdir -p $W
cp $B/Hypersomnia.html $B/Hypersomnia.js $B/Hypersomnia.wasm $B/Hypersomnia.data $W/
ln -s $REPO/cmake/web/assets $W/assets

python3 - $W/Hypersomnia.data <<'EOF'
import sys
p = sys.argv[1]
d = bytearray(open(p, 'rb').read())
replacements = [
	(b'"server_list_provider": "http://masterserver.hypersomnia.io:8410",', b'"server_list_provider": "http://127.0.0.1:18420",'),
	(b'"webrtc_signalling_server_url": "wss://masterserver.hypersomnia.io:8000",', b'"webrtc_signalling_server_url": "ws://127.0.0.1:18000",'),
	(b'"connect_token_provider": "https://hypersomnia.io/connect_token",', b'"connect_token_provider": "http://127.0.0.1:18420/connect_token",'),
]
for old, new in replacements:
	assert d.count(old) == 1, old
	assert len(new) <= len(old)
	new = new + b' ' * (len(old) - len(new))
	i = d.find(old)
	d[i:i + len(old)] = new
open(p, 'wb').write(bytes(d))
print("pointed the web build at the local masterserver")
EOF

#!/bin/bash
# Copies a web build and points its packaged default_config.json at a local masterserver:
# the three URLs are replaced in place in Hypersomnia.data with text of the same length (padded with spaces),
# so the package offsets stay valid without repackaging.
# USER_FILES=<dir> also writes every file under <dir> into the game's /user right after IDBFS loads, e.g.
# conf.d/test.json pointing at a remote test master, or cached_auth.json - a Discord sign-in the user copied
# from the real web build, so the game starts signed in.
# Usage: [USER_FILES=dir] prepare_web.sh <web build dir> <output dir>
set -e
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
B=$1
W=$2
rm -rf $W
mkdir -p $W
cp $B/Hypersomnia.html $B/Hypersomnia.js $B/Hypersomnia.wasm $B/Hypersomnia.data $W/
ln -s $REPO/cmake/web/assets $W/assets

if [ -n "$USER_FILES" ]; then
	python3 - "$USER_FILES" $W <<'PY'
import json, os, sys
src, w = sys.argv[1], sys.argv[2]
files = {}

for root, _, names in os.walk(src):
	for name in names:
		path = os.path.join(root, name)
		files['/user/' + os.path.relpath(path, src)] = open(path).read()

# pre_run from common.js, with the files written before the game starts
open(w + '/inject_user_files.js', 'w').write("""
pre_run = function() {
  Module.addRunDependency('idbfs');
  FS.mkdir('/user');
  FS.mount(IDBFS, {}, '/user');

  FS.syncfs(true, function (err) {
    const files = %s;

    for (const path in files) {
      FS.mkdirTree(path.substring(0, path.lastIndexOf('/')));
      FS.writeFile(path, files[path]);
      console.log('Injected ' + path);
    }

    Module.removeRunDependency('idbfs');
  });
};
""" % json.dumps(files))

html = open(w + '/Hypersomnia.html').read()
old = '<script src=assets/common.js></script>'
assert html.count(old) == 1
open(w + '/Hypersomnia.html', 'w').write(html.replace(old, old + '<script src=inject_user_files.js></script>'))
PY
	chmod 600 $W/inject_user_files.js
fi

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

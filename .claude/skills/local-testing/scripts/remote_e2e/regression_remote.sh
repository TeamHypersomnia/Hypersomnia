#!/bin/bash
# The remote part of regression.sh (REMOTE=1) - real hosts, real NAT, nothing of production touched:
#   1. uploads the Ubuntu 22.04 headless build and the content to arena (~/yojimbo-test/e2e) and arenaus
#      (~/hs-test-backend), the site to arenaus, and starts the test backend there (backend.sh);
#   2. the site's MMR rules on that backend: run_mmr_tests.py there, as the test official servers pl and au;
#   3. one real ranked match on arena with two clients from here (run_match.sh) - its report must reach the backend;
#   4. a web client from here with WebRTC over TURN only (production's arena-pl, arena-us blocked) joining a community
#      server on arena.
# Prints "REMOTE: all passed" at the end only if every step passed. Needs: build/Release-x64-ubuntu22-headless
# (build_headless_ubuntu2204.sh), build/Release-Web-clang-claude-web (build_web.sh), Firefox, Xvfb.
HERE=$(cd "$(dirname "$0")" && pwd)
SCRIPTS=$(cd "$HERE/.." && pwd)
REPO=$(cd "$HERE/../../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
export ALSOFT_DRIVERS=null VISUAL=true
ARENA_IP=${ARENA_IP:-57.128.242.21}
ARENAUS_IP=${ARENAUS_IP:-154.12.226.211}
R=${ROOTFS22:-$HOME/.cache/hypersomnia-rootfs22}
failed=0

step() { echo "--- $1"; }
fail() { echo "FAIL: $1"; failed=$((failed + 1)); }

step "production before (for the uptime check at the end)"
PROD_BEFORE=$(ssh arena 'ps -eo etimes,args | grep [H]ypersomnia-Headless | head -1 | cut -d" " -f1'; ssh arenaus 'ps -eo etimes,args | grep [H]ypersomnia-Headless | head -1 | cut -d" " -f1')

step "upload"
HEADLESS=$REPO/build/Release-x64-ubuntu22-headless/Hypersomnia
[ -x "$HEADLESS" ] || { echo "no headless build - run build_headless_ubuntu2204.sh"; exit 1; }
# The libc++ the build was linked with, from its Ubuntu 22.04 rootfs (as build_headless_ubuntu2204.sh ships it)
LIBS=$S/libcxx22
mkdir -p $LIBS
for l in libc++.so.1 libc++abi.so.1 libunwind.so.1; do
	cp -L $R/usr/lib/llvm-18/lib/$l $LIBS/ 2>/dev/null || cp -L $R/lib/x86_64-linux-gnu/$l $LIBS/ || { echo "no $l in $R"; exit 1; }
done
for h in "arena:yojimbo-test/e2e" "arenaus:hs-test-backend"; do
	timeout 600 rsync -az "$HEADLESS" ${h}/Hypersomnia && timeout 300 rsync -az "$LIBS/" ${h}/lib/ \
		&& timeout 600 rsync -az --exclude logs --exclude user --exclude cache "$REPO/hypersomnia/" ${h}/hypersomnia/ \
		|| fail "upload to $h"
done
timeout 600 rsync -az --exclude .git --exclude .env --exclude private --exclude node_modules --exclude hosting --exclude infra \
	"$HOME/page/" arenaus:hs-test-backend/page/ || fail "site upload"
scp -q "$HERE/backend.sh" arenaus:hs-test-backend/backend.sh
scp -q "$HERE/match.sh" "$HERE/community.sh" arena:yojimbo-test/e2e/
ssh arenaus 'SECS=3600 timeout 60 bash ~/hs-test-backend/backend.sh start' > $S/remote_backend.out 2>&1
grep -q "13000" $S/remote_backend.out || fail "backend start"

step "MMR rules on the remote site"
ssh arenaus 'cd ~/hs-test-backend/page && mkdir -p ../official_servers.d && for id in pl au; do python3 -c "
import hashlib, sys
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
k = Ed25519PrivateKey.from_private_bytes(hashlib.sha256(f\"hypersomnia-test-server-{sys.argv[1]}\".encode()).digest())
print(k.public_key().public_bytes(serialization.Encoding.Raw, serialization.PublicFormat.Raw).hex())" $id > ../official_servers.d/$id; done; sleep 6; timeout 300 python3 run_mmr_tests.py 13000' > $S/remote_mmr.out 2>&1
grep -a -q -E "All [0-9]+ assertions passed" $S/remote_mmr.out && echo "    MMR: $(grep -a -o -E 'All [0-9]+ assertions passed' $S/remote_mmr.out)" || fail "MMR rules (see $S/remote_mmr.out)"

step "one real ranked match on arena"
SCRATCH=$S timeout 400 $HERE/run_match.sh rA rB 45 > $S/remote_match.out 2>&1
grep -a -q "delivered (HTTP 200)" $S/remote_match.out && echo "    report delivered" || fail "ranked match (see $S/remote_match.out)"

step "web client over TURN only (arena-pl)"
ssh arena 'SECS=900 timeout 30 bash ~/yojimbo-test/e2e/community.sh start > /dev/null'
sleep 14
T=$S/web_relay_$(date +%s)
mkdir -p $T/uf/conf.d $T/profile
echo "{ \"server_list_provider\": \"http://$ARENAUS_IP:18420/server_list\", \"identity_broker_url\": \"http://$ARENAUS_IP:13000\", \"webrtc_signalling_server_url\": \"ws://$ARENAUS_IP:18000\", \"client\": { \"nickname\": \"webrelay\" } }" > $T/uf/conf.d/test.json
USER_FILES=$T/uf $SCRIPTS/prepare_web.sh $REPO/build/Release-Web-clang-claude-web $T/web > /dev/null
cat > $T/profile/user.js <<J
user_pref("browser.shell.checkDefaultBrowser", false);
user_pref("datareporting.policy.dataSubmissionEnabled", false);
user_pref("toolkit.telemetry.reportingpolicy.firstRun", false);
user_pref("webgl.force-enabled", true);
user_pref("devtools.console.stdout.content", true);
user_pref("media.volume_scale", "0.0");
user_pref("media.peerconnection.ice.relay_only", true);
user_pref("network.dns.localDomains", "arena-us.hypersomnia.io");
J
timeout 200 python3 $SCRIPTS/webserve.py $T/web 8766 > $T/web.out 2>&1 &
(setsid Xvfb :79 -screen 0 1280x720x24 > /dev/null 2>&1 < /dev/null &)
sleep 2
DISPLAY=:79 timeout 180 firefox --no-remote --profile $T/profile --new-instance "http://localhost:8766/game/$ARENA_IP:9005" > $T/ff.out 2>&1 &
for i in $(seq 1 34); do
	sleep 5
	grep -a -q -E "Received initial public vars|Lost connection|failed" $T/ff.out && break
done
grep -a -q "Received initial public vars" $T/ff.out && echo "    joined over TURN" || fail "web over TURN (see $T/ff.out)"
for p in $(pgrep -f "firefox --no-remote --profile $T/profile"); do kill $p; done
for p in $(pgrep -f "Xvfb :79"); do kill $p; done
ssh arena 'bash ~/yojimbo-test/e2e/community.sh stop' > /dev/null 2>&1

step "production untouched"
PROD_AFTER=$(ssh arena 'ps -eo etimes,args | grep [H]ypersomnia-Headless | head -1 | cut -d" " -f1'; ssh arenaus 'ps -eo etimes,args | grep [H]ypersomnia-Headless | head -1 | cut -d" " -f1')
paste <(echo "$PROD_BEFORE") <(echo "$PROD_AFTER") | while read before after; do
	[ "$after" -ge "$before" ] || echo "a production process restarted"
done | grep -q restarted && fail "production restarted" || echo "    production uptimes only grew"

[ $failed = 0 ] && echo "REMOTE: all passed" || echo "REMOTE: $failed failed"
exit $failed

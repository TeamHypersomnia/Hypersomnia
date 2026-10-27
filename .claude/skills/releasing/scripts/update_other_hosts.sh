#!/bin/bash
# update_other_hosts.sh <version> - US, NL, DE: the new Headless AppImage put where their service runs it
# (md5 checked against the hub's builds/<version>), then the service restarted. The hub updates through server_update.
V=$1
MD5=$(ssh -o BatchMode=yes arena "md5sum /var/www/app/hosting/builds/$V/Hypersomnia-Headless.AppImage" | cut -d' ' -f1)
[ -n "$MD5" ] || { echo "no $V on the hub"; exit 1; }
echo "md5 $V: $MD5"
REMOTE='
set -e
T=$1
wget -q -O "$T.new" https://hypersomnia.io/builds/'$V'/Hypersomnia-Headless.AppImage
[ "$(md5sum "$T.new" | cut -d" " -f1)" = "'$MD5'" ] || { echo "BAD MD5"; exit 1; }
chmod +x "$T.new"
mv -f "$T.new" "$T"
if [ "$2" = sudo ]; then
	sudo -n systemctl restart hypersomnia
else
	# No sudo: the game process itself (not the AppImage wrapper). A stuck one ignores SIGINT and SIGTERM (DE, 4.3.0) -
	# then SIGKILL; the service has Restart=always.
	P=$(ps -eo pid,args | grep "[u]sr/bin/Hypersomnia --calling-cwd" | awk "{print \$1}" | head -1)
	for SIG in INT TERM KILL; do
		kill -$SIG $P 2>/dev/null || true
		for i in $(seq 1 15); do
			sleep 2
			kill -0 $P 2>/dev/null || break
		done
		kill -0 $P 2>/dev/null || break
		echo "still running after SIG$SIG"
	done
fi
sleep 10
systemctl is-active hypersomnia
"$T" --version 2>/dev/null | head -1
# the running process's age, not the file's version: a process that never restarted still runs the old one
echo "game process up for: $(ps -eo etime,args | grep "[u]sr/bin/Hypersomnia --calling-cwd" | awk "{print \$1}" | head -1)"
'
echo "== US"; ssh -o BatchMode=yes ubuntu@arena-us.hypersomnia.io "bash -s /home/ubuntu/Hypersomnia-Headless.AppImage sudo" <<< "$REMOTE" 2>&1 | tail -3
echo "== NL"; ssh -o BatchMode=yes -p 10600 hypersomnia@80.78.132.21 "bash -s /home/hypersomnia/hypersomnia/Hypersomnia-Headless.AppImage sudo" <<< "$REMOTE" 2>&1 | tail -3
echo "== DE"; ssh -o BatchMode=yes hypersomnia@kestrel.timofei302.su "bash -s /home/hypersomnia/Hypersomnia-Headless.AppImage kill" <<< "$REMOTE" 2>&1 | grep -v "Ich will" | tail -3

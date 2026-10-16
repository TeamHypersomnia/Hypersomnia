#!/bin/bash
# A scratch copy of the site (hypersomnia.io) as the identity broker and the match report endpoint for local tests.
#   broker.sh start [port]   copies the site, makes a test broker key, starts it with --test; prints BROKER_URL and BROKER_PUBLIC_KEY
#   broker.sh stop
#   broker.sh sql "<query>"  runs SQL on its database
# Never port 3000 - that is the user's own site. The copy has a fresh database and no .env.
umask 077
S=${SCRATCH:-/tmp/hypersomnia-nettest}
SITE=${SITE:-$HOME/page}
B=$S/broker_site
PORT=${2:-13500}

case "$1" in
start)
	if ss -tln | grep -q ":$PORT "; then echo "port $PORT is busy"; exit 1; fi
	mkdir -p $B
	rsync -a --delete --exclude .git --exclude .env --exclude 'private/*' --exclude node_modules --exclude hosting "$SITE/" "$B/"
	ln -sfn "$SITE/node_modules" "$B/node_modules"
	mkdir -p $B/private $B/hosting
	if [ ! -f $B/private/broker.key ]; then
		(cd $B && node -e '
			const crypto = require("crypto");
			require("fs").writeFileSync("private/broker.key", "hypersomnia-key-v1:" + crypto.randomBytes(32).toString("hex") + "\n", { mode: 0o600 });')
	fi
	BROKER_PUBLIC_KEY=$(cd $B && node -e '
		const ic = require("./src/identity_cert.js");
		const seed = Buffer.from(require("fs").readFileSync("private/broker.key", "utf8").trim().split(":")[1], "hex");
		console.log(ic.rawPublicKey(ic.privateKeyFromSeed(seed)).toString("hex"));')
	(cd $B && PORT=$PORT NODE_ENV=development BASE_URL=http://127.0.0.1:$PORT OFFICIAL_SERVERS_DIR=${OFFICIAL_SERVERS_DIR:-$B/private/official_servers.d} \
		STEAM_APIKEY=$(cat ${STEAM_API_KEY_FILE:-/dev/null} 2>/dev/null) CRAZYGAMES_PUBLIC_KEY_PEM="${CRAZYGAMES_PUBLIC_KEY_PEM:-}" \
		CRAZYGAMES_GAME_ID="${CRAZYGAMES_GAME_ID:-}" SESSION_SECRET="${SESSION_SECRET:-}" \
		nohup node app.js --test > $B/site.out 2>&1 &)
	for i in $(seq 1 50); do
		curl -s -o /dev/null http://127.0.0.1:$PORT/ && break
		sleep 0.2
	done
	# The pid of what listens on the port, not of a wrapping shell
	lsof -ti tcp:$PORT -sTCP:LISTEN > $B/site.pid
	echo "BROKER_URL=http://127.0.0.1:$PORT"
	echo "BROKER_PUBLIC_KEY=$BROKER_PUBLIC_KEY"
	;;
stop)
	[ -f $B/site.pid ] && kill $(cat $B/site.pid) 2>/dev/null
	rm -f $B/site.pid
	;;
sql)
	(cd $B && node -e '
		const Database = require("better-sqlite3");
		const db = new Database("private/mmr.db");
		const q = process.argv[1];
		if (/^\s*select/i.test(q)) console.log(JSON.stringify(db.prepare(q).all(), null, 1)); else console.log(db.prepare(q).run());
	' "$2")
	;;
esac

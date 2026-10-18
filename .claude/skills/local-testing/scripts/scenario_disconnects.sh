#!/bin/bash
# Disconnects as the player sees them: a filmstrip per case (frames named by seconds since the game started)
# plus the server's log of what it did and when.
# Cases: shutdown (server SIGTERM), killed (server SIGKILL), frozen (server SIGSTOP 6 s), full, elsewhere (the same key
# joins again), password (wrong), afk, client_frozen (client SIGSTOP 6 s), client_killed and client_quit (when the slot frees).
# Usage: CASES="full afk" scenario_disconnects.sh
export ALSOFT_DRIVERS=null VISUAL=true
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
# A fresh directory every run - never removed recursively
T=$S/disconnects_$(date +%s)
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
CASES=${CASES:-"shutdown killed frozen full elsewhere password afk client_frozen client_killed client_quit"}
PORT=10060
mkdir -p $T

server() { # name port slots extra-server-json extra-top-json
	mkdir -p $T/srv_$1/user/conf.d
	cat > $T/srv_$1/user/conf.d/test.json <<J
{ "server_start": { "ip": "0.0.0.0", "port": $2, "slots": $3 },
  "server": { "server_name": "disc $1", "arena": "de_cyberaqua", "notified_server_list": "127.0.0.1:1", "suppress_new_community_server_webhook": true,
              "webrtc_port_range_begin": $(($2 + 9000)), "webrtc_port_range_end": $(($2 + 9005)) $4 } $5 }
J
	(cd $REPO/hypersomnia && setsid timeout 150 $BIN --dedicated-server --appdata-dir $T/srv_$1 --no-update-on-launch > $T/srv_$1.out 2>&1 < /dev/null &)
	sleep 5
	SPID=$(pgrep -f "^$BIN --dedicated-server --appdata-dir $T/srv_$1 ")
}

client_dir() { # dir nickname
	mkdir -p $1/user/conf.d $1/cache
	cat > $1/user/conf.d/test.json <<J
{ "identity_broker_url": "http://127.0.0.1:1", "window": { "fullscreen": false, "size": [1280, 720] }, "client": { "nickname": "$2" } }
J
	# A warm cache: a cold one draws a black world for seconds while neon maps generate.
	[ -d $S/film/c_key/cache/content ] && cp -r $S/film/c_key/cache/content $1/cache/
}

background_client() { # dir connect secs
	(cd $REPO/hypersomnia && setsid timeout $3 xvfb-run -a $BIN --connect "$2" --appdata-dir $1 --no-update-on-launch --mute > $1/game.out 2>&1 < /dev/null &)
}

game_pid() {
	pgrep -f "^$BIN --connect .* --appdata-dir $1 "
}

film() { # case connect secs - frames every 0.5 s, each named by seconds since start
	local C=$T/c_$1 F=$T/frames_$1
	mkdir -p $F
	Xvfb :92 -screen 0 1280x720x24 > /dev/null 2>&1 &
	local XV=$!
	sleep 1
	START=${START:-$(date +%s.%N)}
	(cd $REPO/hypersomnia && DISPLAY=:92 timeout $(($3 + 5)) $BIN --connect "$2" --appdata-dir $C --no-update-on-launch --mute > $C/game.out 2>&1 &)
	while :; do
		local E=$(echo "$(date +%s.%N) - $START" | bc)
		[ $(echo "$E > $3" | bc) = 1 ] && break
		DISPLAY=:92 import -window root -resize 25% $F/$(printf "%05.1f" $E).png 2> /dev/null
		sleep 0.4
	done
	local G=$(game_pid $C); [ -n "$G" ] && kill $G
	kill $XV; sleep 1
	magick montage -label '%t s' $F/*.png -tile 6x -geometry +2+2 -background gray -pointsize 14 $T/$1.png 2> /dev/null
	echo "  filmstrip: $T/$1.png"
}

at() { # seconds description command... - run in the background, logged with seconds since film start
	local D=$1 W=$2; shift 2
	( sleep $D; echo "  $(printf "%5.1f" $(echo "$(date +%s.%N) - $START" | bc)) s: $W"; "$@" ) &
}

srv_log() {
	grep -hE "kick|Kick|isconnect|onnected|timed out|Shutting|password|AFK|elsewhere|full" $T/srv_$1.out | grep -v "masterserver\|Notif" | sed 's/^/    /' | head -${2:-15}
}

for CASE in $CASES; do
	PORT=$((PORT + 1)); unset START
	echo "== $CASE"
	client_dir $T/c_$CASE "disc_$CASE"
	case $CASE in
		shutdown|killed|frozen)
			server $CASE $PORT 8
			SIG=$(case $CASE in shutdown) echo TERM;; killed) echo KILL;; frozen) echo STOP;; esac)
			START=$(date +%s.%N)
			at 10 "server SIG$SIG" kill -$SIG $SPID
			[ $CASE = frozen ] && at 16 "server SIGCONT" kill -CONT $SPID
			film $CASE 127.0.0.1:$PORT 30
			wait
			srv_log $CASE
			kill $SPID 2> /dev/null
			;;
		full)
			server $CASE $PORT 1
			client_dir $T/c_full_occupant occupant
			background_client $T/c_full_occupant 127.0.0.1:$PORT 40
			sleep 8
			film $CASE 127.0.0.1:$PORT 12
			srv_log $CASE
			kill $(game_pid $T/c_full_occupant) $SPID 2> /dev/null
			;;
		elsewhere)
			server $CASE $PORT 8
			START=$(date +%s.%N)
			# A copy of the filmed player's directory - the same key - joins at 10 s, once the filmed one exists.
			at 9 "the same key joins again" bash -c "cp -r $T/c_$CASE $T/c_elsewhere_twin; cd $REPO/hypersomnia; setsid timeout 25 xvfb-run -a $BIN --connect 127.0.0.1:$PORT --appdata-dir $T/c_elsewhere_twin --no-update-on-launch --mute > /dev/null 2>&1 < /dev/null &"
			film $CASE 127.0.0.1:$PORT 20
			wait
			srv_log $CASE
			kill $(game_pid $T/c_elsewhere_twin) $SPID 2> /dev/null
			;;
		password)
			server $CASE $PORT 8 '' ', "server_private": { "server_password": "sekret" }'
			film $CASE 127.0.0.1:$PORT 10
			srv_log $CASE
			kill $SPID
			;;
		afk)
			server $CASE $PORT 8 ', "move_to_spectators_if_afk_for_secs": 5, "kick_if_afk_for_secs": 12'
			film $CASE 127.0.0.1:$PORT 25
			srv_log $CASE
			kill $SPID
			;;
		client_frozen)
			server $CASE $PORT 8
			START=$(date +%s.%N)
			at 10 "client SIGSTOP" bash -c "kill -STOP \$(pgrep -f '^$BIN --connect .* --appdata-dir $T/c_$CASE ')"
			at 16 "client SIGCONT" bash -c "kill -CONT \$(pgrep -f '^$BIN --connect .* --appdata-dir $T/c_$CASE ')"
			film $CASE 127.0.0.1:$PORT 30
			wait
			srv_log $CASE
			kill $SPID
			;;
		client_killed|client_quit)
			server $CASE $PORT 8
			background_client $T/c_$CASE 127.0.0.1:$PORT 40
			sleep 10
			SIG=$([ $CASE = client_killed ] && echo KILL || echo TERM)
			echo "  $(date +%T.%N | cut -c1-12) client SIG$SIG"
			kill -$SIG $(game_pid $T/c_$CASE)
			sleep 8
			srv_log $CASE
			kill $SPID
			;;
	esac
done

#!/bin/bash
# UI test helpers: ui.sh <command> ...
#   start <name> <display> [extra game args...]   fresh-or-existing client dir $U/<name>, game under Xvfb :<display>
#   stop <name> <display>
#   (the game's opened URLs never reach a browser: they go to $U/<name>/opened_urls - see browser_sign_in.sh)
#   shot <display> <out.png>
#   click <display> X Y | type <display> TEXT | key <display> NAME
#   conf <name> [broker_url]                    (re)write the client's test config ("" - no broker)
. "$(dirname "$0")/../test_env.sh"
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$(dirname "$0")/../../../../.." && pwd)
S=${SCRATCH_ROOT:-/tmp/hypersomnia-ui}
U=$S/ui
BIN=${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
X=$REPO/.claude/skills/local-testing/scripts/xinput.py
SITE=http://127.0.0.1:13500
mkdir -p $U

cmd=$1; shift
case $cmd in
conf)
	name=$1; broker=${2-$SITE}
	mkdir -p $U/$name/user/conf.d
	cat > $U/$name/user/conf.d/test.json <<J
{ "identity_broker_url": "$broker", "website_url": "$SITE", "server_list_provider": "http://127.0.0.1:1",
  "launch_at_startup": "MAIN_MENU", "skip_tutorial": true, "prompted_for_sign_in_once": true,
  "window": { "fullscreen": false, "size": [1280, 720] },
  "main_menu": { "leaderboards_provider_url": "$SITE/leaderboards" },
  "client": { "nickname": "${NICK:-uitest}" } }
J
	;;
start)
	name=$1; d=$2; shift 2
	[ -f $U/$name/user/conf.d/test.json ] || $0 conf $name
	if ! pgrep -f "^Xvfb :$d " > /dev/null; then
		(setsid Xvfb :$d -screen 0 1280x720x24 > /dev/null 2>&1 < /dev/null &)
		sleep 1
	fi
	# No real browser: URLs the game opens go to $U/<name>/opened_urls (browser_sign_in.sh plays the browser)
	(cd $REPO/hypersomnia && PATH="$HERE/fake_bin:$PATH" OPENED_URLS=$U/$name/opened_urls DISPLAY=:$d setsid timeout ${SECS:-600} $BIN --appdata-dir $U/$name $GAME_TEST_FLAGS --mute "$@" > $U/$name/game.out 2>&1 < /dev/null &)
	;;
stop)
	name=$1; d=$2
	# Any build (BIN= for the Steam one or not) - the client dir tells which game it is
	for p in $(pgrep -f -- "Hypersomnia --appdata-dir $U/$name "); do kill $p; done
	sleep 1
	;;
xstop)
	d=$1
	for p in $(pgrep -f "^Xvfb :$d "); do kill $p; done
	;;
shot)
	d=$1; out=$2
	mkdir -p $(dirname $out)
	DISPLAY=:$d import -window root $out
	;;
click)
	d=$1; DISPLAY=:$d python3 $X click $2 $3
	;;
type)
	d=$1; DISPLAY=:$d python3 $X type "$2"
	;;
key)
	d=$1; DISPLAY=:$d python3 $X key "$2"
	;;
esac

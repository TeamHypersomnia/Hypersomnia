#!/bin/bash
# Screenshots of a client joining, every half second for N seconds, put together into one contact sheet - to see
# everything the game draws while connecting. Usage: connect_filmstrip.sh <connect string> <appdata dir> <out.png> [seconds=10] [bin]
. "$(dirname "$0")/test_env.sh"
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
CONNECT=$1; A=$2; OUT=$3; SECS=${4:-10}
BIN=${5:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}
F=$(dirname $OUT)/frames_$(basename $OUT .png)
rm -rf $F; mkdir -p $F
cd $REPO/hypersomnia
Xvfb :91 -screen 0 1280x720x24 > /dev/null 2>&1 &
XV=$!
sleep 1
DISPLAY=:91 timeout $((SECS + 5)) $BIN --connect "$CONNECT" --appdata-dir $A $GAME_TEST_FLAGS --mute > $A/filmstrip.out 2>&1 &
G=$!
for i in $(seq -w 1 $((SECS * 2))); do
	sleep 0.5
	DISPLAY=:91 import -window root -resize 25% $F/$i.png 2> /dev/null
done
kill $G 2> /dev/null; wait $G 2> /dev/null
kill $XV
magick montage $F/*.png -tile 5x -geometry +2+2 -background gray $OUT 2> /dev/null || montage $F/*.png -tile 5x -geometry +2+2 $OUT
echo "filmstrip: $OUT"

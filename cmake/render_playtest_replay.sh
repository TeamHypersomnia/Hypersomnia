#!/usr/bin/env bash
set -e

usage() {
	echo "usage: render_playtest_replay.sh --game <executable> --recording <recorded playtest folder>"
	echo "       [--from <secs>] [--to <secs>] [--camera <x>,<y>,<zoom>] [--no-hud] [--size <width>x<height>] [--output <file.mp4>]"
	echo
	echo "Renders a playtest recorded in the editor at exactly 60 fps, with sound, offscreen (Xvfb) - Linux only."
	echo "Without --camera the camera follows the recorded player. --no-hud renders in cinematic mode (no HUD)."
	echo "Copy a ready command with \"Copy render command\""
	echo "in the editor's replay window."
	exit 2
}

GAME=""
RECORDING=""
FROM=0
TO=""
CAMERA=""
NO_HUD=""
SIZE=1920x1080
OUTPUT=""

while [ $# -gt 0 ]; do
	case "$1" in
		--game) GAME=$2; shift 2 ;;
		--recording) RECORDING=${2%/}; shift 2 ;;
		--from) FROM=$2; shift 2 ;;
		--to) TO=$2; shift 2 ;;
		--camera) CAMERA=$2; shift 2 ;;
		--no-hud) NO_HUD=1; shift ;;
		--size) SIZE=$2; shift 2 ;;
		--output) OUTPUT=$2; shift 2 ;;
		*) usage ;;
	esac
done

[ -n "$GAME" ] && [ -n "$RECORDING" ] || usage

for tool in Xvfb ffmpeg; do
	command -v $tool > /dev/null || { echo "$tool is required (pacman -S xorg-server-xvfb ffmpeg)"; exit 1; }
done

GAME=$(realpath "$GAME")
RECORDING=$(realpath "$RECORDING")
CONTENT=$(cd "$(dirname "$0")/../hypersomnia" && pwd)
WIDTH=${SIZE%x*}
HEIGHT=${SIZE#*x}

if [ -z "$OUTPUT" ]; then
	OUTPUT="$RECORDING/render_from_${FROM}s${TO:+_to_${TO}s}.mp4"
fi

OUTPUT=$(realpath -m "$OUTPUT")
WORK=$(mktemp -d /tmp/hypersomnia_render.XXXXXX)

cat > "$WORK/render_config.json" <<EOF
{
	"window": {
		"fullscreen": false,
		"border": false,
		"position": [0, 0],
		"size": [$WIDTH, $HEIGHT]
	}
}
EOF

for n in $(seq 90 140); do
	if [ ! -e /tmp/.X11-unix/X$n ] && [ ! -e /tmp/.X$n-lock ]; then
		DISPLAY_NUMBER=$n
		break
	fi
done

Xvfb :$DISPLAY_NUMBER -screen 0 ${WIDTH}x${HEIGHT}x24 -nolisten tcp > "$WORK/xvfb.log" 2>&1 &
XVFB_PID=$!
FFMPEG_PID=""

stop_helpers() {
	[ -n "$FFMPEG_PID" ] && kill $FFMPEG_PID 2> /dev/null || true
	kill $XVFB_PID 2> /dev/null || true
}

trap stop_helpers EXIT

for _ in $(seq 50); do
	[ -e /tmp/.X11-unix/X$DISPLAY_NUMBER ] && break
	sleep 0.1
done

mkfifo "$WORK/frames.rgba" "$WORK/audio.pcm"

ffmpeg -nostdin -loglevel error -y \
	-thread_queue_size 64 -probesize 32 -analyzeduration 0 \
	-f rawvideo -pixel_format rgba -video_size ${WIDTH}x${HEIGHT} -framerate 60 -i "$WORK/frames.rgba" \
	-thread_queue_size 64 -probesize 32 -analyzeduration 0 \
	-f s16le -ar 48000 -ac 2 -i "$WORK/audio.pcm" \
	-vf vflip -c:v libx264 -preset slow -crf 14 -pix_fmt yuv420p -c:a aac -b:a 320k -movflags +faststart \
	"$OUTPUT" > "$WORK/ffmpeg.log" 2>&1 < /dev/null &
FFMPEG_PID=$!

GAME_ARGS=(
	--replay-playtest "$RECORDING"
	--replay-from "$FROM"
	--replay-frames "$WORK/frames.rgba"
	--replay-audio "$WORK/audio.pcm"
	--apply-config "$WORK/render_config.json"
	--no-update-on-launch
	--no-open-logs
)

[ -n "$TO" ] && GAME_ARGS+=(--replay-to "$TO")
[ -n "$CAMERA" ] && GAME_ARGS+=(--replay-camera "$CAMERA")
[ -n "$NO_HUD" ] && GAME_ARGS+=(--replay-no-hud)

echo "Rendering $RECORDING from ${FROM}s${TO:+ to ${TO}s} at ${WIDTH}x${HEIGHT} (logs in $WORK)..."

GAME_STATUS=0
(cd "$CONTENT" && DISPLAY=:$DISPLAY_NUMBER VISUAL=true DBUS_SESSION_BUS_ADDRESS=disabled: "$GAME" "${GAME_ARGS[@]}" > "$WORK/game.log" 2>&1 < /dev/null) || GAME_STATUS=$?

if kill -0 $FFMPEG_PID 2> /dev/null; then
	timeout 2 sh -c ": > '$WORK/frames.rgba'" || true
	timeout 2 sh -c ": > '$WORK/audio.pcm'" || true
fi

FFMPEG_STATUS=0
wait $FFMPEG_PID || FFMPEG_STATUS=$?
FFMPEG_PID=""

if [ $GAME_STATUS -ne 0 ] || [ $FFMPEG_STATUS -ne 0 ]; then
	echo "Rendering failed (game: $GAME_STATUS, ffmpeg: $FFMPEG_STATUS) - see $WORK/game.log"
	exit 1
fi

echo "$OUTPUT"

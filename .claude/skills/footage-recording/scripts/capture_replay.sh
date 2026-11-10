#!/bin/bash
set -e
here=$(cd "$(dirname "$0")" && pwd)
repo=${REPO:-$(cd "$here/../../../.." && pwd)}
bin=${BIN:-$repo/build/Release-x64-clang-claude/Hypersomnia}
recording=${RECORDING:?RECORDING=<project>/.playtests/recordings/<date>.json is required}
output=${OUTPUT:?OUTPUT=<file.mp4> is required}
work=${WORK:-${TMPDIR:-/tmp}/hypersomnia_footage_capture}
camera=${CAMERA:-}
map=${MAP:-}
from_secs=${FROM:-0}
to_secs=${TO:-}
crf=${CRF:-14}
bars_raise=${BARS_RAISE:-0}
camera_query_mult=${CAMERA_QUERY_MULT:-1.0}
bullets_interpolation=${BULLETS_INTERPOLATION:-INTERPOLATE}
weapon_laser=$([ -n "$LASER" ] && echo true || echo false)
fps=60
size=1920x1080
display=${DISPLAY_NUMBER:-78}

[ -x "$bin" ] || { echo "no game binary at $bin - build it (local-testing build_claude.sh release) or pass BIN="; exit 1; }
[ -f "$recording" ] && [ -f "${recording%.json}.inputs.lz4" ] || { echo "$recording is not a recorded playtest (needs <date>.json and <date>.inputs.lz4)"; exit 1; }
[ -z "$map" ] || [ -f "$map" ] || { echo "no map json at $map"; exit 1; }

while [ -e /tmp/.X11-unix/X$display ]; do
	display=$((display + 1))
done

output=$(realpath -m "$output")
recording=$(realpath "$recording")
appdata=$work/appdata_replay
frames_fifo=$work/replay_frames.fifo
audio_file=$work/replay_audio.s16le
video_only=$work/replay_video_only.mp4
game_log=$work/replay_game.log

mkdir -p "$appdata/user/conf.d" "$(dirname "$output")"
exec 9> "$work/capture.lock"
flock 9
[ -p "$frames_fifo" ] || mkfifo "$frames_fifo"

cat > "$appdata/user/conf.d/replay.json" <<EOF
{ "skip_tutorial": true, "last_seen_changelog_version": "99.0.0",
  "window": { "fullscreen": false, "border": false, "position": [0, 0], "size": [1920, 1080], "max_fps": 1000 },
  "session": { "camera_query_aabb_mult": $camera_query_mult },
  "interpolation": { "modes": { "bullets": "$bullets_interpolation" } },
  "drawing": { "right_bottom_bars_raise": $bars_raise, "draw_weapon_laser": $weapon_laser } }
EOF

replay_args=(--replay-playtest "$recording" --replay-from "$from_secs" --replay-frames "$frames_fifo" --replay-audio "$audio_file")
[ -n "$to_secs" ] && replay_args+=(--replay-to "$to_secs")
[ -n "$camera" ] && replay_args+=(--replay-camera "$camera")
[ -n "$map" ] && replay_args+=(--replay-map "$(realpath "$map")")
[ -z "$HUD" ] && replay_args+=(--replay-no-hud)
[ -n "$NO_PLAYER" ] && replay_args+=(--replay-no-player)

. "$repo/.claude/skills/local-testing/scripts/test_env.sh"
cd "$repo/hypersomnia"

game=""
encoder=""
xvfb=""

stop_capture() {
	[ -n "$game" ] && kill -9 $game 2> /dev/null
	[ -n "$encoder" ] && kill $encoder 2> /dev/null
	[ -n "$xvfb" ] && kill $xvfb 2> /dev/null
	exit 143
}

trap stop_capture TERM INT

setsid Xvfb :$display -screen 0 ${size}x24 > /dev/null 2>&1 < /dev/null &
xvfb=$!
sleep 1

ffmpeg -nostdin -v error -y \
	-f rawvideo -pix_fmt rgba -s $size -r $fps -i "$frames_fifo" \
	-vf "vflip,scale=out_color_matrix=bt709:out_range=tv:flags=accurate_rnd+full_chroma_int+full_chroma_inp" \
	-colorspace bt709 -color_primaries bt709 -color_trc bt709 -color_range tv \
	-c:v libx264 -preset slow -crf $crf -pix_fmt yuv420p -x264-params colorprim=bt709:transfer=bt709:colormatrix=bt709 \
	-an "$video_only" < /dev/null &
encoder=$!

flags_with_captured_sound=${GAME_TEST_FLAGS/--mute/}
DISPLAY=:$display setsid "$bin" --appdata-dir "$appdata" "${replay_args[@]}" $flags_with_captured_sound > "$game_log" 2>&1 < /dev/null &
game=$!

wait -n $game $encoder || true

for _ in $(seq 30); do
	kill -0 $game 2> /dev/null || break
	sleep 1
done

if kill -0 $game 2> /dev/null; then
	echo "the game did not quit 30 s after the encoder - stopping it (see $game_log)"
	kill -9 $game 2> /dev/null || true
fi

game_status=0
wait $game || game_status=$?

if kill -0 $encoder 2> /dev/null; then
	timeout 2 sh -c ': > "$1"' sh "$frames_fifo" || true
fi

encoder_status=0
wait $encoder || encoder_status=$?
kill $xvfb 2> /dev/null || true

if [ $game_status -ne 0 ] || [ $encoder_status -ne 0 ] || [ ! -s "$video_only" ]; then
	echo "capture failed (game: $game_status, encoder: $encoder_status) - see $game_log"
	exit 1
fi

if [ -s "$audio_file" ]; then
	ffmpeg -nostdin -v error -y -i "$video_only" -f s16le -ar 48000 -ac 2 -i "$audio_file" \
		-map 0:v -map 1:a -c:v copy -c:a aac -b:a 320k "$output" < /dev/null
else
	cp "$video_only" "$output"
fi

echo "$output (game log: $game_log)"

#!/usr/bin/env bash
#
# Cuts the rolling of shells out of freesound 337236 (anthousai, bullet shells handful consolidated), eight drops of shells:
#
#   small_shell_roll_N.ogg - the first five drops without their first knock, for light shells
#   shell_roll_N.ogg       - the sixth, seventh and eighth, for heavy shells - without their first knocks either,
#                            and fading in longer, so they don't start with too much of a punch
#
# The recording is quiet, so every roll is brought to a peak of ROLL_PEAK_DB - see cut_to_peak in floor_hit_cutting.sh.
#
# Usage: cmake/asset_scripts/cut_shell_roll_sounds.sh [source.wav] [output directory]

set -euo pipefail

SOURCE="${1:-$HOME/Downloads/337236__anthousai__bullet-shells-handfull-consolidated.wav}"
OUT_DIR="${2:-$(dirname "$0")/../../hypersomnia/content/sfx}"

SMALL_ROLL_FADE_IN=0.010
ROLL_FADE_IN=0.030
FADE_OUT=0.080
QUALITY=10

SMALL_ROLL_PEAK_DB=-14
ROLL_PEAK_DB=-12

# The start and the end of every roll, in seconds - starting right after the first knock.
# The third drop knocks twice before rolling, so both are left out.
SMALL_ROLLS=(
	"0.160 0.680"
	"1.778 2.170"
	"4.440 4.730"
	"6.900 7.410"
	"9.100 9.740"
)

ROLLS=(
	"11.520 11.990"
	"13.730 14.240"
	"16.210 16.730"
)

# The shells ring at these frequencies in Hz through the rolls - notched out narrowly, like in cut_shell_floor_hits.sh.
# The heavy ones ring a bit differently.
SMALL_RINGING_TONES=(3000 9200 11300 15300)
RINGING_TONES=(7200 11300 12300 15300)
RINGING_NOTCH_WIDTH=500
RINGING_NOTCH_GAIN=-32

notches() {
	local filters=""

	for tone in "$@"; do
		filters+="${filters:+,}equalizer=f=$tone:t=h:w=$RINGING_NOTCH_WIDTH:g=$RINGING_NOTCH_GAIN"
	done

	echo "$filters"
}

source "$(dirname "$0")/floor_hit_cutting.sh"

cut_rolls() {
	local prefix="$1" peak_db="$2" fade_in="$3" filters="$4"
	shift 4

	local index=1

	for roll in "$@"; do
		read -r start end <<< "$roll"
		cut_to_peak "$OUT_DIR/${prefix}_$index.ogg" "$start" "$end" "$fade_in" "$FADE_OUT" "$filters" "$peak_db"
		index=$((index + 1))
	done
}

mkdir -p "$OUT_DIR"

cut_rolls small_shell_roll "$SMALL_ROLL_PEAK_DB" "$SMALL_ROLL_FADE_IN" "$(notches "${SMALL_RINGING_TONES[@]}")" "${SMALL_ROLLS[@]}"
cut_rolls shell_roll "$ROLL_PEAK_DB" "$ROLL_FADE_IN" "$(notches "${RINGING_TONES[@]}")" "${ROLLS[@]}"

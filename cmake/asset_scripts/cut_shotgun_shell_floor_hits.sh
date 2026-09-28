#!/usr/bin/env bash
#
# Cuts the shotgun shell bouncing of freesound 523059 (magnuswaker, shell bounce 1):
#
#   shotgun_shell_floor_hit_first_1.ogg, _2.ogg - each of its two first loud bounces, as variations of the first hit -
#                                                 two such bounces in one fall would be too much
#   shotgun_shell_floor_hit_second.ogg          - the rest of it, the same for every fall
#
# The recording is loud, so the parts are brought down to peaks of FIRST_PEAK_DB and SECOND_PEAK_DB -
# about those of the other shells, see cut_to_peak in floor_hit_cutting.sh.
#
# Usage: cmake/asset_scripts/cut_shotgun_shell_floor_hits.sh [source.wav] [output directory]

set -euo pipefail

SOURCE="${1:-$HOME/Downloads/523059__magnuswaker__shell-bounce-1.wav}"
OUT_DIR="${2:-$(dirname "$0")/../../hypersomnia/content/sfx}"

FADE_IN=0.005
FIRST_FADE_OUT=0.050
SECOND_FADE_OUT=0.150
QUALITY=10

FIRST_PEAK_DB=-10
SECOND_PEAK_DB=-12

# The start and the end of every part, in seconds.
FIRST_BOUNCES=(
	"0.285 0.525"
	"0.525 0.725"
)

REST="0.725 1.370"

source "$(dirname "$0")/floor_hit_cutting.sh"

mkdir -p "$OUT_DIR"

index=1

for bounce in "${FIRST_BOUNCES[@]}"; do
	read -r start end <<< "$bounce"
	cut_to_peak "$OUT_DIR/shotgun_shell_floor_hit_first_$index.ogg" "$start" "$end" 0 "$FIRST_FADE_OUT" "" "$FIRST_PEAK_DB"
	index=$((index + 1))
done

read -r start end <<< "$REST"
cut_to_peak "$OUT_DIR/shotgun_shell_floor_hit_second.ogg" "$start" "$end" "$FADE_IN" "$SECOND_FADE_OUT" "" "$SECOND_PEAK_DB"

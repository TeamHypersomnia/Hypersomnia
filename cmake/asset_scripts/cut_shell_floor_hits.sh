#!/usr/bin/env bash
#
# Cuts the five shell drops of freesound 490309 (nox_sound, object_bullet_shell_concrete)
# into shell_floor_hit_first_N.ogg and shell_floor_hit_second_N.ogg - see floor_hit_cutting.sh.
#
# Usage: cmake/asset_scripts/cut_shell_floor_hits.sh [source.wav] [output directory]
# Tweak the fades below and run again.

set -euo pipefail

SOURCE="${1:-$HOME/Downloads/490309__nox_sound__object_bullet_shell_concrete.wav}"
OUT_DIR="${2:-$(dirname "$0")/../../hypersomnia/content/sfx}"

PRE_ROLL=0.005
FIRST_FADE_OUT=0.050
SECOND_FADE_IN=0.005
SECOND_FADE_OUT=0.400
QUALITY=10

# For every drop: the onset of its first hit, the onset of the second hit, the end of the drop - in seconds.
DROPS=(
	"0.010 0.222 0.670"
	"1.792 1.996 2.445"
	"3.260 3.446 3.860"
	"4.426 4.596 5.150"
	"5.964 6.188 6.685"
)

# The casing rings at these frequencies in Hz through all the drops - notched out narrowly,
# so that mostly the knocks remain. The ringing made most of the loudness,
# so the makeup gain brings the knocks back to about as loud as they were.
RINGING_TONES=(3720 7267 9420 10330 14260 15050)
RINGING_NOTCH_WIDTH=400
RINGING_NOTCH_GAIN=-32
RINGING_MAKEUP_GAIN=8dB

FILTERS=""

for tone in "${RINGING_TONES[@]}"; do
	FILTERS+="${FILTERS:+,}equalizer=f=$tone:t=h:w=$RINGING_NOTCH_WIDTH:g=$RINGING_NOTCH_GAIN"
done

FILTERS+=",volume=$RINGING_MAKEUP_GAIN"

PREFIX=shell_floor_hit

source "$(dirname "$0")/floor_hit_cutting.sh"
cut_all_drops

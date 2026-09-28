#!/usr/bin/env bash
#
# Cuts nineteen of the twenty shell drops of freesound 318964 (gryffdavid, bouncing shell casings of various sizes) -
# the twelfth is left out, as the recording gates its bounces off abruptly -
# into small_shell_floor_hit_first_N.ogg and small_shell_floor_hit_second_N.ogg - see floor_hit_cutting.sh.
#
# Usage: cmake/asset_scripts/cut_small_shell_floor_hits.sh [source.wav] [output directory]
# Tweak the fades below and run again.

set -euo pipefail

SOURCE="${1:-$HOME/Downloads/318964__gryffdavid__bouncing-shell-casings-various-sizes.wav}"
OUT_DIR="${2:-$(dirname "$0")/../../hypersomnia/content/sfx}"

PRE_ROLL=0.005
FIRST_FADE_OUT=0.050
SECOND_FADE_IN=0.005
SECOND_FADE_OUT=0.050
QUALITY=10

# Where a fourth number follows, the first part ends there, before faint bounces - see floor_hit_cutting.sh.
DROPS=(
	"0.100 0.329 1.004 0.165"
	"2.488 2.642 3.144"
	"4.502 4.666 5.264"
	"6.838 6.992 7.494"
	"8.318 8.522 9.006"
	"11.188 11.356 11.910"
	"13.076 13.328 13.902"
	"15.262 15.330 15.872"
	"16.838 17.046 17.700"
	"19.018 19.388 19.664 19.225"
	"20.976 21.212 21.766"
	"25.978 26.292 26.862"
	"27.916 28.184 28.754"
	"30.370 30.572 31.078"
	"32.000 32.223 32.702 32.130"
	"34.226 34.350 34.870"
	"35.660 35.738 36.272"
	"36.948 37.224 37.714"
	"39.302 39.569 40.194 39.370"
)

PREFIX=small_shell_floor_hit

source "$(dirname "$0")/floor_hit_cutting.sh"
cut_all_drops

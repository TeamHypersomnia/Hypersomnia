#!/usr/bin/env bash
set -euo pipefail

SOURCE="${1:-$HOME/Downloads/002_Synthwave_15k.mp3}"
ASSET_SCRIPTS="$(cd "$(dirname "$0")" && pwd)"
REPO="$ASSET_SCRIPTS/../.."
AUDIO_SCRIPTS="$REPO/.claude/skills/audio-processing/scripts"
TUTORIAL_JSON="$REPO/hypersomnia/content/menu/tutorial/tutorial.json"
BUILT_MUSIC="${XDG_CACHE_HOME:-$HOME/.cache}/hypersomnia-tutorial-music.ogg"

if [ ! -f "$SOURCE" ]; then
	echo "Download 002_Synthwave_15k.mp3 from https://opengameart.org/content/calm-ambient-2-synthwave-15k first"
	exit 1
fi

mkdir -p "$(dirname "$BUILT_MUSIC")"
python3 "$AUDIO_SCRIPTS/phrase_loop.py" "$ASSET_SCRIPTS/tutorial_music_phrases.json" "$SOURCE" "$BUILT_MUSIC"
python3 "$AUDIO_SCRIPTS/swap_audio_resource.py" "$TUTORIAL_JSON" @calm_ambient_loop "$BUILT_MUSIC"

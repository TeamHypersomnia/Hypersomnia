#!/usr/bin/env bash
set -euo pipefail

SOURCE="${1:-$HOME/Downloads/space_tutorial/music/candidates/src_calm_ambient2.mp3}"
ASSET_SCRIPTS="$(cd "$(dirname "$0")" && pwd)"
REPO="$ASSET_SCRIPTS/../.."
AUDIO_SCRIPTS="$REPO/.claude/skills/audio-processing/scripts"
TUTORIAL_JSON="$REPO/hypersomnia/content/menu/tutorial/tutorial.json"
TUTORIAL_MUSIC="$REPO/hypersomnia/content/menu/tutorial/space/calm_ambient_loop.ogg"

if pgrep -x Hypersomnia > /dev/null; then
	echo "Hypersomnia is running - close it first, the editor would overwrite the tutorial"
	exit 1
fi

python3 "$AUDIO_SCRIPTS/phrase_loop.py" "$ASSET_SCRIPTS/tutorial_music_phrases.json" "$SOURCE" "$TUTORIAL_MUSIC"
python3 "$AUDIO_SCRIPTS/swap_audio_resource.py" "$TUTORIAL_JSON" @calm_ambient_loop "$TUTORIAL_MUSIC"

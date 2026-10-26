#!/bin/bash
# The HYPERSOMNIA_* variables: the game's lists (src/application/main/environment_variables.{h,cpp}), README_SERVER.md's
# table and the config generator (the site's public/assets/scripts/generator.js) must name the same ones - a typo on any
# side shows as a name missing from the others. Usage: PAGE=<the site's repo> check_environment_variables.sh
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
PAGE=${PAGE:?set PAGE to the site repo (Hypersomnia-homepage)}

names() { grep -ohE 'HYPERSOMNIA_[A-Z_]+[A-Z]' "$@" | sort -u; }

# Old names the game only points to their new ones (renamed_tournament_setting, HYPERSOMNIA_ARENA) - read by no one
OLD_NAMES='^HYPERSOMNIA_(ARENA|TOURNAMENT_ARENAS|TOURNAMENT_SPECTATORS|TOURNAMENT_JOIN_TIMEOUT)$'
GAME=$(names $REPO/src/application/main/environment_variables.h $REPO/src/application/main/environment_variables.cpp | grep -vE "$OLD_NAMES")
# The README's table rows only - the prose names a few in passing
README=$(grep -E '^\| `HYPERSOMNIA_' $REPO/README_SERVER.md | names -)
# The generator writes only what it has a field for: not HYPERSOMNIA_CONFIG nor the server key, nor the map to start
# on (it writes the map cycle - the server starts on its first)
NOT_GENERATED='^HYPERSOMNIA_(CONFIG|SERVER_PRIVATE_KEY|MAP)$'
GENERATOR=$(names $PAGE/public/assets/scripts/generator.js | grep -vE "$NOT_GENERATED")

ok=1
compare() {
	local missing
	missing=$(comm -3 <(echo "$2") <(echo "$4"))

	if [ -n "$missing" ]; then
		echo "FAIL: $1 vs $3:"
		comm -23 <(echo "$2") <(echo "$4") | sed "s/^/  only in $1: /"
		comm -13 <(echo "$2") <(echo "$4") | sed "s/^/  only in $3: /"
		ok=0
	fi
}

compare game "$GAME" README_SERVER.md "$README"
compare game "$(echo "$GAME" | grep -vE "$NOT_GENERATED")" generator "$GENERATOR"

[ $ok = 1 ] && echo "PASS: $(echo "$GAME" | wc -l) variables, the same in the game, README_SERVER.md and the generator"
[ $ok = 1 ]

#!/bin/bash
# Missiles fired at one character standing still - damage, death, standing and lying corpses, gore - without bots.
# Builds a map of only a floor, the character's spawn at (0,0) facing +x and "missile" nodes (editor Missile type,
# fired once at the round start), playtests it in the editor and screenshots the character.
#
#   FIREARM      what the character holds - its stance (none, sn69, baka47, awka...) - default none
#   ROUND        the missile node type (an official round: baka47_round, ao44_round, shotgun_red_round...)
#                - default baka47_round; a comma-separated list goes round the shots in turn,
#                e.g. "baka47_round,baka47_round,baka47_round,ao44_round" to finish off the standing corpse with another gun
#   SHOTS        how many rounds, in a column across the body (y from -34 to 34), 220 px apart - default 14
#   ACROSS       the column's span across the body, "from:to" in px - default -34:34; e.g. -44:-36 only its edge
#   PELLETS=N    instead: N volleys of 9 shotgun pellets at random spots across the body (SEED)
#   DIR          degrees the shots come from (0: from +x, in front of the character) - default 0
#   START        how far the first shot starts, px (they fly 4000 px/s) - default 300; e.g. 6000 at SPEED=0.1
#                for the hits to come after the game has loaded (~6 s), so that the standing corpse is on the frames
#   SPEED        --playtest-speed - default 0.1 (slow enough for the screenshots)
#   FRAMES, EVERY   screenshots: how many and the seconds between them - default 40 and 0.3
#   GORE=0       the gore setting off - blood remapped to green, as on CrazyGames
#   PORTAL=D     a portal D px behind the character (where the corpse slides to), its exit 500 px to the side,
#                facing -y: the corpse should come out moving and lying along the exit's direction.
#                250 catches the standing corpse, 700 mostly the lying one (pushed on as it falls)
#   BIN, SCRATCH (the run dir's parent), XDISPLAY (default :79 - started if not running)
#
# Prints the run dir: z_NN.png (800x600 around the character, zoomed 4x), xyz.txt (XYZDBG lines), out.txt.
# The character never respawns, so the corpse stays in view. Its own appdata - the user's config is untouched.
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../../../.." && pwd)
. "$REPO/.claude/skills/local-testing/scripts/test_env.sh"

FIREARM=${FIREARM:-none}
ROUND=${ROUND:-baka47_round}
SHOTS=${SHOTS:-14}
PELLETS=${PELLETS:-0}
DIR=${DIR:-0}
SEED=${SEED:-1}
PORTAL=${PORTAL:-0}
START=${START:-300}
ACROSS=${ACROSS:--34:34}
SPEED=${SPEED:-0.1}
FRAMES=${FRAMES:-40}
EVERY=${EVERY:-0.3}
XDISPLAY=${XDISPLAY:-:79}
GORE_JSON=$([ "${GORE:-1}" = 0 ] && echo false || echo true)
BIN=$(realpath "${BIN:-$REPO/build/Debug-x64-clang-claude/Hypersomnia}")
S=${SCRATCH:-/tmp/hypersomnia-damage-test}

NAME=damage_test_${FIREARM}_$(date +%s)
T=$S/$NAME
mkdir -p "$T/$NAME"

if [ ! -e "/tmp/.X${XDISPLAY#:}-lock" ]; then
	Xvfb "$XDISPLAY" -screen 0 3200x1800x24 > /dev/null 2>&1 &
	sleep 1
fi

python3 - "$T/$NAME/$NAME.json" "$NAME" "$FIREARM" "$ROUND" "$SHOTS" "$PELLETS" "$DIR" "$SEED" "$PORTAL" "$START" "$ACROSS" <<'PY'
import json, sys, math, random
out, name, firearm, round_type, shots, pellets, direction, seed, portal, start, across_span = sys.argv[1:]
across_from, across_to = map(float, across_span.split(':'))
shots, pellets, direction, start = int(shots), int(pellets), float(direction), float(start)
random.seed(int(seed))

# (distance from the character, offset across the shot line, type, rotation jitter)
missiles = []

if pellets:
	for v in range(pellets):
		for k in range(9):
			missiles.append((start + v * 450 + random.uniform(0, 30), random.uniform(-32, 32), 'shotgun_red_round', random.uniform(-2, 2)))
else:
	for i in range(shots):
		across = across_from + (across_to - across_from) * (i / max(1, shots - 1))
		rounds = round_type.split(',')
		missiles.append((start + i * 220, across, rounds[i % len(rounds)], 0))

a = math.radians(direction)
ux, uy = math.cos(a), math.sin(a)
px, py = -uy, ux

nodes = [
	{ "id": "floor", "type": "water_room_floor", "pos": [0.0, 0.0], "size": [3584, 1536] },
	{ "id": "target", "type": "team_spawn", "faction": "METROPOLIS", "pos": [0.0, 0.0], "rotation": 0.0 }
]

for i, (dist, across, t, jitter) in enumerate(missiles):
	nodes.append({
		"id": f"shot_{i}", "type": t,
		"pos": [ux * dist + px * across, uy * dist + py * across],
		"rotation": direction + 180 + jitter,
		"speed": 4000.0
	})

if float(portal) > 0:
	# Behind the character as seen from the shots - where the corpse slides to.
	nodes.append({ "id": "portal_in", "type": "portal", "pos": [-ux * float(portal), -uy * float(portal)], "rotation": direction + 180, "size": [128, 112],
		"enter_time_ms": 0.0, "travel_time_ms": 0.0, "portal_exit": "portal_out", "exit_position": "PORTAL_BOUNDARY" })
	nodes.append({ "id": "portal_out", "type": "portal", "pos": [0.0, -500.0], "rotation": -90.0, "size": [128, 112],
		"enter_time_ms": 0.0, "travel_time_ms": 0.0, "exit_position": "PORTAL_BOUNDARY" })

equipment = { "firearm": None if firearm == "none" else firearm, "melee": None, "backpack": False }

project = {
	"meta": { "name": name },
	"settings": { "default_server_mode": "quick_test", "ambient_light_color": [51, 93, 98, 255] },
	"playtesting": { "mode": "quick_test" },
	"game_modes": { "quick_test": {
		"respawn_time_ms": 100000,
		"equipment": { "metropolis": equipment, "resistance": equipment }
	} },
	"layers": [ { "id": "Layer 1", "nodes": [n["id"] for n in reversed(nodes)] } ],
	"nodes": nodes
}

json.dump(project, open(out, "w"), indent=1)
PY

cd "$REPO/hypersomnia"

HYPERSOMNIA_CONFIG='{"gore": '$GORE_JSON', "window": {"fullscreen": false, "position": [0, 0], "size": [3200, 1800]}, "drawing": {"draw_crosshairs": false, "draw_damage_indicators": false, "snap_zoom_to_fov_size": false, "custom_zoom": 4}}' \
DISPLAY=$XDISPLAY timeout 40 "$BIN" --edit "$T/$NAME" --playtest --playtest-speed "$SPEED" --appdata-dir "$T/appdata" $GAME_TEST_FLAGS > "$T/out.txt" 2>&1 &
GAME=$!

sleep 5

for i in $(seq -w 1 "$FRAMES"); do
	DISPLAY=$XDISPLAY import -window root "$T/full_$i.png"
	sleep "$EVERY"
done

wait $GAME

grep -rh XYZDBG "$T/appdata/logs" > "$T/xyz.txt" 2> /dev/null

for f in "$T"/full_*.png; do
	magick "$f" -crop 800x600+1200+600 +repage "${f/full_/z_}"
done

echo "$T"

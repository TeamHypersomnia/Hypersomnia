# Testing damage, death and gore without bots

For anything that happens to a character when it is hit or dies - damage, knockout, standing corpses, limbs torn off,
lying corpses, bullet holes, blood - don't play against bots: fire a fixed set of missiles at one character standing
still and look at screenshots. Same shots every run, so a change can be compared before/after.

It is built on the editor's **Missile** node type (resources folder "Missiles", one per official round): placed on a map,
it is fired once as the round starts, in the node's direction at its `speed` - as if from the gun whose charge it is
(the sender and multipliers, `missile_node_shot.h`), so damage, headshots, standing time and falling at once are
the gun's. A round no gun shoots stays gunless.

## Running

`scripts/damage_test.sh` makes the map itself (a floor, the character's spawn at (0,0) facing +x, the missile nodes),
playtests it in the editor on its own Xvfb (`:79`, 3200x1800) and screenshots the character:

```
FIREARM=baka47 scripts/damage_test.sh                     # 14 Baka rounds in a column across the body, from the front
FIREARM=sn69 DIR=90 scripts/damage_test.sh                # pistol stance, shot from +y
PELLETS=2 SEED=5 SPEED=0.3 scripts/damage_test.sh         # 2 volleys of shotgun pellets at random spots - many hits at once
ROUND=ao44_round SHOTS=3 scripts/damage_test.sh           # any official round
GORE=0 FIREARM=sn69 scripts/damage_test.sh                # the gore setting off, as on CrazyGames: all blood green
FIREARM=baka47 SPEED=0.1 START=3500 scripts/damage_test.sh # the hits come after the game has loaded: the standing corpse
FIREARM=baka47 PORTAL=250 SPEED=0.3 scripts/damage_test.sh # a portal behind: the standing corpse goes through it
FIREARM=baka47 PORTAL=500 SPEED=0.3 scripts/damage_test.sh # the lying corpse goes through it (pushed on as it falls)
ROUND=pistol_cyan_round SHOTS=10 ACROSS=-46:-30 scripts/damage_test.sh   # SN69 rounds only into the side of the body
ROUND=baka47_round,baka47_round,baka47_round,ao44_round SHOTS=4 scripts/damage_test.sh  # rounds in turn
```

Blasts can't be tested this way yet: the explosive missiles fired by Missile nodes (`electric_missile`,
`skull_rocket_flying`, `amplifier_arm_missile`) hit the character directly but never hurt it with their explosion
(no health event `from_explosion`) - they go through a dead body and there is nothing behind it to explode on.
To see what a blast leaves on a corpse (`got_body_blasted` - its splatter), set the flag on any hit for a test build.

New blood must stay green with `GORE=0`: images are remapped only when their path has a word of
`gore_sprites_match_words`, and only the exact reds of `gore_color_map` (`src/augs/graphics/gore_colors.h`);
particles only with `should_gore_remap`. Count the red pixels left on the frames (orange lights of items are not blood).

- `FIREARM` is what the character holds, i.e. its **stance**: `none` (bare hands), `sn69` (pistol), `baka47` (rifle)...
  Test every stance - the colliders differ, and a weapon held at death falls out of the hands, so a standing corpse
  is in another stance than the character was.
- The character **never respawns** (`respawn_time_ms` 100000) - otherwise it respawns on top of its own corpse.
- The game runs with its **own appdata** and config from `HYPERSOMNIA_CONFIG` (window 3200x1800, zoom 4, no crosshair, no
  damage numbers) - the user's `runtime_prefs.json` is not read, as it would override the window size.
- Prints the run dir: `z_NN.png` (800x600 around the character), `xyz.txt` (every `XYZDBG` log line), `out.txt`.
- `SPEED` is `--playtest-speed`: 0.1 to see each hit, 0.3 to get to what happens after the corpse settles
  (the game is killed after 40 s).
- The game loads for ~6 s with its fresh appdata and the round goes on meanwhile: with the default `START` (300 px) the
  character has mostly fallen by the first frame. To see the standing corpse, start the shots farther (`START=3500` at
  `SPEED=0.1`) - not much farther, a missile's lifetime runs out.
- The character is a box of 40x83 in most stances (only the pistol's frame has its own shape - see `stance_shape.h`),
  rotated by the stance: a rifle's 41 degrees. A shot's point is the bullet's middle, mostly outside the box when it hits.
- `BIN` defaults to `build/Debug-x64-clang-claude` like `run_map.sh`.

Look at a montage first, then crop the frames that matter:

```
cd <run dir> && montage z_0[3-9].png z_1[0-9].png -tile 6x3 -geometry 300x225+2+2 m.png
```

## Pitfalls

- **Blood particles linger in slow motion** - at 0.1 speed a red cloud stays over the body for seconds. It is not
  a bug in the decals; take a later frame or a higher `SPEED`.
- **Blood splatters are under lying corpses** (`GROUND_DECALS`) - splatters spawned inside the body outline are hidden.
- Pellets tear limbs off fast: 3 volleys leave a stump, 1 volley may not kill. 2 is usually right.
- From the side (`DIR=90/270`) the bare-hands body is only ~40 px wide - the default column of shots (±34 px)
  partly misses and may not kill.

## Measuring where things land

Eyeballing 4 px holes on a screenshot is not enough. Log the decision with a temporary `LOG("XYZDBG ...")`
(include `augs/log.h` if the header lacks it), read `xyz.txt`, remove the logs (`rg XYZDBG src` empty) before committing.

`scripts/decal_viz.py <xyz.txt> <standing sprite> <out.png>` draws the standing sprite (e.g.
`metropolis_torso_bare_walk_shot_1`) with each hole where it was laid out on the standing body, next to the lying corpse
with the same hole where it landed, and the drips under them. Each hole keeps the stance it was made in (`stance=` in the
lines, `item_holding_stance`: 1 bare hands, 2 pistol, 3 rifle), so one standing sprite fits only the holes of its stance.
It reads these lines, logged in `move_character_decals_onto` and `spawn_lying_corpse_drip`
(`src/game/detail/decals/character_gunshot_decal.h`):

```
XYZDBG move corpse=<new_owner.get_name()> from_standing=1 flipped=<0|1> stance=<s> mirrored=<0|1> reach=<r> hit=(x;y) standing=(x;y) local=(x;y) fits=<0|1>
XYZDBG drip ok local=(x;y) rot=<corpse rotation> variant=<0..2>
XYZDBG drip skip local=(x;y)
```

`hit` is where the hole was put on the hit, `standing` where it was laid out among the others at the fall,
`local` where it landed on the lying corpse (`fits=0`: it went with a limb the corpse no longer has).

The lying sprite is chosen from the corpse's name (`Lying corpse metropolis noarm` -> `metropolis_torso_corpse_noarm.png`).

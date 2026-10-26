---
name: run-map
description: Start any Hypersomnia map from the CLI for a test - a dedicated server on it (official, downloaded or an edited copy), with or without bots, a spectator or visible client, the editor playtest, and copies of a map with changed rules or items (map_copy.py). Use whenever a test needs a map running - gameplay, bots, shop, UI on screen, rendering - "odpal mapkę", "na testowej mapce", "na fy_iceworld", "stestuj na mapie", also as the base of the bot matches in local-testing.
---

# Running a map for a test

Two scripts in `scripts/`, both with paths derived from the repo:

- `run_map.sh` - a dedicated server on `MAP` (optionally `BOTS=1`) plus a client: `CLIENT=spectator` (xvfb-run, default),
  `display` (on `$DISPLAY`, e.g. your own `Xvfb :78` for `import -window root` screenshots and
  `local-testing/scripts/xinput.py` clicks), or `none`. `SECS`, `KEEP=1` (leave it running, pids in the run dir),
  `SERVER_EXTRA` (more JSON of the "server" section), `LOG_GREP` (lines printed from the server log at the end).
  Example: `MAP=fy_iceworld BOTS=1 SECS=150 LOG_GREP=XYZDBG SCRATCH=<scratchpad> scripts/run_map.sh`
- `map_copy.py <map dir> <dest parent> <new name> 'json.path=value' ...` - a renamed copy with JSON edits, then
  `MAP=<new name> MAP_DIR=<printed dir> scripts/run_map.sh`. E.g. a shop with snipers only:
  `'game_modes.bomb_defusal.shop_allowed_firearms=["hunter","awka","bulldup2000"]'`.

Build into `build/Debug-x64-clang-claude` first (`BIN` defaults to it), never `build/current` (the user's).

## Pitfalls already hit - check these before debugging the game

- **Symlinked maps don't load** - the arena scan skips them: the server logs `Failed to load arena`. Copy the folder
  (`run_map.sh` does).
- **A copy named like an existing map is shadowed** by `content/arenas/<name>` (or the downloads): the edits seem to do
  nothing. Give it a new name, and the JSON inside must be named `<folder>.json` (`map_copy.py` does both).
- Downloaded maps are in `hypersomnia/user/downloads/maps/` (e.g. `fy_iceworld`); `run_map.sh` takes them by name.
- **Bots play only while a human client is connected** - with `CLIENT=none` they stand still. A spectator client is enough.
- Bots get mixed levels by default (`bot_difficulty: PROGRESSIVE`) - for per-difficulty numbers pass
  `SERVER_EXTRA=', "bot_difficulty": "HARD"'` (a var of the "server" section, like "bots").
- A map's rules come from its JSON `game_modes.<mode>` (bomb_defusal: round_time, buy_time, warmup_time,
  shop_allowed_firearms...); items placed on the map are nodes typed by the firearm's resource name (`"awka"`, `"hunter"`,
  `"bilmer2000"`) and come back at every round start.
- Heartbeats go to `127.0.0.1:1` - always check the `Sending heartbeat through UDP to` line it prints; a test server
  must never reach the production masterserver.
- Run one at a time (ports 10091, 19930-19935); a previous run's server lingers ~20 s after the client exits - "port busy"
  right after a run means wait.
- Never run the user's `build/current` build and edit headers at the same time; builds are memory-capped by CMake
  (job pools), don't pass a higher `-j`.

- **Desyncs:** the client compares its state with the server's hash and logs `Client desynchronized at step` -
  `run_map.sh` prints their count; anything above 0 is a determinism bug (a client that joined mid-round
  has the server's snapshot, so a desync right after joining means the snapshot misses something - see
  `never_changes_in_game` in net_solvable_stream.h).

## The editor and playtest

`../build/Debug-x64-clang-claude/Hypersomnia --edit <project dir> --playtest $GAME_TEST_FLAGS` (after `. ../.claude/skills/local-testing/scripts/test_env.sh`) from `hypersomnia/` - always on a scratch
copy under a distinct name (the editor writes `editor_view.json`, autosaves and `navmesh.nav` into it).
Items placed as nodes must also be listed in a layer's `nodes`, or the map fails to load. Keys held with
`local-testing/scripts/hold_keys.py`; don't move the mouse - the crosshair moves relatively.

## Measure, don't eyeball

Add a temporary `LOG("XYZDBG ...")` where the decision is made, build, run with `LOG_GREP=XYZDBG`, parse the lines,
then remove the logs (`rg XYZDBG src` must be empty) and rebuild before committing.

## Evaluating generated bot waypoints (random_waypoints_generator)

`scripts/analyze_waypoints.py <log> <out.png>` compares many generated seeds with the author's waypoints on one map:
counts, coverage (distance of free cells to the nearest waypoint), how often each author patrol spot gets a generated
one within 400/600/800 px, seed-to-seed similarity, camp classes, and a picture (free cells, generated patrol density red,
author patrols green, author pushes blue). It reads `XYZDBG cell x y`, `XYZDBG off <type> <faction> x y <quality> <ahead>`,
`XYZDBG gen <seed> <type> <faction> x y <camp_secs_min> <look_far>` and `XYZDBG avg_us N` lines - log them from a temporary
hook in `create_random_waypoints` (first round only: free navmesh cells, the author's waypoints with `rate_patrol_spot`,
then 200 seeds of `generate_random_waypoints` with `existing` cleared, timed), run `MAP=<map> CLIENT=none SECS=20`
with the hook behind an env var, and measure times with `BIN=build/current/Hypersomnia` (Debug is ~15x slower).
Results of 2026-10-18: 81-88% of author patrol spots within 600 px per round, 0.2-1.3 ms per generation.

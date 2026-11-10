---
name: footage-recording
description: Record Hypersomnia gameplay footage for videos (devlogs, trailers, the site) - stage a shot in the editor (quick test spawns, bots walking to markers, playtest spawns), let the editor record the playtest, render the recording offline at exact 60 fps with sound (capture_replay.sh - camera, HUD, laser, no player, from/to, slow motion), re-render an old recording on a changed map, and keep every source safe. Use whenever footage is needed - "nagraj footage", "nagraj to", "wyrenderuj replay", "zrób ujęcie", "capture", "nagraj jeszcze raz z nowymi światłami", "potrzebuję klipu do filmiku", or the user pastes a "[hypersomnia replay moment]" line. Editing the footage into a video is a separate matter (not this skill).
---

# Footage recording

Footage comes from **recorded editor playtests rendered offline**: the user (or bots) plays once in the editor, the
recording is deterministic, and `scripts/capture_replay.sh` renders it under Xvfb at exactly 60 fps with the game's
own sound - any camera, with or without HUD, laser or the player, as many times as needed.

## 0. Back up first - every time

**Copy every recording that a capture uses to a persistent folder before anything else** (`cp -r`, never the
session scratchpad; ask the user where if no project folder exists yet). The editor deletes the oldest unnamed
recordings of a map beyond a limit - it was 50 and deleted two devlog 6 sources: one came back only through ext4
journal forensics, the other is lost (only its rendered clip survived). The limit is 1000 now, but backups stay
the rule. Also back up the rendered takes: a re-capture overwrites `OUTPUT` - copy the previous take aside first
(keep every take; the user compares them).

## 1. Recording a playtest

- The editor records **every offline playtest** (Space in the editor, or `--edit <project> --playtest`) into
  `<project>/.recorded_playtests/<date>/`: `map.json` (the map as it was), `playtest.json` (mode, rng offset, spawn,
  nickname, `duration_secs`, `name`), `inputs.bin` (inputs and dynamic vars of every step, state hash after every
  step), optional `speed_ranges.json`. Under 2 s it is dropped.
- A **named** recording (Rename in the Recorded playtests list - a tab next to History; or the `name` field in
  `playtest.json`) is kept forever. Name the ones a video uses.
- In the dev checkout `hypersomnia/user/projects` is a symlink to `content/arenas`, so a user's project folder can
  sit among the official maps (`.recorded_playtests/` is gitignored everywhere).
- Shots needing a human (aiming, a specific fight) - the user plays them. Shots of bots alone - stage them (below)
  and start the playtest yourself with `--edit <project> --playtest` under the local-testing rules (own build,
  `$GAME_TEST_FLAGS`, Xvfb), then quit; the recording appears in the project.
- Do not move, rewrite or inject a map whose project is open in the user's editor (its `editor_view.json` is
  rewritten while open - check its mtime against the game's start time; the editor overwrites on save/autosave).

### Staging a shot (editor nodes and settings)

- **quick_test_spawn** (Quick test mode): you spawn on the topmost one (in the layers); every other one gets a bot
  named after the node that stands still, holding the node's equipment (or the mode's). Faction, custom equipment,
  "Bot respawns" (off by default).
- **bot_test_navigation_end** (Quick test): every quick_test_spawn bot pathfinds once to the nearest free end (ends
  have no faction; bots take them in faction order - Metropolis, Atlantis, Resistance - and within a faction in layer
  order, each the nearest free end in a straight line) and stands there facing the end's direction. "Sprint" (run
  otherwise), "Hide weapons on arrival" (like H). The navmesh is generated whenever the map has such ends. The editor
  draws each bot's path to its end (View > Bot test paths, on by default) - check them before recording. The devlog 6 ending (three bots
  walking to a window to watch the nebula) was staged this way.
- **Playtest spawn** on team spawns (bomb defusal and other arena modes): a faction with any spawns only there;
  "Bot camps here from the start", "Custom equipment" every round.
- Quick test **"Spawn money"** (mode inspector): off = bodies drop no coins; on = coins from kills and torn-off limbs.
- Playtesting **"Enemies behind walls"** (Hidden / Silhouettes / Full) and "Draw enemy crosshairs" in the project's
  playtesting settings.
- **camera_zoom** zones of the map drive the replay camera; **default_zoom** in the map sets the base zoom.

## 2. Rendering a recording - `scripts/capture_replay.sh`

```
RECORDING=<project>/.recorded_playtests/<date> OUTPUT=<file.mp4> [env...] scripts/capture_replay.sh
```

| env | meaning |
|---|---|
| `FROM`, `TO` | recording seconds (default whole) |
| `CAMERA=x,y,zoom` | fixed camera (default: follows the recorded player) |
| `HUD=1` | with HUD (default: cinematic, no HUD) |
| `LASER=1` | weapon laser on (default off - the user's own game has it on; match the original take!) |
| `NO_PLAYER=1` | the recorded player moved out of every frame (backdrops, scenery) |
| `BARS_RAISE=<px>` | lifts the right-bottom money/health bars (0 for footage) |
| `CRF` | x264 quality (14) |
| `BIN` | game binary (default `build/Release-x64-clang-claude/Hypersomnia` of this checkout) |
| `WORK` | the frames fifo, the raw sound, the video before muxing, throwaway appdata, `replay_game.log`, the capture lock |
| `CAMERA_QUERY_MULT`, `BULLETS_INTERPOLATION` | knobs left from the bullet-trail investigation - keep the defaults |

What it does: a private Xvfb display, a throwaway appdata with a conf.d (1920x1080 borderless, `max_fps` 1000, no
tutorial/changelog, laser, bars raise), the frames through a fifo into ffmpeg - H.264 CRF 14, **BT.709 tagged** -
and the OpenAL loopback sound (48 kHz) into a plain file, muxed in as AAC 320k after the game ends. If ffmpeg dies,
the game dies of SIGPIPE and the script stops with an error - nothing hangs. A flock serializes captures (one at a
time on the machine).

- **The binary must come from the same checkout as the content** (it runs in `<repo>/hypersomnia`), and its
  simulation must match the recording's: after a gameplay change (e.g. the limb fly direction fix) old recordings
  **desync** - the game log says "desynced at step N" and the picture after that is not what was played. Build a
  Release with the local-testing `build_claude.sh release` when needed.
- Rendering runs 240 renders per second and keeps every 4th frame (60 fps), with the sound of all renders: at 60
  renders/s bullet trails came out short compared with the user's game; 240 matches it, and slow motion stays smooth.
- Speed: about 17 s of footage in 5 minutes on this machine. Run it in the background and wait on the process.
- The sound is quiet (peaks -20..-40 dB, HRTF on) - normalize when editing; it is in sync with the picture (each
  frame's sound starts at the frame). The ear is always at the recorded player, also with a free or fixed camera, and
  a hidden player (`NO_PLAYER`) still makes its own sounds.
- Get a camera for `CAMERA` from the user: free camera on in the replay window, "Copy moment link" - the line carries
  `camera=x,y,zoom` and `at=<secs>`.

### After every capture - verify

`ffprobe` (1920x1080, 60 fps, bt709, the expected duration), then pull frames at a few times (`ffmpeg -ss T -i ...
-frames:v 1`) and look at them; when re-capturing a shot, diff frames against the previous take (same character
positions, the laser line if the original had it). Never report a take as done without looking at its frames.

## 3. Re-rendering an old recording on a changed map - `scripts/inject_map.sh`

The inputs replay on any map with the same geometry and spawns, so lighting, colors and decorations can change after
the fact: `RECORDING=<rec> BACKUPS=<persistent folder> [MAP=<map.json>] scripts/inject_map.sh` backs the recording
up, then copies the map (default: the project's current json) into `<rec>/map.json`; capture as usual.

- "desynced at step 0" is expected afterwards (the map's hash changed); check the frames instead - the characters
  must stand where they did in the original take.
- Keep each take under its own name (e.g. `<shot>__<what changed>_take_<HH-MM>.mp4`) and let the user compare;
  devlog 6 offered them in a combo of its ending panel.
- Small map fixes for one shot go into the recording's own `map.json` (back it up first): a camera_zoom zone grown
  so the camera stops zooming out when the player leaves its edge (camera zones don't change the state hash), or
  `default_zoom` for a closer shot.

## 4. Slow motion - `speed_ranges.json`

Next to a recording: `{"ranges": [{"from_secs": 11.0, "to_secs": 16.0, "speed": 0.5}]}` (recording seconds, speed
clamped to 0.05-4). Replays and captures play those parts at that speed - the simulation and the sounds (lower pitch)
- and a capture gets 2+ frames per step, so 0.5x stays smooth. Write the file by hand (there is no UI for it); keep
the ranges apart - the first range covering a moment wins.

## 5. The replay window and moment links

- Recorded playtests list: double-click or the play icon replays, right-click renames/reveals/deletes.
- Replay keys: Space pause/resume, Left/Right seek 5 s (Shift 1 s), numpad speed (0 resets), Alt+P hides the window,
  Esc quits. Free camera: drag with RMB, zoom with the wheel. "Hide HUD", "Copy moment link", "Reveal in explorer".
- A moment link: `[hypersomnia replay moment] map=<map> recording="<folder>" at=<secs>s [camera=x,y,zoom]
  | --replay-playtest "<folder>" --replay-from <secs> [--replay-camera ...]`. With such a line in
  the clipboard, switching to the game while it shows the editor's project selector opens that map and replays it
  paused at that moment (only a recording inside `<project>/.recorded_playtests/`; a broken project shows an error).
  When the user pastes one, take `recording`, `at` and `camera` from it.

## 6. Footage recorded by hand (screen recordings)

Some shots the user records on the desktop (Ctrl+Print, files in `~/Videos/gameplay/`):
- The screen recorder draws the Adwaita cursor with holes - devlog 6 repaints it with its `scripts/cursor_fix.py`
  (in the devlog 6 folder - ask the user where it is) into separate fixed copies; originals untouched.
- The first frame can be held ~250 ms - trim the start.
- Ask the user to turn off Redshift/night light before recording - it went into a hand recording as extra gamma.
  Xvfb captures never see it.

## 7. Color - every encode and decode

- Encode: BT.709 matrix and tags (`-colorspace/-color_primaries/-color_trc bt709`, `-x264-params
  colorprim=bt709:transfer=bt709:colormatrix=bt709`), limited range, `flags=accurate_rnd+full_chroma_int+full_chroma_inp`
  on every scale. Untagged/BT.601 encodes showed oversaturated in mpv.
- Decode for compositing: `scale=in_color_matrix=bt709:in_range=tv:flags=accurate_rnd+...`.

## 8. If a recording got deleted

Stop writing to that disk. The data blocks usually survive; the folder's inode was found again in the ext4 journal:
the user dumps the journal with `sudo debugfs -c -R 'dump <8> <file on another disk>' /dev/<partition>` (Claude has no
sudo password - the user runs sudo commands), old copies of the inode table in the journal still hold the deleted
files' extents (inode -> block numbers), and `dd` of those blocks from the partition gives `map.json`, `playtest.json`
and `inputs.bin` back. Verify by capturing the recovered recording and comparing frames with an old render.

## Example pipeline

devlog 6 (ask the user for its folder): `scripts/capture_replay.sh` (the original of this skill's script),
`scripts/ending_footage_new_map.sh` (backup + map injection + capture in one), clip caches named
`<map>_<recording>_from_<s>_to_<s>_hud_raise_<px>_laser_<0|1>_r240.mp4` and reused when they cover the needed range,
and `recordings_backup/` with every source recording.

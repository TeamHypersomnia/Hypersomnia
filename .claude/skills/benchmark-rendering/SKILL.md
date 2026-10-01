---
name: benchmark-rendering
description: Measure the FPS and GPU frame time impact of a rendering pass, FBO, shader or other rendering change in Hypersomnia by A/B toggling it within a single game run. Use whenever the user says "zrób benchmark X", "zrob benchmark X", "zbenchmarkuj X", "benchmark X", or asks how much FPS/GPU time something rendered costs (e.g. "fps impact z i bez X").
---

# Benchmarking a rendering change (A/B in one run)

The fastest reliable method: temporarily add a global toggle for the measured thing,
flip it automatically every few seconds inside ONE game run, and log running averages
of GPU frame time (GL timer query) and FPS for both variants. Then remove the temporary code and rebuild.

One run with alternating phases cancels out scene, thermal and clock differences between separate runs.

## Facts that save time

- `src/view/frame_profiler.h` measures only CPU time and has no long averages - useless for GPU passes.
  The rendering script only enqueues commands; the GPU cost of an FBO clear or fullscreen quad never shows there.
- FPS alone hides GPU costs: on the dev machine (RX 5700, ~1080p) the game is CPU-bound -
  ~1110 FPS with a ~0.5 ms GPU frame. Always report the GPU time, not just FPS.
- **Always pass `/range`** to launch straight into the shooting range, no input needed:
  `../build/current/Hypersomnia /range`. It's a URL-style location argument (`src/cmd_line_params.h`, also `/menu`, `/editor`, `/tutorial`).
  Confirm in the log: `Activity SHOOTING_RANGE was specified from the command line.` and `Launched mode: SHOOTING_RANGE`.
  Without it, the game launches `last_activity` from `hypersomnia/user/runtime_prefs.json` - which is whatever the user
  did last (editor, menu, a server...) - and only if `skip_tutorial` is true, else the tutorial.
  `launch_at_startup: MAIN_MENU` in the config would send it to the menu too.
- A different scene (editor, a map on a server) needs a different measuring setup - ask the user if the range isn't representative.
- **A specific map, with a fixed camera:** open it in the editor with `--edit <project dir>`, e.g.
  `../build/current/Hypersomnia --edit user/projects/de_cyberaqua_bench` (cwd = `hypersomnia/`).
  Confirm `Launched mode: EDITOR` and that no `DTOR finished: ~editor_setup` follows right after loading -
  builds before 2026-10-01 fell through to the project selector with `--edit` (missing `break` in `work.cpp`).
  - Never benchmark on the official `content/arenas/*` or the user's own `user/projects/*`: the editor writes
    `navmesh.nav`, `editor_view.json` and autosaves there. Copy the arena to `user/projects/<name>_bench`,
    rename its json to match the folder, drop `last_saved.json`, and set `meta.name`.
  - The editor camera comes from `editor_view.json` (`panned_camera.transform.pos`, `zoom`) - set it to frame
    what is measured, e.g. zoom 0.45 at the average position of the lights to catch many of them.
- **Never use `--apply-config` for a benchmark variant:** the patch lands in `user/runtime_prefs.json` when the game quits
  (even on `timeout`'s SIGTERM), silently changing the user's settings. Back up `runtime_prefs.json` and diff it after the run,
  or toggle the setting in code with a `/* BENCH TEMP */` flag. The editor also saves its camera to `editor_view.json` on quit -
  reset it before every run so the scene stays the same.
- **CPU-side light work:** the profiler has `light_ray_casts`, `light_aabb_queries`, `lights_recalculated`,
  `lights_reused` and `light_jobs_cpu_us` per frame (`frame_profiler.h`, summed by the light jobs in
  `launch_visibility_jobs.cpp` from the per-thread `thread_physics_query_counters`). Log them in the A/B harness
  instead of the GPU query when the measured thing is CPU work.
  The light jobs run in parallel on the thread pool, so their summed CPU time barely moves the FPS on a many-core machine -
  report the CPU sum, not just FPS.
- Uncapped already: `vsync_mode` is `OFF` in `default_config.json`, and `max_fps` is disabled (`OFF_max_fps` in runtime prefs).
  Verify if the numbers look capped (e.g. exactly 60/144/400 FPS).
- Launch with cwd = `hypersomnia/`: `cd hypersomnia && timeout 76 ../build/current/Hypersomnia /range > <scratchpad>/bench.txt 2>&1`.
  From the repo root the game fails to read its config and litters the root with empty `cache/`, `user/`, `logs/`.
- LOG output goes to stdout, so it lands in the redirected file even when `timeout` kills the game.
- The user allows launching and killing the game by yourself for benchmarks.
- **Check that no other game instance runs before measuring:** `ps -eo pid,lstart,args | grep "[b]uild/.*Hypersomnia"`.
  The user often keeps one open (launched via `ninja run -C build/current`). It skews the absolute CPU numbers and FPS -
  ask them to close it rather than killing it, then rerun. An A/B in one run keeps the ratio roughly right even then.

## Gotchas: FPS caps and vsync

Check these before trusting the numbers. A capped frame rate makes both variants look identical.
Suspicious FPS values are the round ones: 60, 144, 165, 240, 400, or anything exactly flat across phases.

- **In-game vsync:** `window.vsync_mode` (`OFF` / `ON` / `ADAPTIVE`, `src/augs/window_framework/window_settings.h`).
  Default `OFF` in `hypersomnia/default_config.json`, but `hypersomnia/user/runtime_prefs.json` and any file in
  `hypersomnia/user/conf.d/` override it. Grep all three for `vsync_mode`.
- **In-game FPS limit:** `window.max_fps` is a `maybe<int>`. In JSON, `"max_fps": 400` means ENABLED at 400,
  `"OFF_max_fps": 400` means disabled (value kept). Values below 10 are ignored (`work.cpp`, `max_fps.value >= 10`).
  The limiter method `max_fps_method` (`SLEEP` / `SLEEP_ZERO` / `YIELD` / `BUSY`) changes the frame pacing, but not the GPU time.
- **Driver vsync (Mesa, AMD):** the driver can force vsync regardless of the game - `vblank_mode` env var or `~/.drirc`.
  Launch with `vblank_mode=0` to be sure: `vblank_mode=0 timeout 76 ../build/current/Hypersomnia /range ...`.
- **Compositor / hidden window:** a minimized or fully covered window may get throttled by the compositor or the driver.
  The game itself does not throttle unfocused windows. Keep the game window visible during the run.
- **Swap moment:** `performance.swap_window_buffers_when` (`AFTER_HELPING_LOGIC_THREAD` by default) moves where the frame blocks.
  Keep it the same for both variants. Since the toggle lives in one run, this is automatic.
- **Settings changed in the game menu** land in `runtime_prefs.json` on a graceful quit. `timeout` kills with SIGTERM,
  so a benchmark run shouldn't change them - but don't open the settings mid-benchmark.
- **The GPU time is immune to all of the above.** `GL_TIME_ELAPSED` measures the executed commands only,
  so even with a cap it gives the right diff. Only the FPS column gets flattened by caps.

## Steps

1. **Toggle.** Add a global flag next to the measured code, e.g. in the rendering script's header:
   ```cpp
   /* BENCH TEMP */
   inline std::atomic<bool> bench_skip_<thing> = false;
   ```
   Read it once per frame (`const bool bench_skip = bench_skip_<thing>.load();`) and skip ALL of the measured work with it:
   FBO binds and clears, draw calls, and the composite fullscreen quad.
   Mark every temporary edit with `/* BENCH TEMP */`.

2. **Measure.** Paste `bench_snippet.cpp` (next to this file) into `src/work.cpp`, around the
   `renderer_backend.perform(...)` loop in the render thread (search for `rendering_result.clear();`),
   and add `#include "augs/graphics/OpenGL_includes.h"` after the `renderer_backend.h` include.
   Set the flag name in the snippet. It:
   - waits 8 s of warmup, then alternates 3 s phases: even = WITH the thing, odd = WITHOUT,
   - drops the first 0.5 s of each phase (commands are built a frame or two before they run on the GPU),
   - wraps each frame's GL commands in a `GL_TIME_ELAPSED` query (ring of 8, read back 8 frames later),
   - logs cumulative averages after every WITH+WITHOUT cycle as `BENCH cycles=...`.

3. **Build** with the `build` MCP tool.

4. **Run** ~76 s (8 s warmup + ~11 cycles), then grep:
   ```
   cd hypersomnia && (vblank_mode=0 timeout 76 ../build/current/Hypersomnia /range > <scratchpad>/bench.txt 2>&1); grep -E "BENCH|GL error|Launched mode" <scratchpad>/bench.txt
   ```
   Wait until the diff stabilizes over the last few cycles. The first cycle is noisy.

5. **Revert** only the temporary edits: `git restore` the touched files, but only after checking with `git diff`
   that they contain no other uncommitted changes of the user. Otherwise remove the `/* BENCH TEMP */` hunks by hand.

6. **Rebuild.** The binary still contains the toggling code until you do. Don't skip this.

7. **Report** a table: GPU frame time and FPS with/without, the diff in ms and as % of the GPU frame,
   sample counts, resolution (window size from `runtime_prefs.json`) and GPU (`lspci | grep -i vga`).
   Scale the estimate for other resolutions by the pixel count, and state clearly that other hardware wasn't measured.
   Mention the memory cost of any removed FBO: width × height × 4 B for RGBA8 (+ stencil if `WITH_STENCIL`).

## Reference results

The light shadows cache on de_cyberaqua in the editor (Oct 2026, RX 5700, 25 lights in view, all with heights, zoom 0.45):
light jobs 3.06 → 0.50 ms of CPU per frame summed over threads (6x less), AABB queries 1319 → 25, 0 ray casts either way
(lights with a height skip the visibility polygon and use only AABB queries), 407 → 467 FPS.
With point light heights off (the visibility polygon path, 19 lights at another camera): 2471 → ~1 ray casts per frame,
3.96 → 0.18 ms of CPU, 409 → 517 FPS. GPU time +0.07 ms with the cache although it draws the same
geometry - likely GPU clocks at the higher FPS, not a cost.

The sun shadow guard band (384 px past two screen edges) on the same scene: +0.006-0.010 ms GPU on a 1.9 ms frame (~0.5%),
FPS unchanged, +5 MB of VRAM at 1080p.

The shell smoke `overlay_smoke` FBO pass with no shells on screen (Sep 2026, RX 5700, 1918×1060):
0.518 vs 0.498 ms GPU per frame (+0.02 ms, ~4%), no FPS difference (~1110 both), 8 MB of VRAM.

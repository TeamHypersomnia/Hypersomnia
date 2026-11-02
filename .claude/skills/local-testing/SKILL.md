---
name: local-testing
description: Test Hypersomnia end to end without the user, mainly networking - local masterserver, dedicated servers and CLI clients (UDP and WebRTC), server keys, certs and the identity broker, the web build in Firefox under Xvfb, game UI via screenshots and XTest clicks, the hypersomnia.io site in a scratch copy, isolated builds (native, Steam, web, Ubuntu containers for the remote hosts) and test servers on arena and arenaus. Use whenever the user asks to test something "lokalnie", "headless", "z CLI", "bez mojej ingerencji", "postaw dedyka i połącz się", to check a UI change on screen, or to verify networking, ranked/auth, WebRTC or web build changes before a release. Also for gameplay logic (dropping, picking up, inventory, buying, bots, physics) - "przetestuj", "stestuj sobie", "sprawdź czy działa w grze".
---

# Local testing

**After a change to the network architecture (netcode, connecting, identity and accounts, the server's auth, the
masterserver, the site's match reports and broker): run `scripts/regression.sh`** (about 13 minutes, `scenario_slot_reuse.sh` included; not for gameplay,
UI or rendering changes) - every
check below in a fresh scratch directory and a PASS/FAIL table at the end; `SOAK=1` adds the netcode soak (~35 min),
`REMOTE=1` the remote part (`remote_e2e/regression_remote.sh`: the MMR rules on the remote test site, one real ranked match
through real NAT, a web client over TURN only), `ONLY="mmr keys"` runs some checks only (the scratch site starts
for any of them - it needs the site's repo and its ports free). Every check has its own timeout: never wrap
regression.sh in a shorter outer one - an outer timeout or Ctrl+C stops it only after the running check, whose
scenario runs on until then. Such a fix is tested with it before it is
committed; a new kind of such change gets its check added there. The site's MMR rules are tested by signed simulated reports
(`run_mmr_tests.py` in the site), real ranked matches only for one or two end-to-end cases.

Everything runs from scripts in `scripts/`, each isolated: own appdata dirs, own scratch dir, and they check their ports
are free (the scenarios share 18000/18410/18420/18430/10002 - run one at a time)
(`SCRATCH`, default `/tmp/hypersomnia-nettest` - point it at the session scratchpad).
Build into your own build folder first (`scripts/build_claude.sh`, rule 2) and pass it as `BIN` (default
`build/Debug-x64-clang-claude/Hypersomnia` of the checkout the script lies in; the tournament and favorite scripts take the
headless server as `BIN` and the client as `CLIENT_BIN`). **The binary must come from the same checkout as the scripts** -
they run it in that checkout's `hypersomnia/` (its content): another worktree's newer build fails at start
(`ensure_eq(id.indirection_index, ...)` in `test_scene_images.cpp` - images it expects that aren't there).
A new script sources `scripts/lib.sh` (`wait_until`, `wait_listening`, `cleanup_on_exit <its dir>` - nothing left running
after Ctrl+C or regression.sh's timeout) and waits for log lines, never a fixed sleep.

## Isolation rules - read first, all of them were learned the hard way

1. **Heartbeats go to the production masterserver by default.** `notified_server_list` lives in the `"server"` section;
   a top-level key is silently ignored. Every test server needs
   `"server": { "notified_server_list": "127.0.0.1:18430", "suppress_new_community_server_webhook": true }`.
   Check `Sending heartbeat through UDP to` in the server log before anything long. A miss lists the test server
   publicly and posts "new server" notifications to the community channels. A server hosted from the client
   (`/host`, Host a server in a UI test) uses the client's config - give it a fresh `--appdata-dir` with that conf.d.
   The generator's scripts tested for real (Docker, a VPS) are made to list the server, so conf.d doesn't apply: block
   the master on that machine first (iptables DROP to 57.128.242.21 for UDP and TCP other than 80/443 - the same IP
   serves the downloads - in OUTPUT, FORWARD and DOCKER-USER).
2. **Never use the `build` MCP or `cmake/build.sh`.** Both work on `build/current`, which the user's builds use;
   `build.sh` also re-points the symlink. Build with **`scripts/build_claude.sh debug|headless|release|steam [worktree]`**:
   it configures `build/<Config>-x64-clang-claude[-suffix]` of that worktree the first time (a fresh worktree's empty
   submodules copied from the main checkout), waits for the machine's build lock - one build at a time, any other `ninja`
   running counts too (a second one runs the machine out of RAM; `MAX_BUILDS=2` only for small ones) - and builds with
   `nice -n 19 ninja -j10`, through ccache when it is installed (one cache for every worktree: a fresh worktree builds in
   minutes; generated sources go into the build dir, so builds in several worktrees never clash). The web and the Ubuntu
   container builds take the same lock. Two agents never build in one folder, nor edit one checkout at once - each its own worktree
   (`~/Hypersomnia-<name>`) with its own `build/`.
   **While implementing and testing, build and test only `build/Debug-x64-clang-claude`** (the fast Debug native one,
   no debug info - like the user's `cmkdf`). The Steam, headless, web and CrazyGames variants only when the change touches
   their code (`#if` paths, headless-only sources, web or Steam APIs) - and even then only once, literally at the end of
   the batch, after the user has tested; never in the middle of iterating. Rebuilding all five for every small fix made
   the iterations too slow. **A commit needs only the Linux build** - a variant that stops compiling is fixed later, at
   the very end; build the other variants before a commit only for a big architectural change across platforms.
   Exception: a change to headless-only code (tournaments, crash recovery) is tested with `-headless` while iterating.
3. **Never `pkill -f <pattern>` (or `pgrep -f`) in a command whose own text contains the pattern** - it kills your own shell
   (exit 144). Put the kill in a script file and run the script, or kill by exact PID. A script written by a heredoc in the
   same Bash call counts too (the calling shell's command line holds the heredoc): write it in one call, run it in another.
4. **Port 3000 is the user's local hypersomnia.io** (the site repo, `node app.js`, its `private/mmr.db`). Never post to it.
5. **Check ports are free first** (`ss -tulpn`, `lsof -ti tcp:<port>`) and abort if busy - a server that fails to bind
   silently leaves the requests going to whatever else listens there.
6. **Silence, nothing on the user's screen.** Every script sources `scripts/test_env.sh` (the one place for it:
   `ALSOFT_DRIVERS=null`, `VISUAL=true`, `DBUS_SESSION_BUS_ADDRESS=disabled:` - without it the game's file dialogs
   and "show in folder" go through the user's desktop portal and open on the user's screen; the fake zenity/xdg-open
   in `identity_ui/fake_bin` take over instead - and `$GAME_TEST_FLAGS` = `--no-update-on-launch --mute --no-open-logs`)
   and passes `$GAME_TEST_FLAGS` to every game it starts - a new script does the same, a game started by hand too
   (`. .claude/skills/local-testing/scripts/test_env.sh` first). Firefox profiles get `user_pref("media.volume_scale", "0.0");`.
   No window on the user's screen either: every game started for a test - `--unit-tests-only` too - under `xvfb-run -d`
   (not `-a`: parallel starts share one Xvfb and the first to exit takes it down) or a private `Xvfb :NN` (the one exception: benchmark-rendering, which measures the real GPU).
7. On remote hosts, production runs as `Hypersomnia-Headless.AppImage --as-service` (masterserver, official server) - find it with `ps`.
   Work only in `~/yojimbo-test`, restart only by your own PIDs, and verify afterwards that the production processes still
   have their old uptime (`ps -eo etime,args | grep Hypersomnia-Headless`).
8. **Short timeouts, look every minute.** Every test command gets an aggressive `timeout` (seconds, not minutes); a
   background shell is checked about every minute, builds excepted. Never pipe a script that backgrounds a server into
   `tail`/`head` - the server keeps the pipe open and it hangs; redirect to a file.

## Scenarios (scripts/)

| Script | What it proves |
|---|---|
| `broker.sh start\|stop\|sql` (`SITE`: the site's repo, default `~/page`) | A scratch copy of the site on 13500 as the identity broker and report endpoint, with its own broker key; prints `BROKER_URL` and `BROKER_PUBLIC_KEY` to export for the scenarios |
| `scenario_keys.sh` | Master with `official_servers.d` (servers official by their `--print-server-key`), official casual (de) + ranked (pl) + community servers, 3 CLI clients: signed heartbeats, every client self-signed (unlinked keys never ask the broker) and taken by ranked too, `.keys/account.key` 0600. `COMMUNITY_PASSWORD`, `KEEP_RUNNING`, `*_SECS` |
| `scenario_webrtc_ranked.sh` | Native client over WebRTC (`--connect rtc:ranked-pl:1` - a ranked instance's alias has its number) through the local signalling server to the official ranked server by its alias, with a self-signed cert (an unlinked key) |
| `scenario_web_firefox.sh [connect] [wait]` | The real web build in Firefox under Xvfb joining `/game/<connect>` (default `pl`) over WebRTC with a cert from the local broker; screenshot in `$SCRATCH/web_test/screen.png` |
| `scenario_outage.sh` | No masterserver and no broker, joining by address: a self-signed cert plays casual and ranked (every valid cert is taken; a broker's cert without a master: `scenario_ranked_corners.sh`) |
| `scenario_crazygames.sh` | The CrazyGames build in Chromium with a stubbed SDK (`crazygames_sdk_stub.py`, token signed by `CG_KEY`) joins an official ranked server over WebRTC as `crazygames_...` |
| `scenario_steam.sh` | The Steam build with the user's running Steam: a real Web API ticket -> broker cert -> ranked (broker started with `STEAM_API_KEY_FILE`) |
| `scenario_ranked_report.sh` | A whole ranked match to its natural end in about a minute: the test map `de_e2e_short` (de_cyberaqua, max team score 2, 5 s rounds, no warmup) on a ranked server, two key clients clicking Auto-assign, the signed report at the local broker, the site's match and MMR rows |
| `regression.sh` (`ONLY=...`, `SOAK=1`, `REMOTE=1`) | The checks of the network architecture, a PASS/FAIL table: unit tests, netcode's own tests, the site's cert vectors / MMR / link flow, password limit (per address), keys, admin, outage (no master, no broker), WebRTC ranked and a crowd of 6, the handshake limit, a whole ranked report, slot reuse, the ranked corners |
| `build_claude.sh <config> [worktree]` | An agent's own build (rule 2): `debug`, `headless`, `release`, `steam`; `JOBS`, `CMAKE_ARGS`, `TARGET` |
| `scenario_slot_reuse.sh` (`CASES=...`) | Servers never quietly stop taking players, with real games on a server of 1 slot: a new player after each SIGTERM / SIGKILL / SIGSTOP gets in and the slot is free within its time; the full server refuses with "The server is full"; the same key killed and started again at once 5 times always gets back in, then a new one too; UDP and WebRTC in turns; the master restarted under a running server; a new player after a whole ranked match. PASS/FAIL table, ~5 min. Run it after any change to connecting, slots, kicks or timeouts |
| `scenario_tournament.sh` (`CASES=...`) | Tournaments end to end, a PASS/FAIL table: fixed teams with a bye, an open first round (auto close, leaving before the start, 2v2), crashes (SIGKILL) mid-match, in a freeze after someone left, between rounds and in an open round - resumed by the same flags, nothing replayed or lost; the live results page's JSON on the way; the page in the masterserver's list. Run it after any tournament change (~50 min alone; the cases are listed at its top). `SHARD=0..3` runs a copy on its own ports and displays; `SHARDS=2` with `SHARD=0` and `SHARD=1` splits the cases between two at once (~33 min) - 3 at once overload the machine (a few cases then fail on timing - rerun those alone). Ctrl+C or a kill takes down everything the run started. It runs a copy of itself - editing it meanwhile is safe. Clients run small (480x270, `LP_NUM_THREADS=2`) - four full-size ones starve each other into timeouts |
| `check_environment_variables.sh` (`PAGE=<site repo>`) | The `HYPERSOMNIA_*` names in the game (`environment_variables.{h,cpp}`), `README_SERVER.md`'s table and the config generator are the same - after adding or renaming one |
| `scenario_ranked_corners.sh` (`CASE=unlink\|broker_down`) | A linked account (a key linked to a fake Steam id straight in the site's DB, its broker behind a stoppable proxy) in a ranked match: mid-match its key is unlinked on the site, or its broker goes down; it is killed and started again - it must rejoin as the same `steam_` account (its saved steam_cert.json) and the report must name that account. Note: a key put in by hand is linked only after the game's link check - started with `--connect` right away, it joins as `key_`; the script lets alice save her steam_cert.json first |
| `scenario_disconnects.sh` (`CASES=...`) | Filmstrips of each disconnect as the player sees it, with timings: server SIGTERM/SIGKILL/SIGSTOP, full, the same key joining again, wrong password, AFK, client SIGSTOP/SIGKILL/SIGTERM. Expected: every kick reason shows at once; a dead server or a dropped network shows "Nothing came from the server for 10 seconds" (native only); a 6 s server freeze is survived; a killed client frees its slot in about 5 s, a quitting one at once |
| `scenario_map_download.sh <out>` (`MAP`, `EXT`, `LATENCY`, `LOSS`, `KILL_AT`) | A client without the server's map downloads it - over UDP or from an HTTP provider (`slow_http.py` serves one at a set rate, with CORS for the browser); a long download never disconnects, a server killed meanwhile shows "Nothing came from the server for 10 seconds". In the browser: `scenario_web_firefox.sh` with `MAP`, `EXT` (and `KILL_SERVER=1`, `TAB_AWAY=<s>`) |
| `scenario_lan_favorite.sh` (`P`, `DISP`) | A LAN party whose router blocks broadcast: a favorite on the local network is asked directly too and brings all its host's instances into Local - a tournament (2 matches) and a casual server (2 instances) on ports broadcast never reaches, one favorite each; checks the client log and screenshots Browse servers (clicked at 190,497 in a 1280x720 window, after Escape from the tutorial) |
| `scenario_manual_favorite_listed.sh` (`DISP`, `KEEP_RUNNING`) | A favorite added by hand (key + address) of a server also on the local master's list: one row, the list's, with the typed name and a lit star, Unlisted empty (no ping of its own); `KEEP_RUNNING=1` leaves it up to click the star (Forget? asks, the row stays with the list's name) |
| `scenario_browser_tournaments.sh` (`KEEP_RUNNING`, `FAVORITES`, `stop`) | The browser's tournament rows: a local master with an official tournament (2 matches), a community one (open, 4 matches) and a casual server; Browse servers screenshotted (imgui_big.ini), the list checked (the casual server on it too). `KEEP_RUNNING=1` for clicking: a tournament row expands/collapses, its star saves the first instance (key + `t1`), the filter shows a match under its tournament |
| `scenario_demo.sh` | A client records a demo while it plays on a local server, another start replays it (`demo://<path>`) - the replay must load and run without errors |
| `scenario_webrtc_crowd.sh [N=10]` | N native clients over WebRTC from one address join one community server at the same moment - all must get in |
| `connect_filmstrip.sh <connect> <appdata> <out.png> [secs] [bin]` | A contact sheet of a client joining, a screenshot every half second - everything the game draws while connecting |
| `scenario_menu_mmr.sh <shot.png>` (`PLAYER`, `BIN`) | After `scenario_ranked_report.sh`: the main menu's leaderboards header shows the player's place and MMR from the local site |
| `scenario_admin.sh` | A key linked to a Steam account on a server with a password: with only the `steam_` account in `admin_profiles` it is asked for the password (outside the Steam build a linked key is no admin), with its `key_` too it gets in without one |
| `scenario_password_limit.sh` | 7 joins without the password: 5x "Wrong password", then "Too many wrong passwords" |
| `link_flow_test.py`, `crazygames_cert_test.py` | The site's link/unlink/start endpoints and the broker's CrazyGames token checks, at the HTTP level |
| netcode's own tests (handshake, replays, forged challenges) | `ONLY=netcode scripts/regression.sh` - built from yojimbo's netcode.c with `NETCODE_ENABLE_TESTS`; a failed check aborts (exit 132) after naming it |
| `pw_driver.js`, `fake_steam_session.py` | Chromium (Playwright in `~/.cache/hs-playwright`) driven over HTTP for clicking the web build; a Steam session cookie for the local site |
| `[VARIANT=crazygames] build_web.sh` | Web Release build (`-DUSE_O3=1`) in `build/Release-Web-clang-claude-web[-crazygames]`, the same steps as `build.sh Release Web` |
| `[USER_FILES=dir] prepare_web.sh <web build> <out>` | Copies a web build and points its packaged config at the local master and the broker on 13500 (see web.md); USER_FILES also writes files into its `/user` before start |
| `webserve.py <dir> <port>` | Serves a web build with COOP/COEP; any path serves the game, so `/game/<id>` joins right away |
| `steam_tickets.py IDENTITY...` | Steam Web API tickets from the user's running Steam client, to probe `AuthenticateUserTicket` without the game |
| `xinput.py click x y` / `rclick x y` / `drag x1 y1 x2 y2 [x3 y3 ...]` (rests 1.5 s at the points in between) / `type TEXT` / `key NAME` / `scroll x y n` (0 notches = only move the mouse) | Clicks, drags (e.g. a window edge), types, presses keys and scrolls on `$DISPLAY` through XTest (no xdotool needed) |
| `focus_bounce.py <window name> [secs]` | Takes the X focus from the game's window and gives it back (alt-tab to a browser and back - Xvfb has no window manager); `xclip -selection CLIPBOARD -in` sets the clipboard |
| `local_stack.sh [web build] [hours]` | Leaves running, for the user to test by hand: master, official ranked [PL] and casual [DE] servers, the web build on http://localhost:8765 pointed at them; `local_stack.sh stop` |
| `build_headless_ubuntu2204.sh` | Headless Release build inside an unprivileged Ubuntu 22.04 rootfs (bwrap) - glibc 2.35, runs on both arenaus and arena; prints where the libc++ to ship lies |

Pass/fail is read from the logs - the scripts grep the meaningful lines: `Received an identity cert for X`,
`Authenticated "nick". ID: steam_...|key_...`, `Received initial public vars`.

## UI checks

Run the game under `Xvfb :77 -screen 0 1280x720x24` with a 1280x720 non-fullscreen window (conf.d `"window"`),
`import -window root shot.png` to screenshot, read the image, and `python3 xinput.py click x y` to click.
A fresh appdata starts in the tutorial: `xinput.py key Escape` opens the menu (Settings at ~156,555).
The appdata remembers the last connect string - the first screen may be a reconnect dialog with Abort.
A failure log is saved in `<appdata>/logs/` (`exit_failure_debug_log.txt`, `ensure_failed_debug_log.txt`) - read it yourself.
The account and identity UI (Link with Steam, Restore, Backup, RCON levels): **`identity_ui.md`**.

## Gameplay checks - measure in the log, not in screenshots

Gameplay logic (where items fly, what gets dropped or picked up, what bots buy) is checked faster and more reliably by numbers
than by driving the game by hand:

1. Add a temporary `LOG("XYZDBG ...")` where the decision is made (e.g. the drop direction vs the aim direction in
   `perform_transfer.cpp`), build.
2. Run the map with bots: `MAP=<map> SECS=200 LOG_GREP=XYZDBG scripts/bots_match.sh` - a thin wrapper of the
   **run-map** skill's `run_map.sh` (any map, edited copies via `map_copy.py`, and the pitfalls of loading maps).
   Bots play only while a human is connected; the script adds a spectator client. Within a minute bots buy, fight, die and drop their items.
3. Parse the `XYZDBG` lines with a short python script (angles, counts, orders), then remove the logs and rebuild.

Actions only a player does (E on an item, H, G) - the editor playtest of a scratch map copy under Xvfb, see the
run-map skill.

## Soak: servers must never quietly stop taking players (scripts/soak)

The worst bug class: after N connections a table fills up and the server silently refuses everyone.
With real games: `scenario_slot_reuse.sh` (in the table above). At the netcode level:
`soak/scenario_handshake_limit.sh` (after `build_soak.sh`): the per-address handshake limit is hit from the LAN address and the same address gets in again a moment later.
`build_soak.sh` builds a netcode-level client from the game's own netcode; then:

- `soak_netcode.sh [scale]` - ~9000 connect/disconnect cycles, half abandoned mid-handshake, by address and with the server's key,
  each with a new session key; 0 failures expected, a wrong key must fail, then a real client must join.
- `soak_webrtc.sh [cycles]` - native clients over WebRTC killed at random moments, more than the 64 WebRTC ids;
  then a full join must work. A native community server's WebRTC id is its address (`browser_connect_string`).

## More - read the file when the test needs it

- **`web.md`** - the web build headless: its config replaced in place, Firefox and Chromium (pw_driver.js), the web account
  key and Link with Steam on the web, and its pitfalls.
- **`identity_ui.md`** - the account and identity UI by clicking: the helpers, coordinates, RCON levels, what each state looks like.
- **`remote.md`** - arena and arenaus, the remote end to end (official server, ranked, accounts, joins by key, the server
  browser and invites), Steam and CrazyGames identities, the hypersomnia.io site's scratch copy, and their pitfalls.

## Findings worth knowing

- A server list check right after a start can be empty: the first heartbeat goes ~10 s after start - poll the list (up to
  20 s), never one fixed sleep. After a server's stop its entry is gone at once (a goodbye).
- Scripts that `cd $REPO/hypersomnia` must not use `$(dirname "$0")` afterwards - resolve `HERE` to an absolute path first.
- A key added to `official_servers.d` is picked up at the master's next rescan (~40 s) - until then the list shows the server
  as community. The server itself never knows; a ranked match of a key the site does not know as official is refused at the report.
- **A ranked match that ends by itself:** tournaments force overtime (win by two) and idle players only win their defending rounds
  (time runs out, no bomb), so a tournament match of idle clients never ends - use a regular ranked server, where 1:1 after both
  halves ends it, and click Auto-assign (585,430 in 1280x720) in the faction window. A map's rules (max_team_score - at least 2 -,
  round_time, warmup_time, freeze_time, buy_time, round_end_time) are in its JSON under game_modes.bomb_defusal; a server takes
  test maps from user/downloads/maps, clients download them from it directly when hypersomnia.io has no such map.
- **A reboot clears /tmp** - the session scratchpad, test keys and the broker copy are gone; anything worth keeping goes into
  this skill's scripts, and a worktree's work is committed before a long run. The Ubuntu 22.04 rootfs defaults to `~/.cache/hypersomnia-rootfs22` (`ROOTFS22=`).
- **Soak RSS:** a server's RSS jumps ~25 MB with the first ~2000 connections and then stays flat (+0.1 MB per 2000) - per-client
  buffers touched once, not a leak. To tell, measure after several batches against one server, not just before/after one run,
  and never with a build running at the same time.
- Release builds of the netcode tests: run `stdbuf -o0 ./bin/test` - without it a failed `check` shows the last *buffered* test name,
  not the failing one.
- The headless-only sources (`HEADLESS`: tournament, crash recovery) are not in the client build: after removing a var, also
  `build_claude.sh headless`.
- Symptom `ignored packet. sequence 0000000000000000 already received (replay protection)` and a client that connects
  but never gets `Received initial public vars`: packets sent outside netcode's normal path without advancing the sequence.
- netcode refuses to create a client or server with `override_send_and_receive` but without both overrides
  (`error: override_send_and_receive requires both ...`) - the web build connects nowhere.
- Over WebRTC every peer is 127.255.255.255 inside netcode, with its id as the port; the per-address handshake limit and bans
  use the data channel's real address instead.
- Upstream yojimbo tests: `cmake -S src/3rdparty/yojimbo -B $SCRATCH/yjbuild && cmake --build ... && ./bin/test`
  (Unix Makefiles - the Ninja generator clashes on a target named `test`).

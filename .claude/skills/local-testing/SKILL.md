---
name: local-testing
description: Test Hypersomnia end to end without the user, mainly networking - local masterserver, dedicated servers and CLI clients (UDP and WebRTC), server keys, certs and the identity broker, the web build in Firefox under Xvfb, game UI via screenshots and XTest clicks, the hypersomnia.io site in a scratch copy, isolated builds (native, Steam, web, Ubuntu containers for the remote hosts) and test servers on arena and arenaus. Use whenever the user asks to test something "lokalnie", "headless", "z CLI", "bez mojej ingerencji", "postaw dedyka i połącz się", to check a UI change on screen, or to verify networking, ranked/auth, WebRTC or web build changes before a release.
---

# Local testing

Everything runs from scripts in `scripts/`, each isolated: own appdata dirs, own scratch dir, and they check their ports
are free (the scenarios share 18000/18410/18420/18430/10002 - run one at a time)
(`SCRATCH`, default `/tmp/hypersomnia-nettest` - point it at the session scratchpad).
Build into your own build folder first and pass it as `BIN` (default `build/Debug-x64-clang-claude/Hypersomnia`).

## Isolation rules - read first, all of them were learned the hard way

1. **Heartbeats go to the production masterserver by default.** `notified_server_list` lives in the `"server"` section;
   a top-level key is silently ignored. Every test server needs
   `"server": { "notified_server_list": "127.0.0.1:18430", "suppress_new_community_server_webhook": true }`.
   Check `Sending heartbeat through UDP to` in the server log before anything long. A miss lists the test server
   publicly and posts "new server" notifications to the community channels.
2. **Never use the `build` MCP or `cmake/build.sh`.** Both work on `build/current`, which the user's builds use;
   `build.sh` also re-points the symlink. Configure with plain `cmake` into `build/<Config>-x64-clang-claude[-suffix]`
   and build there with `nice -n 19 ninja -j10`. Generated sources go into the build dir, so parallel builds don't clash.
   **While implementing and testing, build and test only `build/Debug-x64-clang-claude`** (the fast Debug native one,
   no debug info - like the user's `cmkdf`). The Steam, headless, web and CrazyGames variants only when the change touches
   their code (`#if` paths, headless-only sources, web or Steam APIs), or before a release / to check a Release build -
   rebuilding all five for every small fix made the iterations too slow.
3. **Never `pkill -f <pattern>` (or `pgrep -f`) in a command whose own text contains the pattern** - it kills your own shell
   (exit 144). Put the kill in a script file and run the script, or kill by exact PID.
4. **Port 3000 is the user's local hypersomnia.io** (the site repo, `node app.js`, its `private/mmr.db`). Never post to it.
5. **Check ports are free first** (`ss -tulpn`, `lsof -ti tcp:<port>`) and abort if busy - a server that fails to bind
   silently leaves the requests going to whatever else listens there.
6. **Silence.** Test clients play sound on the user's speakers. Run native clients with `ALSOFT_DRIVERS=null`
   (OpenAL's null backend, "No Output"; the scenario scripts export it) and give Firefox profiles
   `user_pref("media.volume_scale", "0.0");`.
   Also `VISUAL=true`: a game that exits with a failure opens its failure log with `$VISUAL` - a tab in the user's editor.
7. On remote hosts, production runs as `Hypersomnia-Headless.AppImage --as-service` (masterserver, official server) - find it with `ps`.
   Work only in `~/yojimbo-test`, restart only by your own PIDs, and verify afterwards that the production processes still
   have their old uptime (`ps -eo etime,args | grep Hypersomnia-Headless`).

## Scenarios (scripts/)

| Script | What it proves |
|---|---|
| `broker.sh start\|stop\|sql` | A scratch copy of the site on 13500 as the identity broker and report endpoint, with its own broker key; prints `BROKER_URL` and `BROKER_PUBLIC_KEY` to export for the scenarios |
| `scenario_keys.sh` | Master with `official_servers.d` (servers official by their `--print-server-key`), official casual (de) + ranked (pl) + community servers, 3 CLI clients: signed heartbeats, every client self-signed (unlinked keys never ask the broker) and taken by ranked too, `.keys/account.key` 0600. `COMMUNITY_PASSWORD`, `KEEP_RUNNING`, `*_SECS` |
| `scenario_webrtc_ranked.sh` | Native client over WebRTC (`--connect rtc:ranked-pl:1` - a ranked instance's alias has its number) through the local signalling server to the official ranked server by its alias, with a broker cert |
| `scenario_web_firefox.sh [connect] [wait]` | The real web build in Firefox under Xvfb joining `/game/<connect>` (default `pl`) over WebRTC with a cert from the local broker; screenshot in `$SCRATCH/web_test/screen.png` |
| `scenario_outage.sh` | No masterserver, broker down vs up, joining by address: self-signed plays casual and is told ranked needs an account; a broker cert plays both |
| `scenario_crazygames.sh` | The CrazyGames build in Chromium with a stubbed SDK (`crazygames_sdk_stub.py`, token signed by `CG_KEY`) joins an official ranked server over WebRTC as `crazygames_...` |
| `scenario_steam.sh` | The Steam build with the user's running Steam: a real Web API ticket -> broker cert -> ranked (broker started with `STEAM_API_KEY_FILE`) |
| `scenario_ranked_report.sh` | A whole ranked match to its natural end in about a minute: the test map `de_e2e_short` (de_cyberaqua, max team score 2, 5 s rounds, no warmup) on a ranked server, two key clients clicking Auto-assign, the signed report at the local broker, the site's match and MMR rows |
| `scenario_ranked_corners.sh` (`CASE=unlink\|broker_down`) | A linked account (a key linked to a fake Steam id straight in the site's DB, its broker behind a stoppable proxy) in a ranked match: mid-match its key is unlinked on the site, or its broker goes down; it is killed and started again - it must rejoin as the same `steam_` account (its saved steam.cert) and the report must name that account. Note: a key put in by hand is linked only after the game's link check - started with `--connect` right away, it joins as `key_`; the script lets alice save her steam.cert first |
| `scenario_disconnects.sh` (`CASES=...`) | Filmstrips of each disconnect as the player sees it, with timings: server SIGTERM/SIGKILL/SIGSTOP, full, the same key joining again, wrong password, AFK, client SIGSTOP/SIGKILL/SIGTERM. Expected: every kick reason shows at once; a dead server or a dropped network shows "Nothing came from the server for 10 seconds" (native only); a 6 s server freeze is survived; a killed client frees its slot in about 5 s, a quitting one at once |
| `scenario_password_limit.sh` | 7 joins without the password: 5x "Wrong password", then "Too many wrong passwords" |
| `link_flow_test.py`, `crazygames_cert_test.py` | The site's link/unlink/start endpoints and the broker's CrazyGames token checks, at the HTTP level |
| netcode's own tests (handshake, replays, forged challenges) | Not in any build target: `printf 'void netcode_test(void);int netcode_init(void);void netcode_term(void);int main(void){netcode_init();netcode_test();netcode_term();return 0;}' > main.c` in a scratch dir, then `clang -O1 -DNETCODE_ENABLE_TESTS=1 -DNETCODE_DEBUG=1 -I$Y/netcode -I$Y/include main.c $Y/netcode/netcode.c -lsodium -lm -o nctest` (`Y=src/3rdparty/yojimbo`) and `stdbuf -oL ./nctest` - a failed check aborts (exit 132) after naming it |
| `pw_driver.js`, `fake_steam_session.py` | Chromium (Playwright in `~/.cache/hs-playwright`) driven over HTTP for clicking the web build; a Steam session cookie for the local site |
| `[VARIANT=crazygames] build_web.sh` | Web Release build (`-DUSE_O3=1`) in `build/Release-Web-clang-claude-web[-crazygames]`, the same steps as `build.sh Release Web` |
| `[USER_FILES=dir] prepare_web.sh <web build> <out>` | Copies a web build and points its packaged config at the local master and the broker on 13500 (see below); USER_FILES also writes files into its `/user` before start |
| `webserve.py <dir> <port>` | Serves a web build with COOP/COEP; any path serves the game, so `/game/<id>` joins right away |
| `steam_tickets.py IDENTITY...` | Steam Web API tickets from the user's running Steam client, to probe `AuthenticateUserTicket` without the game |
| `xinput.py click x y` / `type TEXT` / `key NAME` / `scroll x y n` | Clicks, types, presses keys and scrolls on `$DISPLAY` through XTest (no xdotool needed) |
| `focus_bounce.py <window name> [secs]` | Takes the X focus from the game's window and gives it back (alt-tab to a browser and back - Xvfb has no window manager); `xclip -selection CLIPBOARD -in` sets the clipboard |
| `local_stack.sh [web build] [hours]` | Leaves running, for the user to test by hand: master, official ranked [PL] and casual [DE] servers, the web build on http://localhost:8765 pointed at them; `local_stack.sh stop` |
| `build_headless_ubuntu2204.sh` | Headless Release build inside an unprivileged Ubuntu 22.04 rootfs (bwrap) - glibc 2.35, runs on both arenaus and arena; prints where the libc++ to ship lies |
| `build_headless_ubuntu_container.sh` | The same in Ubuntu 24.04 (glibc 2.39) - arena only |

Pass/fail is read from the logs - the scripts grep the meaningful lines: `Received an identity cert for X`,
`Authenticated "nick". ID: steam_...|key_...`, `Ranked matches need`, `Received initial public vars`.

## The web build headless - how it works

- **Config without rebuilding:** the web build packages `default_config.json` into `Hypersomnia.data` uncompressed.
  `prepare_web.sh` replaces the provider lines in place with text of the same length padded with spaces,
  so the file packager offsets stay valid. (In the game UI the same can be set in Settings - Advanced - Network.)
- **Joining from the URL:** the page passes `window.location.pathname` as a CLI argument; `/game/<id>` connects to that WebRTC id.
- **Firefox:** `firefox --no-remote --profile <fresh dir> --new-instance URL` under `Xvfb`, with `user.js` setting
  `devtools.console.stdout.content=true` - the game's console output then lands in Firefox's stdout.
  WebGL works through llvmpipe. Allow ~60 s for the 100 MB data to load.
- The web account is a key in its own IDBFS database (`/hypersomnia_keys/account.key`, apart from `/user`), made on the first start. Link with Steam (Settings - Account)
  opens `<website>/link_key/start?state=..&key=..&sig=..&device=..&os=..` (the state signed with the key) in a new tab (native:
  in the browser); signed in through Steam anew (every link), the player confirms the Steam account on that page and the key is
  linked at once - the game asks for its cert every 3 s and sees the Steam account there (no redirect back, no local listener,
  no code; the same natively, in a tab or an iframe). A refusal (another account's key, 32 keys) shows on the page only; the
  game stops waiting after 15 min. Unlink from Steam is one signed GET `/link_key/unlink?t&key&sig`, answered at once.

## UI checks

Run the game under `Xvfb :77 -screen 0 1280x720x24` with a 1280x720 non-fullscreen window (conf.d `"window"`),
`import -window root shot.png` to screenshot, read the image, and `python3 xinput.py click x y` to click.
A fresh appdata starts in the tutorial: `xinput.py key Escape` opens the menu (Settings at ~156,555).
The appdata remembers the last connect string - the first screen may be a reconnect dialog with Abort.

### Identity UI end to end (scripts/identity_ui)

Helpers, all under `SCRATCH_ROOT` (default `/tmp/hypersomnia-ui`; the broker is `broker.sh` with `SCRATCH=$SCRATCH_ROOT/nettest`,
started with `STEAM_API_KEY_FILE` and a `SESSION_SECRET` kept in `$SCRATCH_ROOT/nettest/session_secret`):
- `ui.sh conf <name> [broker url]` (a client in `$SCRATCH_ROOT/ui/<name>`, main menu, the scratch site's leaderboards; `NICK=`),
  `ui.sh start <name> <display> [game args]`, `stop`, `shot <display> <png>`, `click`, `type`, `key`. `BIN=` for the Steam build.
  The game never opens a real browser there: `fake_bin/xdg-open` writes the URLs to `<client>/opened_urls`.
- `browser_sign_in.sh [steam id64]` (with `OPENED_URLS=<client>/opened_urls`) plays the player's browser for Link with Steam:
  opens the last `/link_key/start` URL signed in to Steam on the scratch site and clicks Link - the game then learns it by itself.
- account.key is read only at the start and after Restore / Start a new account - a key swapped on disk counts from the
  next start (restart the client to test it). Every start of a key without steam.cert (not one made just now) asks the broker
  quietly whether it is linked ("Asking quietly whether the key is linked") - linked elsewhere, it plays as the Steam account;
  never a modal. Not in the Steam build or CrazyGames.
- `cp imgui_big.ini <client>/user/imgui.ini` first: Settings at 20,20 1240x680, so whole panes fit a screenshot -
  then Settings 155,555, Account 126,112 with Info 102,144 / Link with Steam 245,144 / Backup 385,144 - the panes' own fields
  move with their content, so screenshot first. Without it (the default 1280x720 layout): Info 307,278 / Link 440,278 / Backup 590,278,
  Link with Steam... 397,337. Every confirmation and result is a centered modal. Backup of a key linked to Steam
  says only "Your progress is on your Steam account - nothing to back up...".
- `srv.sh start|stop <name> <port> [TEAMS_EQUAL]` - a server trusting the scratch broker.
- `link_key.py <account.key> [steam id64]` - links that key to a fake Steam session's account, as confirming the game's
  link page would (prints the confirm POST's status)
  (a Steam account takes at most 32 keys - a test account full of them answers 409).
- `../link_flow_test.py <site dir> <site url>` (with `SESSION_SECRET`) - the whole link protocol against the site, bans and replays included.
- `slow_broker.py <port> <delay>` - the broker answering after `delay` s (a closed port is a dead broker).
- `ui.sh conf <name> ""` - no identity broker at all. `srv.sh` wipes the server's dir, so every start is a **new server key**:
  a client joining with the old key (`--connect <address>,<key>`, its last join) gets "another key than the one you chose".
  `BROKER_KEYS_JSON='[]'` - a server trusting no broker; `SERVER_EXTRA` / `PRIVATE_EXTRA` - more JSON fields (see srv.sh).
- **RCON levels:** make the client's key first (`node -e` in the broker site with `src/identity_cert.js`: `privateKeyFromSeed`,
  `rawPublicKey`; write `hypersomnia-key-v1:<seed hex>` to `<client>/user/.keys/account.key`), then
  `PRIVATE_EXTRA=', "admin_profiles": [ { "account": "key_..", "rcon": "RCON" } ]'` (RCON comes only from admin_profiles -
  no address is trusted, not even loopback; for a dev server put your own key_ account there with "MASTER"). In game F8; at 1280x720 the tabs Arenas 383,252 / Vars 651,252 / Maintenance 761,252,
  the arena combo 450,284, Apply & Save 397,578. The server logs "Authorized the admin profile ... for basic rcon",
  "<nick> changed arena to ...", "Restarting the server due to rcon's request". BASIC: arena, game mode, map cycle, match commands,
  kick/ban, restart/shutdown/updates/logs; not Vars, not "Clear all" of the runtime prefs.
- `crazygames_sdk_stub.py <any pem> <game id> - <out.js>` - a CrazyGames guest (no token, no user).
- The fake `zenity` writes its arguments to `$ZENITY_LOG` (the file filters, the save dialog's title) and "chooses" `$FAKE_ZENITY_FILE`.
- `ui.sh stop` finds the client by its dir, whatever `BIN` started it.
- An expired steam.cert: rewrite its last field (the unix time it arrived) to 5 hours ago.

What each state looks like: an unlinked key asks the broker once per start, quietly, and the leaderboards say
`Account: Key account (key xxxxxxxx...)`; Link first asks /cert (an already linked key is "Successfully linked" at once, otherwise
the link page in the browser); a valid steam.cert is used at once after a restart; an expired one shows the cached Steam profile and,
after loading, a dimmed "Signing in to your Steam account..."; a dead broker gives one "Not signed in" message per run (with
Join anyway / Cancel before a `--connect`), then "Not signed in to Steam. Retry" under the profile; Retry that works shows
"Signed in to Steam" at the top; Restore shows one modal (Fetching... / Linked to your Steam account / Welcome back, or Retry and
Continue unlinked); a key unlinked elsewhere silently loses its steam.cert; a damaged account.key shows Backup in a modal.

## Soak: servers must never quietly stop taking players (scripts/soak)

The worst bug class: after N connections a table fills up and the server silently refuses everyone.
`build_soak.sh` builds a netcode-level client from the game's own netcode; then:

- `soak_netcode.sh [scale]` - ~9000 connect/disconnect cycles, half abandoned mid-handshake, by address and with the server's key,
  each with a new session key; 0 failures expected, a wrong key must fail, then a real client must join.
  (Before the fixes of 2026-10-11 a server stopped taking anyone after ~1060 abandoned handshakes.)
- `soak_webrtc.sh [cycles]` - native clients over WebRTC killed at random moments, more than the 64 WebRTC ids;
  then a full join must work. A native community server's WebRTC id is its address (`browser_connect_string`).

## Remote hosts (arena, arenaus)

`arena` and `arenaus` are this user's ssh aliases (`ssh arena`, `ssh arenaus`) - for someone else they may be named
differently or not exist: if `ssh <alias> true` fails, ask the user for their test hosts.
Use them for serious tests that a local setup can not show, without asking first - e.g. anything that needs real public
addresses: a test masterserver there (see Remote end to end below) listing a server by its public IP, so joins from the
server list match by the public address (locally every server is a "local server", matched by its LAN address, which gives
no key to expect), key mismatches against the list, NAT/WebRTC from outside. Same rules as below: only our own dirs and PIDs.

- `arena` = Ubuntu 25.04, glibc 2.41; `arenaus` = Ubuntu 22.04, glibc 2.35. A binary from this Arch machine (glibc 2.44+)
  does not run there - build in the Ubuntu container (24.04 for arena, 22.04 for arenaus) and ship libc++/libc++abi/libunwind
  from the rootfs next to it with `LD_LIBRARY_PATH`.
- `arena` has ufw: ports 9000-9010/udp are open and unused - use 9005. `arenaus` has no firewall.
- Upload the content dir (`hypersomnia/`, ~35 MB) with rsync, excluding `logs`, `user`, `cache`.
- Never copy production secrets anywhere but `~/yojimbo-test` (arena) or `~/hs-test-backend` (arenaus), and delete them after the test.

## Remote end to end: official server, ranked, accounts (scripts/remote_e2e)

The whole production flow on real hosts without touching production:
`remote_e2e/official.sh start <broker key>` (arena) - an official host for playing by hand: casual c1, c2 and ranked r1 on 9005-9007 in one process, its `official.sh key` goes into arenaus's `official_servers.d/pl`.
`arenaus:~/hs-test-backend` runs a test masterserver (UDP 18430, HTTP 18420) and a copy of the site (port 13000, clean DB,
its own `broker.key`, Node from the official tarball - the host has none) via `backend.sh start|stop|status|broker_key`; both read its `official_servers.d`.
`arena:~/yojimbo-test/e2e` runs a dedicated server with its own server key via `match.sh`, heartbeating to that master
and reporting to that site; `run_match.sh` puts its key in arenaus's `official_servers.d/pl` and passes the test broker's key.
`run_match.sh A B` plays one match with two local CLI clients (account keys made in `$SCRATCH/e2e/client_<name>/user/.keys/account.key`,
or `ID_A`/`ID_B` to override the ids); `db.sh show|sql` reads or changes the site DB.

- **Joins name the server's key (connect_target.h):** `--connect <key>` (found on the list, else the favorites' last address;
  without a list yet it waits up to 4 s), `--connect <address>,<key>` (straight there, expecting the key), or an
  address alone (nothing verified). A join from the server list is `<address>,<key>` and so is the last join saved in
  runtime_prefs (`client_connect`) - the next start joins without the masterserver. A mismatch: "doesn't match the server list"
  (list key) or "another key than the one you chose" - no Connect anyway anywhere. `remote_e2e/community.sh start` - a
  community server on 9005 with a new key each start, listed on the test master; `crash` then `start silent` - a server whose
  list entry (60 s) keeps the previous key. Never `kill -9` the pid file's pid (it is `timeout`'s). A client with
  `"server_list_provider": "http://<arenaus>:18420"` and `"last_activity": "MAIN_MENU"` joins from Browse servers.
- **Server browser and invites:** the list is cached at `<appdata>/cache/server_list.bin` (shown in the browser only, until a fresh one comes - joins, quick play and invites never use it) and downloaded at every start in the
  background (even with `--connect`); favorites are `user/favorite_servers.json` (`key`, `label`, `address`, `name`, `manual` - a
  manual one is Add an unlisted server: shown under Unlisted, never matched with the list), passwords
  `user/server_passwords.json`, the browser tab in `user/gui_state.json` (`server_browser_tab`: LOCAL, ALL, FAVORITES). Esc - Invite to join opens a window from what the game
  knows (no list download): links, address, key, ★; joined over the LAN it shows the public address from the list. The log
  line `Steam rich presence connect: <address>,<key>` shows what a Steam friend would join (also in non-Steam builds).
  Main menu keys: C - Connect to server, P - Quick play. `backend.sh` and the e2e servers stop by themselves after 2 h
  (`SECS=` for `community.sh`) - a list that stops coming is usually that.
- **Idle CLI clients never get onto a team** - joining players stay spectators. Only tournament mode assigns teams:
  `match.sh` writes a two-team `tournament.json` with the clients' account ids (`key_...`, or `steam_...` once linked).
- **Tournament mode forces overtime**, so idle players never finish by score. Use `"ranked": { "max_rejoins": 0 }` and kill one
  client after the match goes live: it abandons at once and the report goes out immediately (`losers_abandoned`).
  Also disable the AFK kicks (`move_to_spectators_if_afk_for_secs`, `kick_if_afk_for_secs` = 999999).
- Wins count on the site only from 5 rounds (`MIN_ROUNDS_TO_COUNT_WINS`), so these abandon matches show W 0.
- **Signing in to the site as a Steam user without Steam:** `fake_steam_session.sh <steam id64> [name]` inserts a passport
  session into the test DB and prints a `connect.sid=...` cookie (signed with the `SESSION_SECRET` that `backend.sh` sets).
  `fake_steam_session.py <site dir> <id> <name> <state>` signs in anew for that link page's state (as the Steam sign-in started
  from it does); `curl -b "$cookie"` then reaches `/link_key/start?state=..&t=..&key=..&sig=..` (the confirm page) and a POST of
  the same fields links the key; a page or unlink not newer than the key's last change (`key_link_changes`) is refused;
  `POST /profile/unlink_key` with `key=key_...` unlinks; the game's unlink is `GET /link_key/unlink?t&key&sig` -
  `browser_sign_in.sh` plays the browser for the link page.
- **Linking from the game by clicking:** a client with `"website_url": "http://<arenaus>:13000"` in conf.d, under Xvfb and `ui.sh`'s
  fake xdg-open: Settings - Account - Link with Steam - Link with Steam..., then `browser_sign_in.sh` against that site
  (it needs the site's session secret for the fake Steam session). Check with `db.sh show`.
  For a quick link without UI: `db.sh sql "INSERT INTO associations ..."`.

## Real identities: Steam (needs the user's running Steam client)

- **Steam:** `build/Debug-x64-clang-claude-steam/Hypersomnia --connect <server> --appdata-dir <scratch>` from `hypersomnia/`
  (it writes `steam_appid.txt` there, gitignored), with `"identity_broker_url"` pointing at a test broker. Only the broker needs
  the real Steam Web API key (`STEAM_API_KEY_FILE=<file> broker.sh start` locally, or `~/hs-test-backend/steam_web_api_key` on arenaus,
  0600, never printed; `shred -u` it right after the test). The game servers never ask Steam anything.
- The web build in the e2e: `USER_FILES=<dir> prepare_web.sh` with `<dir>/conf.d/test.json` holding the test master's and broker's
  URLs (an IP does not fit in place in `Hypersomnia.data`). Join with `/game/ranked-pl:1`.
- CrazyGames: `VARIANT=crazygames build_web.sh`; the broker checks the JWT with `CRAZYGAMES_PUBLIC_KEY_PEM` (a test key pair) and
  `CRAZYGAMES_GAME_ID`.
- Steam verifies a Web API ticket only for an identity of at most 31 characters. `steam_tickets.py` gets tickets
  for any identities to probe the Web API directly.

## The hypersomnia.io site

Test only in a scratch copy of the site repo (ask the user where it is; rsync without `.git`, `.env` and `private/*.db*`, `node_modules` symlinked).
`src/db.js` always opens `../private/mmr.db` next to the code (it ignores `DB_PATH`) - put a test DB exactly there.
Run with `PORT=<free port> node app.js --test` (or `broker.sh`). Test mode seeds `private/official_servers.d/{pl,au}` with the public keys of
the seeds `sha256("hypersomnia-test-server-<id>")`; `run_mmr_tests.py` signs its reports with them.
`run_mmr_tests.py` reads `private/mmr.db` too; 25/46 of its assertions fail on the baseline as well (environment, not code).

## Findings worth knowing

- **The pgrep trap also fires through heredocs:** a `pgrep -f PATTERN` inside a script written by a heredoc in the same Bash
  call matches the calling shell, whose command line holds the heredoc text - exit 144. Write the script in one call, run it in another.
- **Site to a remote host:** `rsync` it without `.git`, `.env`, `private/`, `node_modules`, `hosting` and **`infra`** (725 MB - took 10 minutes).
- `broker.sh stop` finds its pid under `$SCRATCH` - stopping with another `SCRATCH` than the start leaves the old one on 13500
  ("port 13500 is busy"); check `readlink /proc/<pid>/cwd` before killing a leftover.
- `SESSION_SECRET` for `fake_steam_session.py` must be the one the site was started with: keep it in a file (`$SCRATCH/session_secret`).
- **Clicking the game in Chromium (pw_driver.js):** ImGui misses a press and release in one frame - the driver moves, waits and
  clicks with a 120 ms delay. A tab the game opened (Sign in with Steam) takes the focus and the game tab is throttled to almost no
  frames: switch back with `/page?i=0` before clicking. A fresh driver is a fresh browser profile - a new account key each start.
- The game's console in the browser: `curl localhost:<port>/console`; wait for `Received an identity cert` before clicking -
  the first-start nickname popup comes up right after. Uncaught exceptions show there as `PAGE ERROR:` - a page stuck after
  "App data directory" is usually one (e.g. a ReferenceError in common.js).
- **Never `prepare_web.sh` while that web build is building** - it copies a half-written wasm and the game hangs at start.
- A scenario's server list check right after start can be empty: the first heartbeat goes ~10 s after start - wait 14 s.
- Scripts that `cd $REPO/hypersomnia` must not use `$(dirname "$0")` afterwards - resolve `HERE` to an absolute path first.
- A key added to `official_servers.d` is picked up at the master's next rescan (~40 s) - until then the list shows the server
  as community. The server itself never knows; a ranked match of a key the site does not know as official is refused at the report.
- In `run_match.sh` tournament mode the server name is built from the rosters, so with `key_...` ids it is a long `[key_...` string.
- On the site, a `key_` account gets its MMR row only with a match it did not abandon - the abandoning loser shows no row.
- **A reboot clears /tmp** - the session scratchpad, test keys, the broker copy and a rootfs under /tmp are gone. Commit before long
  runs; the Ubuntu 22.04 rootfs now defaults to `~/.cache/hypersomnia-rootfs22` (`ROOTFS22=`), the rest is recreated in seconds.
- **Soak RSS:** a server's RSS jumps ~25 MB with the first ~2000 connections and then stays flat (+0.1 MB per 2000) - per-client
  buffers touched once, not a leak. To tell, measure after several batches against one server, not just before/after one run,
  and never with a build running at the same time.
- After the WebRTC soak, `grep -c "did not open its DataChannel in time"` in the server log shows the 15 s freeing of unopened peers working.
- **Commit before anything long.** The machine was rebooted twice mid-test (network outages) - each time /tmp, the scratch scripts
  and the test broker were gone; anything worth keeping goes into this skill's scripts, not the scratchpad.
- The site's link limits are per minute: running `link_flow_test.py` several times in a row from localhost hits "Too many attempts" -
  wait a minute, that is the limit working.
- Release builds of the netcode tests: run `stdbuf -o0 ./bin/test` - without it a failed `check` shows the last *buffered* test name,
  not the failing one.
- **A ranked match that ends by itself:** tournaments force overtime (win by two) and idle players only win their defending rounds
  (time runs out, no bomb), so a tournament match of idle clients never ends - use a regular ranked server, where 1:1 after both
  halves ends it, and click Auto-assign (585,430 in 1280x720) in the faction window. A map's rules (max_team_score - at least 2 -,
  round_time, warmup_time, freeze_time, buy_time, round_end_time) are in its JSON under game_modes.bomb_defusal; a server takes
  test maps from user/downloads/arenas, clients download them from it directly when hypersomnia.io has no such map.
- The site counts wins only from 5 rounds (MIN_ROUNDS_TO_COUNT_WINS): short test matches update MMR but show W 0 / L 0.
- The headless-only sources (`HEADLESS`: tournament, crash recovery) are not in the client build: after removing a var, also build
  `build/Debug-x64-clang-claude-headless`.

- Symptom `ignored packet. sequence 0000000000000000 already received (replay protection)` and a client that connects
  but never gets `Received initial public vars`: packets sent outside netcode's normal path without advancing the sequence.
- netcode 1.4.8 refuses to create a client or server with `override_send_and_receive` but without both overrides
  (`error: override_send_and_receive requires both ...`) - the web build connects nowhere.
- The web build is single-threaded: synchronous `emscripten_fetch` returns nothing (status -1). Use `augs::async_get`.
- Over WebRTC every peer is 127.255.255.255 inside netcode, with its id as the port; the per-address handshake limit and bans
  use the data channel's real address instead.
- The Steamworks SDK is not in the repo: unpack a `steamworks_sdk_*.zip` (ask the user where it is) with
  `cmake/steam_integration/steamworks/put_sdk_here.sh <zip>` into the gitignored `steamworks/sdk`. A Steam build (`-DDEVELOP_STEAM_INTEGRATION=1
  -DUSE_SYSTEM_FREETYPE=0`) compiles without the Steam client; testing tickets needs it running and a `steam_web_api_key` on the server.
- A server shutting down right after an immediate heartbeat request skipped its goodbye and lingered on the list
  (fixed in ce4720e4a); after a stop, the list should be empty at once.
- Upstream yojimbo tests: `cmake -S src/3rdparty/yojimbo -B $SCRATCH/yjbuild && cmake --build ... && ./bin/test`
  (Unix Makefiles - the Ninja generator clashes on a target named `test`).

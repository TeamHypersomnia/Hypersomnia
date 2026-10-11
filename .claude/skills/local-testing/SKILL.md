---
name: local-testing
description: Test Hypersomnia end to end without the user, mainly networking - local masterserver, dedicated servers and CLI clients (UDP and WebRTC), secure connect tokens and authentication, the web build in Firefox under Xvfb, game UI via screenshots and XTest clicks, the hypersomnia.io site in a scratch copy, isolated builds (native, Steam, web, Ubuntu containers for the remote hosts) and test servers on arena and arenaus. Use whenever the user asks to test something "lokalnie", "headless", "z CLI", "bez mojej ingerencji", "postaw dedyka i połącz się", to check a UI change on screen, or to verify networking, ranked/auth, WebRTC or web build changes before a release.
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
| `scenario_udp_secure.sh` | Master with `official_servers.d`, official casual (de) + ranked (pl) + community servers, 4 CLI clients: token endpoint 200/404, secure vs insecure, ranked does not answer insecure clients at all (no token from an unreachable master: the client times out), identity key auth (`key_...`), key file only where needed (0600), same account on reconnect, rate limit |
| `scenario_webrtc_secure.sh` | Native client over WebRTC (`--connect rtc:ranked-pl`) through the local signalling server: token by WebRTC alias, secure connection, key auth |
| `scenario_web_firefox.sh [connect] [wait]` | The real web build in Firefox under Xvfb joining `/game/<connect>` (default `pl`), secure over WebRTC; screenshot in `$SCRATCH/web_test/screen.png` |
| `build_web.sh` | Web Release build (`-DUSE_O3=1`) in `build/Release-Web-clang-claude-web`, the same steps as `build.sh Release Web` |
| `[USER_FILES=dir] prepare_web.sh <web build> <out>` | Copies a web build and points its packaged config at the local master (see below); USER_FILES also writes files into its `/user` before start |
| `webserve.py <dir> <port>` | Serves a web build with COOP/COEP; any path serves the game, so `/game/<id>` joins right away |
| `steam_tickets.py IDENTITY...` | Steam Web API tickets from the user's running Steam client, to probe `AuthenticateUserTicket` without the game |
| `xinput.py click x y` / `type TEXT` / `key NAME` / `scroll x y n` | Clicks, types, presses keys and scrolls on `$DISPLAY` through XTest (no xdotool needed) |
| `local_stack.sh [web build] [hours]` | Leaves running, for the user to test by hand: master, official ranked [PL] and casual [DE] servers, the web build on http://localhost:8765 pointed at them; `local_stack.sh stop` |
| `build_headless_ubuntu2204.sh` | Headless Release build inside an unprivileged Ubuntu 22.04 rootfs (bwrap) - glibc 2.35, runs on both arenaus and arena; prints where the libc++ to ship lies |
| `build_headless_ubuntu_container.sh` | The same in Ubuntu 24.04 (glibc 2.39) - arena only |

Pass/fail is read from the logs - the scripts grep the meaningful lines: `connected securely with a token issued for X`,
`Authenticated "nick". ID: key_...`, `This server requires a secure connection`, `Received initial public vars`.

## The web build headless - how it works

- **Config without rebuilding:** the web build packages `default_config.json` into `Hypersomnia.data` uncompressed.
  `prepare_web.sh` replaces the provider lines in place with text of the same length padded with spaces,
  so the file packager offsets stay valid. (In the game UI the same can be set in Settings - Advanced - Network.)
- **Joining from the URL:** the page passes `window.location.pathname` as a CLI argument; `/game/<id>` connects to that WebRTC id.
- **Firefox:** `firefox --no-remote --profile <fresh dir> --new-instance URL` under `Xvfb`, with `user.js` setting
  `devtools.console.stdout.content=true` - the game's console output then lands in Firefox's stdout.
  WebGL works through llvmpipe. Allow ~60 s for the 100 MB data to load.
- Discord sign-in can not be done headless: its redirect is `<origin>/assets/discord_redirect.html` and must be allowed
  in the Discord app. Web ranked therefore stops at sign-in; casual servers work fully.

## UI checks

Run the game under `Xvfb :77 -screen 0 1280x720x24` with a 1280x720 non-fullscreen window (conf.d `"window"`),
`import -window root shot.png` to screenshot, read the image, and `python3 xinput.py click x y` to click.
A fresh appdata starts in the tutorial: `xinput.py key Escape` opens the menu (Settings at ~156,555).
The appdata remembers the last connect string - the first screen may be a reconnect dialog with Abort.

## Soak: servers must never quietly stop taking players (scripts/soak)

The worst bug class: after N connections a table fills up and the server silently refuses everyone.
`build_soak.sh` builds a netcode-level client from the game's own netcode; then:

- `soak_netcode.sh [scale]` - ~9000 connect/disconnect cycles, half abandoned mid-handshake, insecure and secure tokens,
  against a community and an official dedicated server; 0 failures expected, then a real client must join.
  (Before the fixes of 2026-10-11 a server stopped taking anyone after ~1060 abandoned handshakes.)
- `soak_webrtc.sh [cycles]` - native clients over WebRTC killed at random moments, more than the 64 WebRTC ids;
  then a full join must work. A native community server's WebRTC id is its address (`browser_connect_string`).

## Remote hosts (arena, arenaus)

- `arena` = Ubuntu 25.04, glibc 2.41; `arenaus` = Ubuntu 22.04, glibc 2.35. A binary from this Arch machine (glibc 2.44+)
  does not run there - build in the Ubuntu container (24.04 for arena, 22.04 for arenaus) and ship libc++/libc++abi/libunwind
  from the rootfs next to it with `LD_LIBRARY_PATH`.
- `arena` has ufw: ports 9000-9010/udp are open and unused - use 9005. `arenaus` has no firewall.
- Upload the content dir (`hypersomnia/`, ~35 MB) with rsync, excluding `logs`, `user`, `cache`.
- Never copy production secrets anywhere but `~/yojimbo-test` (arena) or `~/hs-test-backend` (arenaus), and delete them after the test.

## Remote end to end: official server, ranked, accounts (scripts/remote_e2e)

The whole production flow on real hosts without touching production:
`arenaus:~/hs-test-backend` runs a test masterserver (UDP 18430, HTTP 18420) and a copy of the site (port 13000, clean DB,
Node from the official tarball - the host has none) via `backend.sh start|stop|status`; both read its `official_servers.d`.
`arena:~/yojimbo-test/e2e` runs a dedicated server with its own `official_server_secret` via `match.sh`,
heartbeating to that master and reporting to that site. `run_match.sh A B` plays one match with two local CLI clients
(pre-generated identity keys in `$SCRATCH/e2e/client_<name>/user/identity.key`, ids in `id_<name>`); `db.sh show|sql` reads or changes the site DB.

- **Idle CLI clients never get onto a team** - joining players stay spectators. Only tournament mode assigns teams:
  `match.sh` writes a two-team `tournament.json` with the clients' `key_...` ids. Give the server any `steam_web_api_key`
  (e.g. "e2e-dummy"), otherwise tournament mode switches to nickname auth (`nick_...`).
- **Tournament mode forces overtime**, so idle players never finish by score. Use `"ranked": { "max_rejoins": 0 }` and kill one
  client after the match goes live: it abandons at once and the report goes out immediately (`losers_abandoned`).
  Also disable the AFK kicks (`move_to_spectators_if_afk_for_secs`, `kick_if_afk_for_secs` = 999999).
- Wins count on the site only from 5 rounds (`MIN_ROUNDS_TO_COUNT_WINS`), so these abandon matches show W 0.
- **Signing in to the site as a Steam user without Steam:** `fake_steam_session.sh <steam id64> [name]` inserts a passport
  session into the test DB and prints a `connect.sid=...` cookie (signed with the `SESSION_SECRET` that `backend.sh` sets).
  `curl -b "$cookie"` then reaches `/profile`; `POST /profile/link_code` returns a page with the 8-character code,
  `POST /profile/unlink_key` with `key=key_...` unlinks.
- **Linking from the game by clicking:** a client with `"website_url": "http://<arenaus>:13000"` in conf.d, under Xvfb:
  Settings - Client (~833,218) - Account (~417,278); click the code field (~415,580), `xinput.py type CODE`, click Link (~588,582),
  scroll down to read "Linked. ..." (a reused code shows "Invalid or expired code"). Check with `db.sh show`.
  For a quick link without UI: `db.sh sql "INSERT INTO associations ..."`.

## Real identities: Steam and Discord (needs the user)

One match covers both: the web build signed in with the user's Discord against the Steam dev build signed in with
their running Steam client, on the arena test server, reported to the arenaus test site.

- **Discord token:** the user signs in on https://play.hypersomnia.io and runs in the devtools console
  `FS.readFile('/user/cached_auth.json', {encoding: 'utf8'})`. Save it 0600 in the scratchpad only. Note that `prepare_web.sh` then builds it into the served `inject_user_files.js`:
  any local process can fetch it while `webserve.py` runs - stop it right after the test and delete the copy.
  `USER_FILES=<dir> prepare_web.sh` writes `<dir>/cached_auth.json` and `<dir>/conf.d/test.json` (the test master's
  URLs - the connect token URL with an IP does not fit in place in `Hypersomnia.data`) into `/user` before the game starts.
  Serve on a port other than 8765 if `local_stack.sh` still runs there. Join with `/game/ranked-` (the alias the
  tournament-named test server gets). Signing out revokes the token; from localhost the revoke fails on CORS, which keeps it.
- **Steam:** the user's Steam client must run. `build/Debug-x64-clang-claude-steam/Hypersomnia --connect <arena>:9005
  --appdata-dir <scratch>` from `hypersomnia/` (it writes `steam_appid.txt` there, gitignored). The server needs the real
  Steam Web API key: the user puts it in a file, copy it to `arena:~/yojimbo-test/e2e/steam_web_api_key` (0600, never print it),
  `match.sh` reads it; `shred -u` it right after the test.
- Tournament teams are the real ids: `match.sh start discord_<id> steam_<id64>`. Kill one client once both are
  `Authenticated` - the abandon reports at once.
- Steam verifies a Web API ticket only for an identity of at most 31 characters. `steam_tickets.py` gets tickets
  for any identities to probe the Web API directly.

## The hypersomnia.io site

Test only in a scratch copy of the site repo (ask the user where it is; rsync without `.git`, `.env` and `private/*.db*`, `node_modules` symlinked).
`src/db.js` always opens `../private/mmr.db` next to the code (it ignores `DB_PATH`) - put a test DB exactly there.
Run with `PORT=<free port> node app.js --test`. Test mode seeds `private/official_servers.d/{pl,au}` with the secrets `pl`, `au`.
`run_mmr_tests.py` reads `private/mmr.db` too; 25/46 of its assertions fail on the baseline as well (environment, not code).

## Findings worth knowing

- Symptom `ignored packet. sequence 0000000000000000 already received (replay protection)` and a client that connects
  but never gets `Received initial public vars`: packets sent outside netcode's normal path without advancing the sequence.
- netcode 1.4.8 refuses to create a client or server with `override_send_and_receive` but without both overrides
  (`error: override_send_and_receive requires both ...`) - the web build connects nowhere.
- The web build is single-threaded: synchronous `emscripten_fetch` returns nothing (status -1). Use `augs::async_get`.
- Over WebRTC the netcode client takes packets only from the server address in the connect token; a secure token's
  public address list must point at the data channel's internal address.
- The Steamworks SDK is not in the repo: unpack a `steamworks_sdk_*.zip` (ask the user where it is) with
  `cmake/steam_integration/steamworks/put_sdk_here.sh <zip>` into the gitignored `steamworks/sdk`. A Steam build (`-DDEVELOP_STEAM_INTEGRATION=1
  -DUSE_SYSTEM_FREETYPE=0`) compiles without the Steam client; testing tickets needs it running and a `steam_web_api_key` on the server.
- A server shutting down right after an immediate heartbeat request skipped its goodbye and lingered on the list
  (fixed in ce4720e4a); after a stop, the list should be empty at once.
- Upstream yojimbo tests: `cmake -S src/3rdparty/yojimbo -B $SCRATCH/yjbuild && cmake --build ... && ./bin/test`
  (Unix Makefiles - the Ninja generator clashes on a target named `test`).

---
name: local-testing
description: Test Hypersomnia end to end without the user, mainly networking - local masterserver, dedicated servers and CLI clients (UDP and WebRTC), secure connect tokens and authentication, the web build in Firefox under Xvfb, game UI via screenshots and XTest clicks, the hypersomnia.io site in a scratch copy, isolated builds (native, Steam, web, Ubuntu containers for the remote hosts) and test servers on arena and arenaus. Use whenever the user asks to test something "lokalnie", "headless", "z CLI", "bez mojej ingerencji", "postaw dedyka i połącz się", to check a UI change on screen, or to verify networking, ranked/auth, WebRTC or web build changes before a release.
---

# Local testing

Everything runs from scripts in `scripts/`, each fully isolated: own appdata dirs, own ports, own scratch dir
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
6. On remote hosts, production runs as `Hypersomnia-Headless.AppImage --as-service` (masterserver, official server) - find it with `ps`.
   Work only in `~/yojimbo-test`, restart only by your own PIDs, and verify afterwards that the production processes still
   have their old uptime (`ps -eo etime,args | grep Hypersomnia-Headless`).

## Scenarios (scripts/)

| Script | What it proves |
|---|---|
| `scenario_udp_secure.sh` | Master with `official_servers.d`, official casual (de) + ranked (pl) + community servers, 4 CLI clients: token endpoint 200/404, secure vs insecure, ranked kicks insecure clients, identity key auth (`key_...`), key file only where needed (0600), same account on reconnect, rate limit |
| `scenario_webrtc_secure.sh` | Native client over WebRTC (`--connect rtc:ranked-pl`) through the local signalling server: token by WebRTC alias, secure connection, key auth |
| `scenario_web_firefox.sh [connect] [wait]` | The real web build in Firefox under Xvfb joining `/game/<connect>` (default `pl`), secure over WebRTC; screenshot in `$SCRATCH/web_test/screen.png` |
| `build_web.sh` | Web Release build (`-DUSE_O3=1`) in `build/Release-Web-clang-claude-web`, the same steps as `build.sh Release Web` |
| `prepare_web.sh <web build> <out>` | Copies a web build and points its packaged config at the local master (see below) |
| `webserve.py <dir> <port>` | Serves a web build with COOP/COEP; any path serves the game, so `/game/<id>` joins right away |
| `xclick.py x y` | Moves and left-clicks on `$DISPLAY` through XTest (no xdotool needed) |
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
`import -window root shot.png` to screenshot, read the image, and `python3 xclick.py x y` to click.
The appdata remembers the last connect string - the first screen may be a reconnect dialog with Abort.

## Remote hosts (arena, arenaus)

- `arena` = Ubuntu 25.04, glibc 2.41; `arenaus` = Ubuntu 22.04, glibc 2.35. A binary from this Arch machine (glibc 2.44+)
  does not run there - build in the Ubuntu container (24.04 for arena, 22.04 for arenaus) and ship libc++/libc++abi/libunwind
  from the rootfs next to it with `LD_LIBRARY_PATH`.
- `arena` has ufw: ports 9000-9010/udp are open and unused - use 9005. `arenaus` has no firewall.
- Upload the content dir (`hypersomnia/`, ~35 MB) with rsync, excluding `logs`, `user`, `cache`.
- Never copy production secrets anywhere but `~/yojimbo-test`, and delete them after the test.

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
- Upstream yojimbo tests: `cmake -S src/3rdparty/yojimbo -B $SCRATCH/yjbuild && cmake --build ... && ./bin/test`
  (Unix Makefiles - the Ninja generator clashes on a target named `test`).

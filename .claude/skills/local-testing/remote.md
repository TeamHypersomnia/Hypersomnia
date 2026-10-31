# Remote hosts, the site, real identities

Part of the local-testing skill (SKILL.md: isolation rules first - rule 7 for the remote hosts).

## Remote hosts (arena, arenaus)

`arena` and `arenaus` are this user's ssh aliases (`ssh arena`, `ssh arenaus`) - for someone else they may be named
differently or not exist: if `ssh <alias> true` fails, ask the user for their test hosts.
Use them for serious tests that a local setup can not show, without asking first - e.g. anything that needs real public
addresses: a test masterserver there (see Remote end to end below) listing a server by its public IP, so joins from the
server list match by the public address (locally every server is a "local server", matched by its LAN address, which gives
no key to expect), key mismatches against the list, NAT/WebRTC from outside. Same rules as below: only our own dirs and PIDs.

- `arena` = Ubuntu 25.04, glibc 2.41; `arenaus` = Ubuntu 22.04, glibc 2.35. A binary from this Arch machine (glibc 2.44+)
  does not run there - build with `build_headless_ubuntu2204.sh` (Ubuntu 22.04, glibc 2.35: runs on both - a binary needs
  at least the glibc it was built with) and ship libc++/libc++abi/libunwind from the rootfs next to it with `LD_LIBRARY_PATH`.
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
  `"server_list_provider": "http://<arenaus>:18420/server_list"` and `"last_activity": "MAIN_MENU"` joins from Browse servers.
- **Server browser and invites:** the list is cached at `<appdata>/cache/server_list.bin` (shown in the browser only, until a fresh one comes - joins, quick play and invites never use it) and downloaded at every start in the
  background (even with `--connect`); favorites are `user/favorite_servers.json` (`key`, `label`, `address`, `name`, `manual` - a
  manual one is Add an unlisted server: shown under Unlisted, never matched with the list), passwords
  `user/server_passwords.json`, the browser tab in `user/gui_state.json` (`server_browser_tab`: LOCAL, ALL, FAVORITES). Esc - Invite to join opens a window from what the game
  knows (no list download): links, address, key, ★; joined over the LAN it shows the public address from the list. The log
  line `Steam rich presence connect: <address>,<key>` shows what a Steam friend would join (also in non-Steam builds).
  Main menu keys: C - Connect to server, P - Quick play. `backend.sh` and the e2e servers stop by themselves after 2 h
  (`SECS=` for `community.sh`) - a list that stops coming is usually that.
- **Idle CLI clients never get onto a team** - joining players stay spectators. Only tournament mode assigns teams:
  `match.sh` starts the server with `--tournament-teams "<a>; <b>"` - the clients' account ids (`key_...`, or `steam_...` once linked).
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

## Pitfalls

- **Site to a remote host:** `rsync` it without `.git`, `.env`, `private/`, `node_modules`, `hosting` and **`infra`** (725 MB - took 10 minutes).
- `broker.sh stop` finds its pid under `$SCRATCH` - stopping with another `SCRATCH` than the start leaves the old one on 13500
  ("port 13500 is busy"); check `readlink /proc/<pid>/cwd` before killing a leftover.
- `SESSION_SECRET` for `fake_steam_session.py` must be the one the site was started with: keep it in a file (`$SCRATCH/session_secret`).
- In `run_match.sh` tournament mode the server name is built from the rosters, so with `key_...` ids it is a long `[key_...` string.
- On the site, a `key_` account gets its MMR row only with a match it did not abandon - the abandoning loser shows no row.
- The site's link limits are per minute: running `link_flow_test.py` several times in a row from localhost hits "Too many attempts" -
  wait a minute, that is the limit working.
- The Steamworks SDK is not in the repo: unpack a `steamworks_sdk_*.zip` (ask the user where it is) with
  `cmake/steam_integration/steamworks/put_sdk_here.sh <zip>` into the gitignored `steamworks/sdk`. A Steam build
  (`scripts/build_claude.sh steam`) compiles without the Steam client; testing tickets needs it running and a `steam_web_api_key` on the server.

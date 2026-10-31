# Identity UI end to end (scripts/identity_ui)

Part of the local-testing skill (SKILL.md: isolation rules and "UI checks" first).

Helpers, all under `SCRATCH_ROOT` (default `/tmp/hypersomnia-ui`; the broker is `broker.sh` with `SCRATCH=$SCRATCH_ROOT/nettest`,
started with `STEAM_API_KEY_FILE` and a `SESSION_SECRET` kept in `$SCRATCH_ROOT/nettest/session_secret`):
- `ui.sh conf <name> [broker url]` (a client in `$SCRATCH_ROOT/ui/<name>`, main menu, the scratch site's leaderboards; `NICK=`),
  `ui.sh start <name> <display> [game args]`, `stop`, `shot <display> <png>`, `click`, `type`, `key`. `BIN=` for the Steam build.
  The game never opens a real browser there: `fake_bin/xdg-open` writes the URLs to `<client>/opened_urls`.
- `browser_sign_in.sh [steam id64]` (with `OPENED_URLS=<client>/opened_urls`) plays the player's browser for Link with Steam:
  opens the last `/link_key/start` URL signed in to Steam on the scratch site and clicks Link - the game then learns it by itself.
- account.key is read only at the start and after Restore / Start a new account - a key swapped on disk counts from the
  next start (restart the client to test it). Every start (not of a key made just now) asks the broker quietly whether the key
  is linked ("Asking quietly whether the key is linked") - unlinked on the site, it drops its steam_cert.json; linked elsewhere
  (`link_key.py`), Settings open with "This key is linked to a Steam account - is it yours?" until answered. Not in the Steam
  build or CrazyGames.
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
- `ui.sh conf <name> ""` - no identity broker at all. `srv.sh` starts in a fresh dir each time, so every start is a **new server key**:
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
- An expired steam_cert.json: set its "received_at" (the unix time it arrived) to 5 hours ago.

What each state looks like: an unlinked key asks the broker once per start, quietly, and the leaderboards say
`Account: Key account (key xxxxxxxx...)`; Link first asks /cert (an already linked key is "Successfully linked" at once, otherwise
the link page in the browser); a valid steam_cert.json is used at once after a restart; an expired one shows the cached Steam profile and,
after loading, a dimmed "Signing in to your Steam account..."; a dead broker gives one "Not signed in" message per run (with
Join anyway / Cancel before a `--connect`), then "Not signed in to Steam. Retry" under the profile; Retry that works shows
"Signed in to Steam" at the top; Restore shows one modal (Fetching... / Linked to your Steam account / Welcome back, or Retry and
Continue unlinked); a key unlinked elsewhere silently loses its steam_cert.json; a damaged account.key shows Backup in a modal.

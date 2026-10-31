# The web build headless - how it works

Part of the local-testing skill (SKILL.md: isolation rules first).

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
  no code; the same natively, in a tab or an iframe). The game then asks "Link to this Steam account?" (the profile link,
  [Link] [Cancel] - Cancel unlinks; steam_cert.json is saved only after Link); linking moves no progress. A refusal (another
  account's key, 32 keys) shows on the page only; the page is good for 3 min, then the game stops waiting.
  Unlink from Steam is one signed GET `/link_key/unlink?t&key&sig`, answered at once.

## Pitfalls

- **Clicking the game in Chromium (pw_driver.js):** ImGui misses a press and release in one frame - the driver moves, waits and
  clicks with a 120 ms delay. A tab the game opened (Sign in with Steam) takes the focus and the game tab is throttled to almost no
  frames: switch back with `/page?i=0` before clicking. A fresh driver is a fresh browser profile - a new account key each start.
- The game's console in the browser: `curl localhost:<port>/console`; wait for `Received an identity cert` before clicking -
  the first-start nickname popup comes up right after. Uncaught exceptions show there as `PAGE ERROR:` - a page stuck after
  "App data directory" is usually one (e.g. a ReferenceError in common.js).
- **Never `prepare_web.sh` while that web build is building** - it copies a half-written wasm and the game hangs at start.
- The web build is single-threaded: synchronous `emscripten_fetch` returns nothing (status -1). Use `augs::async_get`.

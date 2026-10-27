---
name: releasing
description: Releasing a new version of Hypersomnia - the order of every step (site, nginx, game tag, CI, signing, servers, web platforms), the build command of every platform, and the pitfalls that broke past releases. Use when the user says "wydajemy", "release", "tag", "wypchnij", "deploy", or asks what is left before a release.
---

# Releasing Hypersomnia

Write the release's own checklist first (a markdown file outside the scratchpad, opened for the user in nvim) from the
order below, with the release's specifics added, and tick it off as you go. Ask the user before every step that
reaches production unless they have said to go ahead with all of them.

## Before the tag

1. **Every variant compiles** - not only the Linux one you worked in. The web build fails on code that only exists
   natively (a blocking HTTP `Post`, native sockets) and CI would only show it after the tag:
   - Linux native (debug, fast): your own `build/Debug-x64-clang-claude`
   - headless: `build/Debug-x64-clang-claude-headless`; for the servers' glibc: `scripts/build_headless_ubuntu2204.sh`
     (local-testing skill)
   - Steam: `build/Debug-x64-clang-claude-steam` (`-DDEVELOP_STEAM_INTEGRATION=1`)
   - web: `VARIANT= scripts/build_web.sh`, CrazyGames: `VARIANT=crazygames scripts/build_web.sh` (local-testing skill;
     needs emsdk - `EMSDK=<emsdk dir>`)
   - Windows and macOS: only CI builds them - read the CI result, never assume
2. **Tests** after network or server changes: the local-testing skill's `regression.sh`, the tournament suite
   (frozen copy, 4 shards), the Docker matrix if the setup scripts or the generator changed, one web run in Firefox
   (`scenario_web_firefox.sh`; the web asks for a nickname and shows the changelog on a fresh profile - click through).
3. **The changelog**: the new version's date is the release day (`## [X.Y.Z] - YYYY-MM-DD`); balance changes as tables
   (changelog skill).
4. **Forks and submodules pushed** - a submodule commit that is only local fails every CI job at checkout
   ("not our ref"): `git submodule foreach 'git log --oneline @{u}..HEAD 2>/dev/null'` must print nothing.
   The netcode fork (`src/3rdparty/yojimbo`, branch `hypersomnia-phase3`) pushes only over ssh:
   `git push git@github.com:TeamHypersomnia/yojimbo.git hypersomnia-phase3`.
5. The user's own playtest of a RelWithDebInfo build (`build/current`).

## The order of the release (do not swap)

1. **What the new game needs from the site goes first** (nginx locations, endpoints): the site repo's
   `infra/` - `ansible-playbook -i inventory/hypersomnia.io.yml playbooks/site.yml --tags nginx` (`--check --diff`
   first). Check with curl.
2. **Push the site** (its master) - but deploy its pages only after the game (step 7): the generator writes commands
   for the new version.
3. **Tag and push the game**: `cmake/push_tag.sh X.Y.Z` (the tag first, then master - CI reads the version from the
   tag). Watch CI (`gh run list`); four builds (Windows, Linux, macOS, headless) and the Docker release on the tag.
4. **Web builds at the tag** (HEAD must be exactly at it): the user's aliases `cmkrw` (play), `cmkrwi` (itch),
   `cmkrcg` (CrazyGames) - `cmake/build.sh Release Web '-DUSE_O3=1 [-DBUILD_ITCH=1 | -DBUILD_CRAZYGAMES=1]
   -DGENERATE_DEBUG_INFORMATION=0'` with `BUILD_FOLDER_SUFFIX=release-web | release-web-itch | release-crazygames`,
   then ninja there. `cmake/build.sh` repoints `build/current` - the last one built wins.
   All three at once: `scripts/build_webs.sh X.Y.Z` (refuses if HEAD is not at the tag; logs in `$LOGS`, default
   `~/.cache/hs-release`). It deletes each folder's `generators_output/generated/hypersomnia_version.cpp` first -
   an old one baked 3.0.0 into a 4.0.0 web build - and prints the `commit_tag` each build got: check it is X.Y.Z.
   Watch CI meanwhile: `scripts/wait_ci.sh <sha>`.
5. **Sign and upload** (the user - a Trezor): `deploy_last_builds` (the admin shell repo's `sign_and_upload_last_builds
   --set-to-latest`, then `server_update` only if the upload worked). hypersomnia.io first, Steam last; steamcmd may
   need retries. Fallback: `sign_deploy_last_builds`.
   A failed upload (e.g. a mistyped Steam password - steamcmd asks for it mid-run) skips `server_update`; run it
   all again from the already downloaded binaries (`/tmp/hypersomnia-deployment`) instead of downloading anew:
   `(cd <admin shell repo> && STEAM_LOGIN=geneotech UPLOAD_TO_STEAM=1 SKIP_DOWNLOADING=1 ./sign_and_upload_last_builds --set-to-latest) && server_update`.
   Without Steam: `UPLOAD_TO_STEAM=0`.
6. **The hub** (the site's host, it runs a game server too): anything the new game changes on disk that the site reads
   (e.g. 4.3.0 moved `downloads/arenas` to `downloads/maps`, which `hosting/arenas` points at) - stop the game there
   before its self-update, run the playbook's `data` tag, update, start. Then check that its game folder was not
   emptied by the self-update (4.0.0 and 4.1.0 did): restore from the site's `hosting/builds/X.Y.Z`,
   `systemctl reset-failed`, start the masterserver and the game.
   Cause (fixed 2026-10-21 in the admin shell's `remote-shell/update_hypersomnia`): both services (masterserver and
   game server) run the same AppImage and updated themselves at once into the same `.new` file. Now `server_update`
   downloads into `.server_update`, checks `--version`, swaps, then restarts. `sync_repo` rsyncs the admin shell's
   working tree - keep that fix in it (commit it).
   Check: `ssh arena 'bash -s' < scripts/hostcheck.sh` - version X.Y.Z, both processes with a fresh uptime.
7. **Deploy the site's pages**: `playbooks/homepage.yml` (or `site.yml --tags homepage`).
8. **The other official servers**: `server_update` updates only the hub. US, NL and DE: `scripts/update_other_hosts.sh
   X.Y.Z` (md5 against the hub's `hosting/builds/X.Y.Z`, swap, restart; DE has no sudo - SIGINT to the game process).
   Then all 10 official servers ([PL], [DE], [NL], [US] Central, each also as "R") on the new version:
   `curl -s https://masterserver.hypersomnia.io:8420/server_list_json` - field `server_version`. Community servers
   (e.g. [US] New York) update on their own.
9. **Steamworks**: set the new BuildID live.
10. **Docker**: `ghcr.io/teamhypersomnia/hypersomnia-server:X.Y.Z` and `:latest` exist (Docker_release.yml waits for
    the deployed AppImage).
11. **Web, after the natives**: `deploy_web` (play.hypersomnia.io), `deploy_web_itch` (butler; from `build/current` =
    release-web-itch), `cgdrag` (CrazyGames; `build/current` -> release-crazygames). All three read `build/current`:
    repoint it (`ln -sfn Release-Web-clang-<suffix> build/current`) before each, and leave it on release-crazygames
    for the user's `cgdrag`. Check: md5 of the 4 files on play.hypersomnia.io against the local build;
    `butler status Hyperdev/Hypersomnia:html` shows X.Y.Z processed.

## After

- Maps uploaded with the release (or a map changed on the hub): servers download maps only at start - restart all
  official servers: `~/rep/servers/restart_all.sh` (the servers' config repo).
- Note the release in the release decisions file (`~/hypersomnia-phase3-decisions.md`, a "X.Y.Z - RELEASED" line).

- Smoke: the server list in the game and in the browser, a ranked match report on the site, the generator opens.
- Back up the site's DB (`dbsyncback`) - before any test on production that sends ranked reports, and after the
  first real matches.
- Production tests of the server setup scripts: on arenaus only, in a test folder, never touching the production
  processes (check their uptime after); test servers off the public list afterwards.

## Pitfalls that already happened

- A submodule commit not pushed: every CI job failed at checkout (4.3.0, yojimbo).
- The web build not compiling (4.3.0): found only because it was built before the tag.
- `/tmp` full of test runs (inodes): every command fails with ENOSPC - the scratchpad's test folders are throwaway.
- The hub's game folder emptied by the self-update (4.0.0, 4.1.0).
- Steam upload stalled ("No progress while uploading chunks") and the rest ran on: upload first, servers after.
- A web build with an old version file (4.0.0 showed 3.0.0): fixed by `build_webs.sh` deleting it.
- A mistyped Steam password (4.2.0): rerun with `SKIP_DOWNLOADING=1`, servers were not touched.
- `server_update` thought to update all servers: it updates only the hub (4.0.0-4.2.0) - `update_other_hosts.sh`.
- The site deployed before the game: the generator wrote commands the old game didn't understand - pages after the
  game, only what the new game needs (nginx) before.

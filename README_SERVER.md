# Host a Hypersomnia server

**[Use the config generator](https://hypersomnia.io/generator)** - it writes everything for your setup: the config, the commands, the ports to open. This page is the reference for when something doesn't work, and for what the generator doesn't cover.

- [Troubleshooting](#troubleshooting)
- [Ports](#ports)
- [Config files](#config-files)
- [Environment variables](#environment-variables)
- [Admins](#admins)
- [Tournaments](#tournaments)
- [Several servers, ranked](#several-servers-ranked)
- [The server key](#the-server-key)
- [Command line flags](#command-line-flags)

# Troubleshooting

**The server isn't on the [server list](https://hypersomnia.io/servers).**
Look in the log for errors. The server must reach `masterserver.hypersomnia.io` (outgoing UDP `8430`); `notified_server_list` must not be changed in the config.

**It's on the list, but nobody can join.**
The UDP ports are closed - open them in the system's firewall and in the cloud provider's panel (AWS, Hetzner, OVH...). Browser players need `9000` too.

**Players with the new version can't join.**
The server is outdated. It updates on every start and every day at 03:00 (`--daily-autoupdate`; Docker by default) - restart it to update right away.

**I changed `conf.d/`, nothing changed.**
The setting was changed from the panel before - see [the order](#the-order).

**Docker: `Permission denied` in the log.**
The data folder must belong to the user `999`: `sudo chown -R 999:999 /opt/hypersomnia`.

**AppImage: `fuse: failed to exec fusermount` / `Cannot mount AppImage`.**
Add `--appimage-extract-and-run`, or install `fuse` (`fuse2` on Arch).

**Where are the logs?**
On the standard output (`docker compose logs`, `journalctl --user -u hypersomnia`) and in `logs/` next to `user/`. A server that started fine logs its name and key:

```
Server name: My server
Server key: 3f9a2c... (.../.keys/server.key)
```

Still stuck? Ask on [Discord](https://discord.gg/YC49E4G).

# Ports

| Port | Protocol | For |
|---|---|---|
| `8412` | UDP | Players of the game (Steam, Windows, Linux, macOS) |
| `9000` | UDP | Players in the browser ([play.hypersomnia.io](https://play.hypersomnia.io)) |
| `8430` (outgoing) | UDP | The server list - usually allowed already |

Several servers take the next ports: `8412-8414`, `9000-9002` for three.

# Config files

The user folder: `~/.config/Hypersomnia/user/` (AppImage), `/opt/hypersomnia/user/` (AppImage as a separate user), `/opt/hypersomnia/` (Docker).

- `conf.d/*.json` - your config, in alphabetical order. The server never changes them.
- `conf.d/private.json` - `server_private`: passwords, the Discord webhook, admins. Kept apart so the rest can be shown to others; `chmod 600`. Docker compose keeps the password and the webhook in a `.env` next to `compose.yaml` instead.
- `runtime_prefs.json` - what was changed from the administration panel.
- `server_bans.json` - the bans.

All settings, with comments: [`default_config.json`](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/hypersomnia/default_config.json). The sections for a server: `server_start` (port, slots), `server` (name, arenas, map cycle, bots, rules, updates), `server_private` (never sent to players), `instances` (how many servers).

An arena is `"name"` (its default game mode) or `"name game_mode"`, e.g. `"fy_iceworld bomb_defusal"`. Community arenas ([hypersomnia.io/arenas](https://hypersomnia.io/arenas)) are downloaded the first time they're needed.

### The order

Later ones win:

1. `default_config.json` - comes with the game.
2. `conf.d/*.json`.
3. `HYPERSOMNIA_CONFIG`, then the other [environment variables](#environment-variables).
4. `runtime_prefs.json` - **the panel's changes override the files on every start.** Changed a file and nothing happened? Remove the setting from `runtime_prefs.json` (or reset it in the panel) and restart.
5. `--apply-config <file>`.

# Environment variables

| Variable | What it sets | Example |
|---|---|---|
| `HYPERSOMNIA_SERVER_NAME` | The name on the server list | `My server` |
| `HYPERSOMNIA_ARENA` | The first arena | `fy_iceworld bomb_defusal` |
| `HYPERSOMNIA_MAP_CYCLE` | The arenas that follow each other | `de_silo,de_metro` |
| `HYPERSOMNIA_MAP_CYCLE_RANDOM` | `off`: the cycle in its order, not shuffled | `off` |
| `HYPERSOMNIA_ADMINS` | [Admins](#admins) with full rights | `steam_7656119...,0x8aed41f2...` |
| `HYPERSOMNIA_PASSWORD` | Only players who know it can join | |
| `HYPERSOMNIA_SLOTS` | Players per server | `10` |
| `HYPERSOMNIA_SERVER_INSTANCES` | How many servers | `3` |
| `HYPERSOMNIA_PORT`, `HYPERSOMNIA_WEB_PORT` | The first server's ports | `8500` |
| `HYPERSOMNIA_BROWSER_PLAYERS` | `off`: no browser players, no browser ports | `off` |
| `HYPERSOMNIA_DAILY_UPDATE` | When to update every day, or `off` | `05:00` |
| `HYPERSOMNIA_TIME_ZONE` | Where the server is - happy hours follow it | `Europe/Berlin` |
| `HYPERSOMNIA_DISCORD_WEBHOOK` | Posts joins and match results to your Discord | |
| `HYPERSOMNIA_TOURNAMENT` | A [tournament](#tournaments) for this many players | `16` |
| `HYPERSOMNIA_TOURNAMENT_TEAM_SIZE` | `2` for 2v2 and so on | `2` |
| `HYPERSOMNIA_TOURNAMENT_TEAMS` | Teams set up beforehand: one per line or `;`, players by `,` | `alice, bob; carol, dave` |
| `HYPERSOMNIA_TOURNAMENT_ARENAS` | An arena per bracket round, the final last | `de_cyberaqua,de_silo` |
| `HYPERSOMNIA_CONFIG` | Anything else - JSON, like a file in `conf.d/` | |
| `HYPERSOMNIA_SERVER_PRIVATE_KEY` | The [server key](#the-server-key), instead of the file | |

# Admins

An admin is an account: in the game, `Settings → Account` → copy. `steam_...` in the Steam version, `0x...` elsewhere (a key linked to Steam is its own `0x...` account - add both). Join the server and press `Esc` for the administration panel.

The generator makes full admins. Others go into `admin_profiles` by hand, e.g. `{ "account": "0x8aed41f2...", "rcon": "RCON", "badge": "MOD" }`:

| `rcon` | Can |
|---|---|
| `MASTER_RCON` | Everything |
| `RCON` | Kick, ban, the arena, the game mode and the map cycle - not the server's settings |
| `NONE` | Nothing - only the badge |

# Tournaments

![A tournament's matches in Browse servers](docs/images/server_browser_tournament_open.png)

A knockout tournament: a server for every match, the winners go on round by round.

- Open first round: a `Match #N (any pair)` server per pair of the bracket (16 players 1v1: 8); players join any of them, the first ones there make the match - named by them once full, e.g. `[alice] vs [bob]`.
- Set up beforehand: lines 1 and 2 play each other, 3 and 4...; players set those exact nicknames (or the teams are account ids). Teams short of 2, 4, 8, 16...: the first lines advance without playing.
- Once all the players it's for came, round 1 closes by itself: whoever waits alone advances without playing, the empty servers close. Fewer came? `Close round 1` (administration panel, `Maintenance`) does the same.
- Admins join any match as spectators - to watch, and to close round 1.
- After a crash, start it again with the same settings - it carries on mid-round. Once over, the same settings don't start it again; results in `tournament.completed.<time>.state.json`.
- It never updates in the middle of it.
- On a LAN everyone sees the matches together under `Ongoing Tournament`; on the public list only official servers' tournaments show so.

# Several servers, ranked

`"instances": { "casual": 1, "ranked": 2 }` - one process, consecutive ports, named `My server #1`, `#2`... (`c1`, `r1`, `r2` on the list). Restarting one restarts all.

- A ranked match starts when the warmup ends or all type `/go`, and freezes while someone is missing. It counts on the [leaderboards](https://hypersomnia.io/leaderboards/bomb-defusal) only on official servers.
- Ranked takes only signed-in players, at most 2 per IP (`max_ranked_players_per_ip`); keep the clock right (`chrony` / `systemd-timesyncd`) or sign-ins fail.
- Settings for ranked only: `"instance_vars": { "ranked": { ... } }`.
- Panel changes on `c1` apply to all, on `r1` to the ranked ones, elsewhere until the next restart.

# The server key

`.keys/server.key` in the user folder, made on the first start. Players know your server by it.

> [!WARNING]
> **Back it up and keep it secret.** Without it a reinstalled server is a new server to the players; with it anyone can pose as yours.

- Print the public key: `--print-server-key`. All servers of one machine share it.
- Docker without a volume: pass it in `HYPERSOMNIA_SERVER_PRIVATE_KEY`.
- **Official servers**: send us the output of `--print-server-key` on [Discord](https://discord.gg/YC49E4G).

# Command line flags

| Flag | What it does |
|---|---|
| `--daily-autoupdate` | Update every day at 03:00 if there's a new version |
| `--as-service` | For systemd: exit on restart, systemd starts it again |
| `--appdata-dir <dir>` | Keep `user/`, `logs/`, `cache/` in `<dir>` |
| `--apply-config <file>` | One more config file, applied last |
| `--tournament <players>` | A [tournament](#tournaments) for this many players |
| `--tournament-team-size <n>` | `2` for 2v2 and so on |
| `--tournament-teams <teams>` | Teams set up beforehand, e.g. `"alice, bob; carol, dave"` |
| `--tournament-arenas <arenas>` | An arena per bracket round, e.g. `de_cyberaqua,de_silo` |
| `--print-server-key` | Print the server's public key and quit |
| `--sync-external-arenas-and-quit` | Download all community arenas and quit |

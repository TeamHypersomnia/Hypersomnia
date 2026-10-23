# Host a Hypersomnia server

**[Use the config generator (recommended)](https://hypersomnia.io/generator)**

The **config generator** will walk you step by step to a working Hypersomnia server instance - a persistent server or a tournament, with Docker or the AppImage. This page explains the rest.

A Hypersomnia server runs on any Linux machine or VPS - as a Docker container or a single AppImage file. It needs two UDP ports, updates itself, shows up on the [server list](https://hypersomnia.io/servers) on its own, and you manage it from inside the game.

- [Ports](#ports)
- [Updates](#updates)
- [Tournaments](#tournaments)
- [Advanced](#advanced)
  - [Become an admin](#become-an-admin)
  - [Configuration](#configuration)
  - [Several servers, ranked](#several-servers-ranked)
  - [The server key, official servers](#the-server-key-official-servers)
- [Troubleshooting](#troubleshooting)
- [Command line flags](#command-line-flags)

# Ports

| Port | Protocol | For |
|---|---|---|
| `8412` | UDP | Players of the game (Steam, Windows, Linux, macOS) |
| `9000` | UDP | Players in the browser ([play.hypersomnia.io](https://play.hypersomnia.io)) |
| `8430` (outgoing) | UDP | The server list: `masterserver.hypersomnia.io` - usually allowed already |

Open the first two, e.g. with `ufw`:

```sh
sudo ufw allow 8412/udp
sudo ufw allow 9000/udp
```

With a cloud provider (AWS, Hetzner, OVH...) open them in its firewall panel as well.

Other ports? Set them under `Ports` in the [config generator](https://hypersomnia.io/generator).

# Updates

The game is in active development, and players with a newer version can't join an older server - keep it updated:

- **Docker:** nothing to do. The server updates itself on every start, and every day at 03:00 (the host's time) it restarts to do so - `restart: unless-stopped` brings it back up.
- **AppImage:** it updates itself on every start, and with `--daily-autoupdate` every day at 03:00 - it starts again by itself, or systemd starts the service again.
- **A tournament** never updates in the middle of it.

Change the hour or turn it off under `Show advanced options` in the [config generator](https://hypersomnia.io/generator).

<!-- toc highlight icon=trophy -->

# Tournaments

![A tournament's matches in Browse servers](docs/images/server_browser_tournament_open.png)

One setting turns your server into a knockout tournament: a server for every match, all of them open at once, and the winners go on round by round through the bracket until one team is left. No file, no list of players.

Pick the `Tournament` tab of the [config generator](https://hypersomnia.io/generator) - it gives you the commands and the ports to open.

## How it goes

1. Players join any free match server - until it's full, it says `Match #1 (any pair)`. The first ones there make the match, and it starts, named by its players.
2. The winners go on to the next bracket round - those servers let in only them.
3. Fewer players came? `Close round 1` in the administration panel: whoever waits alone advances without playing, the empty servers close.
4. Progress is saved all the time. After a crash, start it again with the same settings (Docker does it by itself) - it carries on, mid-round. Once it's over, the same settings don't start it again; the results are in `tournament.completed.<time>.state.json` in the user folder.

On the local network (a LAN party) every player sees the matches together under `Ongoing Tournament`, as above. On the public server list only the official servers' tournaments are shown so.

## Fixed teams and pairings

To decide the teams and who plays whom yourself, choose `Set up beforehand` under `Player pairings` in the [config generator](https://hypersomnia.io/generator): one team per line, lines 1 and 2 play each other, 3 and 4, and so on. Players set these exact nicknames to be let in - or the teams can be account ids.

<!-- toc highlight icon=gear children -->

# Advanced

Admins, the config files, several servers on one machine and the server key.

## Become an admin

The server knows its admins by their accounts: an `Ed25519` key that every game generates for itself, or a Steam account in the Steam version.

1. In the game, open `Settings → Account` and click the copy icon next to your account. You get your account id:
   - `steam_7656119...` in the Steam version,
   - `0x...` with an account key (the game outside Steam, or the browser).
2. Tick `Admin account id` in the [config generator](https://hypersomnia.io/generator), paste it there and use the result.
3. Restart the server, join it and press `Esc` - the administration panel opens.

The generator makes full admins. Moderators with fewer rights go into the config by hand - in `admin_profiles` of `server_private` (in `conf.d/private.json`), e.g. `{ "account": "0x8aed41f2...", "rcon": "RCON", "badge": "MOD" }`:

| `rcon` | What the admin can do |
|---|---|
| `MASTER_RCON` | Everything: all settings, restarts, kicks and bans. |
| `RCON` | Kick, ban, change the arena, the game mode and the map cycle - but not the server's settings. |
| `NONE` | Nothing - only the `[ADMIN]` badge (or `badge`) next to the name. |

> [!NOTE]
> A `steam_...` account is an admin only when it joins from the Steam version. A key linked to Steam (the game outside Steam, the browser) is its own `0x...` account - add both to be an admin from either.

The panel's `Players` tab kicks and bans by account. Bans are kept in `server_bans.json` in the user folder - you can edit it by hand.

## Configuration

The complete reference of all settings, with comments: [`default_config.json`](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/hypersomnia/default_config.json). For a server, these sections matter:

| Section | What's in it |
|---|---|
| `server_start` | `port`, `slots` |
| `server` | Name, arenas, map cycle, bots, game rules, web clients, updates |
| `server_private` | Admins, password, Discord webhook - never sent to players |
| `instances` | How many casual and ranked servers to run - see [Several servers](#several-servers-ranked) |

### Where the config goes

- **Docker:** [environment variables](#environment-variables) in `compose.yaml` or the `docker run` command, or a `.json` file in `/opt/hypersomnia/conf.d/`. Apply it with `docker compose up -d` or `docker restart hypersomnia-server`.
- **AppImage:** `~/.config/Hypersomnia/user/conf.d/server.json` (`/opt/hypersomnia/user/conf.d/` with a separate user). Apply it by starting the server again, or `systemctl --user restart hypersomnia`. The [environment variables](#environment-variables) work too - as `Environment=` lines in the service.

Any number of `.json` files can go into `conf.d/` - they apply in alphabetical order. The server never changes them.

`server_private` - passwords, webhooks, admins - goes apart from the config, so it can be shown to others when asking for help: into `conf.d/private.json` readable only by the server (`chmod 600`), or for Docker compose into a `.env` next to `compose.yaml` (`chmod 600 .env`), which fills in its `${HYPERSOMNIA_PASSWORD}` (passwords and webhooks; admins stay in `compose.yaml`). The [config generator](https://hypersomnia.io/generator) writes them so.

### Environment variables

Every field of the [config generator](https://hypersomnia.io/generator) has its own variable - the generator writes them for Docker:

| Variable | What it sets | Example |
|---|---|---|
| `HYPERSOMNIA_SERVER_NAME` | The name on the server list | `My server` |
| `HYPERSOMNIA_ARENA` | The arena, and the game mode after a space if not its default | `fy_iceworld bomb_defusal` |
| `HYPERSOMNIA_MAP_CYCLE` | The arenas that follow each other | `de_silo,de_metro` |
| `HYPERSOMNIA_MAP_CYCLE_RANDOM` | `off` plays the cycle in its order - by default the arenas after the first one come in random order | `off` |
| `HYPERSOMNIA_ADMINS` | [Admins](#become-an-admin) with full rights | `steam_7656119...,0x8aed41f2...` |
| `HYPERSOMNIA_PASSWORD` | Only players who know it can join | |
| `HYPERSOMNIA_SLOTS` | Players per server | `10` |
| `HYPERSOMNIA_SERVER_INSTANCES` | How many servers, on consecutive ports | `3` |
| `HYPERSOMNIA_PORT`, `HYPERSOMNIA_WEB_PORT` | The first server's ports, `8412` and `9000` by default | `8500` |
| `HYPERSOMNIA_BROWSER_PLAYERS` | `off` - no players in the browser, no browser ports | `off` |
| `HYPERSOMNIA_DAILY_UPDATE` | When to update every day, or `off` | `05:00` |
| `HYPERSOMNIA_TIME_ZONE` | Where the server is, for happy hours | `Europe/Berlin` |
| `HYPERSOMNIA_DISCORD_WEBHOOK` | Posts who joined and the match results to your Discord | |
| `HYPERSOMNIA_TOURNAMENT` | A [tournament](#tournaments) for this many players | `16` |
| `HYPERSOMNIA_TOURNAMENT_TEAM_SIZE` | `2` for 2v2 and so on | `2` |
| `HYPERSOMNIA_TOURNAMENT_TEAMS` | The teams set up beforehand: one per line (or separated by `;`), players separated by commas | `alice, bob; carol, dave` |
| `HYPERSOMNIA_TOURNAMENT_ARENAS` | An arena for each bracket round | `de_cyberaqua,de_silo` |
| `HYPERSOMNIA_CONFIG` | Anything else - the whole config as JSON, like a file in `conf.d/` | |
| `HYPERSOMNIA_SERVER_PRIVATE_KEY` | The [server key](#the-server-key-official-servers), instead of the file | |

They override `conf.d/` and `HYPERSOMNIA_CONFIG`.

### A typical config

```json title="conf.d/server.json"
{
  "server_start": {
    "port": 8412,
    "slots": 10
  },

  "server": {
    "server_name": "[DE] Berlin",

    "arena": "de_cyberaqua",
    "cycle": "LIST",
    "cycle_list": [ "de_cyberaqua", "de_silo", "fy_minilab bomb_defusal", "de_metro" ]
  },

  "server_private": {
    "server_password": "", // set it to let in only those who know it
    "discord_webhook_url": "", // posts who joined and the match results to your Discord
    "admin_profiles": [
      { "account": "steam_76561198000000000", "rcon": "MASTER_RCON" }
    ]
  }
}
```

An arena is written as `"name"` (its default game mode) or `"name game_mode"`, e.g. `"fy_minilab bomb_defusal"`.
Official arenas come with the game. Community arenas ([hypersomnia.io/arenas](https://hypersomnia.io/arenas)) are downloaded by the server the first time it needs them - players get them from the same catalogue when they join.

### Changes made in the panel come first

> [!WARNING]
> Whatever you change in the administration panel is saved to `runtime_prefs.json` in the user folder, and it **overrides** `conf.d/` on every start.
> If you change a setting in `conf.d/` and nothing happens, it was changed from the panel before: remove it from `runtime_prefs.json` (or reset it in the panel) and restart.

The full order, later ones win:

1. `default_config.json` - comes with the game.
2. `conf.d/*.json` - in alphabetical order.
3. `HYPERSOMNIA_CONFIG` - as if it were the last file in `conf.d/` - and then the other [environment variables](#environment-variables).
4. `runtime_prefs.json` - the changes made from the panel.
5. The file given with `--apply-config`, if any.

## Several servers, ranked

One process can run several servers - set how many under `Server instances` in the [config generator](https://hypersomnia.io/generator). Ranked ones go into the config by hand, e.g. `"instances": { "casual": 1, "ranked": 2 }`.

- They take consecutive ports: the casual one at `8412`, the ranked ones at `8413-8414` - and in the browser `9000`, `9001-9002`. Open all of them (the generator lists them).
- They're named `My server #1`, `#2`... and labelled `c1`, `r1`, `r2` on the server list.
- A ranked server plays matches: a match starts when the warmup ends, or once all players type `/go` - with at least one player in each team. When someone drops out, the match freezes until they come back.
- Ranked matches count on the [leaderboards](https://hypersomnia.io/leaderboards/bomb-defusal) only on [official servers](#the-server-key-official-servers). On your own server they are still good for a match with friends that waits for whoever drops out.
- Settings just for the ranked servers go into `instance_vars.ranked`, e.g. `"instance_vars": { "ranked": { "friendly_fire": true } }`.

In Docker, map the ports as ranges:

```yaml
ports:
  - '8412-8414:8412-8414/udp'
  - '9000-9002:9000-9002/udp'
```

Restarting one of them from the panel restarts all of them. Changes made in the panel on `c1` are kept for all of them, on `r1` for the ranked ones; on the others they last only until the next restart.

## The server key, official servers

On its first start the server makes its key: `.keys/server.key` in the user folder - `/opt/hypersomnia/.keys/server.key` with Docker, `~/.config/Hypersomnia/user/.keys/server.key` with the AppImage. Players know your server by it - their favorites remember it, and their connections are encrypted with it.

> [!WARNING]
> **Back up `server.key` and keep it secret.** A server reinstalled without it is a new server to the players. Anyone who has it can pose as your server.

- Moving to another machine: copy `server.key` over.
- In Docker without a volume, pass the key in `HYPERSOMNIA_SERVER_PRIVATE_KEY` (the one line from `server.key`), or a new one is made every time the container is recreated.
- Print the public key with `./Hypersomnia-Headless.AppImage --print-server-key`.
- All the servers of one machine share the key. Servers in different regions should have different keys, and servers that need different passwords need different keys.

**Official servers** are hosted by volunteers together with the developers: send us the output of `--print-server-key` on [Discord](https://discord.gg/YC49E4G). Your server then shows as official, and its ranked matches count on the [leaderboards](https://hypersomnia.io/leaderboards/bomb-defusal). Nothing secret changes hands.

Good to know for ranked servers:
- They take only players signed in with an account (Steam, CrazyGames or an account key) - at most 2 from one IP address (`max_ranked_players_per_ip`).
- Keep the machine's clock right (`systemd-timesyncd` or `chrony`) - a player's sign-in is valid for 2 hours, so a clock that is off turns players away.
- Happy hours (2x MMR, Friday to Sunday 19:00-21:00) follow the machine's time zone, or `Time zone` under `Show advanced options` in the [config generator](https://hypersomnia.io/generator). A machine in UTC (most VPSes and containers) has none until you set one.

# Troubleshooting

**The server isn't on the server list.**
Look in the log for errors. Check that the server can reach `masterserver.hypersomnia.io` (UDP `8430`), and that `notified_server_list` isn't changed in your config.

**It's on the list, but nobody can join.**
The UDP ports are closed: open them in the system's firewall and in the cloud provider's panel - see [Ports](#ports). Browser players need `9000` too.

**I changed `conf.d/`, nothing changed.**
The setting was changed from the panel before - see [Changes made in the panel come first](#changes-made-in-the-panel-come-first).

**Docker: `Permission denied` in the log.**
The data folder must belong to the user `999`: `sudo chown -R 999:999 /opt/hypersomnia`.

**AppImage: `fuse: failed to exec fusermount` / `Cannot mount AppImage`.**
Add `--appimage-extract-and-run`, or install `fuse` (`fuse2` on Arch) and tick `FUSE installed?` in the [config generator](https://hypersomnia.io/generator).

**Players with the new version can't join.**
The server is outdated - see [Updates](#updates). Restart it to update right away.

**Where are the logs?**
On the standard output (`docker compose logs`, `journalctl --user -u hypersomnia`), and in the `logs/` folder next to `user/`. A server that started fine shows its name and key there:

```
Server name: My server
Server key: 3f9a2c... (.../.keys/server.key)
```

Still stuck? Ask on [Discord](https://discord.gg/YC49E4G).

# Command line flags

| Flag | What it does |
|---|---|
| `--daily-autoupdate` | Update every day at 03:00 if there's a new version. Same as `"daily_autoupdate": true`. |
| `--as-service` | For systemd: when the server restarts (e.g. to update), it just exits and systemd starts it again. |
| `--appdata-dir <dir>` | Keep everything (`user/`, `logs/`, `cache/`) in `<dir>` instead of `~/.config/Hypersomnia`. |
| `--apply-config <file>` | One more config file, applied after all the others. Only one. |
| `--tournament <players>` | Run a [tournament](#tournaments) for this many players. |
| `--tournament-team-size <n>` | `2` for 2v2 and so on. |
| `--tournament-teams <teams>` | The teams set up beforehand, e.g. `"alice, bob; carol, dave"` - instead of `--tournament`. |
| `--tournament-arenas <arenas>` | An arena for each bracket round, e.g. `de_cyberaqua,de_silo`. |
| `--print-server-key` | Print the server's public key and quit. |
| `--sync-external-arenas-and-quit` | Download all community arenas (<100 MB) and quit. |

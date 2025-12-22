# Hypersomnia dedicated server guide 

You can either use the *headless* server [`AppImage`](#manual-appimage-setup) or the [docker container](#docker-setup). 

The server `AppImage` is known to work on Ubuntu 22.04 or later.

Check out this [handy script to quickly deploy the `AppImage` as a service](https://git.libregaming.org/hyperdev/Gameserver/src/branch/main/scripts/deploy_hypersomnia.sh), used for onFOSS events.

- [Docker setup](#docker-setup)
  * [Prerequisites](#prerequisites)
  * [Example with `docker run`](#example-with-docker-run)
  * [Example with `docker compose`](#example-with-docker-compose)
- [Manual `AppImage` setup](#manual-appimage-setup)
  * [(Optional) Download all community maps (<100 MB)](#optional-download-all-community-maps-100-mb)
  * [libfuse](#libfuse)
- [Configuration](#configuration)
  * [Ports](#ports)
  * [CLI flags](#cli-flags)
    - [--as-service](#--as-service)
    - [--daily-autoupdate](#--daily-autoupdate)
    - [--appdata-dir](#--appdata-dir)
    - [--apply-config](#--apply-config)

# Docker setup

You can build the [`Dockerfile`](./Dockerfile) yourself, or use the official image:

```bash
docker pull ghcr.io/teamhypersomnia/hypersomnia-server:latest
```

## Prerequisites

- [Docker engine](https://docs.docker.com/engine/install/) - or any OCI engine capable of running OCI containers.
- Hypersomnia server directory with permissions for the user `999`.

## Example with `docker run`

Simply run:

```bash
SERVER_DIR=/opt/hypersomnia
mkdir $SERVER_DIR
chown 999:999 $SERVER_DIR
cd $SERVER_DIR

docker run \
  --detach \
  --restart unless-stopped \
  --volume $SERVER_DIR:/home/hypersomniac/.config/Hypersomnia/user \
  ghcr.io/teamhypersomnia/hypersomnia-server:latest
```

This already creates `/opt/hypersomnia` directory with proper permissions. Any other directory may be chosen.

## Example with `docker compose`

Simply run:

```bash
wget https://raw.githubusercontent.com/TeamHypersomnia/Hypersomnia/refs/heads/master/docker-compose.yaml
docker compose up -d
```

The [latest `docker-compose.yaml`](/docker-compose.yaml) will be used, for example:

```yaml
services:
  hypersomnia-server:
    image: ghcr.io/teamhypersomnia/hypersomnia-server:latest
    pull_policy: daily
    volumes:
      - /opt/hypersomnia:/home/hypersomniac/.config/Hypersomnia/user
    ports:
      - '8412:8412/udp'
      - '9000:9000/udp'
    restart: unless-stopped
```

It is recommended to use the `latest` tag and [`pull_policy` set to `daily`](https://github.com/compose-spec/compose-spec/blob/main/spec.md#pull_policy) for the server to keep up with frequent updates.

It is currently impossible to configure the server via environment variables.

Since we're mounting a volume, the server will create the `conf.d/` directory at:

```bash
/opt/hypersomnia/conf.d/
```

This is now the folder for your [`.json` configuration files](#configuration).

# Manual `AppImage` setup

```sh
wget https://hypersomnia.io/builds/latest/Hypersomnia-Headless.AppImage
chmod +x Hypersomnia-Headless.AppImage
nohup ./Hypersomnia-Headless.AppImage --appimage-extract-and-run --daily-autoupdate > /dev/null 2>&1 &
```

This will run the server in the background.

## (Optional) Download all community maps (<100 MB)

```sh
./Hypersomnia-Headless.AppImage --appimage-extract-and-run --sync-external-arenas-and-quit
```

The maps will be saved to ``~/.config/Hypersomnia/user/downloads/arenas``.
Anyone who connects will be able to play them - upon connection, the player will download the custom map from the same catalogue (over HTTPS) or directly from your server over UDP should the map catalogue be offline.

## libfuse

Note you can skip the ``--appimage-extract-and-run`` flag if you have [fuse](https://packages.ubuntu.com/jammy/fuse) installed. It's ``fuse2`` on Arch.

``--appimage-extract-and-run`` will work out-of-the-box but involves an extraction step into ``/tmp``.

Not a big deal as the ``Hypersomnia-Headless.AppImage`` is rather small (< 30 MB) but it's something to keep in mind.

# Configuration

See [``default_config.json``](./hypersomnia/default_config.json) for the complete configuration variables reference.
- You will be interested in `server_start`, `server` and `server_private` sections.

Create a ``server.json`` file in ``~/.config/Hypersomnia/user/conf.d/`` (or in `/opt/hypersomnia/conf.d`, wherever you mounted your container). This will be your server configuration.

A good start would be:

```json
{
  "server_start": {
    "slots": 10
  },

  "server": {
    "server_name": "[VA] Vatican City",

    "sync_all_external_arenas_on_startup": true,
    "daily_autoupdate": true, // set to false for the docker build, managed by pull_policy

    "arena": "de_cyberaqua",

    "cycle": "LIST",
    "cycle_list": [
      "de_cyberaqua",
      "de_silo",
      "de_metro",
      "de_duel_practice",
      "de_facing_worlds"
    ]
  },

  "server_private": {
    // "discord_webhook_url": "https://discord.com/api/webhooks/put_your/secret_here",
    "master_rcon_password": "...",
  },

  "num_casual_servers": 1, // explained later
}
```

You can put any number of files in the `conf.d` folder - they will be applied in lexicographic (natural) order.

The files in ``conf.d/`` are *never* modified. Another config file will be created once the server starts: ``~/.config/Hypersomnia/user/runtime_prefs.json``, but it will be *both read and written to* as it contains vars changed during the server operation, like e.g. the current arena or vars tweaked through administration panel (RCON). **It will override** the changes in ``conf.d``.

To be able to access the administration panel of your server, make sure to setup ``master_rcon_password`` - . Open your game client. Setup your RCON password in ``Settings -> Client``. Then, press ``Esc`` when you're on your server to open the administration panel.

If you have a Discord server, you might want to setup a [Webhook](https://support.discord.com/hc/en-us/articles/228383668-Intro-to-Webhooks) and set `discord_webhook_url` to the webhook URL. This will notify your community whenever someone connects, as well as report all match results.

## Config loading order:

1) ``default_config.json``, comes with the game.
2) All configs inside ``~/.config/Hypersomnia/user/conf.d/``, in lexicographical order, if any.
    - This means that if the vars you later tweak from the administration panel are already specified in ``conf.d/``, *they will be overridden every time the server starts.*
3) ``~/.config/Hypersomnia/user/runtime_prefs.json``, if any.
4) Config specified by ``--apply-config`` flag, if any.

## Ports

You'll need these ports open:
- One UDP port for native clients (``8412`` is recommended).
- One UDP port for [Web clients](https://play.hypersomnia.io).
	- ``9000`` by default - multiple web clients will be multiplexed.
	  - Optionally, you can disable UDP multiplexing and use multiple UDP ports.

For example:

```json
  "server": {
    "webrtc_udp_mux": true,
    "webrtc_port_range_begin": 9000,
    // "webrtc_port_range_end": 9020 // only matters if "webrtc_udp_mux": false
  },
  "server_start": {
    "port": 8412
  }
```

With these settings, you will only need to expose UDP ports ``8412`` and ``9000``.

## Many server instances

By default there will be only one server instance and it will exactly match your ``conf.d/`` files.

However, you can easily manage multiple server instances by adding:

```json
"num_ranked_servers": 4,
"num_casual_servers": 1
```

This will result in:

![last_region_select](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/345beed2-f66b-4a8a-a507-55d108c1909e)

The instances will get incrementing ports, starting from the specified ``port`` in ``server_start``.
Analogously for the web, the ports will be incrementing by 1 starting from ``webrtc_port_range_begin``, with UDP muxing force-enabled for every instance.

It's worth mentioning, that docker compose **does not** support range mappings, so you have to explicitly list each port mapping in the `ports:` section.

For example:
```yaml
ports:
  - '8412:8412/udp'
  - '8413:8413/udp'
  - '9000:9000/udp'
  - '9001:9001/udp'
```

The server instances will also get unique server names - with added ``#1``, ``#2``, etc. suffixes.

The process first creates Ranked servers, then Casual server instances, so in case of US servers, Ranked ones will have ports ``8000-8004`` and the Casual one will be at ``8005``.

Casual server instances will be run with exactly the same configuration as Ranked instances, except their ``server.ranked.autostart_when`` variable will be set to ``"NEVER"``. This disables the entire ranked logistics.

If you restart/shutdown just one server instance from the panel, all servers will follow.
So that the runtime changes to config vars propagate to the ``config.json`` file, connect to the first Casual server to tweak them - or first Ranked server instance if there are no Casual servers. Changes to other instances will be ephemeral and only persist for the current server run.

## CLI flags

Additionally to the config vars, you can tweak the server behavior from the CLI.

### --as-service

Adjusts server restarting behavior so the server is suitable to be run as a **systemd service**.

- Prevents spawning another process when the server is about to restart (e.g. in order to apply updates).
  - A non-service process restarted itself by spawning a separate process right before shutting down.
  - In case of a systemd service however, it is the **systemd** that is responsible for restarting the service process.
- Allows to propagade all CLI flags like e.g. ``--appdata-dir`` across server restarts.
  - A non-service process was able to do this by respecifying them in the CLI of the newly spawned process.
  - A service-based server has to save its CLI flags in a temporary file in ``logs/launch.flags`` before shutting down - this file will be read on subsequent server launch. Among others, this will e.g. suppress "new server" Telegram/Discord notifications from being posted whenever the server restarts.

### --daily-autoupdate

Causes the server to update itself every 24 hours at 03:00 AM (your local time), if a newer game version is available. This flag is highly recommended so you don't have to keep up with frequent game updates (the game is in active development). You can change the autoupdate hour with ``daily_autoupdate_hour`` var in config.

### --appdata-dir

The server will use ``~/.config/Hypersomnia`` as its "AppData" folder by default - this is where it will store its ``user``, ``cache`` and ``logs`` folders.
This is important as it determines where to store your config files.

If the default ``~/.config/Hypersomnia`` folder is unavailable for some reason, ``--appdata-dir`` parameter comes to the rescue.

You can easily:

```sh
nohup ./Hypersomnia-Headless.AppImage --appdata-dir ./servers/1 --daily-autoupdate > /dev/null 2>&1 &
```

This will use ``./servers/1`` instead of ``~/.config/Hypersomnia`` and will thus apply the config files at ``./servers/1/user/conf.d/``.

### --apply-config 

Applies another config file *after* ``config.json`` and ``conf.d/``. **Can only use this flag once in the whole command.**

E.g. this:

```sh
nohup ./Hypersomnia-Headless.AppImage --apply-config ./some_config.json --appdata-dir ./servers/1 --daily-autoupdate > /dev/null 2>&1 &
```

will read:

- ``./servers/1/user/conf.d/``
- ``./servers/1/user/runtime_prefs.json``
- ``./some_config.json``

In this order.

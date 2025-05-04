# Hypersomnia dedicated server guide 

The dedicated server is known to work on Ubuntu 22.04 or later.

You should easily be able to run it on other distributions like Arch Linux.

We also have a [Dockerfile](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/Dockerfile).

Check out this [handy script to quickly deploy the server as a service](https://git.libregaming.org/hyperdev/Gameserver/src/branch/main/scripts/deploy_hypersomnia.sh), used for onFOSS events.

- [Basic setup](#basic-setup)
  * [(Optional) Download all community maps (<100 MB)](#optional-download-all-community-maps-100-mb)
  * [libfuse](#libfuse)
- [Configuration](#configuration)
  * [Ports](#ports)
  * [CLI flags](#cli-flags)
    - [--as-service](#--as-service)
    - [--daily-autoupdate](#--daily-autoupdate)
    - [--appdata-dir](#--appdata-dir)
    - [--apply-config](#--apply-config)

# Basic setup

```sh
wget https://hypersomnia.xyz/builds/latest/Hypersomnia-Headless.AppImage
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

Create a ``~/.config/Hypersomnia/user/config.force.json`` file. This will be your server configuration.

The reason the config file is called ``config.force.json`` is because it will *never* be modified and will *always* override changes to configuration done at runtime. Another config file will be created once the server starts: ``~/.config/Hypersomnia/user/config.json``, but it will be *both read and written* as it contains vars changed during the server operation, like e.g. the current arena or vars tweaked through administration panel (RCON).

To be able to access the administration panel of your server, setup ``master_rcon_password`` - see ``default_config.json`` for complete config vars reference. Open your game client. Setup your RCON password in ``Settings -> Client``. Then, press ``Esc`` when you're on your server to open the administration panel.


## Config loading order:

1) ``default_config.json``, comes with the game
2) ``~/.config/Hypersomnia/user/config.json``, if any
3) Config specified by ``--apply-config`` flag, if any
4) ``~/.config/Hypersomnia/user/config.force.json``, if any
    - This means that if the vars you later tweak from the administration panel are already specified in ``config.force.json``, *they will be overridden every time the server starts.*
5) ``~/.config/Hypersomnia/user/config.private.json``, if any
    - This one is helpful if you want to store confidential vars like API keys separately from ``config.force.json``.

## Ports

You'll need these ports open:
- One UDP port for native clients (``8412`` is recommended).
- One UDP port for [Web clients](https://hypersomnia.io).
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

By default there will be only one server instance and it will exactly match your ``config.force.json`` file.

However, you can easily manage multiple server instances by adding:

```json
"num_ranked_servers": 4,
"num_casual_servers": 1
```

This will result in:

![last_region_select](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/345beed2-f66b-4a8a-a507-55d108c1909e)

The instances will get incrementing ports, starting from the specified ``port`` in ``server_start``.
Analogously for the web, the ports will be incrementing by 1 starting from ``webrtc_port_range_begin``, with UDP muxing force-enabled for every instance.

They'll also get unique server names - with added ``#1``, ``#2``, etc. suffixes.

The process first creates Ranked servers, then Casual server instances, so in case of US servers, Ranked ones will have ports ``8000-8004`` and the Casual one will be at ``8005``.

Casual server instances will be run with exactly the same configuration as Ranked instances, except their ``server.ranked.autostart_when`` variable will be set to ``"NEVER"``. This disables the entire ranked logistics.

If you restart/shutdown just one server instance from the panel, all servers will follow.
So that the runtime changes to config vars propagate to the ``config.json`` file, connect to the first ranked server to tweak them - or first Casual server instance if there are no Ranked servers. Changes to other instances will be ephemeral and only persist for the current server run.

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
This is important as it determines where to store your ``config.force.json`` file.

If the default ``~/.config/Hypersomnia`` folder is unavailable for some reason, ``--appdata-dir`` parameter comes to the rescue.

You can easily:

```sh
nohup ./Hypersomnia-Headless.AppImage --appdata-dir ./servers/1 --daily-autoupdate > /dev/null 2>&1 &
```

This will use ``./servers/1`` instead of ``~/.config/Hypersomnia`` and will thus apply the config file at ``./servers/1/user/config.force.json``.

### --apply-config 

Applies another config file *after* ``config.json``, but *before* ``config.force.json``. **Can only use this flag once in the whole command.**

E.g. this:

```sh
nohup ./Hypersomnia-Headless.AppImage --apply-config ./some_config.json --appdata-dir ./servers/1 --daily-autoupdate > /dev/null 2>&1 &
```

will read:

- ``./servers/1/user/config.json``
- ``./some_config.json``k
- ``./servers/1/user/config.force.json``

In this order.

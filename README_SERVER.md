# Hypersomnia dedicated server guide 

The dedicated server is known to work on Ubuntu 20.04 or later.

You should easily be able to run it on other distributions like Arch Linux.

**Make sure to check out the [Dockerfile](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/Dockerfile) for an up-to-date working server setup.**

- [Basic setup](#basic-setup)
  * [(Optional) Download all community maps (<100 MB)](#optional-download-all-community-maps-100-mb)
  * [libfuse](#libfuse)
- [Configuration](#configuration)
  * [Ports](#ports)
  * [Setup folders for many server instances](#setup-folders-for-many-server-instances)

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
Anyone who connects will be able to play them - the server will send the player any custom map currently played on the server.

## libfuse

Note you can skip the ``--appimage-extract-and-run`` flag if you have [fuse](https://packages.ubuntu.com/focal/fuse) installed. It's ``fuse2`` on Arch.

``--appimage-extract-and-run`` will work out-of-the-box but involves an extraction step into ``/tmp``.

Not a big deal as the ``Hypersomnia-Headless.AppImage`` is rather small (< 30 MB) but it's something to keep in mind.

# Configuration

To be able to manage your server, remember to set the ``master_rcon_password`` in ``~/.config/Hypersomnia/user/config.force.json`` - see ``default_config.json`` for complete reference. Open your game client. Setup your RCON password in ``Settings -> Client``. Then, press ``Esc`` when you're on your server to open the administration panel.

``--daily-autoupdate`` flag causes the server to update itself every 24 hours at 03:00 AM (your local time), if a newer game version is available. This flag is highly recommended so you don't have to keep up with frequent game updates (the game is in active development). You can also set the flag in RCON settings (press F8 and go to Vars tab).

## Ports

You'll need:
- One UDP port for native clients (``8412`` is recommended).
- One or more UDP ports for [Web clients](https://hypersomnia.io).
	- ``9000-9020`` by default, but you can use just a single port with *UDP multiplexing*.

For example:

```lua
  server = {
    webrtc_udp_mux = true,
    webrtc_port_range_begin = 9000
    -- , webrtc_port_range_end = 9020 -- only matters if webrtc_udp_mux = false
  },
  server_start = {
    port = 8412
  }
```

With these values, you will only need to expose UDP ports ``8412`` and ``9000``.

## Setup folders for many server instances

The server will use ``~/.config/Hypersomnia`` as its "AppData" folder by default - this is where it will store its ``user``, ``cache`` and ``logs`` folders.

This is problematic if you want to run several server instances as a single Linux user.

``--appdata-dir`` parameter comes to the rescue!

Let's first create appdata folders for every server instance we want to run.
I recommend no more than 2 servers per vCore, with no more than 10 slots per server.

```sh
make_server_dir() {
	name="servers/$1"
	mkdir -p $name

	ln -s ~/.config/Hypersomnia/user/downloads/arenas $name/user/downloads/arenas
}

make_server_dir "1"
make_server_dir "2"
make_server_dir "3"
make_server_dir "4"
```

You can then run several server instances like this:

```sh
nohup ./Hypersomnia-Headless.AppImage --apply-config ./config.common.lua --appdata-dir ./servers/1 --daily-autoupdate > /dev/null 2>&1 &
nohup ./Hypersomnia-Headless.AppImage --apply-config ./config.common.lua --appdata-dir ./servers/2 --daily-autoupdate > /dev/null 2>&1 &
nohup ./Hypersomnia-Headless.AppImage --apply-config ./config.common.lua --appdata-dir ./servers/3 --daily-autoupdate > /dev/null 2>&1 &
nohup ./Hypersomnia-Headless.AppImage --apply-config ./config.common.lua --appdata-dir ./servers/4 --daily-autoupdate > /dev/null 2>&1 &
```

The servers will share ``config.common.lua`` and *later* apply their server-specific config in e.g. ``./servers/2/user/config.force.json``.

They will also share the community maps so as to not have to download them again.

**Make sure ./servers/2/user/config.force.json, ./servers/3/user/config.force.json etc. specify different ports!**

<a name="intro"></a>
<div align="center">

![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/8ccf0d22-317d-459d-8c19-3b4830968545)

[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)
[![Build Status](https://github.com/TeamHypersomnia/Hypersomnia/workflows/Linux%20build/badge.svg)](https://github.com/TeamHypersomnia/Hypersomnia/actions)
[![Build MacOS binary](https://github.com/TeamHypersomnia/Hypersomnia/workflows/MacOS%20build/badge.svg)](https://github.com/TeamHypersomnia/Hypersomnia/actions)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
<a href="https://discord.gg/YC49E4G"><img src="https://discordapp.com/api/guilds/284464744411430912/embed.png"></img></a>

Online shooter with relentless dynamics.  
Challenge your friend to an intense duel, or gather two clans to fight a spectacular war.  

Written in modern C++, without a game engine!
Forever free and open-source :heart: 

# Download the game

<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia-for-Windows.zip"> <img src="https://hypersomnia.xyz/images/windows_icon.svg" height=70 hspace=1> </a>
<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia.AppImage"> <img src="https://hypersomnia.xyz/images/linux_icon.svg" height=70 hspace=1 vspace=20> </a>
<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia-for-MacOS.dmg"> <img src="https://upload.wikimedia.org/wikipedia/commons/2/22/MacOS_logo_%282017%29.svg" height=70 hspace=1 vspace=20> </a>
<br undefined>

<!---
<img src="https://hypersomnia.xyz/images/windows_icon.svg" height=16 hspace=1></a> 
-->

Only 27 MB!

<!---
<img src="https://hypersomnia.xyz/images/linux_icon.svg" height=16 hspace=1></a> is built with ``-g``
-->

*Archives are [digitally signed.](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/signing_keys.h) You can [verify signatures.](https://hypersomnia.xyz/builds/latest/)*

# Watch gameplay

[![IMAGE ALT TEXT](https://img.youtube.com/vi/CHLPzZqANlM/0.jpg)](https://www.youtube.com/watch?v=CHLPzZqANlM "Video Title")

## Latest developer journals


[![IMAGE ALT TEXT](https://img.youtube.com/vi/Nlh_dyd_V7w/0.jpg)](https://www.youtube.com/watch?v=Nlh_dyd_V7w "Video Title")

[![IMAGE ALT TEXT](https://img.youtube.com/vi/URWjNtUArDo/0.jpg)](https://www.youtube.com/watch?v=URWjNtUArDo "Video Title")

</div>

# Table of contents

- [Introduction](#introduction)
- [Features](#features)
- [Tech highlights](#tech-highlights)
- [Background](#background)
- [Quick gameplay instructions](#quick-gameplay-instructions)
- [How to build](#how-to-build)
  - [Windows instructions](#windows-instructions)
  - [Linux instructions](#linux-instructions)
    - [Distribution-specific dependencies](#distribution-specific-dependencies)
    - [One-shot launch](#one-shot-launch)
    - [Detailed instructions](#detailed-instructions)
    - [File dialogs integration](#file-dialogs-integration)
  - [MacOS instructions](#macos-instructions)
- [Contributing](#contributing)

# Introduction

![Hypersomnia_discordthread](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/1365f4a0-a517-4257-8148-8d01c3231635)

*Hypersomnia* is a competitive arena released as free software.

The game has been online and playable since 2017. It brings together:

- the tactics of *Counter-Strike*, 
- the dynamics of *Hotline Miami*,
- the pixel art nostalgia of oldschool RPGs..
- ..and the potential for endless creativity thanks to an in-game map editor!

*Hypersomnia* aims to be **the ultimate open-source 2D shooter** - a grand community project extensible without limit.

Declare allegiance to one of the three factions whose apple of discord is a disparity between prevailing notions of moral excellence:

**Metropolis. Atlantis. Resistance.**

Will you take revenge for the unethical simulation of an inferior universe?
Will you support the cruel experiments to win total control over metempsychosis? 
Or will you join the underground civilization that awaits the end of war in this dangerous afterlife reality?

# Features

- **[24 unique firearms!](https://hypersomnia.xyz/weapons)**
  - And an extra 4 grenade types, 7 melee weapons as well as 6 magic spells!
- **[10 community maps](https://hypersomnia.xyz/arenas)** and counting!
- **2 game modes**: *Bomb defusal* and *Gun game*.
- An **in-game map editor** that lets you host a **work-in-progress map** to *instantly* play it with your friends, **even behind a router!**

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/1744ed72-612a-4af1-afbc-1a9d0c5750b3)

  - Your friends will **automatically download your map** with all its custom resources!

    *(like in CS 1.6)*

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/daeb96d1-0ea5-41c6-89f8-3ef25013570d)


# Tech highlights

- **[rectpack2D,](https://github.com/TeamHypersomnia/rectpack2D) written for packing textures, became famous and was used in [Assassin's Creed: Valhalla.](https://www.youtube.com/watch?v=2KnjDL4DnwM&t=2382s)**
  - Used also by [a drone manufacturing company](https://pages.skydio.com/rs/784-TUF-591/images/Open%20Source%20Software%20Notice%20v0.2.html) and in [2 scientific papers.](https://scholar.google.com/scholar?hl=en&as_sdt=0%2C5&q=teamhypersomnia&btnG=)
- My Entity-Component-System **[idea from 2013](https://gamedev.stackexchange.com/questions/58693/grouping-entities-of-the-same-component-set-into-linear-memory/)** describes techniques similar to **[Unity Engine patent from 2018.](https://patents.google.com/patent/US10599560B2/en)**
  - **[See this discussion.](https://github.com/TeamHypersomnia/Hypersomnia/issues/264)**
- Networking is based on **cross-platform simulation determinism**. 
  - This technique is traditionally used by RTS games with hundreds of continuously moving soldier units. 
    - It is impractical to continuously update every single one of them through the network. 
    - Instead, only the player inputs are transmitted ("I moved mouse here", "I pressed this button") - the clients simulate *everything else* locally, on their own. Think playing chess with your friend over the phone. You won't ever say aloud the entire state of the chessboard, only the movements ("Queen to H5").
  - But *Hypersomnia* is not an RTS - since it's physics-based, it uses *floats* heavily, not just *integers*.
    - When floating point calculations are involved, simulation determinism becomes **extremely hard.**
      - See why in this [excellent article by Glenn Fiedler.](https://gafferongames.com/post/floating_point_determinism/)
    - To achieve it in *Hypersomnia*, I had to: 
      - Use the same compiler - ``clang`` - on **all** OSes. It's very nice of LLVM to be ieee754-compliant by default.
      - Replace all math functions like ``std::sin``, ``std::sqrt`` by these from [STREFLOP.](https://nicolas.brodu.net/programmation/streflop/index.html)
      - ..after which ``streflop::sqrt`` became a huge bottleneck - thankfully, I found another efficient [ieee754-compliant implementation.](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/3rdparty/streflop/libm/flt-32/e_sqrtf.h)
  - Apart from worrying about floats.. 
    - I had to watch out even when iterating ``std::unordered_map`` - often replacing them with deterministically ordered ``std::map``.
    - I had to use a [portable RNG (xorshift)](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/augs/misc/xorshift.hpp), ditching the entire ``<random>`` header (its implementation differs across OSes).
  - Another magical trick was needed to ensure the **physics itself is fully deterministic:**
    - Problem: when a new client connects, it receives the initial physics state - but the packets contain just the body positions, rotations and velocities.
    - In particular, it doesn't contain the "hot" physics state such as the already tracked contacts, or the already built/balanced quadtree for fast collision detection.
    - What if there are already some players on the server, and several bodies overlap the moment a new player joins?
    - The new client will recreate the entire internal physics state on its own, using just the pos/vel/rotation data, but the contacts might be created in a different order than that of the existing clients! The fresh quadtree might end up completely different too from the one that has already seen many insertions/deletions during a long gaming session - so it might later report collisions in different order.
    - My solution was one of the greatest stroke of epiphany I ever experienced:
      - When a new client connects, **force already connected clients to completely rebuild the physics state from the positions, rotations and velocities as if they have just connected themselves.** *drops the mic*
      - As long as building the internal physics data from pos/vel/rotation data is deterministic, the in-game clients will have the *exact same state* as the newly connected client.
      - I apply the same principle in many other areas of networking. Throughout my codebase, I call this process *[cache reinference.](https://wiki.hypersomnia.xyz/reinference)*
  - All this would be very hard to pull off in a commercial engine, or straight impossible in a closed source one.
  - Yet in *Hypersomnia*, you can have thousands of dynamic crates or bullets on the map - as long as there are e.g. two player-controlled characters, the traffic will be around ``40 kbit/s (= 5 KB/s)`` under a tickrate of 60 Hz. **Only player-controlled characters** contribute to traffic. 
    - Even the silliest details like bullet shells are fully synchronized *without* paying for it with network traffic.
  - As a bonus, lag is hidden exceptionally well as the game doesn't just mindlessly "extrapolate" frames visually - rather, it simulates the entire game world forward *offscreen* to accurately predict the future.
  - For the very curious - here's an [article from my old abandoned blog](https://teamhypersomnia.github.io/newscast/full-blown-networking-with-latency-hiding-for-physics), showing how I achieved this as early as in 2016.
- You can host a working game server *from the main menu* - the game is able to **forward ports out of the box!**
  - The algorithm requires a third party server - in this case, the masterserver (server list keeper) facilitates the traversal.
  - Very simple in principle. Using Google STUN servers, we first detect the NAT type for both the client and the server (conic, symmetric or no NAT), as well as the difference in ports between packets outgoing to two different addresses ("port delta").
  - Both parties send packets to the masterserver which in turn relays what was the last external port ``P`` of the other party.
  - Both parties then spam e.g. 10-20 UDP packets at ports ``P + delta * n``.
  - Even a pair of symmetric NATs will form a connection if they have a deterministic port delta - although after a while. The connection will fail if either the client or the server has a symmetric NAT with randomized port selection. In practice however, most routers are conic which makes punchthrough work instantly.
- Cute fish and insect AI **[with flocking behaviors.](https://www.youtube.com/watch?v=0vlUOO5l0jw)** Full source: [movement_path_system.cpp.](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/game/stateless_systems/movement_path_system.cpp)
  - These too are synchronized through the network!
    - Other players see fish and insects in the same positions, even though they **do not contribute to network traffic.**
  - Fish and insects react to shots and explosions!
    - And once they're scared, they keep closer to members of the same species.
- Anyone can host **[the entire *Hypersomnia* server infrastructure.](https://github.com/TeamHypersomnia/Hypersomnia-admin-shell)**
  - The Editor, the game server *and* the masterserver (server list keeper) are **all embedded in the same game executable**, on every OS. 
  - You could run a separate server list for your own community around a completely modded version of *Hypersomnia*!
- Memory pool [implementation](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/augs/misc/pool/pool.h) with:
  - **Contiguous storage**.
    - All game objects in existence are kept **linearly in memory** (in a simple ``std::vector``).
      - It means blazing-fast iteration of all game objects of the same type, as well as trivial pool serialization.
  - **O(1)** allocation (mostly a glorified ``push_back``).
  - **O(1)** free (via ``std::swap`` with the last element and ``pop_back`` - a very known idiom).
  - **O(1)** dereference.
    - The *only* caveat being: a dereference involves **four** reads from memory (instead of just **one** with a direct pointer and **two** with a pointer + array index):
    - It has to first read the ``indirection_array`` pointer **(1)**.
      - The indirection array contains the current index of the requested object within the vector of all allocated objects.
      - This index is located at ``indirection_array[identifier]`` **(2)**.
    - Then it reads the actual ``objects`` vector pointer **(3)**.
    - Finally, it reads the object itself at ``objects[indirection_array[identifier]]`` **(4)**.
    - (Note that the ``objects`` and ``indirection_array`` will usually be cached already so subsequent dereferences will be reduced to two fetches from RAM at most).
    - This works because ``indirection_array`` is kept up-to-date whenever objects are allocated or removed.
  - **Fully resizable**. You don't have to pass a maximum pool size. It will expand on its own.
  - **Fully deterministic.** Given the same sequence of allocations and frees starting from a given initial pool state, the allocations will produce the same integer identifiers and objects will have exactly the same order in the linear memory.
    - When a client connects and the server sends it the initial world state, it includes the entire **internal state** of the game object pools.
      - It means the pools themselves are *a part of the game simulation* - I don't have to send any "creation" notification events through the network whenever entities are created as the clients **deterministically simulate allocations on their own** resulting in identical object identifiers as well as their orders in memory.
  - **Perfectly undoable allocations and frees.** 
    - The Editor requires all commands to implement a deterministic undo/redo, especially ones that create or delete a resource.
    - Suppose you create a custom Material in the editor and it now has an integer identifier ``I``.
      - You later set this Material to some walls on the scene. The walls now refer to identifier ``I``.
      - Now you want to undo all the way back to before you even created the Material.
      - The Material gets deleted from the pool.
      - Now you actually want to *redo* all of these commands back.
      - You first redo the Material allocation command.
      - But if the pool uses a non-undoable allocation scheme, the material could now be allocated with the id ``I+1``.
      - This means that the subsequent commands (that alter the walls material) will set an identifier that is now invalid - ``I``.
      - To solve this problem, my memory pool implementation provides ``undo_last_allocate`` apart from the standard ``free``. It's used by the commands that have to undo creating a resource (which means deleting it). It will free the object in a way that the next allocation will result in an identical integer id and internal pool state (with allocated objects order) as before freeing the resource. It is an optional feature for the sake of editor commands that is never used during actual gameplay. Analogously, there is an ``undo_last_free``.
- Built-in self-updater: the game will download and apply updates automatically.

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/0479fca0-f836-4f6a-b5a5-dc2317bb4b95)

    - Not only that, [it will verify](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/application/main/verify_signature.h) that the update came from the hardcoded [developer public key](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/signing_keys.h), with a call to ``ssh-keygen``.
      - If the [build hosting](https://hypersomnia.xyz/builds/latest/) were hacked and a malicious game version uploaded, the existing game clients **will refuse** to apply the updates.
- ..and I'm signing builds **offline with a [Trezor hardware wallet](https://github.com/TeamHypersomnia/Hypersomnia-admin-shell/blob/master/developer_sign_file.sh).**
  This is how *every* update looks like on my end:

  https://github.com/TeamHypersomnia/Hypersomnia-admin-shell/assets/3588717/c4d0764e-382f-45db-bc13-c626723e47e1

- **Discord** and **Telegram** notifications when:
  - a new game version is deployed,

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/0725d856-0b9d-47c8-a76a-ea3a887318e9) 

  - players connect,
  - a 1v1 "duel of honor" begins (auto-detected whenever there's only 1 player per faction), 

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/ca286048-989a-4289-afd0-bb991c63b8ad) 

  - community server is hosted, 

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/7c543b0c-7239-4cd9-947b-503001c7f2b6) 

  - or a match comes to an end. This includes full player stats and the MVP.

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/2cc549f4-3dc7-4b6d-aca4-85c60bb6cafc) 

- Beautifully simple JSON format for the game maps. This short json:

  ```json
  {
   "settings": {
    "ambient_light_color": [53, 25, 102, 255]
   },

   "nodes": [
    {
     "id": "floor",
     "type": "dev_floor_128",
     "pos": [640, 0],
     "size": [1536, 1536]
    },
    {
     "id": "light",
     "type": "point_light",
     "color": [255, 165, 0, 255],
     "pos": [100, 100],
     "radius": 700
    },
    {
     "id": "wall",
     "type": "dev_wall_128",
     "pos": [-384, 0],
     "size": [512, 1536]
    },
    {
     "id": "crate",
     "type": "crate",
     "pos": [64, 448]
    },
    {
     "id": "wood1",
     "type": "hard_wooden_wall",
     "pos": [192, -192]
    },
    {
     "id": "wood2",
     "type": "hard_wooden_wall",
     "pos": [561.819, 206.436],
     "size": [384, 128],
     "rotation": -15
    },
    {
     "id": "aquarium",
     "type": "aquarium",
     "pos": [1408, 0],
     "rotation": 90
    },
    {
     "id": "baka1",
     "type": "baka47",
     "pos": [698, -8]
    },
    {
     "id": "baka2",
     "type": "baka47",
     "pos": [454, -264]
    }
   ]
  }

  ```

  Instantly produces:

  ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/b108f985-fe79-4a7d-8708-4f1d1da7b93e)

- And speaking of the Editor.. 
  - It's created with the excellent [ImGui](https://github.com/ocornut/imgui).
  - You're working *directly* on the game world. 100% WYSIWYG.
  - Supports custom resources. It's enough to paste folders with PNGs, WAVs, OGGs to the map directory..
      - ..alt-tab back to the game and it will automatically pick it up!

	![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/8d5300fa-18cc-4979-9487-88f3b05a517a)

  - Supports GIFs as well! Just drag&drop them on the scene and they will be animated in-game, right out of the box!
  - It's possible to playtest a *work-in-progress map* with **a single click**:

    ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/1744ed72-612a-4af1-afbc-1a9d0c5750b3)

    - After which you instantly appear in the game's server browser:

      ![image](https://github.com/TeamHypersomnia/Hypersomnia/assets/3588717/3d8dbd53-3f84-40fe-9847-1c3ac0df1780)

    - You will enter the game as the host.
    - The connecting clients will automatically download the map in its current version with all its custom resources.
      - And they can later **create their own remake** - maps are saved in JSON, after all!
    - ESC will let you stop the session and go back to the Editor **exactly as you left it**, enabling ultra-efficient iteration cycles.
    - This is possible because the server, the game and the editor are **all within the same executable.**
    - The official Discord will also be notified that you're playtesting a map, so others can join in on the fun!

  - The Editor took well over a year to implement.
  - It paid off big time - there is now a neat [catalogue of community maps](https://hypersomnia.xyz/arenas) - every one of them downloadable.

# Background

*Hypersomnia* has been in development **since 2013** (as seen in the commit history).

It didn't take 10 years of uninterrupted coding, though - in the meantime, I worked commercially to cover my costs of living. This [cute minigame for PUBG](https://www.youtube.com/watch?v=tSP5P0QGWa4) was my last programming gig, and the proceeds allowed me to focus entirely on *Hypersomnia*.

I use a lot of 3rdparty libraries like ``Box2D`` (physics) or ``yojimbo`` (transport layer) - [everything not on this list,](https://github.com/TeamHypersomnia/Hypersomnia/tree/master/src/3rdparty) however, is written pretty much from scratch, in pure C++.

Many believe that writing games without an engine is no more than *reinventing the wheel*, or put more bluntly, a complete waste of time.

**I hope this project serves as a great testament to the opposite.**

Had I never embarked on this journey, I would have never made some of the interesting discoveries detailed in [Tech highlights section.](#tech-highlights)
Video game internals are just so vast and interdisciplinary that they have limitless potential for creative breakthroughs, and it is a waste to never even entertain the idea that some widely used solutions can be replaced by something absolutely ingenious.

# Quick gameplay instructions

From: https://hypersomnia.xyz/guide

You can reconfigure all bindings by going to Settings->Controls tab.

- WSAD: move around
- Shift - sprint, it drains stamina.
- Space - dash, it drains quite a lot of stamina.
- Left control - move quietly.
- B - open the shop. Usually, the shop is off during warmup.
- M - change team. You should just pick Auto-assign to preserve balance.
- G - drop the most recently wielded item.
- H - hide the most recently wielded item. Can fail if you have no space in your inventory!
- E - Pick up items/defuse the bomb. If you move when defusing, the timer is reset, so stay still. You can shoot, though. Just remember to keep the trigger in the bomb's range, they have to touch.
- C - pull out the bomb (as a terrorist)
- Wheel Down - quickly throw a knife (or another melee weapon you happen to have). Note you can throw a knife during any activity, even when reloading or planting a bomb.
- Wheel Up - quickly throw two knives (or other melee weapons you happen to have).
- Middle mouse button - quickly throw a Force grenade.
- Mouse4 - quickly throw a Flashbang.
- Mouse5 - quickly throw a PED grenade (destroys Personal Electricity).
- Q - quickly switch to the most recently drawn weapon - or any other at your disposal. Can fail if you have no space in your inventory for the currently held weapon! 
- 0, 1, 2, ..., 9 - choose weapons from the hotbar.
- TAB - match statistics, e.g. the players and their scores.
- F8 - admininistration panel for your server where you can change maps or restart the match.
- Tilde (~) - release the mouse cursor to let you interact with GUI. You probably won't use it often, but it's useful if you want to drop a specific item from your inventory - simply press the right mouse button whilst dragging it. Pressing the tilde again puts you back into the game and lets you regain control over the crosshair. 
- LMB: 
  - Use the item in your right hand, so:
  - for the bomb: plant it
  - for a nade: unpin it
  - for a firearm: shoot it
  - for a melee weapon: a wide swing
- RMB - Use the item in your left hand, or a secondary function of the item in your right hand (only if your left hand is unoccupied), for example - burst fire for the AO44 revolver. Also: a narrow, powerful swing for a melee weapon. Unpins the nade to throw it under your feet, instead of throwing it far.

# How to build

Currently, *Hypersomnia* is only buildable using ``clang``.
Your operating system must be 64-bit.  

<!--
Formerly, the game was buildable under modern ``gcc`` versions,  
and also using ``MSVC`` (the Microsoft's compiler shipping with Visual Studio),  
but it quickly became too much of a hassle to support these compilers as we use **modern C++ constructs** throughout the entire codebase.  
``gcc``, for example, would sometimes simply crash on some really template-heavy code.  

The project's ``CMakeLists.txt`` contains clauses for both ``MSVC`` and ``gcc``,  
so in the future, it might be possible to build the game under these compilers if they catch up to ``clang``.  
-->

Irrespectively of the OS, you will need the following software in order to build *Hypersomnia*:  

- The newest **CMake**.
- **git** to clone the respository and later generate version information.
- [**ninja**](https://ninja-build.org/) to carry out the build.
- [LLVM](https://releases.llvm.org/) toolchain version 13 or newer.
	- For Windows, you can use [this installer](https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/LLVM-13.0.1-win64.exe), or a newer one. 
	- For Linux, use your distro-specific package. Make sure to install ```libc++```, ```libc++abi``` and ```lld``` as well.
	- For MacOS, the version that comes pre-installed with **Xcode** is good enough.
- [OpenSSL](https://www.openssl.org/) needed by the auto-updater to download latest game binaries over HTTPS.
  - On Windows, you can get the appropriate installer here: https://slproweb.com/download/Win64OpenSSL-1_1_1d.exe

Once dependencies are installed, go to the directory where you wish to have your *Hypersomnia* project downloaded,
open git bash and paste:

```
git clone --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
```

The ``--recurse-submodules`` is necessary to clone the submodules as well.

Wait for the download to complete.
Next steps depend on the platform you are on.

On all platforms, you can choose among three building configurations:

- ``Debug`` - the fastest to build and provides debug information.   
	Recommended for day-to-day development.
- ``Release`` - No debug information. Use only for production builds. Specifies ``IS_PRODUCTION_BUILD=1`` C++ preprocessor define that disables assertions in performance-critical areas.
- ``RelWithDebInfo`` - Same as ``Release`` but with debug info and with many assertions ("ensures") compiled-in.
	Preferred choice for testing the developed game while full speed is required.

## Windows instructions

Refer to [appveyor.yml](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/appveyor.yml) file for up-to-date building procedure. A short overview of the process now follows.

Prerequisites:
- **Visual Studio 2022 Community** or newer.

Open up the terminal. Setup the environment:

```
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

(If the file cannot be found, it means you are not using Visual Studio 2022 Community. You will have to look for a corresponding ``vcvarsall.bat`` location on your own.)

Next, run these commands:

```
cd Hypersomnia
mkdir build
cd build
set CONFIGURATION=Release
cmake -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER=lld-link -DARCHITECTURE="x64" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DGENERATE_DEBUG_INFORMATION=0 ..
ninja
```

Note: your computer **might start lagging heavily** for the duration of the build as ``ninja`` will use all available cores for compilation.

If you intend to develop the game, it is best to use "Debug" configuration for the fastest builds.

If you want to somehow customize your build, e.g. disable certain game features, refer to the beginning of ```CMakeLists.txt``` to see which options you can pass to the ```cmake``` command.

If the game builds successfully, issue this command to launch it:

```
ninja run
```

<!-- Note that the 64-bit version is more likely to be kept up to date. -->

<!--
Resultant ```.sln``` and ```.vcxproj``` files should appear in the ```build/``` directory.
Open ```Hypersomnia.sln``` file, select **Release** configuration and hit **F7** to build the game.
**F5** should launch it.
-->

If, for some reason, some step fails, refer to the latest working Appveyor build and the relevant ```appveyor.yml``` file.

## Linux instructions

Refer to [Linux_build.yml](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/.github/workflows/Linux_build.yml) file for up-to-date building procedure - it is constantly in flux. A short overview of the process now follows.

Current platforms are actively tested and supported:

- Arch Linux with i3 window manager - the developer's machine.
- Ubuntu, as this is where the dedicated server is deployed.

#### Distribution-specific dependencies

Arch Linux:

- ``libc++ lld pkg-config libx11 libxcb xcb-util-keysyms libsodium``
- Might need more - [let us know](https://github.com/TeamHypersomnia/Hypersomnia/issues) if this list is missing something.

Ubuntu:

```
sudo apt-get install cmake ninja-build libxcb-keysyms1 libxcb-keysyms1-dev libxi6 libxi-dev alsa-oss osspd-alsa osspd libasound2 libasound2-dev p7zip p7zip-full libgl1-mesa-dev libxcb-glx0-dev libx11-xcb-dev
```

### One-shot launch

Once the dependencies are all set, this is the complete script for building and launching the game from scratch, with RelWithDebInfo configuration:

```
git clone --depth 1 --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
cd Hypersomnia
export CXX=clang++; export CC=clang;
cmake/build.sh RelWithDebInfo x64
ninja run -C build/current
```

#### Detailed instructions
 
Use your favorite shell to enter the repository's directory.
Then run:

```
cmake/build.sh [Debug|Release|RelWithDebInfo] [x86|x64] ["ADDITIONAL CMAKE FLAGS"]
```
For example:

```
export CXX=clang++; export CC=clang;
cmake/build.sh Debug x64
```
After which, the resultant ``build.ninja`` should appear in the build/Debug-x64-clang directory.

Example for gcc:

```
export CXX=g++; export CC=gcc;
cmake/build.sh Debug x64
```

After which, the resultant ``build.ninja`` should appear in the build/Debug-x64-gcc directory.

#### Invoking ninja

<!--
If you are building with ``clang``, make sure to call these exports before invoking ``ninja``:

```
export CXX=clang++; export CC=clang;
```

This is because some third-party libraries - freetype, for example - generate their Makefiles only after calling ``ninja`` on the Hypersomnia's ``build.ninja`` itself.
(if GCC build fails for some reason, add respective exports as well.)
-->

There are several additional ninja targets defined:

```
ninja run
```
Launches the game normally.

```
ninja tests
```
Launches unit tests only and exits cleanly.

The above targets set the working directory automatically to ```${PROJECT_SOURCE_DIR}/hypersomnia```.

If, for some reason, some step fails, refer to the latest working ```Linux_build.yml``` file.

If the game fails to launch, it should automatically open a log file with the relevant message using ```$VISUAL``` executable.

### File dialogs integration

You might want to follow some additional configuration to make the experience better on Linux.

#### Opening and revealing files

The *Hypersomnia* editor can reveal files in explorer.
Also, the game may prompt you to select a file, e.g. to choose the avatar.
On Windows, this is accomplished through an ``IFileDialog``.  
Needless to say, such a class does not exist on Linux.  
*Hypersomnia* provides shell scripts for common file managers in ``hypersomnia/detail/unix/managers``.  
You'll need one for choosing a file to open and one for revealing files in explorer.  
``cd`` to ``hypersomnia/detail/unix`` and, assuming you want to use ``ranger`` as your file manager, create symlinks as such:  

```
ln -s managers/reveal_file_ranger.zsh reveal_file.local 
ln -s managers/open_file_ranger.zsh open_file.local
```

None of the symlinks will be tracked by git.  

Currently, the following file managers are supported:
- [ranger](https://github.com/ranger/ranger) through ``--choosefile`` and ``--selectfile`` options

## MacOS instructions

Refer to [MacOS_build.yml](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/.github/workflows/MacOS_build.yml) file for up-to-date instructions.

# Contributing

To understand the repository's folder structure, make sure to read the [documentation](https://wiki.hypersomnia.xyz/folder_structure).

Pull requests are welcomed, should they even be typo fixes, missing const guarantees or changes in nomenclature.
If you however plan to add a completely new feature, create a relevant [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues) so that everybody knows about it,
because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.

A WIP documentation can be found at [wiki](https://wiki.hypersomnia.xyz).

Make sure to check out [TeamHypersomnia](https://github.com/TeamHypersomnia) for other repositories that are useful when setting up your own custom servers.

If you have questions or you fail to build *Hypersomnia*, create an [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues).
Or if you just can't wait to utter some brilliant ideas with regard to the game, please do so, too!

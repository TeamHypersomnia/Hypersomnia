<a name="intro"></a>
<div align="center">

# **Online 2D shooter with relentless dynamics** <br><br> [![Hypersomnia_github](https://github.com/ArtistDev44/Hypersomnia/blob/master/Hypersomnia_Github.png)](https://hypersomnia.io/)

[![Windows Build](https://github.com/TeamHypersomnia/Hypersomnia/actions/workflows/Windows_build.yml/badge.svg)](https://github.com/TeamHypersomnia/Hypersomnia/actions/workflows/Windows_build.yml)
[![Linux build](https://github.com/TeamHypersomnia/Hypersomnia/actions/workflows/Linux_build.yml/badge.svg)](https://github.com/TeamHypersomnia/Hypersomnia/actions/workflows/Linux_build.yml)
[![MacOS build](https://github.com/TeamHypersomnia/Hypersomnia/actions/workflows/MacOS_build.yml/badge.svg)](https://github.com/TeamHypersomnia/Hypersomnia/actions/workflows/MacOS_build.yml)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![Discord](https://discordapp.com/api/guilds/284464744411430912/embed.png)](https://discord.gg/YC49E4G)

*Hypersomnia* is a community driven project that aims to be **the ultimate FOSS 2D extensible shooter**.

It is written in modern C++, **without any game engine** and *comes with an [in-game map Editor!](https://hypersomnia.io/editor/official)*

[**Watch the trailer**](https://www.youtube.com/watch?v=L4zSA34fD_E)

</div>

## Contents 🔤

1. [Introduction](#introduction-)
2. [Downloads](#downloads-)
3. [Background](#background-)
4. [Tech highlights](#tech-highlights-)
5. [Links](#links-)
	1. [Project](#links-)
	2. [Community](#links-)
	3. [Game](#links-)
	4. [Hosting Downloads](#links-)
6. [Credits 🇵🇱](https://teamhypersomnia.github.io/PressKit/credits)

## Introduction 🎮

Hypersomnia has been online and playable since 2017. It brings together:

- the tactics of **Counter-Strike**,
- the dynamics of **Hotline Miami**,
- the **pixel art nostalgia** of oldschool RPGs,
- and the potential for endless creativity thanks to an **in-game map editor!**

**Today, it contains :**

- an **interactive tutorial** for new players
- **[24 unique firearms!](https://hypersomnia.xyz/weapons)**
  - And an extra 4 grenade types, 7 melee weapons as well as 6 magic spells!
- **[10 community maps](https://hypersomnia.xyz/arenas)** and counting!
- an **[in-game map editor](https://hypersomnia.io/editor/official)** 
	- that lets you host a **work-in-progress map** to *instantly* play it with your friends, **even behind a router** after your friends downloaded your map and this resources.
    *(like in CS 1.6)*

## Downloads ⏬

> only 29MB !

<div align="center">

<br>

<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia-for-Windows.zip"> <img src="https://hypersomnia.xyz/assets/images/windows_icon.svg" width=75 hspace=1></a>
<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia.AppImage"> <img src="https://hypersomnia.xyz/assets/images/linux_icon.svg" width=100 hspace=1 ></a>
<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia-for-MacOS.dmg"> <img src="https://upload.wikimedia.org/wikipedia/commons/2/22/MacOS_logo_%282017%29.svg" width=80 hspace=1></a>
<br undefined>

<a href="https://store.steampowered.com/app/2660970/Hypersomnia"> <img src="https://upload.wikimedia.org/wikipedia/commons/8/83/Steam_icon_logo.svg" width=140 hspace=1 width=80 hspace=40 vspace=20></a>

</div>

> [!NOTE]  
> *All archives are [digitally signed](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/signing_keys.h) with [verified signatures.](https://hypersomnia.xyz/builds/latest/)*


## Background 🕗

*Hypersomnia* has been in development **since 2013**.

It didn't take 10 years of uninterrupted coding, though - in the meantime, I worked commercially to cover my costs of living. I saved money to be able to work less and focus on *Hypersomnia*. My financial decisions now let me develop the game full-time.

I use a lot of 3rdparty libraries like ``Box2D`` (physics) or ``yojimbo`` (transport layer) - [everything not on this list,](https://github.com/TeamHypersomnia/Contents
Hypersomnia/tree/master/src/3rdparty) however, is written pretty much from scratch, in pure C++.

Many believe that writing games without an engine is no more than *reinventing the wheel*, or put more bluntly, a complete waste of time.

> **I hope this project serves as a great testament to the opposite.**

Had I never embarked on this journey, I would have never made some of the interesting discoveries detailed in [Tech highlights section.](###tech-highlights)
Video game internals are just so vast and interdisciplinary that they have limitless potential for creative breakthroughs, and it is a waste to never even entertain the idea that some widely used solutions can be replaced by something absolutely ingenious.

<div align="end">

*Patryk Czachurski alias* ***Pythagoras***

</div>

## Tech highlights 🖧

<sub> <a href="github.io"> long version </a>

- **[rectpack2D,](https://github.com/TeamHypersomnia/rectpack2D) written for packing textures, became famous by [Assassin's Creed: Valhalla.](https://www.youtube.com/watch?v=2KnjDL4DnwM&t=2382s)**

- Networking based on **cross-platform simulation determinism**. 
	- Only the player inputs are transmitted
    - Even the silliest details like bullet shells are fully synchronized *without* paying for it with network traffic
  - The lag is hidden exceptionally well as the game simulates the entire game world forward *offscreen* to accurately predict the future

- Memory pool [implementation](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/src/augs/misc/pool/pool.h) with:
  - **Contiguous storage**.
    - All game objects in existence are kept **linearly in memory**
      - It means blazing-fast iteration of all game objects of the same type, as well as trivial pool serialization.

### Accounts

- In the browser version, you can **sign in with Discord** and play ranked matches to compete on the [global Leaderboards](https://hypersomnia.xyz/leaderboards)!
	- You can also [associate your Discord with Steam](https://hypersomnia.xyz/profile) **sharing the same rating!**

- **Discord** and **Telegram** notifications when:
  - a new game version is deployed
  - a 1v1 "duel of honor" begins (auto-detected whenever there's only 1 player per faction),
  - community server hosted
  - full player stats and the MVP at the end of a game

### Hosting

- You can host a working game server *from the main menu* - the game is able to **forward ports out of the box!**
  - Fish and insects react to shots and explosions!
    - And once they're scared, they keep closer to members of the same species.
- Anyone can host **[the entire *Hypersomnia* server infrastructure.](https://github.com/TeamHypersomnia/Hypersomnia-admin-shell)**
  - The Editor, the game server *and* the masterserver (server list keeper) are **all embedded in the same game executable**, on every OS.
  - You could run a separate server list for your own community around a completely modded version of *Hypersomnia*!
  - **Perfectly undoable allocations and frees.**

### Editor & Play-testing

  - created with the excellent [ImGui](https://github.com/ocornut/imgui).
	  - working *directly* on the game world. 100% WYSIWYG.
 	 - Supports custom resources. It's enough to paste folders with PNGs, WAVs, OGGs and even GIF out of the box to the map directory !
   - possible to play-test a *work-in-progress map* with **a single click**:
    - You can publish your map directly as the host
	    - The connecting clients will automatically download the map in its current version with all its custom resources.
      - And they can later **create their own remake** - maps are saved in JSON, after all!
    - ESC will let you stop the session and go back to the Editor **exactly as you left it**, enabling ultra-efficient iteration cycles.
    - This is possible because the server, the game and the editor are **all within the same executable.**
    - The official Discord will also be notified that you're play-testing a map, so others can join in on the fun!

## Links 🔗

<div align="center">

***Project*** | ***Community*** | ***Game*** | ***Hosting downloads***
|---|---|---|---|
[**Contribute :heart:**](linktoanother.mddocument) | [Discord](https://discord.gg/YC49E4G) | [Build the game](newlink) | [Docker Server](https://www.docker.com/app/uploads/2022/03/Moby-logo.png)
[Website](https://hypersomnia.xyz/) | [Telegram](https://t.me/hypersomnia_io) | [Commands](linktoanew.mdfile)  | [Linux Server AppImage](https://hypersomnia.xyz/builds/latest/Hypersomnia-Headless.AppImage)
[Wiki](https://wiki.hypersomnia.xyz/index.html) | [Documentation](https://wiki.hypersomnia.xyz/docs) | [Servers](https://hypersomnia.xyz/servers)
[PressKit](https://github.com/TeamHypersomnia/PressKit/blob/main/README.md#intro) | [Tutorial map editor](https://www.youtube.com/watch?v=q1rfIy06_xo) | [Universe](https://wiki.hypersomnia.xyz/universe) | 

</div>

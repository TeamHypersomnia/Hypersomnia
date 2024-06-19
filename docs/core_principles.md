---
title: Core principles
tags: [getting_started]
summary: |
    General rationales and ideology.
permalink: core_principles
---

## Goals

- The primary goal is to have [fun](https://en.wiktionary.org/wiki/fun#Noun).

- The secondary goal is to create a [fun](https://en.wiktionary.org/wiki/fun#Adjective), fast-paced top-down game,
and when that succeeds, to extend it into an MMO with an interesting set of social mechanics.

- The tertiary goal is to have the game be enriched by diverse improvements from its fans,  
or, in other words, having an awesome and accessible community.

## Commercial usage

{% include tip.html content="The official Hypersomnia is and will always remain, regardless of its stage of development, **free for everyone** to play without charge, tamper with and host custom servers with." %}

Hypersomnia is licensed under [AGPL-3.0](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/LICENSE.md), which briefly means that you can do whatever you want with the code and other content, as long as:
- The binaries you distribute provide their complete source code.
- The custom servers you host also expose their complete source code, in one way or another.

In particular, **you can charge whatever you want** for copies of your Hypersomnia, essentially making a business out of it, provided you meet those requirements, and thus letting the community thrive.

{% include important.html content="There may come a time when, once the MMO stage is reached, an *official* kind of game server becomes a demanded feature. Needless to say, such a thing may cost lots of money. **We will make efforts to fund it from donations that do not create any privileges for the benefactors, keeping all players equal** (with maybe just a hall of fame on the homepage). If that does not succeed, the official server may introduce some harmless privileges that could be bought and would not break the game for everyone. Purchasable character skins is the most radical step we are ready to take. If that still does not succeed, the official servers will just not exist and the game will continue to develop as usual. It will then be left to you, dear fan, if you ever decide it's worth it, to come up with a business scheme that sustains a persistent world; one that works and you consider ethical." %}

{% include important.html content="There may come a time when Hypersomnia gets an official single-player campaign. In that case, the game may benefit from some licensed content, for example music. Needless to say, it might be impossible to distribute such content under the same rules as described in this section. The game will explicitly ask the user if they wish to download the licensed content in order to play the single-player campaign, and make it clear that the downloaded files are not to be used with freedoms that the rest of the game can. So that any fan does not use such content accidentally in their own copy, the licensed content will be downloaded to completely outside of the source tree, and obviously, will not be tracked by git at all. We will make efforts to ensure that **anything inside the repository itself is safe** to use under the rules described in this section." %}

## Extensibility 

Much of the codebase described here could be readily applied to just about any kind of game.  
You might find some topics like [entities](entity), [components](component) or [systems](system) defined in a rather general fashion, 
not necessarily pertaining to any particular Hypersomnia mechanic, or even to any specific game genre.  

It should be remembered though, that the [main focus](#goals) of this project **is not to create a universal game engine**.  
In particular, most of the tweaks will be possible almost exclusively by direct interaction with the game's C++ source code.
That is why, on one hand, you may find ```augs/``` to be a game-agnostic "framework", but on the other hand, you will find:
- hardcoded C++ enumerations of [render layers](render_layer) with well-defined roles;
- a very game-specific, natively coded [rendering routine](illuminated_rendering) in C++ that uses these render layers and speaks directly to OpenGL without any kind of general rendering framework;
- little to none script support (except configuration files, e.g. [``config.json``](config.json)), at least not until there is such a demand in the community.

To reach out to the non-tech-savvy audience, a full-flegded [editor](editor) is developed. 

In particular, **no scripted plugin system** is planned to ever come about.
We believe that the C++ codebase may be made so easily extendable that any fan of Hypersomnia, that is coincidentally a programming adept,
could easily add their modifications, that would in turn be reviewed by the community in order to be finally merged into the official game.

This is because:
- C++ code is way, way more easier to reason about (and thus maintain) than some obscure plugin code written in a dynamically-typed script.
- C++ is more performant than any scripting language.
- Possible conflicts between community extensions might be resolved at the compilation stage, and thus very early.

## Using external code

Although a great part of the game is hand-written, this project **is not, in fact, about writing a game from scratch**.

You can notice that there is [plenty of third-party code](https://github.com/TeamHypersomnia/Hypersomnia/tree/master/src/3rdparty) used in Hypersomnia.
At that point, you might ask, why not choose a standalone game framework that has all of this functionality built-in?  

We believe it is beneficial to manually pick the libraries that the game is proven to need at this exact moment:

- If there is no particular reason to use other audio formats than ``wav`` or ``ogg`` in the game, why also waste time compiling some other obscure formats?
- Game frameworks might not always be up-to-date with latest improvements to specific libraries, like ``OpenAL Soft`` or ``enet``.
- You are in complete control of what to build, and what to not build. It is useful if you want to iterate faster - then you can, for example, [completely exclude networking or sound from the game](cmakelists#build-settings), resulting in faster builds.
- You can learn something new! After all, you're not writing it from scratch, you're just putting a little more time to assemble your perfect toolset!


---
title: Core principles
tags: [getting_started]
summary: |
    General rationales and ideology.
permalink: core_principles
---

## Goals

The primary goal is to have [fun](https://en.wiktionary.org/wiki/fun#Noun).

The secondary goal is to create a [fun](https://en.wiktionary.org/wiki/fun#Adjective), fast-paced top-down game,
and when that succeeds, to extend it into an MMO with an interesting set of social mechanics.

The tertiary goal is to have the game be enriched by diverse improvements from its fans,  
or, in other words, having an awesome and accessible community.

## On extensibility 

Much of the codebase described here could be readily applied to just about any kind of game.  
You might find some topics like [entities](entity), [components](component) or [systems](stateless_system) defined in a rather general fashion, 
not necessarily pertaining to any particular Hypersomnia mechanic, or even to any specific game genre.  

It should be remembered though, that the [main focus](#goal) of this project **is not to create a universal game engine**.  
In particular, most of the tweaks will be possible almost exclusively by direct interaction with the game's C++ source code.
That is why, on one hand, you may find ```augs/``` to be a game-agnostic "framework", but on the other hand, you will find:
- hardcoded C++ enumerations of [render layers](render_layer) with well-defined roles;
- a very game-specific, natively coded [rendering routine](illuminated_rendering) in C++ that uses these render layers and speaks directly to OpenGL without any kind of general rendering framework;
- little to none script support (except configuration files, e.g. [``config.lua``](config_lua)), at least not until there is such a demand in the community.

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


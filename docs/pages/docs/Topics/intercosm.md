---
title: Intercosm
tags: [topics, ECS] 
hide_sidebar: true
permalink: intercosm
tip: This page is a manual targeted both for programmers and content designers.
summary: |
    An **intercosm**, a short for an *interactive cosmos*, is an object that holds a [cosmos](cosmos), [viewables](viewable) and extra information like [id](entity_id) of the [entity](entity) that is to be controlled during the game.  

    It serves a purpose that is similar to that of **game maps** in other projects.  
    It is the highest-level object that the [debugger setup](editor_setup) works with; other setups usually just read and play it to fit their purpose, e.g. a server may use it for its deathmatch session.
---

## Overview

An intercosm holds the minimum [significant state](significant_state) required to run an interactive simulation of the game.  
An [author](author) has complete control over its contents.

## File formats

An intercosm can currently be read or written to disk in two different ways:

- As an ``.int`` file, which contains a binary representation of the content, native to the machine that the intercosm was created on.
	- **Purpose:** to have a lightning-fast format for manipulation of files while they are still being worked on.
	- **Drawbacks:** having just the ``.int`` file, without having a copy of the game whose version it was originally created with, makes it impossible to port an intercosm to newer versions of Hypersomnia.
		- In practice, the game is continuously versioned, so there will always be a way to do this, unless the Hypersomnia repositories themselves are wiped out of the face of the universe.
		- Additionally, ``.int`` files are always stamped with the version they were created with (which in turn can always be read by any version of Hypersomnia), so it will be possible to know which version of the game is required to export it to a more portable format.

- As a ``.json`` file, which contains a textual, human-readable representation of the entire content.
	- **Purpose:** to have an intercosm that **may be later ported** to newer versions of Hypersomnia.
		- Note: if you save to a ``.json`` file, it does not automatically become usable by newer versions of Hypersomnia. The format itself just **makes it possible** to port an intercosm to a newer version, unlike the binary representation. Effort still must be made to possibly rename some important fields, remove unused ones, or fill in the fields that had not existed by the time of creating the intercosm.
	- **Drawbacks:** an order of magnitude slower saving and loading than a plain binary ``.int`` file.

As an [author](author), you will most often use only the ``.int`` format because of its performance, especially when targeting only a particular Hypersomnia version or mod.  

The only use-case you would have for the ``.json`` format as an author is when you actually want to port your work to a different version of Hypersomnia.  

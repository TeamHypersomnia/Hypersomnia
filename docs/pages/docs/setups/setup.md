---
title: Setup
tags: [topics] 
hide_sidebar: true
permalink: setup
summary: |
    **Setups** are the highest-level objects in the entire source code. They are allocated and used directly by [``main.cpp``](main_cpp).  
    Usually, each *setup* has a single dedicated button in the game's main menu.  

    An application may choose which setup to launch on startup via a relevant [``config.lua``](config_lua) setting.
---

## Interface

Each setup is required to expose several **``public``** member functions that the ``main.cpp`` uses to run a viable program session.
Examples:

- ``const cosmos& get_viewed_cosmos() const`` - return a reference to the [cosmos](cosmos) to be viewed in this frame.
- ``entity_id get_viewed_character_id() const`` - return an [id](entity_id) of the [entity](entity) that is used as the viewer in this frame.
- ``auto get_audiovisual_speed()`` const - get a floating-point scalar that determines the speed with which to advance all [audiovisual systems](audiovisual_system).


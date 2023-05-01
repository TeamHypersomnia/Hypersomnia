---
title: Systematic function
tags: [topics, ECS] 
hide_sidebar: true
permalink: systematic_function
note: This name, except as a part of another name, does not appear anywhere in the source code. It is only introduced for the sake of this wiki.
summary: |
    A function is called *systematic* if it operates on a [message queue](message) or if it operates on all [entities](entity) in the [cosmos](cosmos) that match a certain criterion.
---

## Introduction

Systematic functions are high-level calls that solve an abstract problem.  
Care should be taken to name them so that there is little to no doubt what kind of problem that is.  

Ideally, a systematic function should only be concerned with iteration and offload the more complex stuff to another source file, where other systematic functions are free to reuse that logic.  
For example, a function that is concerned with detonatable entities - ``demolitions_system::detonate_fuses`` - ignites explosions using ``src/game/detail/standard_explosion.h``, a header used in some other domains as well.

## Common types of systematic functions

- Iterates through a message queue and produces side effects.
    - For example, iterates through [intents](intent_message) and sets relevant flags.
        ```cpp
        movement_system::set_movement_flags_from_input
        ````
- Iterates through a message queue and generates messages to another queue.
    - For example, one that adds the children entities of deleted entities into the deletion queue.

        ```cpp
        deletion_system::mark_queued_entities_and_their_children_for_deletion
        ````

- Iterates through a processing list and produces side effects.
    - For example, one that iterates through all entities having a [sentience](sentience_component) and regenerates their [health points](health_points), [PE](personal_electricity) and [CP](consciousness_points).

        ```cpp
        sentience_system::regenerate_values_and_advance_spell_logic
        ````

## Other examples

- ```gun_system::launch_shots_due_to_pressed_triggers```
- ```sentience_system::apply_damage_and_generate_health_events```
- ```sound_system::update_sound_properties```

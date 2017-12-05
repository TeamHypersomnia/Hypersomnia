---
title: Systematic function
tags: [topics, ECS] 
hide_sidebar: true
permalink: systematic_function
note: This name does not appear anywhere in the source code itself and is only introduced for the sake of this wiki.
summary: |
    A function is called *systematic* if it processes all [entities](entity) in the [cosmos](cosmos) that match a certain criterion or if it processes a [message queue](message).
---

## Introduction

Systematic functions are the highest-level building blocks of the game logic.  
Ideally, a systematic function should only be concerned with iteration and offload the complex logic to somewhere like ``src/game/detail``,  
where other systematic functions are free to reuse that logic. 

Usually, all systematic functions are called, directly or indirectly by another systematic function, inside [```cosmos::advance_systems```](cosmos#advance-systems), every time a game steps. 

## Common types of systematic functions

- Iterates through a message queue and produces side effects.
    - For example, iterates through [intents](intent_message) and sets relevant flags.
        ```cpp
        movement_system::set_movement_flags_from_input
        ````
- Iterates through a message queue and generates events to another message queue.
    - For example, one that queries children entities of deleted entities into the deletion queue.

        ```cpp
        destroy_system::mark_queued_entities_and_their_children_for_deletion
        ````

- Iterates through a message queue and generates events to another message queue.
    - For example, one that queries children entities of deleted entities into the deletion queue.

        ```cpp
        destroy_system::mark_queued_entities_and_their_children_for_deletion
        ````

## Other examples

- ```gun_system::launch_shots_due_to_pressed_triggers```
- ```sentience_system::apply_damage_and_generate_health_events```

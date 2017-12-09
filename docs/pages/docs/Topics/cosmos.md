---
title: Cosmos
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos
summary: |
    The **cosmos** stores [entities](entities), [components](components), [common cosmos state](common_cosmos_state) and all [caches inferred](inferred_state) from the three. It is a fancy term for what is commonly understood as the "game world". Its methods allow to, for example, create entities, access and modify them via returned [handles](entity_handle), clone or delete them. The cosmos also provides an *advance* method that calls all [stateless systems](stateless_system) to effectively move the game forward in time.
---

## Overview

The cosmos is arguably the most complex structure in the entire codebase.
There are exactly two fields in the class:
- *significant*, representing the part of the cosmos that is [significant](significant_state);
- *inferred*, which is a storage of all [caches](inferred_cache) that are at any time regenerable from the contents of the *significant* field.

The reason that these two are coupled into a single object is because one is so rarely (if ever) needed without the other that separating them would only accomplish so much as to introduce unnecessary noise, in the form of twice as many references passed across the entire game's logic.  

There are also many other helper methods for performing common tasks on the cosmos, e.g. getting a handle to an entity referenced by an [id](entity_id), [guid](entity_guid), getting a [processing list](processing_lists_cache) of a given kind, or getting all entities matching a given name. While, under the principle of separation of concerns, it would make sense to distribute such functions across multiple source files or even classes, there is currently no benefit seen in doing that.

## The advance function

The *advance* function accepts [entropy](cosmic_entropy) along with a reference to all [logical assets](logical_asset) in order to perform a single simulation step.  
It initializes all [message](message) queues on [TLS](https://en.wikipedia.org/wiki/Thread-local_storage) and clears them when the step finishes, to ensure no messages persist beyond duration of the step.  
If some messages were to linger between two consecutive steps, one would need to save them to disk or even synchronize through the network to ensure [determinism](determinism), effectively making messages another case of [significant state](significant_state).  
It makes much more sense to just design the *advance* function so that all posted messages are handled in the same simulation step.

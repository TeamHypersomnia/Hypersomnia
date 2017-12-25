---
title: Cosmos
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos
summary: |
    The **cosmos** stores [entities](entities), [components](components), the [cosmos clock](cosmos_clock) and all [caches inferred](inferred_state) from the three.  
    It represents the part of the application model that continuously changes as the [time flows forward](solver).  
    It could be thought of as the "game world".  
---

## State

The cosmos is arguably the most complex structure in the entire codebase.  
Its methods allow to, for example, create entities, access and modify them via returned [handles](entity_handle), clone or delete them.

There are, however, exactly these *member fields* in the class:

- 
### Significant

    Represents the part of the cosmos that is [significant](significant_state). It holds:
    
    - All [entities](entity).
    - All [components](component).
    - [cosmos clock](cosmos_clock).
    

- 
### Inferred

    The storage of all [caches](inferred_cache) that are at any time regenerable from the contents of the [significant](cosmos#significant) field.

- 
### guid_to_id

    todo: move to inferred?

<br/>
The reason that these two are coupled together into a single object is because one is so rarely (if ever) needed without the other that separating them would only accomplish so much as to introduce unnecessary noise, in the form of twice as many references passed across the entire game's logic.  

## Notes

There are also many other helper methods for performing common tasks on the cosmos, e.g. getting a handle to an entity referenced by an [id](entity_id), [guid](entity_guid), getting a [processing list](processing_lists_cache) of a given kind, or getting all entities matching a given name. While, under the principle of separation of concerns, it would make sense to distribute such functions across multiple source files or even classes, there is currently no benefit seen in doing that.


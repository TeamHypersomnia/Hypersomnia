---
title: Cosmos common
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos_common
summary: |
    The **cosmos common** contains state that is not tied to any particular [entity](entity) or [component](component), but is still required by the [solvers](solver).  
    The solvers, however, **cannot modify it**. It can only ever be altered during the content creation stage, e.g. in [editor](editor_setup).  
---

## Overview

There are certain examples of state that is better held common for all game objects.  
For example: if there exist 200 entities named "Road", it makes no sense to store 2 hundred ``std::string``s containing a value of "Road".  
It would be better if those 200 entities could share a single ``std::string`` that they could refer to by a simple identifier.

So, for each such entity, we store a **flavour identifier** inside its [entity_solvable_meta](entity_solvable_meta), which is a simple **integer**.  
Then we store a map from the **type identifier** into an [entity flavour](entity_flavour) object (that is a part of the cosmos common significant) that contains the corresponding ``std::string``.  

This has several advantages:  
- Less state to be synchronized through the network.
- Changing the name from "Road" to "EvenBetterRoad" for all road entities involves a single reassignment in the map, instead of iterating through all entities.

## Fields

Similarly to [cosmos solvable](cosmos_solvable), the **cosmos common** consists of two parts:

- 
### Significant

    Represents the part of the **cosmos common** that is [significant](significant_state). It holds:
    
    - All [logical assets](logical_asset).
    - All [entity flavours](entity_flavour).
    - Descriptions of spells and their properties (e.g. [PE](personal_electricity) points needed to cast).
    - Numerical settings for some stateless systems (e.g. epsilons for visibility and pathfinding systems).
    - Many more.
    

- 
### Inferred

    The storage of all [caches](inferred_cache) that are at any time regenerable from the contents of the [significant](#significant) field.
    
    - A map of names into corresponding type identificators.

## Notes

Implementation note: **always ensure that the cosmos solvable is reinferred after reinferring the cosmos common.** That is because, if the order of operation was reversed...  
~~...well, actually, nothing bad should happen. That is because physics world caches will have a flag of whether a particular ``b2Shape`` belongs to an entity, and thus should be freed on destruction, or if it belongs to the common, and should not touch it at all.~~
**However**: it might be the case that a ``b2PolygonShape`` or so might end up at a completely different address then from before reinference. Better safe then sorry.

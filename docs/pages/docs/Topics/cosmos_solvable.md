---
title: Cosmos solvable
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos_solvable
summary: |
    There exist two *equivalent* definitions for a **cosmos solvable**:  
    1. that part of the entire game's [replicated state](replicated_state) which is in any way mutable by a [solver](solver);  
    2. that, which stores [entities](entities), [components](components), the [cosmos clock](cosmos_clock) and all [caches inferred](inferred_state) from the three.  


    Intuitively, it represents the part of the application model that continuously changes as the time flows forward.  
    It could be thought of as the "game world".  
---

## State

An overview of the member fields of the class.


- 
### Significant

    Represents the part of the cosmos solvable that is [significant](significant_state). It holds:
    
    - All [entities](entity).
    - All [components](component).
    - [cosmos clock](cosmos_clock).
    

- 
### Inferred

    The storage of all [caches](inferred_cache) that are at any time regenerable from the contents of the [significant](cosmos_solvable#significant) field (given also the [cosmos_solvable::significant](cosmos_solvable#significant)).

- 
### guid_to_id

    While this field is nothing else than a [inferred cache](inferred cache), it is stored separately from the [cosmos_solvable::inferred](#inferred) because quick guid-based access is often needed even when all other caches are temporarily destroyed, e.g. when preparing new significant state from a [cosmic delta](cosmic_delta). 


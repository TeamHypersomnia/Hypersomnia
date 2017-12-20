---
title: Cosmos common state
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos_common_state
summary: |
    The **``cosmos common state``** is a case of [significant state](significant_state) not tied to any particular [entity](entity) or [component](component).  
    [Solvers](solver) can only reference it through ``const&``, as ideally it should only ever be modified during the content creation stage, e.g. in [editor](editor_setup).  
  
    The above properties also hold for [logical assets](logical_asset),  
    but the difference is that the **cosmos common state** is *not* regenerable from any viewable, nor from any other state.
---

## Overview

There are certain examples of state that is better held common for all game objects.  
For example: if there exist 200 entities named "Road", it makes no sense to store 2 hundred ``std::wstring``s containing a value of "Road".  
It would be better if those 200 entities could share a single ``std::wstring`` that they could refer to by a simple identifier.

So, for each such entity, we store a **type identifier** inside a [type component](type_component), which is a simple **integer**.  
Then we store a map from the **type identifier** into an [entity type](entity_type) object (that is a part of the cosmos common state) that contains the corresponding ``std::wstring``.  

This has several advantages:  
- Less state to be synchronized through the network.
- Changing the name from "Road" to "EvenBetterRoad" for all road entities involves a single reassignment in the map, instead of iterating through all entities.

### Other examples

There are other examples of such state:
- Descriptions of spells and their properties (e.g. [PE](personal_electricity) points needed to cast)
- Numerical settings for some stateless systems (e.g. epsilons for visibility and pathfinding systems)

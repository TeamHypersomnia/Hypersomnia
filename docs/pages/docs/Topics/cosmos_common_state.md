---
title: Cosmos common state
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos_common_state
summary: |
    The **``cosmos_common_state``** is a part of the [cosmos](cosmos)'s [significant state](significant_state) ([``cosmos::significant``](cosmos#overview)) that is not tied to any particular [entity](entity).  
    Notably, it also contains [cosmos meta](cosmos_meta).
---

## Overview

There are certain examples of state that is better held common for all game objects.  
For example: if there exist 200 entities named "Road", it makes no sense to store 2 hundred ``std::wstring``s containing a value of "Road".  
It would be better if those 200 entities could share a single ``std::wstring`` that they could refer to by a simple identifier.

So, for each such entity, we store a **name identifier** inside a [name component](name_component), which is a simple **integer**.  
Then, in an object of class ``entity_name_metas`` (that is a part of the cosmos common state) we store a map from the **name identifier** into the corresponding ``std::wstring``.  

This has several advantages:  
- Less state to be synchronized through the network.
- Changing the name from "Road" to "EvenBetterRoad" for all road entities involves a single reassignment in the map, instead of iterating through all entities.

### Other examples

There are other examples of such state:
- Descriptions of spells and their properties (e.g. [PE](personal_electricity) points needed to cast)
- Numerical settings for some stateless systems (e.g. epsilons for visibility and pathfinding systems)

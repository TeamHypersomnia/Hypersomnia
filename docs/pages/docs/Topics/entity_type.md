---
title: Entity type
tags: [topics, ECS]
hide_sidebar: true
permalink: entity_type
summary: |
  An **entity type** contains information that is stored once is later shared by one or more entities.  
  An [entity](entity) specifies its **type** via its [type component](type_component). 
---

## Map of type identifiers

Inside the [cosmos_common_state](cosmos_common_state), there is an object named ``types``.
It maps type identifiers found in the [type component](type_component) to the respective type information object.

## Future work

So that working with content is easier, and so that less memory is wasted, each component should specify fully a separate type information.
This would serve to separate data that is more like "input" to the processing, and the ever-changing caches, like
- ``gun::muzzle_velocity`` would become a part of ``type``, but
- ``gun::recoil_player_instance`` would remain as part of the component. 

Type creation would be a separate stage that would usually never occur in logic.  
In fact, logic step could simply provide only a const getter for the type information.

Types could be created, copied and then modified, or destroyed.

This could be incrementally introduced (not right away), but an entity could be created while being given the type right away. Then the cosmos could add all needed components automatically.  
Or, we could leave things as they are and only implement this in editor as the default behaviour.

There won't be many types, but access will be frequent, thus we could allocate all component types there statically, without much of a memory hit.

Some components exist only as types without instance counterpart or the opposite.
- would it make sense to abstract away access to types and components, both as just components?
	- note it will not reduce the amount of work needed to transform the code, except for components that fall just into one category. You will somehow need to differentiate for access of fields from different categories.
	- would be safer not to, because of the constness of types.

### Performance

Some concern could be falsely raised to the fact that an entity's type id first need to be obtained, and only later the type information;  
as, compared to just a single component lookup, this could potentially be slower

However, at the time of having an entity handle, the type id is already available, possibly in cache.  
The only thing left is to perform the lookup in the type information map.
Then, the only thing to compare performance-wise is a lookup in an ``unordered_map`` vs a [``pool``](pool).
If the container was optimized for deterministic identificators, it could possibly be faster;
Note that types won't be altered during logic, usually only during the creation of content.  
So, they could be stored in a fixed-sized container. **There can even be a vast performance improvement**. 

### Polygon shape concern

Deserves a separate topic because it is the biggest component currently, second only to the [container component](container_component). 
The decrease in memory usage will be *enormous*. Let's add to it the fact that we were always instancing a ``b2PolygonShape`` for each entity.  
That is not at all efficient.

#### Destruction data

One concern that may be raised is the fact that, if we get to implement destruction, then the two halves might need different shapes.  
That would, intuitively, imply that the logic code might need to create new types dynamically.  
That is however far from the truth.  

Since the calculation of halves from given scars is pretty much deterministic,
we can just store the scars in a component and let some inferred state calculate and store the partitioned shape.  
Then, when new scar is added, reinfer.

---
title: Entity type
tags: [topics, ECS, flyweights]
hide_sidebar: true
permalink: entity_type
summary: |
  An **entity type** contains information that is stored once and is later shared by one or more [entities](entity).  
  Formally, an **entity type** is a [flyweight](flyweight) whose instance is an [entity](entity).  
  An [entity](entity) specifies its **type** via its [type component](type_component). 
---

## Map of type identifiers

Inside the [cosmos_common_state](cosmos_common_state), there is an object named ``types``.  
It maps type identifiers found in the [type component](type_component) to the respective **entity type**.

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
That was not at all efficient.

#### physics assignment operator

There is further potential in reducing the amount of shape that the [physics world cache](physics_world_cache) keeps internally.
Unless the shape was allocated for a custom polygon shape component, the ``physics_world_cache::operator=`` should disregard copying of shapes.
This will additionally dramatically improve performance of copying the cosmoi along with their inferred states.

Additionally, ``b2PolygonShape``s and others will need to be inferred from the vertex data. That is because the struct is virtual, so may prevent direct std::memcpy, and also because it will hold convex-partitioned information.

### Inferred state

Some things like polygon shape component might need some inferred state.
That state should most likely be held separate from the cosmos, because it will hardly ever change and copying it every time the state of the cosmos needs be copied around would be a waste of time.
It will most likely only ever change during the stage of content creation.
Thus, in ordinary multiplayer gameplays, it should be inferred exactly once.


### Frequent new types

There are examples of data that, on one hand, will very frequently be shared identical,  
but, on the other hand, may have cases where practically each new entity needs a slight alteration of it.  
  
A perfect example is a shape polygon component.  

While things like bullet types or rifle types may each have just one shape for all instances,  
take for example terrain: each of its pieces might possibly have a completely different shape,  
especially if it is some kind of curvy corridor or dungeon.  
These terrain pieces might, to the exception of their shape, behave identically, so it would make sense to have just one type for them.

In that case, we may:
- require the [author](author) to create a new type for each of the piece;
	- or somehow facilitate this or maybe do this under the hood?
- create a dynamic component that, on its own, will not consume memory when not used, but can be named for example "custom polygon shape"
	- it would take precedence over what would possibly be found in the type.
	- the problem will not be frequent besides that thing with polygon, so it is a viable solution.
	- additionally, the dynamic component might have different assumptions about storage, since it will not be so common; it could even have plain ``std::vector``s instead of fixed-size containers.
		- although I would not advocate this.
	- problem arises when we will need to copy the game world. Precisely, how do we determine if a shape is owned by the physics world cache? 
		- we need to take proper measures so as to never copy the shapes that are indeed flyweights; as opposed to the custom shapes.

### Per-field overrides

Consider this: usually, we specify damping values once for a given type of physical entity.  
However, the logic itself may want to alter the damping value due to some special circumstances (being inert, riding a motorcycle, sprinting, etc.).
Two approaches can be taken:

- ``std::optional<field_type> overridden_field;``
- ``field_type field_scalar;``

We'll probably use scalars most of the time.

### Components considerations

An author should not be concerned with adding components to a new entity, that properly correspond to what they've already set in the definitions.  
Regardless of the fact that the definitions are statically allocated (and thus always "present"), the author should be able to specify which definitions they are interested in.  

Additionally, the author might want to specify initial values for the component that is added, that corresponds to the given definition type.
For example, some physical bodies might want to have some particular initial velocity set.  

However, the logic should, for the sake of performance and simplicity of code, always assume that a correspondent definition is present, so that we can take full advantage of the static allocation.  
Thus the logic should derive usage of definitions from the presence of components.  
However, the presence of components will be initially derived from the presence of definitions.
It will thus be always required to define at least an empty type that specifies the correspondent component type,  
event if the component type itself does not need any definition-like data.

Per each definition, we hold a boolean value of whether the definition is set. 
It would the most idiomatic to hold a tuple of ``std::optional`` definitions, but a lot better cache coherency will be achieved if the boolean values will be stored in a separate variable like so:

``std::array<bool, DEFINITION_COUNT_V> enabled_definitions;``

A concern could be raised because with this design, performance of serializing types could suffer.  
That is because, if we serialize linearly without regard for which definitions are set, we serialize possibly a lot of unneeded data.  
On the other hand, if we do serialize only that which was set, we may suffer from instruction cache misses as the code will not degrade to a simple memcpy anymore.

However, the type information:
	- Will not at all be that large;
	- Will not at all be copied around frequently (pretty much never, just the cosmos) 
	- Should have the topmost access performance, which would suffer from facilitating the structure for better serialization performance.

A custom readwrite overloads will be in order, both for lua and binary, which will take note of which initial component values are set, and serialize only the according definitions;  
At that point, serialization performance pretty much does not matter at all, and disk space may or may not come in handy.


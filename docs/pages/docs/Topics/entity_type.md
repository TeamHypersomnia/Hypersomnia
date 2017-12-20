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

## Overview


So that working with content is easier, and so that less memory is wasted, each entity is divided into two kinds of data:
1. state that should never change and is supposed to be shared between entities,
	- ``gun::muzzle_velocity``
2. state whose instance is needed by each single entity, since it is constantly in flux.
	- ``gun::trigger_pressed`` 
  

The first is more like "input" to the processing, in which [authors](author) in particular are interested.  
Most of the time, only the programmers are concerned with the second type of data.

- An *entity type* contains a tuple of [definitions](definition), along with a flag for each definition to signify whether it is *enabled*.
	- An author may specify which definitions to enable.
	- The flags will be held separate:``std::array<bool, DEFINITION_COUNT_V> enabled_definitions;``
		- A lot better cache coherency than the more idiomatic ``std::optional``.
	- An *enabled* definition implies that the entity needs a component of type ``definition_type::instance`` (if specified) for the definition to be ever used by the logic.  
		- If instance type is specified, it additionally stores an **initial value** for the component.
- Types can be created, copied and then modified, or destroyed.  
	- It is a separate stage that never occurs in logic itself.
		- In fact, the [logic step](logic_step) simply provides only const getters for the type information.
	- Should only be done by [authors](author).
	- In particular, the **enabled** flags for definitions may change even though some entities of this type already exist.
	- In particular, the **initial value** for a component may change even though some entities of this type already exist.
		- This is not important. The field, in practice, serves two purposes:
			- Used by the logic when it gets a type identifier to spawn (e.g. ``gun::magic_missile_definition``), so that it can set reasonable initial values. Thus, a change to initial value needs not updating here as the logic will naturally catch up, storing only the type identifier.
			- Optionally as a helper for the author when they want to spawn some very specific entities. The author shouldn't worry that the initial value changes something for the existent entities.
- There won't be many types, but access is **frequent** during [solve](solver#the-solve). 
	- We allocate all definitions **statically**, as memory won't suffer relatively to the speed gain.
- On creating an entity, the chosen type's id is passed.  
	- The [cosmos](cosmos) adds all components implied by the definitions are added automatically.
- Some components do not need any definition data.
	- examples: child, flags, **sender**
		- Looks like most of them should anyway be transparent to the author.
		- Their existence is either fundamental or implied by a specific circumstance in the logic (as logic can add or remove components (rarely))
	- ~~They will have an empty definition struct that will not take up space and will define "implied_component" type.~~
		- We've decided that the author should not be concerned with such obscurities and existence of these components should not be implied by definitions, but by a circumstance.  

<!--
		- if a definition implies more than one component, perhaps they should be merged?
		- That ensures that each component has a corresponding definition.
		- missile will imply both missile and sender?
			- we'll just add the sender component where necessary and where it wasn't yet added.
			- the sender, child components will anyway be hidden from the author as they are detail.~~
-->
- Some definitions do not need any instance data.
	- Example: render component.
		- That doesnt change anything at all, except for the way of getting that data.
- Some definitions might be so heavily used that an immutable copy in the entity itself would be beneficial.
	- Example: render component.
		- Perfectly viable; just declare it with a ``static constexpr bool`` inside the definition and make the field constant.
		- Care: if the type information changes, so must the copied definition. So let's save it for later.
- Some definitions and components serve exactly the same role but the component counterpart lets us customize each instance without creating new type for each.
	- Example: shape polygon component.
		- We then must disable storing of an initial value; or just never specify the implied component type in the first place.

We might be tempted to make some of the definitions fundamental, thus always defined as "enabled".
However, there is no benefit seen in doing so. It is a wiser choice to have a flag for each definition and assume that the contents of a default-constructed definition are always in a valid state.
Provide a "set" function?

<!--
### Performance

Some concern could be falsely raised to the fact that an entity's type id first need to be obtained, and only later the type information;  
as, compared to just a single component lookup, this could potentially be slower.

However, at the time of having an entity handle, the type id is already available, possibly in cache.  
The only thing left is to perform the lookup in the type information map.
Then, the only thing to compare performance-wise is a lookup in an ``unordered_map`` vs a [``pool``](pool).
If the container was optimized for deterministic identificators, it could possibly be faster;
Note that types won't be altered during logic, usually only during the creation of content.  
So, they could be stored in a fixed-sized container. **There can even be a vast performance improvement**. 
-->

### Polygon shape concern

Deserves a separate topic because it is the biggest component currently, second only to the [container component](container_component).  
The decrease in memory usage will be *enormous*. Let's add to it the fact that we were always instancing a ``b2PolygonShape`` for each entity.  
That was not at all efficient.

#### physics world cache copy-assignment operator

There is further potential in reducing the amount of shape that the [physics world cache](physics_world_cache) keeps internally.
Unless the shape was allocated for a custom polygon shape component, the ``physics_world_cache::operator=`` should disregard copying of shapes.
This will additionally dramatically improve performance of copying the cosmoi along with their inferred states.

Additionally, ``b2PolygonShape``s and others will need to be inferred from the vertex data. That is because the struct is virtual, so may prevent direct std::memcpy, and also because it will hold convex-partitioned information.

**We should disregard tailoring the assignment operator until we get to networking, where we'll probably switch to another physics engine with the features we need.**

### Inferred state

Some things like polygon shape component might need some inferred state.
That state should most likely be held separate from the cosmos, because it will hardly ever change and copying it every time the state of the cosmos needs be copied around would be a waste of time.
It will most likely only ever change during the stage of content creation.
Thus, in ordinary multiplayer gameplays, it should be inferred exactly once.

<!--
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
-->

### Per-field overrides

Consider this: usually, we specify damping values once for a given type of physical entity.  
However, the logic itself may want to alter the damping value due to some special circumstances (being inert, riding a motorcycle, sprinting, etc.).
Two approaches can be taken:

- ``std::optional<field_type> overridden_field;``
- ``field_type field_scalar;``

We'll probably use scalars most of the time.

<!--
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

A concern could be raised because with this design, performance of serializing types could suffer.  
That is because, if we serialize linearly without regard for which definitions are set, we serialize possibly a lot of unneeded data.  
On the other hand, if we do serialize only that which was set, we may suffer from instruction cache misses as the code will not degrade to a simple memcpy anymore.

However, the type information:
	- Will not at all be that large;
	- Will not at all be copied around frequently (pretty much never, just the cosmos) 
	- Should have the topmost access performance, which would suffer from facilitating the structure for better serialization performance.

-->
## Storage

Inside the [cosmos common state](cosmos_common_state), there is an object named ``types``.  
It maps type identifiers found in the [type component](type_component) to the respective **entity type**.  
Preferably, the container should be a constant size vector as the type ids will just be consecutive integers.  
We will treat a type with blank name as not set.  

Care must be taken that the storage is not treated like trivially copyable types during byte readwrite.

A custom readwrite overloads will be in order, both for lua and binary, which will take note of which initial component values are set, and serialize only the according definitions;  
At that point, serialization performance pretty much does not matter at all, and disk space may or may not come in handy.

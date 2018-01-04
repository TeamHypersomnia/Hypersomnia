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

- An *entity type* contains a tuple of [definitions](definition), a tuple of initial [component](component) values, and an array of flags for each definition to signify whether it is *enabled*.
	- An author may specify which definitions to enable.
	- The flags will be held separate:``std::array<bool, DEFINITION_COUNT_V> enabled_definitions;``
		- A lot better cache coherency than the more idiomatic ``std::optional``.
	- An *enabled* definition implies that the entity needs a component of type ``definition_type::instance`` (if specified) for the definition to be ever used by the logic.  
		- If instance type is specified, it additionally stores an **initial value** for the component.
- A type with a blank name is treated as being 'not set' (a null type).
	- We will always require the author to set a non-empty name for a type.  
- Types can be created, copied and then modified, or destroyed.  
	- It is a separate stage that never occurs in logic itself.
		- In fact, the [logic step](logic_step) simply provides only const getters for the type information.
	- Should only be done by [authors](author).
	- In particular, types can be deleted, in which case the [vector of types](#storage) will have to be shrunk. Then we will need to remap types of all existing entities.
	- In particular, the **enabled** flags for definitions may change even though some entities of this type already exist.
		- We warn the author and ask if the existent entities should be recreated with new components with new initial values (preserving what was already set).
			- The warning can be ticked to never pop up again.
	- In particular, the **initial value** for a component may change even though some entities of this type already exist.
		- This is not important. The field, in practice, serves two purposes:
			- Used by the logic when it gets a type identifier to spawn (e.g. ``gun::magic_missile_definition``), so that it can set reasonable initial values. Thus, a change to initial value needs not updating here as the logic will naturally catch up, storing only the type identifier.
			- Exists as an optional helper for the author when they want to spawn some very specific entities. The author shouldn't worry that the initial value changes something for the existent entities.
	- Disallow deleting a type if entities with that type exist.
		- The author will be able to change the type of an existing entity, optionally keeping the values that were already set in components.
	- If a field of a definition changes, (during content creation), reinfer all objects of this type with help of the [type id cache](type_id_cache); currently we'll just reinfer the whole cosmos.
- There won't be many types, but access is **frequent** during [solve](solver#the-solve). 
	- We allocate all definitions **statically**, as memory won't suffer relatively to the speed gain.
- On creating an entity, the chosen type's id is passed.  
	- The [``cosmos::create_entity``](cosmos#create_entity) automatically adds all components implied by the enabled definitions.
- Entity can change the type during its lifetime, in which case it should be completely reinferred.
	- That will be helpful when we want to make a change to entity (that is impossible by just changing the components) while preserving correctness of all identificators that point to it.
	- Example: a grenade that changes from a normal body to a bullet body on being thrown (its sprite might change as well).
	- **Care**: some state is sensitive to random deletion. For example, if we nuke the [car component](car_component), some child entities might be left dangling and not get deleted.
		- We might change the design so that there is nothing like ``child_entity_id``; a "delete_with" component should exist instead of "child_component". relational system keeps track of all "delete_with" entities for a "parent" entity every time an make_child_of is invoked. This way, state of the parent might be altered at any time, and its children will still get properly deleted.
		- Most ``child_entity_id`` types will anyway be replaced with a entity_type_id. For other cases, it makes little sense to also enforce deletion of those entities just because they are meant to be related.
- Some components do not need any definition data.
	- examples: child, flags, **sender**
		- Looks like most of them should anyway be transparent to the author.
		- Their existence is either always_present or implied by a specific circumstance in the logic (as logic can add or remove components (rarely))
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
		- That does not change anything at all, except for the way of getting that data.
- Some definitions might be so heavily used that an immutable copy in the entity itself would be beneficial.
	- Example: render component.
		- Perfectly viable; just declare it with a ``static constexpr bool`` inside the definition and make the field constant.
		- Care: if the type information changes, so must the copied definition. **So let's save it for later**.
- There is a case that it would be best if data existed both as a definition and a component.
	- Example: shape polygon component.
	- See: [overrides](#overrides).

We might be tempted to make some of the definitions always_present, thus always defined as "enabled".
However, there is no benefit seen in doing so. It is a wiser choice to have a flag for each definition and assume that the contents of a default-constructed definition are always in a valid state.

## Overrides

There are examples of data that, on one hand, will very frequently be shared identical,  
but, on the other hand, may have cases where practically each new entity needs a slight alteration of it.  
  
A perfect example is a shape polygon component.  

While things like bullet types or rifle types may each have just one shape for all instances,  
take for example terrain: each of its pieces might possibly have a completely different shape,  
especially if it is some kind of curvy corridor or dungeon.  
These terrain pieces might, to the exception of their shape, behave identically, so it would make sense to have just one type for them.

~~Another case is names and custom names.~~  
Names are completely transparent to the logic so custom names will be managed only by domains that need it (client setup with custom nicknames, editor setup with custom overridden names just for marking, etc).

In that case, we may:
- Leave things as they are.
	- **Chosen solution**: We require the [author](author) to create a new type for each of the piece;
		- the editor shall, obviously, facilitate this to the utmost. 
		- while each type will have their own copy of data for speed, the editor can associate types and enable per-definition overrides, so we could create a new type from an existing base type.
			- a type could be per-entity or per-
	- Conceptually the easiest.
	- **Easiest to get right**.
		- We can always improve it if needed.
	- Most memory wasted.
		- But not necessarily: no additional identifiers inside aggregates will be needed.
	- Lowest flexibility, e.g. the solver can't alter that state or it may with additional effort to make that information stateless.
		- The solver shall not alter definition state.
		- If we find that it would be nice to actually alter that definition state, it might be the case that we should always calculate this value from some other circumstances and not just read it straight from the definition. Just like we do with ``resolve_density_of_associated_fixtures`` or ``resolve_dampings_of_body`` 
- ~~Enable dynamical overrides.~~

<!--
	- Mutable or not, by the solver.
		- Preferably not, but it might be useful when partitioning polygons into destructed pieces.
	- They would take precedence over what would possibly be found in the correspondent definitions in the type.
	- When we will need to copy the game world, how do we determine if a shape is owned by an override or a pre-inferred definition? 
		- we need to take proper measures so as to never copy the shapes that are indeed flyweights; as opposed to the custom shapes.
	- The problem will not be frequent besides names and polygons, so it is a viable solution.
	- **Solution**: for each such case, create a dynamic component that, on its own, will not consume memory when not used, but can be named for example "custom polygon shape"
		- additionally, the dynamic component might have different assumptions about storage, since it will not be so common; it could even have plain ``std::vector``s instead of fixed-size containers.
			- although I would not advocate this.
		- Least memory wasted.
		- Best performance as the override checks will be limited to several specific domains.
		- Medium-hard conceptually.
	CCCCCCCCCC		- Additionally, it will be pretty much known from the get-go which data is possibly altered.
			- Nobody will also be surprised if the solver needs to alter that state, because they are plain components.
		- Good flexibility, until something unexpected needs to change.
	- **Solution**: create overrides for definitions and let an entity have its own set of definitions, if so is required.
		- Most memory wasted.
			- An entity with an override costs the size of all the definitions, unless we make each definition dynamically allocated, hindering overridden-entities performance all the more.
			- Additionally, as the type is the same, we must assume the same storage rules as with the definitions. In particular, we won't be able to use a dynamically allocated vector of vertices if the definition object already uses a constant size vector.
		- Worst performance as upon getting any definition we check for an existing override.
			- The only impact is on non-overridden entities.
			- Overidden entities would otherwise still need to get a component.
			- The impact might not be so great for the non-overriden entities as we might make the definiton id to already reside in cache once the type id is in cache.
		- Hardest conceptually.
			- Is it predictable when modifying any definition field? Some state might need to be updated to be consistent.
			- For example, rigid body parameters like type (static or dynamic, if it is a bullet etc.).
				- If we anyway allow for changing a type, then reinference should be enough.
		- Most flexibility in altering the game's behaviour.
	- The memory waste calculations for these two solutions actually assume that there will be a lot of overridden entities. 
		- If there are just a few, than having a dynamic component id for each possible override actually costs more in terms of aggregate size.
		- If there are many, then yes, per-entity overrides are worse. 
		- The actual serialized size should stay pretty much the same.
-->

<!--
### Simple per-field overrides

Consider this: usually, we specify damping values once for a given type of physical entity.  
However, the logic itself may want to alter the damping value due to some special circumstances (being inert, riding a motorcycle, sprinting, etc.).
Two approaches can be taken:

- ``std::optional<field_type> overridden_field;``
- ``field_type field_scalar;``

We'll probably use scalars most of the time.

COMMENTED OUT: Such things will be calculated statelessly.
-->

### Polygon shape concern

Deserves a separate topic because it is the biggest component currently, second only to the [container component](container_component).  
The decrease in memory usage will be *enormous*. Let's add to it the fact that we were always instancing a ``b2PolygonShape`` for each entity.  
That was not at all efficient.

#### physics world cache copy-assignment operator

There is further potential in reducing the amount of shape that the [physics world cache](physics_world_cache) keeps internally.
<!-- Unless the shape was allocated for a custom polygon shape component, the ``physics_world_cache::operator=`` should disregard copying of shapes.-->
The cache should always disregard copying of shapes since they belong to the types, which don't change at all and are shared.  
This will additionally dramatically improve performance of copying the cosmoi along with their inferred states.

``b2PolygonShape``s and others will need to be inferred from the significant vertex data in the entity types.  
That is because the struct is virtual, so may prevent direct std::memcpy, and also because it will hold convex-partitioned information.  

<!-- **We should disregard tailoring the assignment operator until we get to networking, where we'll probably switch to another physics engine with the features we need.** -->

## Storage

Inside the [cosmos common significant](cosmos_common#significant), there is an object named ``types``.  
It maps type identifiers found in the [type component](type_component) to the respective **entity type**.  
Preferably, the container should be a constant size vector as the type ids will just be consecutive integers.  

Care must be taken that the storage is not treated like trivially copyable types during byte readwrite:
A custom readwrite overloads will be in order, both for lua and binary, which will take note of which initial component values are set, and serialize only the according definitions;  
At that point, serialization performance pretty much does not matter at all, and disk space may or may not come in handy.

<!--
- ~~Entity should be reinferred if it changes a type (during content creation).~~ Type id for an entity should stay constant until its death.
	- If at all, changing the type should be implemented as an editor feature. Let us not make this an actual feature in the game code and let us always assume that an entity's type stays constant throughout its lifetime.
	- It will be easier in the beginning if we disallow this and require the author to just create a new entity.
-->
<!--
### Components considerations
An author should not be concerned with adding components to a new entity, that properly correspond to what they've already set in the definitions.  
Regardless of the fact that the definitions are statically allocated (and thus always "present"), the author should be able to specify which definitions they are interested in.  

Additionally, the author might want to specify initial values for the component that is added, that corresponds to the given definition type.
For example, some physical bodies might want to have some particular initial velocity set.  

However, the logic should, for the sake of performance and simplicity of code, always assume that a correspondent definition is present, to avoid noise.  
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

<!--
		- if a definition implies more than one component, perhaps they should be merged?
		- That ensures that each component has a corresponding definition.
		- missile will imply both missile and sender?
			- we'll just add the sender component where necessary and where it wasn't yet added.
			- the sender, child components will anyway be hidden from the author as they are detail.~~
-->

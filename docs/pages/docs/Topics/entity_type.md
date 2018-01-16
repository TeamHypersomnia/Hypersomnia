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
	- An *enabled* definition implies that the entity needs a component of type ``definition_type::implied_component`` (if specified) for the definition to be ever used by the logic.  
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
- The logic should always first check for the existence of component, only later the definition that the component is implied by
	- One less indirection because checking for the id being unset is just a single arithmetic operation when the id is cached.
- On creating an entity, the chosen type's id is passed.  
	- The [``cosmos::create_entity``](cosmos#create_entity) automatically adds all components implied by the enabled definitions.
- Some components do not need any definition data.
	- examples: child, flags, **sender**
		- Looks like most of them should anyway be transparent to the author.
		- Their existence is either always_present or implied by a specific circumstance in the logic (as logic can add or remove components (rarely))
	- ~~They will have an empty definition struct that will not take up space and will define "implied_component" type.~~
		- We've decided that the author should not be concerned with such obscurities and existence of these components should not be implied by definitions, but by a circumstance.  
- ~~Some definitions imply more than one component.~~
	- **Disproved**.
		- A component's existence may only be implied by:
			- Being required by a circumstance.
				- We will for now make them always_present as no circumstantial components exist that would be too big.
			- Being required by a definition.
		- If two components existed such that both are implied by the same definition, it would not make sense to separate those components in the first place.
			- Because one would never be needed without the other.
			- Thus wasting space for additional pool id.
		- Additionally if a component would be necessary dependence of two other components (in which case it would make sense that a definition can imply more than one component) that would make architecture **too complex**.
			- It would require us to prioritize initial values for definitions that share an implied component. Ugh.
	- Which ones would need it by the way?
		- If we're talking missile component, sender will anyways be "always_present" because the circumstance might or might not otherwise need to add it.
- Since there is a bijection between definitions and impliable components, the code can assume that, if a component exists, so shall the relevant definition.
	- Theoretically there will be no crash as we do not hold optionals but actual, properly constructed objects.
- Some definitions do not need any instance data.
	- Example: render component.
		- That does not change anything at all, except for the way of getting that data.
- Some definitions might be so heavily used that an immutable copy in the entity itself would be beneficial.
	- Example: render component.
		- Perfectly viable; just declare it with a ``static constexpr bool`` inside the definition and make the field constant.
		- Care: if the type information changes, so must the copied definition. **So let's save it for later**.
- Scope of initial values
	- Generally it should be always safe to expose any field to the author as sensitive state will be protected by exception throws
	- Though customizing some of them just does not make sense.
		- E.g. ids for some of the always_present components
			- Components will be templatized for the ids so in this case they will hold group identificators
				- Which may actually turn out handy regardless of what we think now
- Do we want to store initial values for always_present components?
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

## Storage

Inside the [cosmos common significant](cosmos_common#significant), there is an object named ``types``.  
It maps type identifiers found in the [type component](type_component) to the respective **entity type**.  
Preferably, the container should be a constant size vector as the type ids will just be consecutive integers.  

Care must be taken that the storage is not treated like trivially copyable types during byte readwrite:
A custom readwrite overloads will be in order, both for lua and binary, which will take note of which initial component values are set, and serialize only the according definitions;  
At that point, serialization performance pretty much does not matter at all, and disk space may or may not come in handy.


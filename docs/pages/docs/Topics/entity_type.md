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

- An *entity type* contains a tuple of [invariants](invariant), a tuple of initial [component](component) values, and an array of flags for each invariant to signify whether it is *enabled*.
	- An existent invariant implies that the entity needs a component of type ``invariant_type::implied_component`` (if specified) for the invariant to be ever used by the logic.  
		- Thus if implied_component type is specified, it additionally stores an **initial value** for the component.
- A type with a blank name is treated as being 'not set' (a null type).
	- We will always require the author to set a non-empty name for a type.  
- Types can be created, copied and then modified, or destroyed.  
	- It is a separate stage that never occurs in logic itself.
		- In fact, the [logic step](logic_step) simply provides only const getters for the type information.
	- Should only be done by [authors](author).
	- In particular, types can be deleted, in which case the [vector of types](#storage) will have to be shrunk. Then we will need to remap types of all existing entities.
	- But disallow deleting a type if entities with that type exist.
		- The author will be able to change the type of an existing entity, optionally keeping the values that were already set in components.
	- In particular, the **initial value** for a component may change even though some entities of this type already exist.
		- This is not important. The field, in practice, serves two purposes:
			- Used by the logic when it gets a type identifier to spawn (e.g. ``gun::magic_missile_definition``), so that it can set reasonable initial values. Thus, a change to initial value needs not updating here as the logic will naturally catch up, storing only the type identifier.
			- Exists as an optional helper for the author when they want to spawn some very specific entities. The author shouldn't worry that the initial value changes something for the existent entities.
	- If a field of a invariant changes, (during content creation), reinfer all objects of this type with help of the [type id cache](type_id_cache); currently we'll just reinfer the whole cosmos.
- There won't be many types, but access is **frequent** during [solve](solver#the-solve). 
	- All possible configurations of invariants will be determined by types at compilation time.
- The logic should always first check for the existence of component, only later the invariant that the component is implied by.
	- One less indirection because checking for the id being unset is just a single arithmetic operation when the id is cached.
- On creating an entity, the chosen type's id is passed.  
	- The [``cosmos::create_entity``](cosmos#create_entity) automatically adds all components implied by the enabled invariants.
- Some components do not need any invariant data.
	- examples: child, flags, **sender**, transform, ~~tree of npo~~ (will be a cache), ~~special physics~~ (will be merged with rigid body)
		- Their existence is implied by:
			- Being always present: child and flags.
				- Always specifiable initial values.
			- Circumstance in the logic or mere thereof possibility, which is implied by...
				- sender
			- **...configuration of components at the entity construction time.**
				- sender by missiles or other kinds of launched objects
				- **Always specifiable initial values when the required configuration of components holds**.
		- The real question is whether they should be visible to the author, as lack of invariant data implies that it is some kind of detail.
			- Otherwise we might simulate empty invariant struct just as well
		- **Chosen solutions**:
			- child, flags - for author, always specifiable in initial values
			- transform - for author, appears specifiable when the entity has no physical components
			- sender - for author, appears specifiable in initial values when it has either missile or explosive
			- Currently no components exist whose existence upon construction would depend on **value** of other components.
			- Currently no components exist whose actual **initial value** upon construction would depend on value/existence of other components.
				- because tree of npo will not be a component soon.
			- rotation_copying
				- instead of rotation_copying_system::update_rotations, implement sentience_system::rotate_towards_crosshairs
					- effectively removes the need for "entity_id stashed_target"
				- instead of rotation_copying_system::update_rotations, implement driver_system::rotate_towards_vehicles
					- even less setup in driver system
				- we have no more cases of rotation copying
			- position_copying
- ~~Some invariants imply more than one component.~~
	- **Disproved**.
		- A component's existence may only be implied by:
			- Being required by a circumstance.
				- We will for now make them always_present as no circumstantial components exist that would be too big.
			- Being required by a invariant.
		- If two components existed such that both are implied by the same invariant, it would not make sense to separate those components in the first place.
			- Because one would never be needed without the other.
			- Thus wasting space for additional pool id.
		- Additionally if a component would be necessary dependence of two other components (in which case it would make sense that a invariant can imply more than one component) that would make architecture **too complex**.
			- It would require us to prioritize initial values for invariants that share an implied component. Ugh.
	- Which ones would need it by the way?
		- If we're talking missile component, sender will anyways be "always_present" because the circumstance might or might not otherwise need to add it.
- Observation: there is a bijection between invariants that imply and impliable components.
	- Therefore, the code can assume that, if a component exists, so shall the invariant (if it exists) which implies this component.
	- Theoretically there will be no crash if we get a invariant that is not enabled.
		- That is because we do not hold optionals but actual, properly constructed objects.
	- If a invariant does not imply any component, its existence can only be queried by actually checking whether it is enabled.
- **Editor**: Recommended invariants
	- Instead of a invariant strictly implying another invariant or more than one component, we can make a notion of "recommended" invariants in the editor. 
	- Thus, the actual logic code should never assume that a invariant exists if one other exists, the same about components.
	- But in the editor, depending on the current invariants configuration, the type editor dialog may recommend:
		- to add a specific component (e.g. interpolation if there exists a render and dynamic body)
		- to remove a specific component that is considered redundant (e.g. interpolation if the type only has a static body)
		- the reason it will not be done automatically is because the author might want to experiment while having full control, and maybe persist some values between various attempts.
	- Currently no case of recommended invariants exist apart from interpolation.
		- if it ever so happens that a circumstantial component like a "sender" would be too costly to be always present, we might also make it a recommended invariant.
			- then the code will obviously have to be modified so that it does not always assume that the component is always present			
		- or we might make it a component added upon construction?
- **Editor**: Initial values
	- in a separate tree node.
	- modifiable are all that are implied by the invariants
	- and those that will be added on construction based on existence of other components. 
		- e.g. sender.
- Components might need some special construction.
	- interpolation might need to save the place of birth.
	- trace might want to roll the dice for its first values.
- Some invariants do not need any instance data.
	- Example: render component.
		- That does not change anything at all, except for the way of getting that data.
- Some invariants might be so heavily used that an immutable copy in the entity itself would be beneficial.
	- Example: render component.
		- Perfectly viable; just declare it with a ``static constexpr bool`` inside the invariant and make the field constant.
		- Care: if the type information changes, so must the copied invariant. **So let's save it for later**.
- Scope of initial values
	- Generally it should be always safe to expose any field to the author as sensitive state will be protected by exception throws
	- Though customizing some of them just does not make sense.
		- E.g. ids for some of the always_present components
			- Components will be templatized for the ids so in this case they will hold group identificators
				- Which may actually turn out handy regardless of what we think now
- Do we want to store initial values for always_present components?
- There is a case that it would be best if data existed both as a invariant and a component.
	- Example: shape polygon component.
	- See: [overrides](#overrides).

## Concern: performance of stateless calculation

If we invariantize a component like sprite, where some domains were free to set any value they wished, e.g.:
- [trace system](trace_system) that lengthens them over time.
- [animation system](animation_system) that sets the sprites in accordance with animation time.

- Performance concern: if we invariantize sprite, we must then calculate it statelessly.
	- Will it only be the problem for the special case domains?
		- Animation and trace ids may be made to fall into cache when sprite is accessed 
			- Which will not cause a fetch for plain sprites
		- Animation will anyway override sprite
			- Animation should be stateless?
				- In that case it might become more expensive to calculate?
	- If it becomes bad, we can make an inferred cache for sprites.
	- If it really becomes bad, we can store inferred caches in components somewhat.
	- Notice that we will update the tree of npo cache for non-physical objects or even make its size predefined to a maximum.
		- That cache will be dependent on many significant states probably.
		- In any case the cost of querying for rendering will be the same.
	- Notice that for the physical bodies it will always make sense that the animations do not stray much from the area of the body. 

Plan:
- Animation will be stateful to make calculation easier.
	- Usually, only one problem domain will use the animation controllably, just like it is with AI and pathfinding.
	- We can always store animation priority so that other domains do not interfere. 
		- We don't have to make everything stateless right away while we're just invariantizing things.
- Tree of npo node component 
	- Currently, update_proxy is only called from set_transform
	- We don't need the component; let's just hold the cache and update it whenever.
		- It is not much critical state so we might update it arbitrarily often, not even needing to protect consistency much.
	- Update the cache when:
		- transform changes
		- sprite changes very greatly
	- Existence of caches is implied by existence of components at entity construction
- Components implied by configuration of components
	- Theoretically could make sense as existence of components is immutable
	- Initial values not customizable, but calculated on the go or left default?
		- tree of npo would be added if there is no rigid body or fixtures but there is a sprite

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
		- while each type will have their own copy of data for speed, the editor can associate types and enable per-invariant overrides, so we could create a new type from an existing base type.
			- a type could be per-entity or per-
	- Conceptually the easiest.
	- **Easiest to get right**.
		- We can always improve it if needed.
	- Most memory wasted.
		- But not necessarily: no additional identifiers inside aggregates will be needed.
	- Lowest flexibility, e.g. the solver can't alter that state or it may with additional effort to make that information stateless.
		- The solver shall not alter invariant state.
		- If we find that it would be nice to actually alter that invariant state, it might be the case that we should always calculate this value from some other circumstances and not just read it straight from the invariant. Just like we do with ``resolve_density_of_associated_fixtures`` or ``resolve_dampings_of_body`` 
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
A custom readwrite overloads will be in order, both for lua and binary, which will take note of which initial component values are set, and serialize only the according invariants;  
At that point, serialization performance pretty much does not matter at all, and disk space may or may not come in handy.


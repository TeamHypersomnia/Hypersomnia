---
title: Entity flavour
tags: [topics, ECS, flyweights]
hide_sidebar: true
permalink: entity_flavour
summary: |
  An **entity flavour** contains information that is stored once and is later shared by one or more [entities](entity).  
  Formally, an **entity flavour** is a [flyweight](flyweight) whose instance is an [entity](entity).  
  An [entity](entity) specifies its **flavour** via its [flavour component](flavour_component). 
---

## Overview

So that working with content is easier, and so that less memory is wasted, each entity is divided into two kinds of data:
1. state that should never change and is supposed to be shared between entities,
	- ``gun::muzzle_velocity``
2. state whose instance is needed by each single entity, since it is constantly in flux.
	- ``gun::trigger_pressed`` 
  

The first is more like "input" to the processing, in which [authors](author) in particular are interested.  
Most of the time, only the programmers are concerned with the second type of data.

- An *entity flavour* contains a tuple of [invariants](invariant) and a tuple of initial values for each of the [components](component), exactly as specified in the [entity's type](entity_type).
- flavours can be created, copied and then modified, or destroyed.  
	- It is a separate stage that never occurs in logic itself.
		- In fact, the [logic step](logic_step) simply provides only const getters for the type information.
	- Should only be done by [authors](author).
	- In particular, flavours can be deleted, in which case the [vector of flavours](#storage) will have to be shrunk. Then we will need to remap flavours of all existing entities.
	- But disallow deleting a flavour if entities with that flavour exist.
		- The author will be able to change the flavour of an existing entity, optionally keeping the values that were already set in components.
	- In particular, the **initial value** for a component may change even though some entities of this flavour already exist.
		- This is not important. The field, in practice, serves two purposes:
			- Used by the logic when it gets a flavour identifier to spawn (e.g. ``gun::magic_missile_flavour``), so that it can set reasonable initial values. Thus, a change to initial value needs not updating here as the logic will naturally catch up, storing only the flavour identifier.
			- Exists as an optional helper for the author when they want to spawn some very specific entities. The author shouldn't worry that the initial value changes something for the existent entities.
	- If a field of a invariant changes, (during content creation), reinfer all objects of this flavour with help of the [flavour id cache](flavour_id_cache); currently we'll just reinfer the whole cosmos.
- There won't be many flavours, but access is **frequent** during [solve](solver#the-solve). 
	- All possible configurations of invariants will be determined by types at compilation time.
- The logic should always first check for the existence of component, only later it should check if the corresponding invariant exists.
	- One less indirection because checking for the id being unset is just a single arithmetic operation when the id is cached.
- On creating an entity, the chosen flavour's id is passed.  
	- Components might need some special construction.
		- interpolation might need to save the place of birth.
		- trace might want to roll the dice for its first values.
		- responsibility of entity constructors.
- **Editor**: Initial values
	- in a separate tree node.
	- modifiable are all that are implied by the invariants
	- and those that will be added on construction based on existence of other components. 
		- e.g. sender.
- We specify no clear bijection between invariant and component existence.
	- We will however make efforts to sensibly populate the ``assert_always_together`` and ``assert_never_together`` guards.
		- Simply because it is intuitive that if a rigid body component exists, so should the invariant.
	- E.g. some invariants do not need any instance data.
		- Example: render component.
			- That does not change anything at all, except for the way of getting that data.
- Some invariants might be so heavily used that an immutable copy in the entity itself would be beneficial.
	- Example: render component.
		- Perfectly viable; just declare it with a ``static constexpr bool`` inside the invariant and make the field constant.
		- Care: if the flavour information changes, so must the copied invariant. **So let's save it for later**.
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

While things like bullet flavours or rifle flavours may each have just one shape for all instances,  
take for example terrain: each of its pieces might possibly have a completely different shape,  
especially if it is some kind of curvy corridor or dungeon.  
These terrain pieces might, to the exception of their shape, behave identically, so it would make sense to have just one flavour for them.

~~Another case is names and custom names.~~  
Names are completely transparent to the logic so custom names will be managed only by domains that need it (client setup with custom nicknames, debugger setup with custom overridden names just for marking, etc).

In that case, we may:
- Leave things as they are.
	- **Chosen solution**: We require the [author](author) to create a new flavour for each of the piece;
		- the editor shall, obviously, facilitate this to the utmost. 
		- while each flavour will have their own copy of data for speed, the editor can associate flavours and enable per-invariant overrides, so we could create a new flavour from an existing base flavour.
			- a flavour could be per-entity or per-
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
The cache should always disregard copying of shapes since they belong to the flavours, which don't change at all and are shared.  
This will additionally dramatically improve performance of copying the cosmoi along with their inferred states.

``b2PolygonShape``s and others will need to be inferred from the significant vertex data in the entity flavours.  
That is because the struct is virtual, so may prevent direct std::memcpy, and also because it will hold convex-partitioned information.  

## Storage

Inside the [cosmos common significant](cosmos_common#significant), there is an object named ``flavours``.  
It maps flavour identifiers found in the [flavour component](flavour_component) to the respective **entity flavour**.  
Preferably, the container should be a constant size vector as the flavour ids will just be consecutive integers.  

Care must be taken that the storage is not treated like trivially copyable types during byte readwrite:
A custom readwrite overloads will be in order, both for json and binary, which will take note of which initial component values are set, and serialize only the according invariants;  
At that point, serialization performance pretty much does not matter at all, and disk space may or may not come in handy.


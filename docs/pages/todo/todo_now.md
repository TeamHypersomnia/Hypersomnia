---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  

- add post construction callbacks and use it e.g. in trace system
- replace "alive"/"dead" checks with optionals of handles
	- and assume that an existing handle always points to an entity
	- a lot of work but it will be worth it
	- in any case do it once everything else works
	
- remove redundant logic for componnt/inv lists once all is done and works
- remove "implied components"

- Constructing entities
	- Solver might want to set some initial component values before inference occurs
		- Should be done in a lambda where a typed entity handle is given 
		- After which cosmic::create_entity will simply infer all caches
		- provide an overload which sets transform
			- static assert if the entity cannot set a transform
	- Most constructions and clones will request general flavours and thus return general handles
		- Thus let's for now expose just those general ones

- Entity groups will be useful later, not until we make a simple deathmatch where we can include some simple weapon/car creation logic etc

- The mess with optionals around getters of transform, aabb, and colliders connection
	- Make it clear which functions get cache content and which actually calculate from the significant
		- Three? kinds of operations:
			- Gets actual value in the cache - fast, works only when constructed - always?
				- **should be a member function of the cache for fast access.**
					- then the logic can just get the cache once.
				- const-valued caches should be gettable.
				- some funny special logic will probably use it to do some additional physics calculations?
				- will probably be nice to pass around the invariant ref to avoid repeated getting
					- or just do so once we determine the bottleneck
			- Calculates the value to be passed to cache - slow, works always
			- Requests for a certain field to be recalculated
				- We know that a driver will only need a correction to damping and not entire body

- Instead of having "force joint" at all, make it so that while processing the cars, they additionally apply forces to drivers to keep them
 
- Let car calculate statelessly from movement flags in the movement component?
 
- Resurrect the unit tests for padding that were deleted along with cosmic delta

- Audiovisual caches should always check if the transform exist because sometimes the transform might be lost even due to having been put into a backpack.
	- especially the interpolation system.
		- **it should crash on transfer to backpack when we correct the transfers **
	- just use find transform
	- use clear dead entities as well because it minimizes sampling errors if e.g. the solve wouldn't run between steps
		- though I think we always run audiovisual advance after step

- There was a crash due to dead entity in driver setup finding a component
	- possible cause is that the gun system destroys item in chamber, but technically they should not trigger END_CONTACT
	as they are devoid of... hey, actually that might be because we still see those entities even if they are in backpack. 
	In that case, if the owner body of an item chamber is badly determined, it might make sense that it crashes
	- ensure that the posted subject in contact listener is alive, but that is most likely the case since we get physical components from them anyway
		- but ensures won't hurt
- Local setup should record session live
	- This is really important in the face of bugs.
	- Or just get rid of test scene setup for now and let it by default launch a scene in editor that records inputs

- Later, bullet trace sounds should be calculated statelessly
- For continuous sounds, sound system should probably assume the same strategy as an inferred cache.

## Later

- it probably makes no sense to use GUIDs for now if we are anyway going to transfer whole pools for determinism
- consider having entity guids in components instead of ids for simplicity of network transfers
	- there ain't really that many and it will be greatly useful

- thoughts about groups, templatized components, guids and network transfers 
	- **we will anyway templatize the components by the type of ids they store to have groups for example**
		- assume some **using stored_entity_id** that will be passed to component templates
		- assert with component introspectors that no entity_ids, pointers, unversioned_ids are stored
			- revive from cosmic delta tests
	- cloners won't have to introspect and rewrite respective child entities but we will always introspect by a single component
		- and just map integer-indexed ids to entities in group
		- entity will also know to which group it belongs thus we can preserve the structure of children
			- and later rewrite contents of respective entities
	- LUA can store per-guid entities and not store pools to maximize compatibility
		- we don't care that it will be recreated with a different order
		- having guids stored in components will make it easier
	- network transfers and binary savefiles can also serialize pools for consistency
		- thus this will work with both guids and entity_ids in components
		- we must ensure the order of entities stays the same for determinism
		- we don't care that much this little space overhead 

- normalize the names of getters in xywh/ltrb
- if a constant size vector needs to be introspected for example for description of fields in a component,
	add some special case code that checks if it is a container or something else
	- padding checker should just check the value type?
		- maybe we should ditch padding checks whatsoever
			- at least for the interior of the container, useless totally
				- maybe just verify the value type

- add "direct_construction_access" for entity handles

- fix errors at unit tests when not statically allocating 
- strip children vector tracker of children caches as we'll take that data from signi
	- was anyway used only for ensuring
- Groups can be defined separately from flavours, e.g. many groups can share the same flavours.
	- if we want to have a group wherein a weapon is spawned with a magazine, we can simply set the inventory slot ids beforehand.
		- We might detect errors early on and disallow some configurations or not care and just throw cosmos_inconsistent_error.
		- Group ids will not be creatable from the logic anyway?
			- Could be useful later for spawning cars or character via the logic etc
- traces
	- maybe traces should be audiovisual?
	- fix the feel of traces (maybe shrink them only horizontally?)
		- **only do this after we have editor**, obviously
	- fix interpolation issue or just customize it when editor is ready
		- should really be done once we have that facility in editor because debugging will be a LOT easier when we have the internals to tweak and inspect
	- should be fast enough
	- otherwise make it a super quick cache?
	- chosen_lengthening_duration_ms should be randomized statelessly with help of guid
	- store just stepped timestamp of when the trace was fired instead of incrementing the passed time 
- let meta.lua have convex partitions and let author just define those convex partitions for simplicity
	- let invariants::polygon have vector to not make things overly complicated
	- polygon component makes triangulation anyway
- audiovisual/inferred caches and reservation
	- if it so happens that std::unordered_map is too slow, we can always introduce constant-sized vectors/maps under STATICALLY_ALLOCATE_ENTITIES
		- each type will specify how many to statically allocate 
		- we can also make caches only for the types that fulfill given conditions of invariants/components existence

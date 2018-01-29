---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  

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

- Constructing entities
	- Introduction of native types will probably change it
	- Solver might want to set some initial component values before inference occurs
		- Should be done in a lambda where a typed entity handle is given 
		- After which cosmic::create_entity will simply infer all caches
	- In any case we know already we will at least need to set transforms in such constructors
		- So we can already make an overload for cosmic::create_entity
 
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

- sentience component should have integrated crosshair
	- provide a get crosshair transform function

- Thoughts about native types
	- We can make the entity_flavour's populators in ingredients just templated functions
		- For the sake of not repeating test scene flavour logic for some different native types
	- We will totally get rid of processing component and calculate statelessly which that which needs to be calculated.
		- We anyway very rarely ever disabled something in processing subjects and we must always account for the worst case.
	- Refer to typed_entity_handle for draft.
	- Rename "entity_flavour" to "entity_flavour" and use "entity flavour" to represent the natively assembled aggregate of invariants and its counterpart
		- entity_flavour_type vs entity_flavour 
	- Since the architecture will be corrected to the point where the solvers won't add any component,
	we could actually introduce native entity flavours.
		- dynamic_obstacle = sprite + render + rigid body + fixtures 
	- The authors should not be concerned with customizing invariant sets but with choosing native, well-understood presets.
	- This could be even introduced incrementally, e.g. entity handle would still perform type erasure to not compile everything in headers.
		- For now the authors may also see just presets without being able to add or remove invariants
		- Specialized handles would be returned by for_each for processing_subjects in the cosmos
		
	- Storage
		- Array of structs vs struct of arrays
			- Storage will be transparent to the logic, even if we don't introduce native types.
				- typed_entity_handle<character> will have perhaps cosmos& and character* 
				- general entity handle will have more than now, as it will have to perform type erasure. It will have type id and void* that will be reinterpret-cast. 
			- Changing ingredients to native types won't require more work than just adding a type specifier to get test scene type. 
				- meta.set will still apply as always
				- enums will also apply because many entity flavours might share the same native type
			- So we don't have to do it now.
			- We will specify storage for native types in tuples, thus we will be able to change SoA to AoS and back with just one compilation flag. 
	
- **moving disabled processing lists out of the significant state**
	- for now ditch the setter of rotation copying when switching driver ownership
	- most stateless, thence easiest is when we just easily determine when not to process an entity without relying on having proper fields set in processing lists
		- we should then only ever optimize when it is needed
	- or it can be part of inferred state which will complicate things a little

## Later
- replace child_entity_id with entity_id and let child component be synchronized
- child component should be always_present + synchronized
- strip children vector tracker of children caches as we'll take that data from signi
	- was anyway used only for ensuring
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


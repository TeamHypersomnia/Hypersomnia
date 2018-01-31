---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  

- Solutions for "children entities": entities that are cloned with some other entity and deleted with it
	- **Chosen solution:** delete_with component that is a synchronized component
		- Most flexibility and separation of concerns; childhood is not really something that will change rapidly, as opposed to damping or density
		- Groups could then easily specify initial values for entities in the group
			- will just be a common practice to set the delete_with to the root member of the group
				- Currently would anyway be only used for crosshair
		- Concern: fast bi-directional access to:
			- parent having just child, 
				- easy: access delete_with component
			- or to child having just parent
				- has to access the children vector cache
				- but what if it has more children with varying functionality?
					- then we can add an enum to the child
				- do we say, we always treat the first child as e.g. a crosshair?
					- NO! We can simply associate a character with any entity as its crosshair!
					- The existential_child itself will only be used for deletion.
					- It may coincidentally point at its parent.
			- The difference is like this:
				- The parent can have many children, thus it makes no sense to make assumptions about order of children in the cache or some enums.
				- We'll just have entity_id's scattered through relevant domains that imply some acting upon the other entity or reading therefrom. 
				- If we have a delete_with component, we may assume some other role of its parent. 
					- **We should actually just make the child be associated coincidentally with just some entity_id**.
		- Concern: childhood type? 
			- Assumption: a child can only have one delete_with parent
				- But it may have more logical parents, e.g. it can be attached to some body as an item?
			- Do we want to assume that, if a child has delete_with set, that the associated parent fulfills some other role as well?
				- Concerns would mix, but that would imply less data which is fine
					- And that lets the rest of data stay raw and not synchronized
				- Rename to "logical_parent"?

- Constructing entities
	- Solver might want to set some initial component values before inference occurs
		- Should be done in a lambda where a typed entity handle is given 
		- After which cosmic::create_entity will simply infer all caches

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

- Thoughts about entity types
	- specifying types
		- tuples/trivially copyable tuples, because they will be easy to introspect and reason about
		- should components be added automatically?
			- we might want to arrange some components for better cache coherency
			- instead, let's just have "assert_component" instead of "implied_component"
		- proposition: a struct [[type]] and "using variants" "using components" inside
			- provides a canonical, consistent way to refer to an entity type everywhere
			- can also easily forward declare such types
			- maybe even enables us to define some custom hacks for the types in the structs?
		- proposition: using [[type]]_variants and using [[type]]_components
			- con: doesn't actually give a simple way to associate these two
	- first implement allocation and identification, processing can for now stay as it is
	- mixin templates won't make sense to be instantiated
		- because of the typed handles
	- typed vs general entity id and handles
		- Entity ids will need to have information about which type the entity is
			- A typed id that can accept two or more entity types still needs a number and is inperformant in the same way that a general handle is(probably)
				- One-typed ids will be very rare anyway
			- type id will be in cache once we access the entity id so it will be only several more cycles to calculate the offset
		- Sytematic processors (cosmos.for_each(...))
			- would acquire a type list of (possibly const-qualified) components and invariants 
				- the list specifies which ones we **get** for sure
				- we don't specify which ones we will try to "find"
				- process all entity types except those that don't have specified components for **get**
			- removing the need for processing lists
		- Thus let's just go with the general ones
		- Only the systematic processors should acquire typed handles via generic lambda callbacks
			- The interface should be compatible with entity handle except get should fail at compile time for non-existent invariants/components
<!--
		- It would be interesting to, for example, typize entity ids 
			- so that only a certain subset of entity types can be referenced by them
			- then we could know in compile time if get<some component> cannot be performed on a handle with a given id
		- Could be introduced at a later time and for now we could just introduce general entity handles and ids everywhere.
-->
	- The authors should not be concerned with customizing invariant sets but with choosing native, well-understood presets.
	- This could be even introduced incrementally, e.g. entity handle would still perform type erasure to not compile everything in headers.
		- For now the authors may also see just presets without being able to add or remove invariants
		- Specialized handles would be returned by for_each for processing_subjects in the cosmos
	- We can make the entity_flavour's populators in ingredients just templated functions
		- For the sake of not repeating test scene flavour logic for some different native types
	- We will totally get rid of processing component and calculate statelessly which that which needs to be calculated.
		- We anyway very rarely ever disabled something in processing subjects and we must always account for the worst case.
	- Refer to typed_entity_handle for draft.
	- Storage (TRANSPARENT)
		- Array of structs vs struct of arrays
			- Storage will be transparent to the logic, even if we don't introduce native types.
				- typed_entity_handle<character> will have perhaps cosmos& and character* 
				- general entity handle will have more than now, as it will have to perform type erasure. It will have type id and void* that will be reinterpret-cast. 
			- Changing ingredients to native types won't require more work than just adding a type specifier to get test scene type. 
				- meta.set will still apply as always
				- enums will also apply because many entity flavours might share the same native type
			- So we don't have to do it now.
			- We will specify storage for native types in tuples, thus we will be able to change SoA to AoS and back with just one compilation flag. 

- Instead of having "force joint" at all, make it so that while processing the cars, they additionally apply forces to drivers to keep them
 
- Let car calculate statelessly from movement flags in the movement component?
 
- Resurrect the unit tests for padding that were deleted along with cosmic delta

- consider having entity guids in components instead of ids for simplicity of network transfers
	- there ain't really that many and it will be greatly useful

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
- Groups can be defined separately from flavours, e.g. many groups can share the same flavours.
	- if we want to have a group wherein a weapon is spawned with a magazine, we can simply set the inventory slot ids beforehand.
		- We might detect errors early on and disallow some configurations or not care and just throw cosmos_inconsistent_error.
		- Group ids will not be creatable from the logic anyway?
			- Could be useful later for spawning cars etc
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

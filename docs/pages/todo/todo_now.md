---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  

- position copying becomes the only way to change the entity's logic transform beside the rigid body and the wandering pixels
	- thus the only one with the problem of properly updating NPO 
		- We don't need NPO to track particle existences for now as there really won't be that many. 200 static ones would be so, so much already.
	- we can make it so that static particle existences don't even have the position copying component
- the only current use: particles and sound existences that chase after the entity
	- **solution**: 
	store a relevant field in each component that potentially needs it, 
	statelessly calculate transform when needed in particles simulation system,
	**statelessly determine whether particles should be activated (due to car engine or others) in particles simulation system** (that info could be stored in child component of chased entity)
	(not just activationness, the amount itself could be calculated)
		- pro: **quality of statelessness**
			- simplicity of code
			- less state transferred through the network
		- pro: particles and sounds can specify different chasings in the same entity
		- con: minimal duplication of code
		- con: worst performance as many WITH_PARTICLES_EXISTENCE need to be iterated through
			- can be optimized later via synchronizers and caches, without breaking content
			- as it's audiovisual mostly, this kind of thing can even be multithreaded
			- **and it moves load off the server**
			- if we're really furious, we can even make one thread continuously regenerate an npo from which then a renderer reads every frame
				- otherwise we would anyway need to update npo tree every frame which would be too big of a load for server
			- it can even be faster to iterate all particles existences while being cache-coherent.
	- additionally, expose a "get_particles_transform" to disambiguate get_logic_transform - the latter should not check for particles existence.
		- because a fixtural entity might additionally have a particles existence that chases upon itself.
			- e.g. truck engine

- Thoughts about native types
	- We will totally get rid of processing component and calculate statelessly which that which needs to be calculated.
		- We anyway very rarely ever disabled something in processing subjects and we must always account for the worst case.
	- Refer to typed_entity_handle for draft.
	- Rename "entity_type" to "entity_flavour" and use "entity type" to represent the natively assembled aggregate of definitions and its counterpart
		- entity_flavour_type vs entity_type 
	- Since the architecture will be corrected to the point where the solvers won't add any component,
	we could actually introduce native entity types.
		- dynamic_obstacle = sprite + render + rigid body + fixtures 
	- The authors should not be concerned with customizing definition sets but with choosing native, well-understood presets.
	- This could be even introduced incrementally, e.g. entity handle would still perform type erasure to not compile everything in headers.
		- For now the authors may also see just presets without being able to add or remove definitions
		- Specialized handles would be returned by for_each for processing_subjects in the cosmos
		
	- Storage
		- Array of structs vs struct of arrays
			- Storage will be transparent to the logic, even if we don't introduce native types.
				- typed_entity_handle<character> will have perhaps cosmos& and character* 
				- general entity handle will have more than now, as it will have to perform type erasure. It will have type id and void* that will be reinterpret-cast. 
			- Changing ingredients to native types won't require more work than just adding a type specifier to get test scene type. 
				- meta.set will still apply as always
				- enums will also apply because many entity types might share the same native type
			- So we don't have to do it now.
			- We will specify storage for native types in tuples, thus we will be able to change SoA to AoS and back with just one compilation flag. 
	

- Bugs:
	- particles spawn not where they should
	- Bad calculation of grenade explosion locations
		- perhaps get_logic_transform?
	- explosion sounds dont get played, but this is due to the previous

- particle existence will later have an entity id which to chase thus it won't need get logic transform
	- check if there are no problems with its NPO detection
	- there might however occur a need to optionally set initial transform in a definition group to one of other group entity (maybe + offset) 

- Rename "fixtures" to "colliders"

- Instead of having "force joint" at all, make it so that while processing the cars, they additionally apply forces to drivers to keep them
- Make it clear which functions get cache content and which actually calculate from the significant
	- Three? kinds of operations:
		- Gets actual value in the cache - fast, works only when constructed - always?
			- **should be a member function of the cache for fast access.**
				- then the logic can just get the cache once.
			- const-valued caches should be gettable.
			- some funny special logic will probably use it to do some additional physics calculations?
			- will probably be nice to pass around the definition ref to avoid repeated getting
				- or just do so once we determine the bottleneck
		- Calculates the value to be passed to cache - slow, works always
		- Requests for a certain field to be recalculated
			- We know that a driver will only need a correction to damping and not entire body
	- Notice that there is no point in having fixtures component

- Implement entity types incrementally.
	- We do it now to not repeat the code when we'll be writing improvements to inferences 
		- gun component 
			- firing engine sound and muzzle particles are "persistent child entities"
				- thus they should become group entities.
					- That will be only possible once we definitionize practically everything.
			- magic missile def is just a child entity to be cloned around as needed.
				- thus it only becomes type id.
		- catridge component
			- whole component should become definitions and its child entity id fields should just become type ids
	- Get rid of component adders and deleters.
		- That will be only possible once we definitionize practically everything in the test scenes.
		- We'll thus need to make sender and others "always present".
- Current slot should stay in the item component as other fields of item will anyway be a private
- **moving disabled processing lists out of the significant state**
	- for now ditch the setter of rotation copying when switching driver ownership
	- most stateless, thence easiest is when we just easily determine when not to process an entity without relying on having proper fields set in processing lists
		- we should then only ever optimize when it is needed
	- or it can be part of inferred state which will complicate things a little
- force joint components should be "custom" in a sense that they can be added and they do not override anything else
	- there should be one cache per joint type, e.g. attachment joint cache which is inferred from current inventory state 
- Describe two kinds of state: constant-divergent and exponentially-divergent
	- tree of NPO, sprites, polygons, renders: constant divergence
	- sentience, fixtures, rigid body: exponential divergence

## Later
- replace child_entity_id with entity_id and let child component be synchronized
- child component should be always_present + synchronized
- strip children vector tracker of children caches as we'll take that data from signi
	- was anyway used only for ensuring
- remove all_inferred_state as it is essentially pointless once we have types implemented
- fix car component to calculate damping statelessly, it will need to be synchronized
	- in practice car won't ever be an item or a driver or have movement... 
- implement instances of cooldowns for casts statelessly when there is such a need
- calculate trace component statelessly
	- fix interpolation issue or just customize it when editor is ready
		- should really be done once we have that facility in editor because debugging will be a LOT easier when we have the internals to tweak and inspect
	- should be fast enough
	- otherwise make it a super quick cache?
	- chosen_lengthening_duration_ms should be randomized statelessly with help of guid
	- store just stepped timestamp of when the trace was fired instead of incrementing the passed time 
- fix the feel of traces (maybe shrink them only horizontally?)
	- **only do this after we have editor**, obviously


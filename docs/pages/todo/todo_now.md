---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  

- Bugs:
	- particles spawn not where they should
	- editor crash


- particle exitence will later have an entity id which to chase thus it won't need get logic transform
	- check if there are no problems with its NPO detection
	- there might however occur a need to optionally set initial transform in a definition group to one of other group entity (maybe + offset) 

- set transforms everywhere?
- Currently, logic transform might be ambiguous if:
	- The entity has a rigid body and also fixtures that belong to some remote rigid body.
		- We will always prefer fixtures if they exist.
- If we want to destroy b2Body data once all fixtures are destroyed, it would be a case without precedent where the state of the parent cache depends on some of its children.
	- Let's just allow bodies to stay without fixtures for now as it will be easier. We'll see what happens.
	- Thus the former owner body won't have to be reinferred once a remote fixture entity gets destroyed.
	- In any case, we could just implement it in the physics engine side that the body gets removed from the world list if the last fixture was removed.

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

- What now? An all-out attack?
- Definitionizing rigid body
	- velocity and transform should always be able to be set directly as they are temporary parameters of the simulation, not statelessly calculated on any state.
		- better wording: these variables are accumulators
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
- Remove owner_body from fixtures component and create a component named "custom rigid body owner" that overrides anything else
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


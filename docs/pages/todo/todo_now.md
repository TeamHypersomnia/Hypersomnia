---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  

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


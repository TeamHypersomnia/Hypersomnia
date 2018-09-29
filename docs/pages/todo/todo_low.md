---
title: Low priority ToDo
hide_sidebar: true
permalink: todo_low
---

- rename world, cosmos to cosm everywhere

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

- it probably makes no sense to use GUIDs for now if we are anyway going to transfer whole pools for determinism

- replace "alive"/"dead" checks with optionals of handles
	- and assume that an existing handle always points to an entity
	- a lot of work but it will be worth it
	- in any case do it once everything else works

- Delta widgets for property editor
	- Will be useful for offsets
	- New tweaker_type?
		- Based on this, set is_delta boolean in change_property_command
		- can_accumulate_v trait

- Shuffled animations
	- Inside randomizing system?

- Commandize "Add player" and "Restart" in editor

- Implement ctrl+tab for settings window
	- shortcuts for first letter shall always focus relevant window

- add dialog asking if we want to leave changes?
	- for now we overwrite

- Later, bullet trace sounds should be calculated statelessly

- Resurrect the unit tests for padding that were deleted along with cosmic delta

- inventory slot handle should itself decide when to use generic handle
	- at this point it is only a performance improvement
	- or perhaps several compilation error checks?


- fix errors at unit tests when not statically allocating 
- Groups can be defined separately from flavours, e.g. many groups can share the same flavours.
	- if we want to have a group wherein a weapon is spawned with a magazine, we can simply set the inventory slot ids beforehand.
		- We might detect errors early on and disallow some configurations or not care and just throw cosmos_inconsistent_error.
		- Group ids will not be creatable from the logic anyway?
			- Could be useful later for spawning cars or character via the logic etc

- fix mess with sound sources
	- currently the app might crash if the sound system exceeds maximum sound sources and main menu suddenly is set
		- we can either let main menu be always present (not in optional), and just clear the intercosm
			- I think this is the way. We can later remove completely main menu from build for the server app.
				- because optional or not, it wastes space on the stack.
	- perhaps the audio context should preallocate some hover/click sounds that are omnipresent?

- implement instances of cooldowns for casts statelessly when there is such a need

- Describe two kinds of state: constant-divergent and exponentially-divergent
	- tree of NPO, sprites, polygons, renders: constant divergence - they do not propagate further
	- sentience, fixtures, rigid body: exponential divergence

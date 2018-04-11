---
title: Low priority ToDo
hide_sidebar: true
permalink: todo_low
---

- Add a flag to build editor or not
	- Because it builds really long

- use per_entity_type_container widely
- add dialog asking if we want to leave changes?
	- for now we overwrite

- Later, bullet trace sounds should be calculated statelessly
- Resurrect the unit tests for padding that were deleted along with cosmic delta

- replace "alive"/"dead" checks with optionals of handles
	- and assume that an existing handle always points to an entity
	- a lot of work but it will be worth it
	- in any case do it once everything else works

- inventory slot handle should itself decide when to use generic handle
	- at this point it is only a performance improvement
	- or perhaps several compilation error checks?

- clang
	- test clang in RAM (tmpfs)
	- set more warnings and fix them
	- consider running static analyzer

- remove redundant logic for componnt/inv lists once all is done and works

- it probably makes no sense to use GUIDs for now if we are anyway going to transfer whole pools for determinism

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


- if a constant size vector needs to be introspected for example for description of fields in a component,
	add some special case code that checks if it is a container or something else
	- padding checker should just check the value type?
		- maybe we should ditch padding checks whatsoever
			- at least for the interior of the container, useless totally
				- maybe just verify the value type

- fix errors at unit tests when not statically allocating 
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


- fix mess with sound sources
	- currently the app might crash if the sound system exceeds maximum sound sources and main menu suddenly is set
		- we can either let main menu be always present (not in optional), and just clear the intercosm
			- I think this is the way. We can later remove completely main menu from build for the server app.
				- because optional or not, it wastes space on the stack.
	- perhaps the audio context should preallocate some hover/click sounds that are omnipresent?

- implement instances of cooldowns for casts statelessly when there is such a need

- rename world, cosmos to csm

- Describe two kinds of state: constant-divergent and exponentially-divergent
	- tree of NPO, sprites, polygons, renders: constant divergence - they do not propagate further
	- sentience, fixtures, rigid body: exponential divergence

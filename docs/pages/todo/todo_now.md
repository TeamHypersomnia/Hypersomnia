---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

describe kinds of sensitive state and document their behaviour

- too costly to be calculated each time and thus cached in some other significant state?
	- currently the case with damping values
	- why not make current damping an inferred state?
		- standard one would be set normally in signi and would not be sensitive with regards to other signi
	- we can make all sensitive state just be subject to synchronizers?
	- what about timers for transfers ~~and reloads~~ (reloads are just timed transfers)?


separate "owner_body" and "current_slot" after all.
they are different concerns whatsoever: rigid body is basically just input, only description of a standard without any special circumstances.

current slot will also have index in container
drivers dont need one as there's always one
solver should never let this state be correupted and it will never be visible to author
so reinferences can even assert on that

component types:
	- circumstantial, immutable by the author

components order matter for reinferences order

order here will be important, in particular, systems should also be ordered well so that:
circumstantial_fixtures_owner_cache
circumstantial_damping_cache
circumstantial_density_cache
are before, for example
hysics_world_cache
which puts everything together.

remember that anyway those two are parts of definitions

what about AI?
- let pathfinding be a circumstantial component?
- we don't care much about that AI alters some state because virtually only ai will use that pathfinding


we consider whole type overrides too complex architeciturally:
- the assumptions about inferred state for entity types are gone; previously it was just generated once and set in stone, always correct
- noise while getting those definitions
- we should discourage solver from ever modifying that state
	- otherwise to look good we would need to always completely destroy that entity and recreate it with components with default sensitive values

shouldnt clone entity use a safe entity handle interface?

Currently, cloning performance of cosmos solvable with static allocation may suffer due to the fact that constant size vector uses default array's operator=, (as component aggregate is trivially copyable).
This means that all slots for entities will get cloned, not just allocated ones.
On the other hand, it is good that some components use default operator= so that all can be cloned in a single pass.

**We will just need to create an operator=(cosmos_solvable_significant&) for the solvable signi that takes note of that and clones the pools manually.**

we should write more carefully about reinferences with entities/components that have relations 

move "game/common_state" files to a more proper location

write about how reinference should ensure consistency of state

WHEN ORDER OF ITEMS IN THE CONTAINER BECOMES RELEVANT, 
This procedure should be fixed or otherwise the reinference might break the order of items!

synchronizers should expose a static reinfer_caches method for a handle that would then be used by entity handles.

fixtures can form scene graph as they have relative transforms.
	position copyings could be calculated statelessly in get_logic_transform, and only changed when performance so requires
transform logic might need refactoring for statelessness?

components should be templatized by id
	lets us define groups in definitions 
	synchronizers will anyway work with entity_id specialization thus not much more will be templatized
	introspectors could static assert against usage of entity ids in non-template components

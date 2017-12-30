---
title: ToDo
hide_sidebar: true
permalink: todo
summary: Just a hidden scratchpad.
---

window should not be concerned with mouse pos pausing and last mouse pos.
let there be some separate mouse wrangler class or sth

fix freetype charsets

animation should work statelessly, in particular it should not set values to sprite.

pass display size and implement augs::get_display

restore the backup if the quickfix works better in grepper there

rename "fundamental" to something else?

fix cosmic_delta so that it operates on solvable state only without handles. 

we could make dynamically allocated tuple of overridden definitions for an entity that would normally just take one id,
that would map to a tuple of ids. as overriddes would be anyway rare that is will not hinder performance
we could always just say that we will always require new type to be defined wherever we would like to override something
so instead of explicitly making the thrown body bullet in the hand fuse logic, we'll just always leave it to the author to create a bullet body and assign a type id into the proper component


Types will have prefabs to be able to choose from.
E.g. a functional gun.
a functional sound effect.
a functional particle effect.
thus we won't hold particle effect inputs in components but just entity ids.


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

editor should print "types of selected entities" and their common properties, identically as with entities/components

editor bindings:
v - begin selection with arrows


we dont want clone entity as separate; use safe entity handle interface to perform the clone instead of meddling directly with the signi
after you're done eating you can create a new branch with existing changes

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
physics_world_cache
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


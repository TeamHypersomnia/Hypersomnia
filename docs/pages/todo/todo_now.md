---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Contents

### Microplanned implementation order:  
- strip children vector tracker of children caches as we'll take that data from signi
	- was anyway used only for ensuring
- separate "relational cache" into several classes
- replace "reinfer all caches of" calls in entity handle with correspondent on_remove and on_add which will then infer or destroy
	- handle calls it either if the member function is present or if component is synchronized
		- so required in the latter case
- separate a "custom_rigid_body_owner" component so that caches/components have 1:1 ratio
	- so that fixtures cache does not need to construct both the relational and fixtural caches
- introduce dependencies between caches (or just for now make them in proper order and reinfer all on adding a sensitive component)
	- e.g. fixtures of bodies would depend on items of slots
	- each cache defines ``using depends_on = type_list<items_of_bodies, ...>`` etc.
		- on inference or destruction, iterate through all caches and reinfer them if they depend on the currently modified one  

#### Later
- remove all_inferred_state as it is essentially pointless once we have types implemented

### Microplanned implementation order (done):  
- rename fundamental to always_present
	- so that it is intuitive that there are no callbacks for adding and removing

as complete entity reinference wont be an explicit op but rather implied by removing all components,

just make it so each synchronizer defines infer and deinfer(or on_add/on_remove as there might be more ops under the hood) and there does not have to be "for each tracker" or something
because complete reinference can use all_inferred_state destructor anyway
so physics world cache would still be a single class because joint, fixtures and rigid body caches wouldnt be able to exist without the other anyway.
	It can also hold children vector  tracker for rigid bodies.
	likewise all caches might just be named after components and not that they track things
static constexpr bool can_inference_fail = true

theoretically some state could be so expensive to calculate that it needs to be a part of signi instead of inferred?

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

order here will be important, in particular, caches should also be ordered well so that:
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


Reiteration #1:

- Getting information about success from reinference
Some configurations of solvable significant do not allow for an inferred counterpart that is consistent.  
E.g. if vector of items inside a container is inferred, and one would like to perform a transfer by 
Here's where helper functions might be necessary. A perform_transfer function should only do as much as predict the outcome of reinferring the item component with newly set current slot,
and if it is successful, do set it and reinfer the item.

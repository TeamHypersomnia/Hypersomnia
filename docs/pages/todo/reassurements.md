---
title: Reassurements
hide_sidebar: true
permalink: reassurements
summary: We don't need to do this yet, because...
---

- It makes little sense to enable editor GUI inside gameplay mode.
	- We'll just make it possible to interact with imgui controls in-game.
	- We don't have some fancy replay-preview stuff like in that other game we saw somewhere...
	- and anyway it's better to move some important objects while paused, while we can snap to grid, etc.

- For now, don't make commands for creating/deleting/duplicating mode vars
	- Just in testbed populate all types with one vars instance

- For now don't worry about maximum size, it's enough until DM
	- In production, we will always reserve max used ever on startup
		- we'll reserve the maximum (which will probably be half the maximum texture GPU size)
	- In debug builds, we'll really do whatever

- Further parallelizing the neon maps is not quite worth the effort.
	- Ultimately, we won't have such giants anyway.

- We will keep the audiovisual state global for all setups.
	- The cosmos address probably won't change in networked scenarions...
	- ...we will just rewrite the solvable, really. That's the most that will happen
		- we won't really juggle around the actual displayed cosmoi, I think
			- even if we do, we can think about it when we get to it.

- what do we do with invalid sprite ids?
	- We guarantee the validity of the ids everywhere. Period.
		- That is because the editor will forbid from setting invalid values.
		- And adding a new value to the container will call a sane default provider.
		- Also removing a currently used id will be forbidden.
	- E.g. sprites, animations or other invariants.

- two animation usages will most probably only ever differ by speed.

- Later if we really need it for descriptions, we can retrieve the old values by accessing the field address, in just the same way as the command logic does.

- for now we'll put an animation id inside wandering pixels,
	- so let's just leave the unique_ptr<field_address> actually_element for later
		- i think we will be able to do this nicely
	- struct_field_address that will be returned by make_struct_field_address

- We will never be met with a situation where no images are in the project.
	- That is because common will always use some,
		- and will be initialized with sensible values from the start
	- and you can't erase images if they are used.


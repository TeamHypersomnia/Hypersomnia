---
title: Reassurements
hide_sidebar: true
permalink: reassurements
summary: We don't need to do this yet, because...
---

- The leak from ImDrawList::CloneOutput() is not important
	- It's only once on exit and we'd be more likely to break something trying to fix that and synchronously deallocate it
	- If we do anything with it, we should prevent it from allocating each frame in the first place

- I'd standardize max_ricochet_angle for materials so there aren't too many
	- Other values, we might customize per-resource, why not

- We can leave _1 _2 _3_ etc for sounds
	- Contrary to animations, these variations are done on augs audio library level
	- Someone might want to reuse the other numbers for something else (or just listen to them separately) so nothing to worry about here
	- the first one automatically uses a variation

- This light model is good because
	- Texture allows you to CAP the max light exposure to ensure color fidelity
	- Whereas the color parameter lets you change how much it actually absorbs light

- BTW property-per-resource is GOOD
	- Because it encourages people to create DIFFERENT sprites for DIFFERENT behaviors
	- So that the player ALWAYS knows what to expect when they see an object

- Note that an occassional static wall intersecting another static wall won't break lighting, the raycasts will just miss it
	- It's only important that we have a proper vertex at the corner

- For now don't worry about maximum texture size, it's enough until DM
	- In production, we will always reserve max used ever on startup
		- we'll reserve the maximum (which will probably be half the maximum texture GPU size)
	- In debug builds, we'll really do whatever

- Finer-grained parallelization of the neon maps is not quite worth the effort.
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

- We will never be met with a situation where no images are in the project.
	- That is because common will always use some,
		- and will be initialized with sensible values from the start
	- and you can't erase images if they are used.

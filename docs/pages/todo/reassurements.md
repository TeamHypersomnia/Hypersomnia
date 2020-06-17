---
title: Reassurements
hide_sidebar: true
permalink: reassurements
summary: We don't need to do this yet, because...
---

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

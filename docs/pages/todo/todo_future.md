---
title: ToDo future
hide_sidebar: true
permalink: todo_bugs
summary: Just a hidden scratchpad.
---

- Switching to fixed point
	- Fix cases like rng.randval(-1.f, 1.f) where the template will resolve to floats
- Maps could be git repositories, maybe even hosted on github
	- Server could specify upstream url
	- Updating a map just involves pulling

- window should not be concerned with mouse pos pausing and last mouse pos.
	- why lol?

- Instead of having "force joint" at all, make it so that while processing the cars, they additionally apply forces to drivers to keep them
- Let car calculate its flags statelessly from movement flags in the movement component of the driver?
	- less noise in the cosmos indeed

- introduce cosmos::retick function that can change delta while preserving timings by updating all stepped timestamps according to lifetimes found in other places

- circular bars might be useful to be specifiable per-cosmos
	- unlikely, though

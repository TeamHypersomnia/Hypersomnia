---
title: ToDo now
hide_sidebar: true
permalink: brainstorm
summary: Just a hidden scratchpad.
---

## Microplanned implementation order

- flavours and entities are concepts disparate enough to warrant separate trees completely
	- same code can handle them, though
	- or is one subset of another?
	- all are the same, except that
		- checkbox target differs
			- actually for entities we may also make them selected?
		- entities browser does not view the flavour properties
		- flavour browser does not view its entities
	- we won't have separate windows, just a radio box at the bottom

- for_each_object_and_id -> for_each_id_and_object, and move it to some range templates
- perhaps entity_flavours should just be a direct pool

- Fix glitch with flicker on the entire screen
	- probably particles animation bug

- use pools and pool ids for:
	- all logical assets
	- viewables
	- flavours
		- later will be sparse pools where performance requires it

- tests of traits: check no ids in invariants, at least
	- screw initial components really

- disallow removing last frame from the animation and always require to create an animation from an existing set of frames
	- simplifies calculations

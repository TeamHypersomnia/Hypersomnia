---
title: ToDo now
hide_sidebar: true
permalink: brainstorm
summary: Just a hidden scratchpad.
---

## Microplanned implementation order

- always_false -> identity_templates

- use pools and pool ids for:
	- all logical assets
	- viewables
	- flavours
		- later will be sparse pools where performance requires it

- make particles hold animation ids, current time and current frame number

- tests of traits: check no ids in invariants, at least
	- screw initial components really

- fix all logical assets to not use those crazy tuples and getters, wow

- disallow removing last frame from the animation and always require to create an animation from an existing set of frames
	- simplifies calculations

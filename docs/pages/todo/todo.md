---
title: ToDo
tags: [planning]
hide_sidebar: true
permalink: todo
summary: Just a hidden scratchpad.
---

- research building with clang on windows?

- Local setup should record session live
	- This is really important in the face of bugs.
	- Or just get rid of test scene setup for now and let it by default launch a scene in editor that records inputs

- Fix rendering order of dropped gun attachments
	- Also make it possible to always render them under?

- consider having entity guids in components instead of entity ids for simplicity of network transfers
	- question is, won't we anyway be sending the whole pool state?
		- it's just several bytes overhead per entity
	- there ain't really that many and it will be greatly useful

- Fill new workspace with test scene essentials
	- This would prevent image ids in common state from being invalid
		- Probably less checks to make, in GUI as well
	- Can make those the first in test scene image enums so that we can stop importing images after some point

- copy gfx and sfx folders on save as...

- let particle definitions be split into the invariant and variant parts, like components
	- pro: better cache coherency

- templatize some dangling memory stream arguments

- in go to dialog, make selection groups appear as the first
	- later we might just make a variant of several types instead of entity_guid in match vector

- fix saving of the editor view state when the work is saved to some non-untitled location
	- notice that, we might later introduce some caches for selections to improve performance
		- e.g. only always calculate the selection's aabb and rotation centers once
	- we might also want to encapsulate setting panning cameras and selections to have stacks
		- where we'll able to set modification flags easily
	- **therefore let's do it when we get to these stacks.**
		- because camera & selection state isn't all that critical
			- and untitled work for testing is always saved anyway
	- we can just always write the camera to disk?
		- it's bad though as it incurs disk activity every time
		- and selection state isn't that small
	- we could either set an "is_modified" flag or track it in history
		- not much modification points for selections, so quite viable

- pass display size and implement augs::get_display

- some hint for windows unicode in imgui: https://github.com/ocornut/imgui/issues/1060

- finish work with atlas and sound regeneration
	- regenerate only seen assets
	- always load diffuse map with the corresponding neon map to avoid unilluminated objects near the camera...
		- ...even though many diffuse maps nearby have been loaded
		- perhaps we won't really need the separation between diffuse and neon, because maximum atlases will be huge anyway

- Fix what happens when too many entities are created
	- **Let the game work when a new entity cannot be created.**
		- Just return a dead handle.

- Shake on shooting with a sniper rifle

- Dashing
	- Assigned to space, since we don't have jumping anyway
	- pure color highlight system could be used to add highlight the dashing entity
	- Gradually increase walking force over time
	- The more the speed during dash, the stronger the dash

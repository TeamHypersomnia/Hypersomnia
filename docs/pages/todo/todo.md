---
title: ToDo
tags: [planning]
hide_sidebar: true
permalink: todo
summary: Just a hidden scratchpad.
---

- enum_boolset begin and end should probably return enums
	- simpler lua i/o
	- no need for for-eaches
	- nice exercise for iterators

- When joining a game, allow choosing the flavour of the character
	- Each might have completely different properties or animations
	- Effectively implements a class

- Arbitrary pasting of entities
	- Vastly useful for importing stuff from testbed maps into existing ones
		- Let alone between community maps
	- This is pretty complex.
		- If we allow pasting across workspaces, we might even have to copy actual asset files.
	- Won't matter until after deathmatch stage
		- Surely?
	- The editor will have to construct the command tree, like
		- paste_flavours + paste_entities, if no requisite flavours are found inside the project at the time of pasting
			- the clipboard will have both the entity and flavour
		- the editor's clipboard can actually become...
			- paste entity flavour + paste entity command, stored, waiting to be executed!
				- the move itself won't need to be stored
	- Cut is just copy + delete

- Allow to change the tickrate in the non-playtesting mode?
	- This will be a session setting, really
	- We'll just restart on re-tick

- Overwrite notice in editor

- Refactor: conditional log macro

- Always initialize the hotbar with some values if it is not yet initialized

- Commands refactor: separation of undo and redo state
	- redoer and undoer objects
		- each has only the required information
		- redoer returns an undoer
		- redo state is always the same

- Balance colliding bullets so that damage to be dealt is subtracted from the stronger bullet
	- Possibly reduce trace size then

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

- Shake on shooting with a sniper rifle

- Dashing
	- Assigned to space, since we don't have jumping anyway
	- pure color highlight system could be used to add highlight the dashing entity
	- Gradually increase walking force over time
	- The more the speed during dash, the stronger the dash

- Note: drone sound (sentience sounds overall) will be calculated exactly as the firing engine sound
	- Sentience humming caches
	- These don't need their playing pos synchronized

- Since mouse motions will probably be the bottleneck of network communication, both coords will usually be able to be sent in one byte
	- Range would be 0 - 16
	- There would be a bit flag for when it exceeds the range
		- Which will actually be often and then we might use two bytes
			- mouse motion should never really exceed 255
				- will also make it harder to use aimbots that instantaneously change mouse location


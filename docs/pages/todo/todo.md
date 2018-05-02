---
title: ToDo
hide_sidebar: true
permalink: todo
summary: Just a hidden scratchpad.
---

- later print shortcuts in the menus for the windows
	- e.g. History			Alt+H

- copy gfx and sfx folders on save as...
	- For now on re-opening the newly saved project these resources will be missing

- add this maybe? https://github.com/jpakkane/naturalsort
- properly templatize entity_id_base, add entity type's key to it

- research building with clang on windows?

- let particle definitions be split into the invariant and variant parts, like components
	- pro: better cache coherency

- templatize some dangling memory stream arguments
- add unique sprite decoration type

- normalize the names of getters in xywh/ltrb

- consider having entity guids in components instead of ids for simplicity of network transfers
	- there ain't really that many and it will be greatly useful

- in go to dialog, make selection groups appear as the first
	- later we might just make a variant of several types instead of entity_guid in match vector

- handle mouse glitches when letting go of moved entities or duplicated ones
	- reset some drag/press state etc.

- fix characters getting typed when alt+something combos are pressed
	- once we have combo maps, there will be a separate function to determine whether the input is to be fetched
		- it will also return true on "character" input
	- a separate function will actually respond to combos

- fix concatenation of shakes
	- shake for ms
	- shake mult
	- impact velocity mult

- visible_entities should be templated by all_type
	- could be instantiated
	- is it necessary though?

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

- implement constrained handles and entity ids
	- because inventory slot handle item getters should return handles that guarantee presence of an item
	- thanks to that we can avoid problems with having many entity types and enlarging the dispatch code

- animation in our architecture
	- should work statelessly, in particular it should not set values to sprite.
	- callbacks
		- normally, one would have std::function per each frame.
		- what we will to is to store a variant per each animation frame.
			- even if the variant is big, the animation will be an asset anyway.
		- then the animation system will perform the logic and e.g. spawn particles and sounds.
		- we won't separate concerns here because we're not writing an engine, rather a game.

- Entity groups will be useful later, not until we make a simple deathmatch where we can include some simple weapon/car creation logic etc
	- Really?
	- What about weapon spawns
		- scene could have predefined weapon entity flavours
		- same for each of initial magazines

- pass display size and implement augs::get_display

- editor should print "types of selected entities" and their common properties, identically as with entities/components

- cars could just hold several particle effect inputs and we would iterate cars to also perform
	- handle cars later please
	- particles simulation system can have a "iterate cars and perform engine particles"
		- would just create particle effect inputs and pass them to the simulation
	- same with sound system

- In production build, let the ensure throw an ensure_exception. 
	- Then, in the editor, when the game is still unstable, we will catch an error during whenever we step the cosmos or change some sensitive valuesa,
	- then upon catching, we will save the last known correct version to hdd.

- For continuous sounds, sound system should probably assume the same strategy as an inferred cache.

- Local setup should record session live
	- This is really important in the face of bugs.
	- Or just get rid of test scene setup for now and let it by default launch a scene in editor that records inputs

- for now do an undo_delete_entity test with a floor and some walls perhaps
- storage format for deleted entities

- ensure should throw so that the editor destructor can perform autosave
	- for cores, just emit them programatically on unix
	- and on windows it makes little sense to abort there, just debugbreak and throw.

- some hint for windows unicode in imgui: https://github.com/ocornut/imgui/issues/1060

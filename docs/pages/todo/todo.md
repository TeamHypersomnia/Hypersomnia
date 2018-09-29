---
title: ToDo
tags: [planning]
hide_sidebar: true
permalink: todo
summary: Just a hidden scratchpad.
---

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

- alt+scroll could disable/enable layers starting from the topmost one

- Fix this: due to a filter, the node disappears during renaming
	- just when constructing a filter, save a name with which it was remembered in cached fae selections
		- i guess we will still be able to do for eaches and all flav id getters even with changed state structure
	- also when it is duplicated
	- Just rebuild a cache each time a filter is modified?
		- Not responsive if for example history is moved around, so no.

- Selection tabs
	- Generalize editor_tab_gui to be also able to handle the selection tabs window
	- Enter on selection opens relevant selection group in tabs
	- Switching tabs with entities should always refocus on the same kind of property
		- Low priority

- Ctrl+Home should center on the whole scene

- Property editor: make container elements tickable and modifiable in bulk

- Editing vertices in editor
	- A generic "unmap_entity_vertices" and "map_entity_vertices" that depend on the context
		- Will be important later, once we want some crazy irregular maps.
		- e.g. setting reach for wandering pixels
		- if fixture, change shape

- settable_as_mixin should have push_bind and pop_bind?

- later print shortcuts in the menus for the windows
	- e.g. History			Alt+H

- copy gfx and sfx folders on save as...
	- For now on re-opening the newly saved project these resources will be missing

- research building with clang on windows?

- let particle definitions be split into the invariant and variant parts, like components
	- pro: better cache coherency

- templatize some dangling memory stream arguments

- normalize the names of getters in xywh/ltrb

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

- pass display size and implement augs::get_display

- editor should print "types of selected entities" and their common properties, identically as with entities/components

- cars could just hold several particle effect inputs and we would iterate cars to also perform
	- handle cars later please
	- particles simulation system can have a "iterate cars and perform engine particles"
		- would just create particle effect inputs and pass them to the simulation
	- same with sound system

- some hint for windows unicode in imgui: https://github.com/ocornut/imgui/issues/1060

- finish work with atlas and sound regeneration
	- regenerate only seen assets
	- always load diffuse map with the corresponding neon map to avoid unilluminated objects near the camera...
		- ...even though many diffuse maps nearby have been loaded
		- perhaps we won't really need the separation between diffuse and neon, because maximum atlases will be huge anyway

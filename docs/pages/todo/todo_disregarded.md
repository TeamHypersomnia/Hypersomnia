---
title: ToDo disregarded
hide_sidebar: true
permalink: todo_disregarded
summary: Just a hidden scratchpad.
---

- let "in rectangular selection" just have "eaten" vector so we don't have to recalculate for each selected entities
	- the state would be ugly, we'd have to save on beginning of drag the state of signi

- rename "significant" to "persistent" and remove the mention of "transferred through the network"editor_tab_signi
	- because "replicated" implies being transferred
	- too much pain in the ass though. We've changed the definition, though.

- fix filesystem design to have tries instead of file_exists
	- disregarded because ours is I guess a common technique

- immutable (const) fields within component aggregate
	- applicable components: type, guid
	- applicable: quick invariant copies
		- what about byte readwrite?
			- it anyway reinterpret casts to mutable bytes
		- what about lua readwrite?
			- it anyways creates and destroys entities
		- what about delta?
			- it anyways creates and destroys entities
		- what about copy assignment?
			- not quite applicable...
			- we could however provide our custom operators which will just do std::memcpy.
		- so we ditch it

- fix that build page warning
	- revert back to that A record if it doesnt work by tomorrow

- disallow moving through history when tweaking a value or during other sensitive operations
	- for tweaking, **simply clear last active id when undoing and nothing should go haywire**
	- e.g. setting a transform of an entity?
	- optionally, id cache clearers can take into account the moved entities
	- but really we should not be able to just ctrl+z/y arbitrarily
	- how do we detect that a field is being dragged?

- What about a byte_vector struct that has a templated constructor?
	- and a templated operator?
	- not quite; the type's operator= could be less efficient


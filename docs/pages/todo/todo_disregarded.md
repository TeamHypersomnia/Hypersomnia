---
title: ToDo disregarded
hide_sidebar: true
permalink: todo_disregarded
summary: Just a hidden scratchpad.
---

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


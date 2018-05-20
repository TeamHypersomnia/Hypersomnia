---
title: ToDo disregarded
hide_sidebar: true
permalink: todo_disregarded
summary: Just a hidden scratchpad.
---

- Old thoughts about atlas
	- Remove atlas saving for now.
	- Separation
		- First: just do neons maps and the rest distinction
			- will rarely be switched anyway
			- we'll probably stay with this at least until deathmatch is complete
			- loading just proggyclean is really negliglible
			- and it takes up tiny amount of atlas space
			- so it does not matter even if we duplicate this also with imgui
			- we'll also probably not care much about our game_gui systems for now
				- it will mostly be for viewing state
	- Asynchronous regeneration
		- We might block for the first time so that GUI doesn't glitch out
		- Stages
			- Problem: to acquire GL_MAX_TEXTURE_SIZE, we must be on the GL context
				- We'll just store the int on init. The delay won't matter that much.
			- (In logic thread) acquire all assets in the neighborhood of the camera
			- (In logic thread) send a copy of loadables definitions for the thread
				- Actually the argument may just be a copy

			- (Diffuse thread) load images and determine best possible packing for diffuses + rest
				- Blit resultant images to a larger one
			- (Neon thread) load images and determine best possible packing for neons
				- Blit resultant images to a larger one
			
			- std::future with a moved-to image?
				- read by the logic thread, which then initializes PBO DMA

			- both threads will output an image
			- upload can be done synchronously at first
				- actually that's the bottleneck

		- Problem: viewable defs can change instantly in structure from intercosm to intercosm and GUI might glitch out
			- We don't care. In practice, won't happen during gameplay.
				- And test scene essentials will usually have same ids across all intercosms
			- Just GUI should safely check for existence and zero sizes just in case
			- That will be only a fraction of a second
			- GUI could really use a separate texture
				- And issue drawcalls
	- Implementation details
		- Do failures need be communicated?
			- Just always make the cached vector with texture entries big enough
		- Make a struct called atlas_distribution
			- And keep there all atlases
			- Because it will be passed around renderers
	- Details
		- IMGUI should preview atlases and tell how much space is left

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


- let invalid map to zero so that we might call "at" on images in atlas vector and always get a sane default
	- actually, no
	- we'll disallow invalids and where we allow, we'll need some contextual handling of their lack
	
- the only performance-critical assets really are image ids for sprites
	- actually flavours as well, so this argument is invalid
<!--
	- images_in_atlas_map can just be a vector of constant size vector, even if ids are the two-integer pool ids
		- because it will always be regenerated for the existent ones whenever the content changes
	- if animation id performance takes a hit, we can always make a cache for them
		- but it is unlikely
	- so, let's use augs::pool that has undos working already
		- in case that we'll want to switch for id relinking pool, 
-->

- add "direct_construction_access" for entity handles
	- wtf?
- let meta.lua have convex partitions and let author just define those convex partitions for simplicity
	- let invariants::polygon have vector to not make things overly complicated
	- polygon component makes triangulation anyway

- separate guids from the cosmos, they are rarely needed
	- wtf?
		- also will be needed if we place guids in components which is likely
	- consider if entities without guids are at all addressed by guids;
	- in any case, we can always iterate if we do not find an entry within the map,
	- and the guids will nicely fit into the inferred cache scheme. 

	- we should just create separate free standing functions 
		- separately for extracting subjects inputs for neon maps, desaturations, diffuses 
			- from loadables map
		- then we can concatenate them if we will

- Editing containers in general edit properties
	- add/remove yields change property to the complete container
	- what about comparing?
		- just use element index in field address, should be ez
		- just don't call the comparator on non existing indices?
		- a separate control in imgui utils for editing containers?
			- could then be also used for settings
			- good idea, it can be compartmenatlized well
				- we'll really just add dup and remove buttons


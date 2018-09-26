---
title: ToDo disregarded
hide_sidebar: true
permalink: todo_disregarded
summary: Just a hidden scratchpad.
---

- Automatically reload only after shooting and when an unloaded gun is wielded while there is no other thing in hands
	- For now, players won't really care about removing magazines by hand
		- We disregard it since it would make automatic reloading a lot harder
			- Actually just pickup of new item
			- And shot
			- And wielding of unloaded weapon exclusively
			- After chambering as well
		- It takes too much time to have any meaning really
			- Well maybe if they wanted to drop it for someone?

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


- Also check that there are no guids in common state
	- No: this might fail if we decide to use guids for components (then they will be present in initial components)

- Animation import
	- It will be useful to not have to re-specify parameters of official animations.
	Will also reduce the load for test scene code where it does not buy us much.
		- Durations - surely
		- Booleans for has_backward and flip - yeah, as well
		- Frame orders?
			- The need is implied by durations
	- Saving format
		- An animation could have a button to "Re-import" frames
			- Then they will be re-read from disk
				- Possibly creating new entries
		- Format: literal, direct: store filenames to be looked for in the current folder
			- Each frame has a filename?
				- Otherwise we would have to disallow animation frames with non-compliant filenames
					- In practice such uses won't be frequent
			- Con: have to re-import any time we create new frames
				- Wait a minute. **We will anyway have to re-import manually**, because the editor has its own copy of frames and durations
				- Therefore it is only a problem if we want to update official resource...
					- ...in which case being explicit is better
				- The artist's intervention will almost always be required anywany, especially for setting new durations et cetera
			- **So we're gonna go with the literal format, at least for now**
				- Nothing stops us from manually writing lua files for official resources to later have no need to re-import and re-export in editor
<!--
		- Semi-literal format: literal if there is a need, don't store filenames if the use corresponds directly to the frames on disk
			- Con: quite complicated workflow? Simplicity sometimes beats the automation?
-->
	- Problem: It would be nice to set durations in bulk, in editor
		- But for that we'll need to play with mass editing of container elements
			- Tick state storage is problematic here
				- We could store an unordered set of integers and map it after the imgui id.
		- Or at least have a tick to say that we want a constant frame rate

- Solution: If overlaying, always calc timing from the actual remaining source time, not against when_born
	- Actually we shouldn't care about overlays...
		- easy enough in audacity
	- Overlay time a property of the modifier?
	- Manual looping will be required and the % operation will have to take the shorter duration into account
	- Pitch both layers
		- Overall set same properties to both
		- Two sources in a cache!

- Btw: Remove sprite component from entities for whom it makes little sense to have it.
	- Actually, no. Has some useful information and the heaviest use case will anyway require it.
	- Player character.

	- tiling_mult for sprite_component?
		- **No**, that makes it even more data to handle. Let's just keep a mult and a bool for tiling.
		- Makes it explicit that it will be used for tiling

- Solution: Refactor the invariants::sprite to not contain the size information.
	- Actually, the default size will be there

- Shift+Arrows shall duplicate the selection and select the new entities
	- Pointless, now that we have the resizing

- Special render layers?
	- Every possibly visible entity kind will have a render layer
		- LIGHTS
		- WANDERING_PIXELS
			- might as well have render layer for dim, why not
		- SOUNDS
			- why not? maybe later we'll derive some graphics effect for them?
		- Some of these can be derived from the layout, e.g. having light invariant qualifies
		- It is to tonpo/visible_entities discretion from which tonpo types we'll acquire particular render layers
		- It will be to other audiovisual systems discretion how and if they want to use more specialized layers, e.g. dim/illuminating wandering pixels
		- sort per layer shall dispatch first and then infer render layers
		- For now though we might want to iterate through all of these lights/wandering pixels
			- For each inside the visible_entities?
				- templatized by enum type
				- in any case, will be possible
				- and makes the code more generic
		- from for_each_iconed, call for_each from visible entities
		- It's only bad because we're not heterogenous enough with the enums
			- e.g. illuminating particles could be assigned to some completely unrelated entity
				- less template code generated, though
			- ideal solution would be to have for_each_kind or something inside visible_entities and filters?

- Bugfix: sometimes floor is not selectable but it's because it has the same layer as road
	- some warning perhaps could be in order?
	- I guess we should just create separate layers if this is so important


- Each team game mode exposes functions: add_player(faction_type), remove_player(faction_type)
	- statically dispatching it

- Along with the reason of disconnect (timeout, ban, kick)
	- Maybe even with the string
	- Solely for viewing for the clients (e.g. "abc was banned from the server")
	- Actually, these should be implemented by means of chat

- Do we have to use a mode_message for player creation?
	- A direct call would simplify retrieval of the entity guid - we need it to route the client's inputs to specific entity
		- Additionally, it would take a step less for the player creation to take effect
	- Problem is, some creation logic might require a step
		- architecturally, not really, especially now that we have a step-less perform_transfer

- Who spawns the initial player entities?
	- And who synchronizes it with all connected endpoints?
		- Modes accept messages and handle it in the next step
			- So we have another kind of a cosmos
		- mode_messages::add_player
	- Mode shall step in parallel to the cosmos, actually it should invoke the step
		- And the solver! It will be to the mode's discretion how the cosmos is to be advanced.
	- The network endpoint code posts player_existence messages
		- the game mode creates player entities and stores new ids in player_created_message

- Can be copied around maps for easy transferring
	- Flavour ids are untransferrable, though
	- Or do we assume that there won't be any flavour ids specifiable?
		- Well actually even requested initial eq has flavour ids, so it's a no-go.

	- lua vs binary files
		- The write time will be negliglible for such small amounts of information
		- Well... anyway let's store it as binary for now, because flavour ids won't be human readable anyway
			- Predefined configs might as well be edited in editor instead of a text file

- Setups expose current mode?
	- if constexpr has round time
	- main would also have to know the arguments like initial cosmos
		- so perhaps it would be better for a setup to interact with main, not the mode itself

- Entity groups will be useful later, not until we make a simple deathmatch where we can include some simple weapon/car creation logic etc
	- Really?
	- What about weapon spawns
		- scene could have predefined weapon entity flavours
		- same for each of initial magazines

- add unique sprite decoration type

- Marker entities
	- Special-purpose components:
		- shape_aabb
			- can be used by wandering pixels!
				- and thus the editor will allow us to change reach easily
		- shape_polygon can be used both for marking and for physical bodies
	- separation between visible shapes and physical shapes is described elsewhere
		- but the same logic would be used nevertheless

- Alt + R only unloads the magazine
	- We can quickly hide the weapon to interrupt reloading
	- Could also be used for scavenging


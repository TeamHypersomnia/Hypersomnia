---
title: ToDo disregarded
hide_sidebar: true
permalink: todo_disregarded
summary: Just a hidden scratchpad.
---

- DISREGARDED:
    - Switch to previous weapon when:
        - throwing nade
        - planting bomb
    - nOT NECESSARILY - why?
        - After planting the bomb, you might want to default to holster to quickly jump off.
            - very important in competitive environments where every second matters


- Perhaps exit the editor to project selector setup?
    - Not really because it introduces another step when we want to host a map for real

- Deprecate multipooldispatchers, it's silly
	- it's actually useful because it has typed id dispatch functions etc
- Check if windows doubly-activates window too

- Internal/special resource names colliding with file names might be a pain the ass
	- E.g. suppose there's no collision
		- Someone renames a sprite whose pseudo-id becomes now equivalent to some special resource's name 
		- Again this is only a problem during serialization - not during game
			- And during serialization we can just add "_" until the name becomes free
	- And it's not even a problem for now because we don't have special resources yet save for modes
	- We don't even have to decide on name

- BELOW IS A NON-QUESTION: We have sizes in sprite resources, so we serialize them!!!
	- So yeah, the cardinal question is do we serialize the metadata for resources that are necessary to preserve logical integrity of the map?
		- Like sprite sizes, number of frames and sound lengths
		- Important because even physical shapes might depend on it
		- I'm thinking we should
	- The problem with augs::maybe remains that if the resource changes its size it will by default write a wrong OFF value
	- Let's leave it at that, we're overthinking, this isn't even critical
		- Only important is the json state
	- I'm thinking yes, let's serialize these sizes
		- Will let anyone skin the map without having the resources
		- Will let visualize the map early without full textures yet
			- Will come in handy in future verisons maybe

- As for resetting maybes, when they're disabled it doesn't matter 
	- When they're enabled only then we can show the colored properties for whatever has changed

- WE'RE JUST USING AUGS::MAYBE WITH A FLAG!!! And leaving current design as is
	- We don't want to write off values to json to not unnecessarily clutter it
	- So the off values will be reloaded anyway with game restart
- old considerations:
	- I think after all we should use an std::optional for the sprite node size
		- It's a complex case of override of a default which can change during runtime
		- Though it sucks then for multi-editing when we enable resize?
			- But in that case we'd want to set it to the same custom size anyway
		- The only reason we want augs::maybe because it'd be comfortable fast-switching to check how it looks like?
			- when ctrl+z would suffice really
	- I'm not sure if the current augs::maybe design is wrong, perhaps we should skip writing it to json is all
		- Otherwise we anyway should have a reset button in place
		- what corner case does exactly solve it to write 0 if size is default?
			- Well, if the resource size changes while its off it will catch on
				- but it won't anyway if we have tweaked it
		- uh, let's maybe leave it at that, it will be reset anyway once we reload the game (since we don't want to serialize default maybes anyway)

- I don't think we should allow "steppable" but "silent" grounds, this will confuse people
	- Actually we can choose a sound and let it have gain = 0
	- So we have a semantic for this indeed

- Can't we make bodies static per-entity?
	- see reassurements.md: property-per-resource is a GOOD thing

- A plus sign to add images directly from the game
	- Maybe not important now that we'll have reveal in explorer

	- So that we don't have to navigate to the map folder
	- although then it'd be good to add a "New folder" button
	- actually we should just have "reveal in explorer" under Project tab or somewhere and be done with it

- The only problem with how we do canon/local config is that some local settings might disappear
	- If we manually set allow nat = false and then we launch a dedicated server, that setting might disappear
		- Is that really that much of a bad thing though?
	- Won't happen with a force config

- Shift+T could move with layer detection just like when putting on scene
	- Point is, how do we quickly move one entity over another with just on-scene operations?
		- I think ctrl+x + ctrl+v. So this is for later. Ctrl+v will always paste nodes above the topmost selected node

- We were worried that invisible nodes not being moved with the visible ones could be unintuitive when mass-moving nodes
	- We could check how it's done in other editors too
	- However it should be expected to work like this
	- Note that even if selecting on-scene for mass transforming, you will never be able to catch the invisible ones in selection
	- We'll worry about it later when someone asks

- If nodes won't ever reference other nodes (only resources), prefabs could just be nodes with a vector of node variants
	- Since these sub-nodes are not meant to be identifiable
	- to_entity_id should also always return the prefab node instead of individual entities
	- *Unfortunately*, nodes will reference other nodes, e.g. a fish node might want to reference an origin node

- Fix akimbo mid-chambering: switch to the other weapon when we drop the current during mid-akimbo
	- Implementation: If the currently mid-chambered doesn't match AND IT'S ALREADY GONE from inventory, switch to the second 
	- It's okay if it's also due to the manual drop

- Maybe let's try using clang's stdlib on Windows?

- Default camera mode in settings, maybe controls?

- consider putting nades and knives to the primary hand for better aiming consistency when quickthrowing
	- problematic because the gun will stop shooting and it will be displaced

- what could cause a predicted effect to be played a second time?
	- wrong3

- Remember to reset input flags of the character itself on performing transfer
	- why? it's good to keep shooting once you change


- We were considering whether to mark the bomb as never predictable,
	- however, the worst that could happen, is that we mispredict that we are dead due to somebody shooting us
		- whereas we are actually alive and we have defused the bomb
		- either way the death impact would be predicted which would be misleading
- write to streflop dev about that copysign should be bivariate

- We could opt for a longer float test running in the background for a second or two

- Don't give release build for testing as it has commented out ensures.
- Perhaps always use find instead of get?

- instead of notify_gui, have more flags, e.g. for pickup notifications
	- pickups are not that predictable after all
	- problem: a pickup could remain undetected
- hotbar selection through wheel won't always succeed
	- actually that's probably not needed, it'd take way too long to navigate to the weapon of choice
	- it will be a LOT more useful to bind wheelup/wheeldown for pulling out a particular class of a weapon, or e.g. throwing a nade
	- even so we must somehow keep track of the current index to allow scrolling further
- dont reload when mag is full only to fill the bullet in the chamber

- fix winapi crash on start with hlgrc
	- it is due to a low opengl version

- Let the character see a little behind it
	- Simply a disk overlay over the stencil
		- Radius would be specified by mode rules
	- problem: we'll be able to see around the corner a little

- Stop the whoosh sound on obstacle hit

- Can't we really avoid the need for finishing traces?
	- An alternative would be to hold the trace in the missile and reinfer without the body when the missile impacts,
	  and prolong its life for a moment
	- Well won't it later be very useful to define some explosion entities?
	- There might even be more components like cascade explosion

- make reveal in explorer work for both files and folders
	- cause it also works for dirs

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
			- loading just proggyclean is really negligible
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
		- The write time will be negligible for such small amounts of information
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

- team & ffa matches should get different logic altogether

- what about AI?
	- let pathfinding be a circumstantial component?
	- we don't care much about that AI alters some state because virtually only ai will use that pathfinding
	
- ensure should throw so that the editor destructor can perform autosave
	- then we won't be able to use std::abort...
	- actually, if some modification caused crash, it would be better if the author can try to edit without it
	- Then, in the editor, when the game is still unstable, we will catch an error during whenever we step the cosmos or change some sensitive valuesa,
	- then upon catching, we will save the last known correct version to hdd.
	- for cores, just emit them programatically on unix
	- and on windows it makes little sense to abort there, just debugbreak and throw.

- Possibly abort the for_each_flavour loop altogether after deleting a flavour?
	- Will only be problematic if the ids will be rewritten in place, which won't even be the case with sparse pools, not even with pointer ids
	- Only problematic with direct index-based ids

- Describe two kinds of state: constant-divergent and exponentially-divergent
	- tree of NPO, sprites, polygons, renders: constant divergence - they do not propagate further
	- sentience, fixtures, rigid body: exponential divergence

- Perhaps something to let us select and manipulate entities in gameplay mode?
	- Won't matter until after deathmatch stage
	- will it be useful?

- visible_entities should be templated by all_type
	- could be instantiated
	- is it necessary though?
- normalize the names of getters in xywh/ltrb

- editor should print "types of selected entities" and their common properties, identically as with entities/components


- Ensure that a single capability only ever mounts a single item at a time?
	- An extreme corner case

	- Store entire folder?
		- it's not that clean, we wouldn't want to restore the path
		- Worst space complexity

			- The layout of values for move command might be different if some entity ids change
				- actually probably not since the types are going to be the same
				- still, if we read some values for one entity and not for others, we have chaos
				- actually this doesnt concern us because we are redoing, not undoing
					- so we only need be concerned about input values for redo
			- leading to chaos?

	- Test gameplay when there is no flavour, basically when the workspace is empty

	- Comfortable import, export and use of mode vars
		- Proposition: Just "modes" file
			- Pro: less work to do now
			- Pro: entire packages of modes in a single file
				- can load them as well
		- Proposition: Mode vars objects per file
			- Pro: Possible organization in the filesystem
			- Pro: Can easily add new modes without launching editor?
			- Con: Format of the filename, have to forbid some characters
		- Server will probably be able to accept a lua script easily
		- With an export, we may simply create a folder with a script file per each var

	- For now, let's only send inputs per step instead of solvable
		- This way we always quickly connect on the beginning of the round which is most frequently the case
		- if you connect late that's your problem, but anyways it should happen rather quickly
		- we can later implement dynamic choice of the way we transmit the initial state
			- solvable's can be trivially estimated
			- and we can keep track of the number of sent entropies this round, as we go
			- this way we can compare which is more space efficient
	- actually why do we need completion? can't we begin to queue up the inputs?
		- we want to generate another solvable message right away or pack all inputs that happened tightly

- For now let's go with winsock + reliable.io to send large blocks
	- We'll learn from netcode and maybe use some functions
	- API will never need to change when we add security.

- solution: netcode.io + sending large blocks code from the example,
	- or we'll take some code from yojimbo that sends blocks

	
- against yojimbo?
	- the message ecosystem that does not play well with our code
	- other deps that we don't need atm
- design augs api so that either netcode or yojimbo can be used underneath

- improvements to redundancy
	- message-wise fragmentation
		- we might need some more effective way to deal with this
			- Solution 1
				- if there are 5 steps to sent, but 2 steps already fill the whole packet,
				- and the other 3 can be packed into the next,
				- simply withhold posting the 3 remaining messages until the other 2 are delivered
				- in this case we might need to increase the rate of sends and send them even before the next tick
			- Solution 2
				- simply always send all messages redundantly but split into several packets
				- client, apart from sending the most recent sequence number before which everything was received,
					- also sends acks redundantly for all packets received out of order
						- clears them once the most recent sequence number implies receiving of these out of order packets as well
				- con: we would probably want to prioritize the 

		- we have a map of sequence to reliable range
		- later entries may arrive before earlier ones
		- we keep a vector of pending entries and only  
	- Unlikely, but a single step information might not fit into a packet
		- e.g. if 300 players move their mouse at once
		- in this VERY unlikely case we might just randomly drop some entropies to not overload the server?
			- simply drop entropies of players who've sent the most commands
				- so if someone coordinates a DoS of this sort, they will remain motionless
				- if someone is innocent but their entropy is dropped, it might happen that:
					- their player moves despite having their buttons pressed
					- their mouse position is completely different than expected
				- in this case, the client should try to repost the entropies that were cut


- perhaps we should implement remote entropy and just return it from the adapter, instead of acquiring templates
	- because we'd anyway like to log the entropies
	- unless we log entropies at the packet level

- Allow to mount directly to chamber

- believe some guys on the net and use crlibm as it seems more solid
	- or just openlibm?

- enable/disable warm starting?

- Prevent re-simulation of ever more steps during resynchronization
	- No steps get resimulated when we have no incoming entropies

- Interpolate velocity value to avoid an ugly trace sound when shooting the rocket launcher at 60hz

- Illuminated rendering could trivially post all jobs for layers at the very beginning and only then prepare the commands
	- Jobs in visibility system
		- We'll remove the range workers completely and the parameter for threads, we'll simply have jobs
		- Each job could draw the light right away to once again avoid dependencies
		- We can still do this before introducing the pool

- We might even be able to remove unordered_map header from the vast majority of the code

- flag for setting when transferred to 'now' when finished reloading?
	- so that there's a delay between finished load and chambering sounds

- Nickname should change if we change it in settings


- Create randomized players like in the good olden times
	- to test the predicted experience
	- we might look into legacy sources for guidance
	- fill in several artificial connections starting from the back of the client array
- Already when the server list is requested, masterserver should request a reverse ping from all servers 
	- Then just automatically re-ping all servers once every 20 seconds

- when the body explodes, spectator automatically switches from the killed person to another

- Enlarge the health bars a little upon damage taken and health increase
	- Also enlarge when no more stamina for dash? Though then we'd have to scale entire bar along with health

- Dead body should not show number indicators for damage given


- Knockout notifiers could show whether the kill was "bloody"

- After death, perhaps the camera should follow the head instead of the dead body
	- Especially since the body may explode
	- And there will be dynamics for the head

- Alright let's variantize the sprite resource editable and have a handler for it in serialization routine

- I think these two flags are very specific that they'll have different defaults across domains
	- However it would still be nice to variantize domain-related properties and then still serialize it nicely

- What I'd really love is focusing on having the simplest JSON api here
	- so no separate physics_wall_illumination but just wall_illumination = false/true
		- as well as omitting it if we have the domain default (true for physical and false for decoration)
	- on the implementation side - std::optional<bool> could be the on/off/default idiom
		- Pretty easy to implement in serializator
		- Nullopt could be rendered differently depending on the selected domain
		- Could be rendered with a combo enum
		- However for the user it would be most intuitive if it was still a checkbox
- Okay, we'll need custom serializators anyway because we'll have to serialize the ids
- Since we'll have a custom serializer we could make sprite editable a variant

- Let's leave the double-click-to-navigate-to-center as is
	- We will anyway use it on small objects only and then it would be confusing as hell
- wth is wsad_player getting rebuilt when we only modify prefab node editable?
	- ok probs due to introspectors

- Damn, footsteps make sprite resources dependent on sprites
	- although all resources are created by the time we rebuild scene so it's a non-issue
- move custom_footstep to general properties as physical ones could theoretically be stepped on as well
    - think crate remnants for example
    - maybe leave it as it is and specify footsteps in physical materials
    - fuck it, leave it as it is and only add footstep in physical material if we really ever get to it

- set max_incoming_connections_v to 63 so that we can fit in the integrated client as the 64th
    - This way we'll be able to identify clients with one less bit
    - Well, it's actually aligned to a byte anyway when serializing so this won't have any effect on bandwidth

- quiet portal should also set the preserve offset flag
    - so we'll probably remove it from portal exit


- Possible bug related to reinference
    - Suppose BeginContact is posted due to a ToI bullet/portal collision
    - At this point the bullet has already flied past the portal
        - But EndContact will only be reported the next simulation step
    - Suppose complete reinference happens before the next step
        - All contacts are destroyed.
        - Will EndContact be triggered? Probably not.
            - *might* actually be called by b2ContactManager::Destroy but really there's no need to keep flags depending
            - we have enough state in contact lists, it's perfect for that
    - Therefore we cannot rely on Begin/End contact to set a flag for whether to increment teleportation progress.
    - For each portal, we need to iterate existing contacts.
    - After reinference the contact will likely stop existing.
    - The only thing BeginContact could really do is trigger quiet portals.
        - But I figure iterating existing contacts will do just as well 
            - The ToI impacts will be reported too this way because they always exist for at least a single step
	- Disregarded: we won't rely on begins and ends
		- we'll always iterate contact lists, for force fields as well
		- We have to anyway because we have to apply forces continuously

- Do not apply impact force if we penetrate
    - Only upon exit
	- MEH, pointless after we fixed it


- We should automatically expire if vel =~ 0 during penetration
    - Because if we damp during penetration the distance travelled will be smaller and smaller
        - which could lead to very long penetration
	- NAH, just constrain lifetimes

- What if instead of raycast we just test overlap with a fabricated b2 edge?
    - PROBLEM: overlap may report something against which we graze, but then if raycasts don't report anything we'll treat it as if it completely overlaps

- !kill in console or autokill 
    - just remember it forces a level back
    - what was the problem with just autokilling when nothing in inventory again?
        - hard to detect against backpacks i guess
    - dont autokill, let's just implement fist fighting


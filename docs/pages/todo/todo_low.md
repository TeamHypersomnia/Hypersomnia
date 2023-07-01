---
title: Low priority ToDo
hide_sidebar: true
permalink: todo_low
---

- crate caption
- send heartbeats when state changes too
    - but dont skip sending heartbeats because the servers will disappear otherwise
    - btw has to reserialize even if heartbeats are same because time_last_heartbeat will be inaccurate
        - unless we treat it as a time of last change which is useful on its own

- rename server_solvable to something else? "vars" at least maybe
    - this should honestly be under a single "server" struct probably, later will be easy to pull off with serialize_in_parent if we have config in json

- percents next to nickname
    - client will have to send them on their own every second like ping, on volatile statistics channel

- Separate the includes maybe so the entire editor project isn't included

- Coloring the light icons would be pretty useful

- later rcon messages and server vars generally could be sent via blocks
	- but this works well enough with low packet loss

- z jakiegos powodu nie pokazuje used lokacji dla flavorow w legacy editorze

- Automate version bumping (write it in one place instead of three)

- Fixing akimbo and the flipping system
	- Sometimes only the weapon might be inside the same hand
	- But sometimes the whole weapon placement might need to be flipped because we're holding it in a secondary hand
		- which is when torso offsets are flipped
	- For flipping the second weapon in akimbo, we don't flip entire torso offset
		- Torso offset is intact, we only flip the anchor and further down the weapon attachments
	- For such complexity, it makes sense for direct attachment offset to manage flipping
		- just it needs to communicate that the geometry itself is to be flipped
		- for now we only need to fix it here
	- We'll need to overhaul the animation frames to remove the need for this crappy logic code

- Determining best official server
	- default: EU
	- Once every second ping all official servers
	- change_with_save to the first one that responds
	- is later chosen as the default for Connect to official universe
	- show ping live in that window next to the server name

- Delta compress the solvable to send against the initial solvable

- plan for full server replays
	- it's just about saving the entropies
	- server shall be frozen and never advance when there are no players
	- keeping timing information in arena server vars
		- we will anyway have to commandize the changing of these rules somehow
	- server entropy different from mode entropy
		- since it will also store player information
	- server entropy serializer

- Lua intercosms are exported a bit different on Linux and Windows
	- We have something like 4.438729871 vs 4.438729872

- Camera is moving strangely in slow motion

- fix predicted commands error for later debugging
	- well, it's not important though

- pretty print - check if convertible to string

- Calculation might be off for orbital streams due to quick movements
	- Offsets are calculated against entities that have already moved
	- Possible solution
		- Save b2Transforms of bodies into the collision_message. These will be the transforms before impact solve.
			- logic will later process (si transformation etc) them if they are at all needed
		- Calculate against these transforms


- Perhaps replace shift+c with some other shortcut?

- Perhaps interpolate velocity values if we detect that they are ugly at 60hz, apart from the case with skull rocket?

- introduce particle multiplier for ring explosions
	- set a reasonable value for things like rocket launcher

- Fix neons shining through the character body
- Bind buying to E so that we can release T for other purposes

- Fix parallel visibility queries

Would you like keep the old settings,
or reset them to factory defaults?

[ Keep ] [ Reset ]

- Rebuy previous
	- Plan for state early
		- note that we don't need any bincompat regarding the mode solvable
		- because anyway we are importing/exporting without that state


- Fix crash when server starts with a non-existent map?
	- Actually that's a solvable error
	- Would be nice to gracefully quit though

- bomb soon explodes theme gets repeated
	- unimportant: perhaps it was 

- refactor the ammo info getters, somehow

- Crosshair recoil also on finished mountings, e.g. of magazines

- Proposition: instead of "logic speed mult" have a set of predictable parameters for the simulation?
	- like, gun intervals speed mult
	- like, physics speed mult
	- so don't store it next to tickrate
	- we could avoid influencing time-related domains

- Arbitrarily colorized text in context help

- Probably somehow disallow arbitrary inferring of relational cache?
	- AFAIK the only place where we arbitrarily infer the relational cache is for motor_joint which isn't used now anyway
	- There was some unresolved crash problem with this.

- Implement recording only of mouse, so differentiate intents

- GUI for the player
	- Timeline
		- IMGUI is the way to go, really
			- Builtin horizontal scrollbar, tooltip, child windows, selection control, etc
		- Yellow marks at positions when when any input is applied
			- Shown with regards to currently controlled entity
		- If recording, show the position marker in orange
		- If replaying, in green
		- Show world time and total editor time
		- If a command was pasted, the marker should be cyan
	- Should be able to zoom out?
	- Moving around like in audacity
- Optionally, make the bomb non-pickupable by other factions
	- For now just set infinite space occupied and don't iterate recursively to check if the item is forbidden

- refactor: stepped_clock
	- has delta + now
	- passed everywhere where now and dt are required
	- cosmos::get_clock() instead of timestamp & fixed delta
	- this prepares us for a possibility that the delta might change and we'll need some history of delta changes

- thoughts about groups, templatized components, guids and network transfers 
	- **we will anyway templatize the components by the type of ids they store to have groups for example**
		- assume some **using stored_entity_id** that will be passed to component templates
		- assert with component introspectors that no entity_ids, pointers, unversioned_ids are stored
			- revive from cosmic delta tests
	- cloners won't have to introspect and rewrite respective child entities but we will always introspect by a single component
		- and just map integer-indexed ids to entities in group
		- entity will also know to which group it belongs thus we can preserve the structure of children
			- and later rewrite contents of respective entities
	- LUA can store per-guid entities and not store pools to maximize compatibility
		- we don't care that it will be recreated with a different order
		- having guids stored in components will make it easier
	- network transfers and binary savefiles can also serialize pools for consistency
		- thus this will work with both guids and entity_ids in components
		- we must ensure the order of entities stays the same for determinism
		- we don't care that much this little space overhead 

- it probably makes no sense to use GUIDs for now if we are anyway going to transfer whole pools for determinism

- replace "alive"/"dead" checks with optionals of handles
	- and assume that an existing handle always points to an entity
	- a lot of work but it will be worth it
	- in any case do it once everything else works

- Delta widgets for property editor
	- Will be useful for offsets
	- New tweaker_type?
		- Based on this, set is_delta boolean in change_property_command
		- can_accumulate_v trait

- Shuffled animations
	- Inside randomizing system?

- Implement ctrl+tab for settings window
	- shortcuts for first letter shall always focus relevant window

- Later, bullet trace sounds should be calculated statelessly

- Resurrect the unit tests for padding that were deleted along with cosmic delta

- inventory slot handle should itself decide when to use generic handle
	- at this point it is only a performance improvement
	- or perhaps several compilation error checks?

- fix errors at unit tests when not statically allocating 
- Groups can be defined separately from flavours, e.g. many groups can share the same flavours.
	- if we want to have a group wherein a weapon is spawned with a magazine, we can simply set the inventory slot ids beforehand.
		- We might detect errors early on and disallow some configurations or not care and just throw cosmos_inconsistent_error.
		- Group ids will not be creatable from the logic anyway?
			- Could be useful later for spawning cars or character via the logic etc

- fix mess with sound sources
	- currently the app might crash if the sound system exceeds maximum sound sources and main menu suddenly is set
		- we can either let main menu be always present (not in optional), and just clear the intercosm
			- I think this is the way. We can later remove completely main menu from build for the server app.
				- because optional or not, it wastes space on the stack.
	- perhaps the audio context should preallocate some hover/click sounds that are omnipresent?

- implement instances of cooldowns for casts statelessly when there is such a need

- Ctrl+I shall open a quick go to gui that will instantiate a chosen flavour

- Improve wielding transfers calculation so that less transfers are made
	- Transfer effects will be fixed automagically then

- Possibly use b2TestOverlap for checking against the camera selection?
- Make a tree out of time measurement profiler calls and get summary just from "fps" or "whole regeneration"

- Property editor: make container elements tickable and modifiable in bulk

- Fix this: due to a filter, the node disappears during renaming
	- just when constructing a filter, save a name with which it was remembered in cached fae selections
		- i guess we will still be able to do for eaches and all flav id getters even with changed state structure
	- also when it is duplicated
	- Just rebuild a cache each time a filter is modified?
		- Not responsive if for example history is moved around, so no.

- fix concatenation of shakes
	- shake for ms
	- shake mult
	- impact velocity mult

- Ctrl+Home should center on the whole scene

- settable_as_mixin should have push_bind and pop_bind?

- Selection tabs
	- Generalize editor_tab_gui to be also able to handle the selection tabs window
	- Enter on selection opens relevant selection group in tabs
	- Switching tabs with entities should always refocus on the same kind of property
		- Low priority
- fix characters getting typed when alt+something combos are pressed
	- once we have combo maps, there will be a separate function to determine whether the input is to be fetched
		- it will also return true on "character" input
	- a separate function will actually respond to combos
- later print shortcuts in the menus for the windows
	- e.g. History Alt+H

- Revival and undoing the knockouts

- Add a match-global rng seed offset to mode state

- Property editor: Checkbox matrix for b2Filter

- Particles and flavours
	- std::unordered_map<particle_flavour_id, vector of particles>
		- We will always simulate all particles that we have in memory.
		- This will add a nice speedup, and also we will easily invalidate particles when particle flavour changes or is deleted.
		- Particle types will also be pooled and will be a separate viewable.

- Validity of files when we delete them arbitrarily from map files
	- Don't worry about that now, they should anyway more or less work
	- Intercosm alone is always good
	- Deleted mode rulesets can invalidate ids
	- so always check if id exists before determining a mode
		- Indeed, we do this
	- We probably shouldn't make these considerations and only ever expose .int and .rulesets? 
	- view can be removed at will though it might break history


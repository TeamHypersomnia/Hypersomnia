---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Improve indicator experience, hide it as soon as it touches c4?
- Fix defusing logic
	- Use button logic will need some improvement 
	- Use button should be queried continuously
		- Preoccupation tracking
			- Once the use button has discovered something to use, it should be locked, i.e. it shouldn't take hold of a car wheel while it is defusing
			- enum for the use button state?
				- IDLE, QUERYING and current preoccupations, e.g.: DEFUSING
				- Since there can be always just a single state at a time, a single enum is required
				- When preoccupation is interrupted (e.g. due to distance), it is always set back to QUERYING
				- When use button is released, it is set to IDLE
				- In case of a single-shot action, e.g. taking hold of wheel, it is always set back to IDLE so that another press is required
		- Different domains might interpret it on different occasions
		- Centralize this completely?
			- Treat is similarly to trigger
			- Anyways we don't need the wheel handling for now
				- Why would we respond to pre-solves there anyway?
		- Set use button to true inside sentience and perform some general logic upon it in sentience system, all in one place
			- Prioritize bombs, obviously 

- Make the bomb non-pickupable by other factions
	- For now just set infinite space occupied and don't iterate recursively to check if the item is forbidden

- Implement bomb-bombsite overlap test

- Add more sounds to the bomb
- Interrupt the started arming/defusing sounds on every play request
- Fix switching between the weapon and the bomb

- Implementing the bomb
	- Global solvable
		- Priority queue with scheduled explosions
		- Universal solve that is used by all solvers, standard or others
	- Bomb should have a backpack category and be too heavy to be put into backpack

- Mode vars gui
	- Can set the current one, resetting the cosmos

- Storage of pre-defined mode informations inside a map
	- ``.modes`` file
		- A map is still functional without modes and modes can always be later specified, so a separate file is justified
		- As flavour ids aren't human readable, makes sense to use binary format for now
	- Each should also have a name, e.g. "test scene", "bomb", "ffa", "tdm", "duel"
		- For easy choosing from the admin console
	- unordered map of ints for each defined profile
		- player stores the current int and a variant of mode instances
		- we std::visit by the mode instance on advancing and then acquire the vars by it
	- Editor view shall keep track of current mode instance?
		- We thought about the player, but we must serialize this per-opened folder.
		- Editor view is suitable because:
			- It is the part of the state that is specific to the editor session, but will never be used during gameplay.
			- This actually implies that the **player should be found within the view**.
			- No problem having player state per-view, just that we'll pause it whenever we switch a tab or quit the editor.
		- What about history of changes made during the mode?
		- Probably player as well.
		- The work won't store any 

- However it is to the discretion of a mode *how* the creation and removal of players happen...
	- ...it is already outside of their scope *when* they happen.
	- Therefore, each mode shall expose add_player and remove_player functions to be called by literally anybody, anytime.
		- As for serialization, these will be some "choreographic events" inserted between steps, or in a case of a network session, "network event"
	- Similarly, change_var shall not be something that the mode bothers with, especially that the calling logic would be pretty much duplicated.
	- There will still be much use of the messages; e.g. mode_messages::game_completed to determine the result

- Mode helpers
	- for_each_character?
		- actually we'll just for now iterate over all sentiences
	- find_player_flavour(faction_type) for player creation

- Test scene game mode
	- The first to be implemented as an architectural draft
	- Test functionality: respawns every character every time it dies or loses consciousness (e.g. after 3 secs), just to a different spawn point

- There will ALWAYS be some game mode whenever a game is running
	- E.g. test scene mode

- Bomb logic
	- Arming counter is increased when:
		- Player is inside the bombsite area
		- Player has all movement flags unset (except shift for sprint)
	- Arming counter is reset when any of WSAD flags goes true
	- When placed, change C4 to a static body with filter that only collides with bullet shells
	- Unarming counter is increased when:
		- Player is inside the bombsite area
		- Player has all movement flags unset (except shift for sprint)
	- arming/unarming durations inside explosive invariant
	- defusable_by boolset with factions

- Mode entropy 
	- As it steps together with the cosmos, will necessarily contain cosmic entropy
	- Mode messages:
		- mode_messages::add_player	
			- For FFA, we just ignore the associated faction
		- mode_messages::remove_player	
		- mode_messages::change_property
			- e.g. for changing round times on the fly
			- could be implemented similarly to editor properties
				- especially since we'll create a property editor for the game mode properties
		- mode_messages::create_entity
			- For admin playing
		- mode_messages::apply_impulse
			- For admin playing

- Modes aren't concerned with the currently viewed entity
	- except test scene mode

- Testbed should populate the test scene mode profile that it is passed

- "I" in editor invokes initialization routine of the chosen game mode
	- Just starts advancing it from the beginning.


- team & ffa matches should get different logic altogether

- basic_team_mode
	- round time
	- number of rounds

- FFA game mode
	- Win condition: None, there is only time limit.
- Duel game mode
	- Win condition: same as TDM, just will be less players
	- Just different kinds of spawns
	- **Actually, we won't create special purpose logic for duels.**
		- There will just be specific maps for playing TDM as duel.
- Team deathmatch game mode
	- Win condition: Consciousness status of any entity of CT side
- Bomb defuse game mode
	- Win condition: Existence of any entity of C4 flavour
		- A bomb can only stop existing due to being detonated
	- Win condition: Consciousness status of any entity of CT side

- Game modes shall be separated into immutable & instance
	- The instance will probably have a mutable copy of the immutable, anyway, for admin tweaks

- Map won't define that "a weapon can't be bought here".
	- It will only provide a list of ready-to-be-used profiles where this could be specified.

- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic

- Game-mode profiles
	- Each map can specify a "profile" with sensible values for a deathmatch, tdm, etc.
		- At the same time, it will be documenting the proper use of the map
	- Will be useful for a built-in test scene
			
- Gameplay mode logic shall detect if a map has enough information to be played in a specific mode

- game mode property is a part of game mode definition
- game mode definition = all game mode properties
- a **game mode marker** is a game mode property of game mode definition that has a spatial representation
	- and thus can be visualized in the world view
	- e.g. C4 trigger or a spawn point

- It makes little sense for an intercosm to have a built-in collection of game mode definitions...
	- ...at least on the grounds that there will be less files overall.
	- because we will anyway have lots of stuff under gfx/ and sfx/
	- and it introduces some stupid corner cases, right?
- so, most of our headache comes from the fact that we have hard time drawing the fine line responsibilities of...
	- ...what is necessary to define in the map itself, e.g. spawn points or flavours
	- ...what should be configurable per every session of a game mode, e.g. roundtime.
	- this dichotomy, once voiced, pretty much speaks for itself.
	- basically, if some game mode properties need to be defined in relation to what is in the map, 
		- then yes, have it in the definition file and configurable in the editor.
		- still can be changed and customized, just saying we'll make UI for this in editor and save/load ops.
	- things like round time, c4 time, they can be specified completely regardless of the intercosm contents.
		- thus they should be textual configs, for which we can nevertheless provide some GUIs.

- Each game mode definition file will be named after its stem, and the extension will determine its type
	- **(Implemented first)** struct team_deathmatch
		- (Implemented first) player flavour id
		- (Implemented first) vector of spawn transforms
		- std optional with preferred duel transforms
	- struct free_for_all
		- player flavour id
		- vector of spawn transforms
	- struct bomb_defuse
		- player flavour id
		- c4 flavour id
		- vector of spawn transforms
		- vector of xywh rects signifying bombsites
	- struct capture_the_flag
		- player flavour id
		- vector of spawn transforms
		- flag flavour id
		- flag base flavour id
		- flag positions
	- struct mmo_server
		- start player flavour 
- special-purpose logic for markers vs entities
	- it might anyway later come in handy to visualize rects and points
	- so we might just as well reuse that logic for markers
- server will accept an ``std::variant<free_for_all, bomb_defuse...>``
	- each will perform its own logic
- a hotkey should toggle visibility of **game mode markers**

- Interoperation of cosmos logic and data defined in game mode properties
- Construction scripts are a different thing and we'll save scripting topics and their determinism for later

- file operations:
	- new project
		- opens new tab and spawns a folder in untitleds directory
	- open folder
		- opens a folder, loads whatever there is
	- save project as
		- choose existing directory
			- ranger can do that 
	- save project
		- if untitled, uses save as instead
		- same as Ctrl+S.
		- writes all visible project files: intercosm and all rulesets visible to the editor. Purges autosave once done.
		- if saving for the first time to a directory, editor may ask
			- Are you sure you want to overwrite 3 file(s)
				/home/pbc/x.int
				/home/pbc/x.hyproj
				/home/pbc/autosave/x.int
			?	
			- (non-existent files and files with 0 size will be excluded)
			- a different popup design will be in order
				- after receiveing dialog's future result, set an optional pending save object
					- thus we constrain asynchronicity as much as possible and escape it as quickly as possible
				- those that have only "okay" can stay as they are
	- (save for later) import intercosm (either lua or int)
		- Will just assign the new intercosm to the one in current project. 
		- We can "only" open projects, thus it is named "import".
		- Will rarely be used, usually just for porting or if someone wants to modify an intercosm downloaded from somewhere.
			- We could transmit projects though.
	- (save for later) Export project for compatibility
		- In lua format only.
		- Will be named like ``ProjectName.compat.lua``
			- Contains the intercosm and rulesets, all important things.

- Transform design
	- It is the case that many entities might share identical origin, in which case it would be unwieldy to update origins for all entities to a new one.
		- E.g. fish in aquarium.
	- It is also the case that the origins might be tied to decorational entities.
		- E.g. aquarium sand.
	- movement path component will have an origin transform which will automatically be moved by the editor
		- the transform component will be kept up to date and it will be the logical transform
		- the rendering code will also only touch this logical transform
	- for editor, the origin could just also be accessed as an independent transform
		- so access_independent_transform -> access_independent_transforms
		- con: more memory wasted? who gives a heck, though...

- Marker entities
	- Special-purpose components:
		- shape_aabb
			- can be used by wandering pixels!
				- and thus the editor will allow us to change reach easily
		- shape_polygon can be used both for marking and for physical bodies
	- separation between visible shapes and physical shapes is described elsewhere
		- but the same logic would be used nevertheless

- Sound should be loaded from the smallest to the biggest
	- So that effects are loaded first
	- New synchronization
		- store std::atomic<bool> next to sound_buffer in loaded_sounds_map
			- set it to false whenever definition changes or it is to be loaded
			- check if future is implemented same way

- find closest point pairs for fish?
	- firstly determine if the setting of the new npo node isn't actually the bottleneck
	- tho probably it's the false positives

- Note: drone sound (sentience sounds overall) will be calculated exactly as the firing engine sound
	- Sentience humming caches
	- These don't need their playing pos synchronized

- Property editor: Checkbox matrix for b2Filter
	- might be useful once we come to glass walls 
	- a list of predefined filters
		- might choose a name, just like enums
		- stored in common assets
	- now sensible filters values will be provided by the testbed

- Shuffled animations
	- Inside randomizing system?

- Editor maximum ease of access
	- Fix problems with grouping on duplication?

- Particles and flavours
	- std::unordered_map<particle_flavour_id, vector of particles>
		- We will always simulate all particles that we have in memory.
		- This will add a nice speedup, and also we will easily invalidate particles when particle flavour changes or is deleted.
		- Particle types will also be pooled and will be a separate viewable.

- Fix what happens when too many entities are created
	- **Let the game work when a new entity cannot be created.**
		- Just return a dead handle.

- Probably somehow disallow arbitrary inferring of relational cache?
	- There was some unresolved crash problem with this.

- Persistence of entity ids in editor and clearing them
	- Cases of storage:
		- Selection
			- Rectangular
			- Significant
		- Groups
	- Cases of invalidation:
		- **Undoing a command that introduces new entities**
			- E.g. one that creates an entity from nothing
			- Look for both undo_last_create and "delete_entity"
		- Commands whose undoing or redoing should automatically select affected entities
			- Purge is justified in this case
			- shouldn't this be pretty much all commands that affect entity existence?
				- No. Redoing delete has no reason to purge selections of some other entities.
					- Clear individually, also for the groups.
			- problem: if, on delete, we remove an entity from the group it belongs to, the undoing of delete doesn't know what to do
				- grouping will be tracked by history
				- thus let the delete command just store its own "ungroup entities" command and invoke it beforehand on all entries
		- Delete command
		- Gameplay mode
			- For now we won't support editor operations inside gameplay mode?
				- Should be easy enough though, we can always just read the deletion commands
			- To ensure space efficiency even with static allocations, we'll just serialize the cosmos to bytes instead of making a full clone
				- Should even be faster considering that recreating some associative containers' structure might be already costly
	- mover should be deactivated when?
		- corner case: delete while move?
		- should work anyway and yeah, deactivate it then

- Grenades shall change shape to circle so that throws can be precise
	- Let hand fuse invariant have an optional radius for the thrown grenade
		- should we reinfer when tweaking?
	- the physics world cache will simply take into consideration if the hand fuse was released, when calculating shape
		- so hand fuse component might want to become a synchronizable

- check in editor if the saving/opening path is a valid folder?
- make reveal in explorer work for both files and folders
	- cause it also works for dirs

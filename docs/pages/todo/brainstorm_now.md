---
title: Brainstorm now
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- refactor image_view to use common funcs?
- thoughts about atlas
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

		- We might do this while making previews as it will be connected - will need to send texture ids to imgui
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

- possibly rename the flavour on setting an image?
	- if it is detected that it is yet unnamed?
	- due to a filter, the node disappears during renaming
		- just when constructing a filter, save a name with which it was remembered in cached fae selections
			- i guess we will still be able to do for eaches and all flav id getters even with changed state structure
	- also when it is duplicated

- always fill new workspace with some test scene essentials?
	- so that e.g. no image ids in common state stay invalid
	- can make those the first in test scene images so that we can stop importing images after some point

- would really, really be cool to have a color picker inside the neon map light color chooser
	- less pain in the ass
	- look for imgui logic to acquire mouse positioning relative to the control

- Ctrl+I could open a quick go to gui that will instantiate a chosen flavour

- particles and flavours
	- std::unordered_map<particle_flavour_id, vector of particles>
		- We will always simulate all particles that we have in memory.
		- This will add a nice speedup, and also we will easily invalidate particles when particle flavour changes or is deleted.
		- Particle types will also be pooled and will be a separate viewable.

- animation metadata could have some very general structs like "character metrics"
	- vec2i: hands position[2]
		- positions of these could be even indicated in the previewed image
	- bool: makes a step?

- rethink our roadmap

- animations tracking
	- We've decided that they might be
		- Named automatically after their first frame
		- Uniquely identified by such

- flavour ids & allocation
	- they are actually quite performance critical
	- would id relinking pool actually be useful here?
		- the for each id structure should be quite easy here
			- literally just common and common/solvable signis
	- they can be sparse though
		- since we care about performance the flavours will be anyway statically allocated
		- and allocation/deallocation speed won't be that important here
			- could still iterate over all ids and serialize only existing
		- we can always easily check if the flavour exists, so no relinking needed?
		- actually relinking still needed if after removing a flavour we allocate a new one

- sparse_pool implementation that avoids indirection?
	- can have IDENTICAL interface as the pool
		- even the pooled object ids can stay the same really
			- just that the indirection index will actually be used as a real index
	- existence of versioning determines whether we need to perform for_eaches
	- versioning could still be there so we can easily undo/redo without for eaches
		- we can let those several bytes slide
	- **we should always be wary of pessimistic cases of memory usage, anyway**
	- for now we can use pools for everything and incrementally introduce sparse_pool
	- once we have sparse_pool, the loaded caches and images in atlas can just be sparse pools as well?
		- though the effect is ultimately the same and it's more container agnostic
- some prettifier for C++ errors? especially for formatting the template names

- might be cool to make container elements tickable and modifiable in bulk, in flavour
	- might actually be done just inside the general edit under container constexpr, with help of notifies etc

- editing containers in general edit properties
	- add/remove yields change property to the complete container
	- what about comparing?
		- just use element index in field address, should be ez
		- just don't call the comparator on non existing indices?
		- a separate control in imgui utils for editing containers?
			- could then be also used for settings
			- good idea, it can be compartmenatlized well
				- we'll really just add dup and remove buttons

- Notice: the set of all assets used by all existent entities in the scene...
	- ...does NOT equal the set of all USED images
		- because flavours might have some set

- preview could be done by a thread_local image & texture
	- NO! Just store it inside editor so that it may be properly cleaned up
	- a good test case for a pbo
	- then send just image id to the imgui renderering routine

- we'll generalize later once images work

- Creating new flavours
	- Might want to specify the flavour right away or not?

- Complex ImGui controls in general property editor
	- In fae tree:
		- Flavours
		- Entities
		- Recoil players
			- Name-able, linear combo box
		- Physical materials
			- Name-able, linear combo box

- Fix what happens when too many entities are created
	- **Let the game work when a new entity cannot be created.**
		- just return a dead handle.
<!--
	- for now let's just have many more, and throw when shit hits the fan
		- just ensure that the author's work is saved and undoable
			- actually it gets corrupt
			- prevent creation of new entities?
				- funny thing could arise, but that's perhaps the best solution that doesn't break the game fatally
				- and the work is safe
				- some dire consequences include no bullets possible to be shot
				- but otherwise it should be fine
		- now that we can have shapes in flavours let's increase the amount of components by lots
-->

- Probably somehow disallow arbitrary inferring of relational cache?

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

- Editing vertices in editor
	- a generic "unmap_entity_vertices" and "map_entity_vertices that depend on the context
		- e.g. setting reach for wandering pixels
		- if fixture, change shape

- cutting/copying/pasting/duplicating entities
	- duplication can happen during either moving or rotation, or just selection
		- actually there's no purpose in facilitating duplication when moving
		- always launch translation on duplication, and connect them in GUI
	- duplication can be implemented in terms of "paste entities" command
		- we'd just gather the content instantly, not from an earlier Ctrl+C
	- if we get to arbitrary pastes, the editor itself will have to construct the command tree, like
		- paste_flavours + paste_entities, if no requisite flavours are found inside the project at the time of pasting
			- the clipboard will have both the entity and flavour
		- the editor's clipboard can actually become...
			- paste entity flavour + paste entity command, stored, waiting to be executed!
				- the move itself won't need to be stored
	- cut = copy + delete

- about grenades and changing of their shape
	- let hand fuse invariant have an optional radius for the thrown grenade
		- reinfer when tweaking also?
	- the physics world cache will simply take into consideration if the hand fuse was released, when calculating shape
		- so hand fuse component might want to become a synchronizable

- separate component/invariant: sentience_animation
	- Animations for the character will be complicated enough to warrant a separate component

	- add rotations
	- in property editor, make transforms always show some special imgui control?
		- **Prefer contextual moving with t on selection**
	- transform setting should always preview the changes as if by applying set_logic_transform
	- some entities don't have their own transforms
		- a warning should appear?
		- actually, there's always a context by which a transform could be set
			- e.g. if an item is held, it might just get its attachment offset moved	

- sometimes floor is not selectable but it's because it has the same layer as road
	- some warning perhaps could be in order?
			
- status bar
	- check how it works in vim
		- changing a mode to normal clears the last message

- Perhaps something to let us select and manipulate entities in gameplay mode?
	- will it be useful?

- handle bitsets properly, e.g. parties and hostile parties
- handle slot categories properly, as well

- Selection tabs
	- Enter on selection opens relevant selection group in tabs
	- switching tabs with entities should always refocus on the same kind of property

- Ctrl+Home should center on the whole scene

- determine what to do with b2Filters
	- for now sensible filters will be provided by the testbed
		- we can add a combo for now?
		- matrix could be cool though, even for debugging
	- it appears that filters aren't really given any special roles.
		- thus it makes sense that they be completely customizable in editor.
		- we will perhaps make amount of categories limited so as to fit b2Filter.
			- max: 16
		- a simple matrix of checkboxes like in unity.
	- enums will just have combos for all values
	- assets will need to have their proper names displayed, unavailable in normal editing (test scene enums are unapplicable obviously)

- generalize editor_tab_gui to be also able to handle the entities/flavours windows
- implement ctrl+tab for tabs, either in settings or anywhere. It should just handle the current window
	- shortcuts shall always focus relevant window

- Also check that there are no guids in common state

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
- all paths for sound and files look first inside:
	- content/ folder
		- e.g. a path could be "official/gfx/assault_rifle.png"
	- local directory of the map
		- e.g. "gfx/my_rifle.png"
	- makes work testbed out of the box

- check in editor if the saving/opening path is a valid folder?
- make reveal in explorer work for both files and folders
	- cause it also works for dirs

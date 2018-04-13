---
title: Brainstorm now
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- importing & using images in editor
	- Simply... keep track of whatever we actually use
		- Images dialog would simply show all used images
		- We will anyway need this logic of traversing all ids
	- Invariants will look like they are picking file paths really
	- Makes no sense to always automatically import all loadables recursively from the folder
		- E.g. because we don't always want all official images imported
	- Or does it?
		- if we do, though, merely adding and removing an image may result in an error in editor
	- Importer window
		- Unimported on filesystem
			- Import next to each folder and file
		- Imported
			- shows how many flavours use an image
			- Un-importing requires to delete all using flavours
				- Which in turn requires deletion of entities with that flavour
	- What if an image for which we've set some neon map values becomes unused?
		- We don't remove it from loadables list. We simply add a "Remove" button next to it.

- how do we reload viewables that will be stored in pool?
	- do we simply compare object vectors?
		- if order changes, that accounts for a change as well because ids must point to correct things
		- do we need to compare whole pools or just objects?
			- I guess pools because identity might've changed
	- what about sparse pools?
		- it won't have indirectors, so object vector check will be enough

- sparse_pool implementation that avoids indirection?
	- can have IDENTICAL interface as the pool
		- even the pooled object ids can stay the same really
	- existence of versioning determines whether we need to perform for_eaches
	- versioning could still be there so we can easily undo/redo without for eaches
		- we can let those several bytes slide
	- we should always be wary of pessimistic cases of memory usage, anyway
	- for now we can use pools for everything and incrementally introduce sparse_pool

- once we have sparse_pool, the loaded caches and images in atlas can just be sparse pools as well?
	- though the effect is ultimately the same and it's more container agnostic

- flavour ids & allocation
	- they are actually quite performance critical
	- would id relinking pool actually be useful here?
		- the for each id structure should be quite easy here
			- literally just common and common/solvable signis
	- they can be sparse though
		- allocation/deallocation speed won't be that important here
		- we can always easily check if the flavour exists, so no relinking needed?
		- actually relinking still needed if after removing a flavour we allocate a new one
	- could pool be rebuilt so that indirection indices are actually real indices?

- always fill new workspace with some test scene essentials?
	- so that e.g. no image ids in common state stay invalid
	- can make those the first in test scene images so that we can stop importing images after some point

- what do we do with invalid sprite ids?
	- makes practically no sense to have an invalid image id
		- invalid sound id could still signify a no-sound which may make sense
	- if we disallow them, we need to somehow handle this in editor
		- e.g. flavour could always choose the latest image id
		- a popup saying "before you create this flavour, import at least one image"
	- still, is there really no way that no invalid ids will take place in sprites?
		- what if we want to un-import an image from the editor?
			- do we have to delete the flavours that use it as well?
			- or do we make a popup forbidding this?
		- what if an image on disk gets deleted and an atlas is required to be regenerated?
			- then stop displaying the cosmos and just be left with imgui
			- we can check in main if the game world atlas exists
	- for things that we guarantee validity of image ids (in sprites), just do always the call to at
	- in stuff like gui however, we'll seriously need to check with mapped or nullptr

- Creating new flavours
	- Might want to specify the flavour right away or not?
	- What do we do about writing image size to sprite?
		- add_shape_invariant_from_renderable can assume always found
			- cause it will be called from editor only on existent ids 
		- when switching from invalid to non-invalid, automatically write size information to both sprite and physics
			- otherwise provide a button to update sizes
			- Buttons:
				- Make 1:1
				- Write to physics

- Complex ImGui controls in general property editor
	- In fae tree:
		- Flavours
		- Entities
		- Recoil players
			- Name-able, linear combo box
		- Physical materials
			- Name-able, linear combo box
		- Images
			- Is there a point in making separation of image ids and paths visible to the author?
				- The question becomes if more than a single flavour is going to use the same image
				- Flavours will very often share logical assets though, by their definition, so it makes sense to have them identified
			- Architecturally, the flavours will store only identificators anyway.
				- So in case if shit hits the fan, we can provide the user with a textbox to change the broken paths
			- Proposition: the editor shall perform i/o upon the extras.lua and meta.lua
				- Because if we consider the game image's path to be an identificator, what happens if it gets moved?
				- It's a pain though because we will have duplicate state
			- Behaviour in sprite invariant
				- Should be displayed as simplified, instead of some mnemonic id?
					- e.g. "corner (walls/1)"
			- Automagic regeneration of shapes
			- Is more than just an image path!
				- We don't have a singular "game image" structure, though. It's divided into loadables and meta.
				- So we'll need a special purpose GUI code that connects them all
		- Sounds
			- Same problem with paths as in the editor
				- Sounds are more likely to be reused
	- Viewables
		- 

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

- asynchronous texture transfers, especially when regenerating atlases

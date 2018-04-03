---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order

- wandering pixels disappeared?
- fix epsilon deviations individually after moving a selection

- handle mouse glitches when letting go of moved entities or duplicated ones

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

- flipping a selection
	- useful for creating symmetrical levels
	- ~~M or m would make a square from one corner~~
		- Just use ctrl+arrows. More intuitive
	- always aligns to aabbs
	- 

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

- fix characters getting typed when alt+something combos are pressed
	- once we have combo maps, there will be a separate function to determine whether the input is to be fetched
		- it will also return true on "character" input
	- a separate function will actually respond to combos

- Perhaps something to let us select and manipulate entities in gameplay mode?
	- will it be useful?

- handle bitsets properly, e.g. parties and hostile parties
- handle slot categories properly, as well

- Selection tabs
	- Enter on selection opens relevant selection group in tabs
	- switching tabs with entities should always refocus on the same kind of property

- Ctrl+Home should center on the whole scene

- Editing vertices in editor
	- a generic "unmap_entity_vertices" and "map_entity_vertices that depend on the context
		- e.g. setting reach for wandering pixels
		- if fixture, change shape

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

- **Chosen solution:** A folder designates a project.
	- **For now we ditch lua exports.**
	- **Project folder structure:**
	- ``ProjectName.tab`` file - the editor tab metadata. History, current camera panning, all selected entities. Designed to let you start where you left off after restarting the editor.
		- **It does not store paths.** 
	- ``ProjectName.int`` file - the meat of the map - all entity flavours (e.g. ak, pistol, sword,  confetti grenade) , floors, obstacles, decorations, also relative paths to used images, sounds etc.
	- ``autosave/`` - a directory with all unsaved files.
		- most intuitive solution
		- it's a little like git index
		- we won't have to create templates and will be able to reuse the logic easily
			- cause it's just like a different project
	- ``game_modes/`` - directory for all game modes. Editor iterates recursively through this directory and loads them into std::vector<std::pair<std::string, team_deathmatch>>.
- on saving an untitled work, we will have to move the folder.
	- And possibly rename all of (tab, int, autosave will be absent though) that is inside to match the folder's name.
	- We'd anyway need this as "save as"
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

- "unique" naming for unique sprite decorations et cetera

- templatize some dangling memory stream arguments
- we should use asynchronous texture transfers, especially when we'll be regenerating atlases


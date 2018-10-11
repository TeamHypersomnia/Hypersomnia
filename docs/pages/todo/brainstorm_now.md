---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Allow for recording of multiple entities
	- Procedure
		- Given overwrite flags
			- motions, intents 
		- Given some existing stream of entropies for some player
			- On begin recording in some middle
				- Purge entropies in accordance with the flags
				- In snapshotted player, accumulate
				- Before passing total collected, always clean it on the editor_player side

- Determinism fixes
	- Change unordered containers to ordered ones in the mode state
	- Change unordered containers to ordered ones in the entropy
	- Change unordered containers to ordered ones in the visible entities
		- Actually, just provide sort inside layers for domains that require ordering
			- e.g. get first bomb

- Since sending of entropy will be highly optimized for space, it makes no sense to have augs::container_with_small_size

- Arbitrary pasting of entities
	- Vastly useful for importing stuff from testbed maps into existing ones
		- Let alone between community maps
	- This is pretty complex.
		- If we allow pasting across workspaces, we might even have to copy actual asset files.
	- Won't matter until after deathmatch stage
		- Surely?
	- The editor will have to construct the command tree, like
		- paste_flavours + paste_entities, if no requisite flavours are found inside the project at the time of pasting
			- the clipboard will have both the entity and flavour
		- the editor's clipboard can actually become...
			- paste entity flavour + paste entity command, stored, waiting to be executed!
				- the move itself won't need to be stored
	- Cut is just copy + delete

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

- Game events log and chat
	- In the same window
	- ImGui or own GUI?
		- We actually have some textbox code we can introduce later for chatting
			- Better control over such an important feature

- Windows back-port
	- use non-multisampling fb configs on Windows
		- proven to improve performance twofold, on linux

- Melee combat
	- Primary and secondary attacks for knives
		- Akimbo is thus handicapped only to the primary
			- But really it makes little sense to carry two knives
	- A melee attack cannot be interrupted, except when a collision of two attacks occurs
	- Attack collisions
		- When hurt triggers of two or more players touch, they are pushed away opposite to their facing

- However it is to the discretion of a mode *how* the creation and removal of players happen...
	- ...it is already outside of their scope *when* they happen.
	- Therefore, each mode shall expose add_player and remove_player functions to be called by literally anybody, anytime.
		- As for serialization, these will be some "choreographic events" inserted between steps, or in a case of a network session, "network event"
	- Similarly, change_var shall not be something that the mode bothers with, especially that the calling logic would be pretty much duplicated.
	- There will still be much use of the messages; e.g. mode_messages::game_completed to determine the result

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

- "I" in editor invokes initialization routine of the chosen game mode
	- Just starts advancing it from the beginning.

- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic

- Game-mode profiles
	- Each map can specify a "profile" with sensible values for a deathmatch, tdm, etc.
		- At the same time, it will be documenting the proper use of the map
	- Will be useful for a built-in test scene
			
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

- server will accept an ``std::variant<free_for_all, bomb_defuse...>``
	- each will perform its own logic

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
				x/x.int
				x/x.hyproj
				x/autosave/x.int
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

- Probably somehow disallow arbitrary inferring of relational cache?
	- There was some unresolved crash problem with this.

- check in editor if the saving/opening path is a valid folder?

- make reveal in explorer work for both files and folders
	- cause it also works for dirs

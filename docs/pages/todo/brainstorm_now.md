---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Context help
	- Enums corresponding to text files
	- Problem: arbitrarily colorized text is not supported in imgui

- Melee combat
	- Primary and secondary attacks for knives
		- Akimbo is thus handicapped only to the primary
			- But really it makes little sense to carry two knives
	- A melee attack cannot be interrupted, except when a collision of two attacks occurs
	- Attack collisions
		- When hurt triggers of two or more players touch, they are pushed away opposite to their facing
	- melee_fighter component
		- so that we don't have to hold this state per each melee weapon

- Allow to change the tickrate in the non-playtesting mode?
	- This will be a session setting, really
	- We'll just restart on re-tick

- file operations:
	- Import intercosm from lua
		- Implies starting a new project or clearing history, which will anyway imply an almost clear workspace.
			- So always start new.
	- Export project for compatibility
		- ``ProjectName.int.lua``
			- Contains the intercosm and rulesets, all important things.
	- Export mode vars
		- ``ProjectName.modes.lua``
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

- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic
	- Seed will have to be sent in the beginning anyway, along with the state
	- Some amount of initial information will need to be transmitted anyway
		- Like current players?
		- Isn't this all a matter of sending the bomb mode state?

- Sound should be loaded from the smallest to the biggest
	- So that effects are loaded first
	- New synchronization
		- store std::atomic<bool> next to sound_buffer in loaded_sounds_map
			- set it to false whenever definition changes or it is to be loaded
			- check if future is implemented same way

- Probably somehow disallow arbitrary inferring of relational cache?
	- There was some unresolved crash problem with this.

- check in editor if the saving/opening path is a valid folder?

- make reveal in explorer work for both files and folders
	- cause it also works for dirs

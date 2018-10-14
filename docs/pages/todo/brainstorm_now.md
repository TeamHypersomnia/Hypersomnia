---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Client-side adjustable crosshair sensitivity
	- Note that we will anyway require SOME form of synchronization of client settings, like nickname
	- **CHOSEN SOLUTION:** How about keeping floats in cosmic entropy?
		- The writing and reading of the entropy for network comms will be **contextual**, anyway
		- Notice that determinism won't be broken if we just change sensi during replay or recording, because the final value will always be held in entropy
	- Would it hurt to keep sensitivity inside the crosshair component
		- Yes, because we'll need to synchronize not just network comms on changing a local setting, but even local plays
			- which would otherwise not be necessary
		- though we have to share this information if we want to use shorts for communicating motion deltas

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

- Since mouse motions will probably be the bottleneck of network communication, both coords will usually be able to be sent in one byte
	- Range would be 0 - 16
	- There would be a bit flag for when it exceeds the range
		- Which will actually be often and then we might use two bytes
			- mouse motion should never really exceed 255
				- will also make it harder to use aimbots that instantaneously change mouse location

- file operations:
	- import intercosm (either lua or int)
		- Will just assign the new intercosm to the one in current project. 
		- We can "only" open projects, thus it is named "import".
		- Will rarely be used, usually just for porting or if someone wants to modify an intercosm downloaded from somewhere.
			- We could transmit projects though.
	- Export project for compatibility
		- In lua format only.
		- Will be named like ``ProjectName.compat.lua``
			- Contains the intercosm and rulesets, all important things.
		- We will only export the intercosm.
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

- Fixing crosshair to work with different screen sizes
	- For now the bound is hardcoded
	- Adjust it on each motion in main
		- In the end, review all motions and accumulate them to a single message
			- Later handles the problem of compression
		- accumulation shall happen later 

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

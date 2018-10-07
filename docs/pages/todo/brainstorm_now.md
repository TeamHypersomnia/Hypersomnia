---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Disable history gui seeking to revisions past the start

- Implement tree node for the children of commands in history gui

- Replaying and commands
	- Replay commands along the solvable's progress as they were applied.
		- pro: don't have to sanitize undos, they are always applied for the solvables upon which the command was originally executed
		- pro: repros replayable even with commands involved
		- pro: can do funny directing stuff
		- con: much space wasted but who cares
	- Implementation details
		- editor history
			- redo shall only move the solvable forward first if there is such a need;
			- only then apply command once
			- undo 
				- if solvable step at which it was applied matches, simply undo once
				- shall only use the seek_to if the solvable step mismatches

- Later if we want to concatenate recorded entropies, we may just hold a step_to_entropy per each concerned player and accumulate them before step
	- No need for some wanky overrides
	- though these overrides should be easy enough, we can even determine the authorized capability for transfers

- Implement view early to know what's going on

- Since sending of entropy will be highly optimized for space, it makes no sense to have augs::container_with_small_size

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

- Implementation order
	- Default chosen mode
	- Initially there should be none
	- Honestly why not choose it at fill stage?
		- No problem just altering the mode before start...
			- Just as we could do it in the first step

- Re-applying process
	- **CHOSEN SOLUTION:** make commands redoable arbitrary number of times, then force-set revision and re-do
		- most to code, probably cleanest architecturally
			- not that much to code
		- best processing performance and least ways it could go wrong, perhaps
	- **RECONSIDER:** undo everything and then re-do when the folder has been restored
		- least to code, worst performance
			- Will it be that bad?
			- Playtests won't be that long anyway
		- although... we'll need to skip some commands while undoing
			- which is sorta dirty
		- least to store as well
		- actually this will save a lot on space
		- and no need to force set revisions
	- always keep a copy of an un-executed command in a separate history
		- this only avoids the need to force-set revision and code re-redoability which isn't all that important
		- worse space complexity, not much to code, not much clean architecturally

- Player start
	- **CHOSEN SOLUTION**: Store only the essentials
		- Don't share a struct for this, just store them in a new struct explicitly one by one
			- That struct will be what should go into unique ptr
			- Contents:
				- Copy of the intercosm
					- From which we will also be acquiring the initial signi for advancing the mode
				- View elements:
					- Group selection state
					- Selection state
				- mode_vars
				- ~~From the player:~~
					- initial mode_state
						- actually, no, because we should always reset it to initial values
				- history?
					- NO! History will be the same, later we will only force-set revision

- Constraints in the playtest mode
	- we'll need to forbid undoing past the revision when we've started the playtest
		- Or just interrupt on undo?
		- Or just create a started playtest command?
			- That's BS becasue it won't exist after re-application
	- History will be left untouched?
		- Though we can always make it different inside GUI
	- we'll need to forbid filling with new scenes?
		- The only things untouched by the fill are history and current_path
		- Shouldn't matter really
	- Some notification might be useful

- Problem: The results of re-applied commands might turn out completely off if...
	- ...some entities were created in-game before e.g. duplicating some entities and later moving them
		- this is because the generated ids won't correspond
	- **CHOSEN SOLUTION:** Allow all commands on the test workspace, but...
		- ...simply sanitize the re-applied commands to only affect **initially existing entities**
		- E.g. duplicating or mirroring once will work just once
		- If we need it in the future we can add some remapping later
		- Just warn about this in GUI somehow
		- Also remove commands for whom affected entities are none in this model


- Manipulating the folder in place during playtest vs keeping another instance and redirecting
	- **CHOSEN SOLUTION**: We should just std::move around on start and finish.
		- Entry points for the folder might be quite many so that's hard to predict
		- Anyway everything in the folder, incl. history, should be quickly std::movable
			- folder has only 1000 bytes

- test the ensure handler

- Existential commands vs those that depend on them
	- Both need be sanitized anyway

- Editor step
	- A number of times that the editor player has advanced, not a cosmos in particular
	- Because a logical step of the cosmos might be reset on every mode round

- Commands could store the editor step when they were executed...
	- ...so that upon undoing, the editor falls back to the correct solvable state

- Editor player, modes and their replays
	- When the player is started:
		- Store 
		- Alternatively, when the player is restored, rewind
			- ...but this is wacky
	- When the re-applying is requested
		- Restore cosmos
		- Restore vars
		- Restore view elements
		- Force set revision in history without making any changes
		- Sanitize all commands one by one as they are being re-applied
	- Existence of initial signi denotes whether the player is playing
		- Actually, we should keep more than a session there, e.g. the saved selection groups?
	- Store optional of state to be restored

- Determinism: Change unordered containers to ordered ones in the mode state
- Determinism: Change unordered containers to ordered ones in the entropy

- Persistence of entity ids in editor and clearing them
	- Where entity ids are in the editor state:
		- Selection
			- Rectangular
			- Significant
		- Selection Groups
		- Entity selector
			- hovered
			- held
		- ids.overridden_viewed
		- Some commands
		- ticked entities in fae gui
	- What about selection groups when something is removed?
		- For playtesting, we should probably store the entire view state along with the intercosm.
			- Actually, store only the entity ids as they are subject to alteration during subsequent stepping.
			- Btw grouping is command-deterministic
	- The only thing we need to do to prevent crash is to always check if the entity is alive
		- A cleaner approach is to implement clear_dead_entities to be called after each step
		- This is because there is more cases of usage than there are cases of state
	- Cases of invalidation:
		- **Undoing a command that introduces new entities**
			- E.g. one that creates an entity from nothing
			- Look for both undo_last_create and "delete_entity"
		- We sometimes completely purge, but it's only for:
			- Fills;
			- Commands whose undoing or redoing should automatically select affected entities.
				- Instantiation, duplication etc.
				- Delete has no reason to purge selections of some other entities.
		- Delete command
	- mover should be deactivated when?
		- corner case: delete while move?
		- should work anyway and yeah, deactivate it then

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
		
- Structuring entropy
	- What a client sends to the server?
		- Necessarily, the contents of mode_player_entropy
		- The cosmic entropy generated by the main
			- The server validates it
			- There really ain't no difference between if the player attaches its entity id or the mode player id
			- In fact it can attach none at all since the server can associate it
			- So it can be further compressed during serialization pass.
	- What a server sends to the client?
		- The single, entire, summed up mode_entropy
	- Contribution of main to the entropy
		- Should main know about the mode entropy at all?
			- Argument against: Indeed, main wouldn't really post any mode entropy as the logic is concerned purely with cosmic content.
				- Main shall only generate cosmic entropy
				- Also, main only ever posts intents with regards to a single controlled entity.
				- Some remapping will be needed.
			- Perhaps not. It should only be concerned with sending inputs to entities according to the id already controlled.
	- Per entropic player id, all vectors of commands
		- Chosen strategy
		- Optionals instead of vectors for one-shots, e.g. team choices
			- But we don't have to differentiate for now
		- Zero-commands will be signified by a zero bit


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

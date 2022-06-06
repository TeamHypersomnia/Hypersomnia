- GUI
	- When choosing say an FFA for an aim_map
		- "There are no custom schemes for this Game mode."
	- OR:
		- First choose a scheme
			- Standard
		- After choosing a scheme, choose a compatible mode
		- Each scheme has default mode selected?
		- Uh, it might be complex
	- Well maybe ditch compatibility? Only specify recommended mode?
	- If we ditch compatibility, then let's just have "rules" format everywhere

- official/rules
	- common.rules
	- difficulties
		- casual.rules
		- competitive.rules
	- schemes
		- crazy_duel.rules ?
		- most will already be map specific, not sure if we'll have any official ones

- Redesigned initial arena folder structure
	- prefabs
		- aquarium
			- aquarium.png
			- aquarium.prefab
	- rules
		- common.rules.yml
		- schemes
			- aim_pistols.rules.yml
			- aim_snipers.rules.yml
	- opt-in compatibility, I guess
		- If all compatible, write empty list
		- Any new modes that we'll later introduce will be exotic anyway
	- arena.project
	- history.bin
		- maybe yaml for compatibility with newer versions? Let's save everything asynchronously anyway
	- private.key
	- view.yml

- Well let the map just have both a default game mode and a default scheme specified
	- Certainly an aim map might want to specify aim_pistols for example as its default scheme
		- And a bomb defusal
	- Don't care if aim doesn't make sense with say CTF
		- The map won't support it in the first place so...

# Editor GUI

- We can pay someone to properly re-map all of the official maps in the new editor.
	- The ultimate test.

# Format consideration when it comes to transmission

- We'll probably just send file-by-file to have nice granularity and progress response'
	- And it doesn't make sense to compress the heaviest of files which will be pngs and oggs

- We'll only compress the project files on the go with lz4

- max_block_size_v is 2 MB.
	- We can easily impose a per-file limit like that

- Actually we can only verify the map upon completely receiving it

- We oughtta move the connected players to spectators right away even while they are still downloading
	- and perhaps show their downloaded %

- For upgrading maps, it would be cool to retransmit only the stuff that changed
	- For quick testing cycles
	- Server easily first sends the temporary_id:path:checksum list
	- Client then requests a list of temporary_ids whose checksums don't match locally
	- Upon loading the map, the server will keep all project files in memory and compress the relevant ones so that it does not have to do it every time someone connects
		- It can also calculate checksums on that occasion, let's not store them in any cache please

- Upgrading and old map retention
	- I guess screw this - the history will be preserved anyway
		- ...though not the assets
		- perhaps it's pointless to send history file then?
			- It might be the biggest of all.
	- For now let's just upgrade the map in-place, if someone wants they can keep a git repository

# Architecture

- Let's keep the folder paths as project identificators
	- And keep the specific file names universal
		- arena.project
		- history.bin

- Redesigned folder structures
	- arenas/mine
	- arenas/official
	- arenas/community
	- Server-specific content packages
		- What if we want to introduce some funny sounds on deaths or purchases?
			- Best if it is not tied to the map
		- Ruleset packages?
		- Rulesets should ultimately never reference anything on the map for them to be universal 
		- "Content overlays"
	- content/overlays
		- arena-agnostic
			- round win/lose sounds
			- za chwile wybuchnie bomba

- A ruleset may be monolithic for simplicity.
	- And it may also hold the mode as a variable.
	- So hold a tuple of all mode-specific RULES (not rulesets) structs
		- But the game mode logic code can still receive only the relevant struct.
- So the final distinction is that the ruleset is a monolithic set of all rules both with the game mode variable
	- and "rules" are just sub-structs with specific rules, perhaps gamemode-specific

- Rock the vote for modes, schemes and difficulties

- We playtest in test_mode ALWAYS anyway, without a notion of competitive etc
	- true game modes and rules will only be finally tested in production

- We're concerned that the same file format is simultaneously:
	- A scheme to be picked
	- A common set of properties regardless of scheme
	- A frigging difficulty too

- Examples
	- We want to have an aim map.
		- Specify supported: Deathmatch, Defusal (for balance)
		- Custom schemes: aimpistol, aimsniper
			- But we want to play them in either game mode.
	- We want to have a map supporting many modes, like TDM, FFA, CTF, Defusal.
		- Maybe we want to play a duel with it.
			
		


- Again
	- Mode
	- Difficulty
	- Custom scheme
		- But these might have explicit game-mode compatibility
	- All have the same file format
		- None specifies/forces a mode
			- Just compatibility
	- Custom schemes might also specify scripts
		- Where compatbility might come in handy too
	- Generalist scripts should exist too
		- but it's a whole different topic I guess



- Rules vs Rulesets
	- Ruleset = a list of rules files
	- rules/global.ruleset.yml
		- rules/standard_welcome.rules.yml
	- duel.ruleset.yml
		- official/rules/quick_respawn.rules.yml
		- official/rules/short_freeze_time.rules.yml 
		- duel_welcome.rules.yml
	- standard.ruleset.yml
		- standard_welcome.rules.yml
		- The naming convention will be such that the first created (and default) ruleset will be named "standard"
		- but you can rename it, the guid will be what will be written
		- The server list might display something like Bomb defusal (Duel) if the currently chosen ruleset for a map is not default
	- separate folder for "rules"
	- GUI
		- "Add a custom rule..."
	- Rules should not be tied to a game mode
	- But, rulesets should specify a game mode
	- Monolithic "rules" structure
	- Rules in general are supposed to be just match vars that can be tweaked actively, by the RCON too
	- "Session rules"
		- Inside server_vars
		- Topmost priority
		- Are re-applied upon choosing another ruleset from the list
	- Current ruleset name is a server_var too
		- Should be sent as a guid really, to not pass paths around the net too
		- No, no guids, we can just iterate known files to find the filename matching the str
	- Official ones
		- standard_economy.rules.yml
		- standard_spawn.rules.yml

- Upgrading community maps
	- Private key stored within arenas/mine/project/project.key, only in the author's folder
	- Clients can download upgraded versions if they successfully verify through GPG
	- Clients refuse downloading otherwise and ask if you want to download to a different folder
		- Should allow downloading 

- Initial scene state
	- What do we put there initially?
		- The minimal setup that suppresses all errors
	- Choose a template
		- Bomb defusal map
	- Or just... supported modes
		- Bomb defusal map
		- Pick at least one
	- Default mode
		- Don't pick the preffix based on that because we might still use tdm_ OR fy_ for a deathmatch map (or even fy_ for a bomb defusal map)
		- Perhaps too much noise in initial creation gui, let's specify that later, because a ruleset will also be chosen by default
	- Depending on the supported modes, different stuff might be put there initially
	- Right, also depending on the supported modes, many more stuff will be needed to suppress errors

# Scene hierarchy mechanics

- This stuff with hierarchies is a mess
	- it becomes one when we put additional stuff into prefab instances

- Components in hierarchies
	- Will be cool to only inspect the single selected component in the inspector
		- Plus we might view it differently on scene depending on whether we want to edit the shape or the sprite
	- Additionally, it would be good to edit collision shapes on the prefab, not just on the images
	- Actually, for sprites, there's no reason
		- Only later for polygonal obstacles

- We should come up with some simplified model perhaps
	- But we certainly want to have components in the hierarchy

- What sucks is that we'll have to attach entities to components and that kinda sucks
	- We could have a list show up in the inspector at the top. It will be with icons and we'll be able to click particular components
	- Yeah, we don't want components in the hierarchy itself because it will be complicated as heck
	- Yeah, we CERTAINLY don't want to implement components themselves as composites
	- Ultimately we can simply show components always above the other attached entities
		- or show them in the inspector after all, we can have a flag for that
		- Still we won't make it possible to attach to components

- Look for some filesystem implementation for imgui
	- We'll also replace the system viewers? At least some of them?
		- Only reveal in explorer will be useful at this point

# Ad-hoc fixes

- We should move all gfx/sfx of official maps to actual official content
	- And simply assign it to properly named biome folders
	- Some complicated prefabs can already be map-specific
	- This way we won't copy the files when someone wants to make a variation out of a template

- Instead of having delete_laying_on..
	- Okay, no, that might be useful to delete weapons for some rulesets (e.g. gentlemanly mode)
	- Actually... perhaps a node reference to disable would be a better option
		- e.g. then we can disable only AWPs on a map like fy_snow

- Keep the variable names really short for the inspector to look good
- View->Window

- Refresh imgui font when changing size in settings

# Project selector setup

# Inspector logic

- Determining what to show in the inspector might prove non-trivial

- Inspector retains focus until another inspectable object is clicked?
	- Or something sets the inspector focus from the code
	- Edited windows should remember the last inspectable object and bring back inspector focus to it

# General

- We'll be able to remove "colorize" and "colorize_neon" from sprite component because the flavours will be created automatically
	- Actually most of sprite component will be able to be eliminated
	- Might be true of other components too
	- Remember that as we have less and less in components will make logic faster and it is the bottleneck
		- rendering can be slower, no matter
	- Yeah, the discrepancy between what we decide to actually put into a component (because it would create so many flavours)...
		- Is the precise reason we'd like to have editor-dedicated structs for serialization
	- BTW even light component could be all flavourized
	- All of this will seriously decrease the amount of stuff we have to hold in the solvable which is frigging GREAT


- Problematically, we can't just add single components and treat them as nodes
	- Because our component sets are pre-defined

- In fact, the entire intercosm will be a cache.
	- All flavours, all solvables, all viewabels defs, all logical assets...
		- Actually the rulesets could be compiled this way too
	- Which is super cool because it will be easy and quick to read it from disk.
		- Though these separate files never occur in the code without each other, still it's easier to load it in particular structures of the cosmos
		- So perhaps let's leave these files at that
	- actually let's make a single file for speed of reading
		- ez pz
	- And write the timestamp after writing that file
		- to always regenerate in case of any doubts about authenticity
		- the same timestamp should be written next to the project file on writeout?
			- and it's always written as the last thing after all resource files are written too
		- keep version stamps too

- Animation
	- A resource
	- Usually next to the frames in the folder
	- A preamble with paths
	- later use ids

- Do we open a separate tab for the resources? Or do we use the inspector?
	- I guess a tab is in order
	- alright, godot actually shows it in the inspector, perhaps it is more intuitive

- Don't show the import files in the filesystem window

- Looks like the changes in filesystem window sync all paths
	- Technically its more readable
	- BUT, GUID-based approach makes a lot more sense
		- See https://github.com/godotengine/godot/issues/15673#issuecomment-357455291
	- We'll just re-scan the filesystem every time we focus the window right? Did we do it like this earlier?

- Select this center window by default to create new tabs in
	

- Nodes without graphical representations might be useful
	- Like just nodes in godot, without anything
	- But under the hood it will just be a sprite decoration that is not selectable in the world (but gets highlighted somehow nicely when we click this node in the hierarchy)

- Actually how do we make those hierarchies?

- Prefab modification and flavour generation workflow
	- First, there exist only prefabs and their instantiations
		- with their arbitrary structue, but certainly different than that of flavours in some respects
		- e.g. resource paths instead of viewable ids

- Entire "solvable" should be a cache
- Entire "viewables" should be a cache
	- Generated from per-resource yaml metadata
	- The entire all_viewables_defs can be inferred from all resources in the folder and from which ones are used

- Supposing that a ruleset can reference map hierarchy state data, how do we apply it in-game in simulation?
	- We could theoretically let the mode logic know about map hierarchy data?
	- Or do we want rulesets to be compiled too?

- The hierarchies will realy exist as some elaborate metas only.
	- The entities that they refer to will have high probability of existing later in-game, so we can just pass it to the mode logic.
		- Though it will be better if we don't use it in the cosmos logic, for clarity.
	- The entities will exist alongside and there won't be any kind of whole-world translation per each modification.
	- Well that's at least true about transform hierarchies.
	- How about prefabs and instance variables?

- Do we want a simplified state structures for all components too?

- Synchronization of editor's hierarchy variables to the actual entities and flavours
	- I do figure it might make little sense to have a viewable id in an editor's sprite structure where we clearly want to serialize paths to resources
	- For simpler stuff, we can just alias types from logic to types for editor 

- Multi-views would be useful..
	- Lol. Neither Unity nor Godot have it.
	- E.g. to edit a scene and show it in context at the same time
	- Can't we really just render multiple cosmoi simply?
		- with get_viewport for each
		- get_custom_viewports
			- if empty, just one default
		- a simple for loop in main
			- and an i index for get_viewed_cosmos
	- Then we would need to use imgui docking EVERYWHERE, right?
		- It's a good idea after all
		- So no custom tabs inside
		- Should play rather nicely with big screens

- Don't include "custom tabs" in "central view".
	- Use imgui docking all the way.
	- We'll pass the custom viewport info anyway. For now, just for the first view window.

- The root node...
	- Do we want it?
	- Okay we actually need it for prefabs right.
		- Because then the children would be unpacked straight in place...
	- Might be good to, you know, keep its name synchronized with the file name.
	- The root node in the map itself will open the common settings for that map

- Play button

- Well we can have a special entity type like "playtest character" for playtests too on some small scenes with prefab rooms for example
	- just to test stuff

- Map testing architecture
	- I'd be after making it a separate setup
	- This can actually be the task of test_setup_scene
		- If we really need hot reload this bad...
				- We can always make the test scene setup hot-reload from disk, remembering the character's position and placing it on the scene
			- Screw recording for now... editor_setup will be the tool for that
				- Actually let's later just move recording utility to the test scene
					- q will 
	
- Unity has a single view for rendering the world too.

- Test scene and minimal test scene will belong to official
- Would be best to keep it a separate setup after all


- Actually let's just have a separate setup: playtest_setup
	- Test scene will be useful for local testing out weapons and training, we don't wanna meddle with that

- We can re-use currently implemented group behaviour for moving children along with the parent
	- Instead of playing with those

- So a lot of hierarchy functionality will just be translated into groups
- When we click something in the hierarchy, we'll just select the parent along with all children 
	- Clicking entity on the scene navigates to its hierarchy location
	- Clicking entity on the hierarchy selects it
	- Double-clicking entity on the hierarchy centers camera upon it

- Errors and warnings tab
	- The map can't be playtested with errors
	- Show the current severity icon next to the play button
	- Implemented errors
		- You need to have at least a single spawn point for Resistance!
		- You need to have at least a single spawn point for Metropolis!
		
- Category: special objects
	- spawn points

- Right, our "object categories" won't really be the entity types.
	- Again, the user should NOT be concerned with our architectural entity types.
		- We'll translate the game hierarchy properly.
	- They will just see cool categories like Special objects->Spawnpoints,Bombsites etc etc


- Against partial hot-reloads
	- We'll implement the setup of initial conditions to such an extent that it won't be needed
		- Resetting the world will be instant anyway
		- And it's a good retry flow, because after a modificaiton cycle our character will be somewhere else anyway

- Against keeping the game inside a separate tab (which would be hard architecturally because of how setups work)
	- Godot does this too
	- We anyway won't be editing anything while in-game because we have to restart the playtest setup to test it anyway
	- We'll focus well on playtesting

- A setup needs to be able to launch another setup easily
	- A callback with a launcher passed?

- We can have a single "Rendered view" window where the scene/prefab will be always rendered
	- It can have its own tabs, the ones implemented in editor_setup
		- An advantage of that is they will automatically be not dockable into other windows, only between each other
	- We'll just make another setup getter - get_custom_viewport
		- In the editor, this will always take the rendered view coordinates MINUS the tabs

- Project settings
	- Map miniature
		- Generate or pick custom
	- Maybe let's use like 0.5x or 0.25x quality by default

- Using YAML might make it easier to the extent that we won't need to pass around the lua state, probably

- Ruleset ecosystem
	- We'll certainly have a default one 
	- Proper overrides server-side
	- A list of rulesets may be good
		- still needs proper undo/redo

- How do we place additional windows like rulesets or project properties?
	- You can just have ruleset folder.
		- And show the contents in inspector upon clicking.
			- Thats just about right.
	- By the way undoable changes to files like that can just be implemented as file snapshots

- Will we ever launch a mode for a playtest?
	- I don't think it's necessary.
		- The warmups will finally never annoy us.
	- Actually let's do.
		- It's just that we'll use the test_mode by default.
			- If we want to test it, the bomb is always plantable regardless of a mode.
	- Shop should always be openable in the test mode too.
		- arghh.. the shop logic is only implemented in bomb defusal mode...


# Architectural concerns

- We'll keep the old editor code both as a reference and as a legacy world inspector
	- The old editor will still be useful as the inspection tool for translated worlds
	- The old editor code to keep won't be an entire setup, but rather only specific windows for flavour/entity editing

- Problem: We want a general editor for both grid-aligned maps and some arbitrary soldat-like maps.

- User won't be concerned with flavours
	- We'll just translate the state of all prefab instances into the required flavors
		- We'll thus essentially implement COWs
	- actually most of properties will only concern flavours
		- we'll probably code GUI for all properties in editor by hand

- Total revamp inside render layers
	- Or do we just translate the editor's stuff into proper render_layers?
		- We'll want arbitrary placement depth though

# Data formats

- General synchronization between nodes and actual entities on scene
	- cosmic entity data -> proxy nodes
		- The only functionality of proxy nodes over the entities will be hierarchicization of transforms and prefabization
	- Or... only ever proxy nodes -> cosmic entity data ?
		- Sibling order -> render layer index (int)
			- we'll want some layers to be always above others though. Like crates above the floor.
	- Some reinference procedure for the proxies
		- on rewrite_change in move entities command just recalculate the transforms of entities attached to descendant nodes
		- yeah, we'll keep track of all entity to node mappings

- .arena file has settings and the path to root node

- Unpacking .arena file (relevant when thinking about the proxy node structure)
	- Recursively from root node
		- Just unprefabize and set transforms in accordance with the hierarchy
			- Though the cosmos should stay in sync at all times
			- So there won't really be much unpacking to do. Only during load once to memory and to create the cache.
			- on save we'll just dump the proxy cosmos to .solvable and flavours to .common

# Interface

## World
- Selecting the topmost 
	- simply gather all candidates that were hit by the query, sort by their nodes and select topmost


## GUI

- Docking
	- Left-down corner dock
		- Filesystem
			- Project (default)
			- Official
				- From here you can drag and drop stuff like aquarium and others
				- BEGIN BY DRAG AND DROPPING SOME PREFABS TO THE SCREEN
			- what about other maps? We might want to use existing collections
				- what if they later get removed from these maps?


- Layout tool
	- Biomes to select
	- Perhaps an easy layout editor?
		- Problematically, we have to lay out the walls somehow automatically?


- Only one project will be open
	- To avoid confusion and overly complex architecture
	- We might want to auto-save only the opened and unsaved nodes (like text files)
		- as well as camera view
		- Well, other common stuff too










- I think let's leave the radar/minimap for later.
	- Most of the current miniature generation functionality will be useful for editor somehow.

- Miniature generation
	- For prefabs, we can just do a single zoomed-out SS
	- For maps, a full-fledged miniature generator would be handy
		- for miniatures, only later - IN GAME - overlay the A/B/C/buyzone markers 
			- especially since we might want to parametrize/scale it
	- make a set of singled-out entities to hide for miniature generation too


# Editor GUI

## Ad-hoc fixes

- We should move all gfx/sfx of official maps to actual official content
	- And simply assign it to properly named biome folders
	- Some complicated prefabs can already be map-specific
	- This way we won't copy the files when someone wants to make a variation out of a template

- Instead of having delete_laying_on..
	- Okay, no, that might be useful to delete weapons

- Keep the variable names really short for the inspector to look good
- View->Window

- Refresh imgui font when changing size in settings

## Project selector setup

## Inspector logic

- Determining what to show in the inspector might prove non-trivial

## General



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
		- In the builder, this will always take the rendered view coordinates MINUS the tabs

- Initial scene state
	- What do we put there initially?
		- The minimal setup that suppresses all errors
	- Choose a template
		- Bomb defusal map
	- Or just... supported modes
		- Bomb defusal map
		- Pick at least one
	- Depending on the supported modes, different stuff might be put there initially
	- Right, also depending on the supported modes, many more stuff will be needed to suppress errors

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



## Architectural concerns

- We'll keep the old editor code both as a reference and as a legacy world inspector
	- The old editor will still be useful as the inspection tool for translated worlds
	- The old editor code to keep won't be an entire setup, but rather only specific windows for flavour/entity editing

- Problem: We want a general editor for both grid-aligned maps and some arbitrary soldat-like maps.

- User won't be concerned with flavours
	- We'll just translate the state of all prefab instances into the required flavors
		- We'll thus essentially implement COWs
	- actually most of properties will only concern flavours
		- we'll probably code GUI for all properties in Builder by hand

- Total revamp inside render layers
	- Or do we just translate the editor's stuff into proper render_layers?
		- We'll want arbitrary placement depth though

### Data formats

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

## Interface

### World
- Selecting the topmost 
	- simply gather all candidates that were hit by the query, sort by their nodes and select topmost


### GUI

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
	- Perhaps an easy layout builder?
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


# Implementation details

- Somehow select this center area by default to create new tabs in
	- Both of prefabs and of files

# Interface

## World
- Selecting the topmost 
	- simply gather all candidates that were hit by the query, sort by their nodes and select topmost

### Views

- Multi-views
	- Can't we really just render multiple cosmoi simply?
		- with get_viewport for each
		- get_custom_viewports
			- if empty, just one default
		- a simple for loop in main
			- and an i index for get_viewed_cosmos

### Transformations

- We can re-use currently implemented group behaviour for moving children along with the parent
	- Instead of playing with those

## Builder UI

### General

- Only one project will be open
	- To avoid confusion and overly complex architecture
	- We might want to auto-save only the opened and unsaved nodes (like text files)
		- as well as camera view
		- Well, other common stuff too

### Play button

- Launches a playtesting_setup on the current scene or prefab

### Window: Errors and warnings

- The map can't be playtested with errors
- Show the current severity icon next to the play button
- Implemented errors
	- You need to have at least a single spawn point for Resistance!
	- You need to have at least a single spawn point for Metropolis!
	- You need to have at least a single buy zone for Resistance!
	- You need to have at least a single buy zone for Metropolis!

### Window: Project Files

- Project (default)
- Official
	- From here you can drag and drop stuff like aquarium and others
	- Move as much content from other official maps to the official repository, so that anyone can take advantage of these

- Don't show the import files

### First time helpers

- BEGIN BY DRAG AND DROPPING SOME PREFABS TO THE SCREEN

### Layout tool

- Biomes to select
- Perhaps an easy layout builder?
	- Problematically, we have to lay out the walls somehow automatically?

### Inspector

#### TWO TABS! 

- Summary and Details
- Summary has stuff like color or image path
- Details has stuff like neon map colors

#### Details

- Determining what to show in the inspector might prove non-trivial

- Inspector retains focus until another inspectable object is clicked?
	- Or something sets the inspector focus from the code
	- Edited windows should remember the last inspectable object and bring back inspector focus to it

### Miniature generation 

- I think let's leave the radar/minimap for later.
	- Most of the current miniature generation functionality will be useful for editor somehow.

- For prefabs, we can just do a single zoomed-out SS
- For maps, a full-fledged miniature generator would be handy
	- for miniatures, only later - IN GAME - overlay the A/B/C/buyzone markers 
		- especially since we might want to parametrize/scale it
- make a set of singled-out entities to hide for miniature generation too

## Choosing a finished map

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

- Well let the map just have both a default game mode and a default scheme specified
	- Certainly an aim map might want to specify aim_pistols for example as its default scheme
		- And a bomb defusal
	- Don't care if aim doesn't make sense with say CTF
		- The map won't support it in the first place so...



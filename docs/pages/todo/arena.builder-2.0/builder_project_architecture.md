# Project naming and identification

- Let's keep the folder paths as project identificators
	- And keep the specific file names universal
		- like arena.project
		- and history.bin

- Synchronization of editor's hierarchy variables to the actual entities and flavours
	- I do figure it might make little sense to have a viewable id in an editor's sprite structure where we clearly want to serialize paths to resources
	- For simpler stuff, we can just alias types from logic to types for editor 

# Scene hierarchy

- The root node...
	- Do we want it?
	- Okay we actually need it for prefabs right.
		- Because then the children would be unpacked straight in place...
	- Might be good to, you know, keep its name synchronized with the file name.
	- The root node in the map itself will open the common settings for that map

- Problematically, we can't just add single components and treat them as nodes
	- Because our component sets are pre-defined

- So we'll show the components in the inspector, in a handy list view

- A lot of hierarchy functionality will just be translated into groups
	- When we click something in the hierarchy, we'll just select the parent along with all children 
		- Clicking entity on the scene navigates to its hierarchy location
		- Clicking entity on the hierarchy selects it
		- Double-clicking entity on the hierarchy centers camera upon it
	- Mass movements of entities or mass selections
	- But we'll need to hierarchize the groups


## Exposing scene hierarchy information to the mode

- Should we do it?
	- What if a ruleset wants to refer to some information that is known only in the scene hierarchy?

- Supposing that a ruleset can reference map hierarchy state data, how do we apply it in-game in simulation?
	- We could theoretically let the mode logic know about map hierarchy data?
	- Or do we want rulesets to be compiled too?

- The hierarchies will realy exist as some elaborate metas only.
	- The entities that they refer to will have high probability of existing later in-game, so we can just pass it to the mode logic.
		- Though it will be better if we don't use it in the cosmos logic, for clarity.
	- The entities will exist alongside and there won't be any kind of whole-world translation per each modification.
	- Well that's at least true about transform hierarchies.
	- How about prefabs and instance variables?

## Object categories

- Our "object categories" won't really be the entity types.
	- Again, the user should NOT be concerned with our architectural entity types.
		- We'll translate the game hierarchy properly.
	- They will just see cool categories like Special objects->Spawnpoints,Bombsites etc etc

- Category: special objects
	- spawn points

- Nodes without graphical representations might be useful too
	- Like just nodes in godot, without anything
	- But under the hood it will just be a sprite decoration that is not selectable in the world (but gets highlighted somehow nicely when we click this node in the hierarchy)

# Implementation details

## Prefabs 

### General

Instead of per-instance overrides (which might be a pain in the ass)
we might want to have "init parameters".
- Perhaps it doesn't make sense but at least think about it.
- Might be simple or complex.
- Perfect for aquarium - we can pass a wall texture, a floor texture to fill it with, list of fish to instantiate etc.
	- In case of aquarium we don't want to just modify the instances by hand but just specify nice parameters for each instance.


### Unpacking (instances too) into flavours

- Prefab modification and flavour generation workflow
	- First, there exist only prefabs and their instantiations
		- with their arbitrary structue, but certainly different than that of flavours in some respects
		- e.g. resource paths instead of viewable ids

## Duality of state. Hierarchy vs Caches

- General synchronization between nodes and actual entities on scene
	- cosmic entity data -> proxy nodes
		- The only functionality of proxy nodes over the entities will be hierarchicization of transforms and prefabization
	- Or... only ever proxy nodes -> cosmic entity data ?
		- Sibling order -> render layer index (int)
			- we'll want some layers to be always above others though. Like crates above the floor.
	- Some reinference procedure for the proxies
		- on rewrite_change in move entities command just recalculate the transforms of entities attached to descendant nodes
		- yeah, we'll keep track of all entity to node mappings

- Unpacking .arena file (relevant when thinking about the proxy node structure)
	- Recursively from root node
		- Just unprefabize and set transforms in accordance with the hierarchy
			- Though the cosmos should stay in sync at all times
			- So there won't really be much unpacking to do. Only during load once to memory and to create the cache.
			- on save we'll just dump the proxy cosmos to .solvable and flavours to .common

- Entire "solvable" should be a cache

- Entire "viewables" should be a cache
	- Generated from per-resource yaml metadata
	- The entire all_viewables_defs can be inferred from all resources in the folder and from which ones are used

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

## Render layers

- Total revamp inside render layers
	- Or do we just translate the editor's stuff into proper render_layers?
		- We'll want arbitrary placement depth though

- .arena file has settings and the path to root node

# (REJECTED) Option: having components in hierarchies

- Will be cool to only inspect the single selected component in the inspector
	- Plus we might view it differently on scene depending on whether we want to edit the shape or the sprite
- Additionally, it would be good to edit collision shapes on the prefab, not just on the images
- Actually, for sprites, there's no reason
	- Only later for polygonal obstacles

- This stuff with hierarchies is a mess
	- it becomes one when we put additional stuff into prefab instances

- We should come up with some simplified model perhaps
	- But we certainly want to have components in the hierarchy

- What sucks is that we'll have to attach entities to components and that kinda sucks
	- We could have a list show up in the inspector at the top. It will be with icons and we'll be able to click particular components
	- Yeah, we don't want components in the hierarchy itself because it will be complicated as heck
	- Yeah, we CERTAINLY don't want to implement components themselves as composites
	- Ultimately we can simply show components always above the other attached entities
		- or show them in the inspector after all, we can have a flag for that
		- Still we won't make it possible to attach to components


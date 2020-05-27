- Flavour-based layers are really pointless and will be confusing. So no.

- In Specials tab, don't have fixed layers, only show all possibilities once we try to add something

- What about grouping (foldering) in special layers?
	- Well you can create a separate Lights layer too, why not
	- Will work pretty much the same way as objects layers
	

- Selecting tile layer for the brushing tools
	- We could even implement the tile layers later?
- I'd guess tile layers shouldn't initially be selectable from the scene itself
	- At most in the z order list or just in the context menu when pressing shift or so

- Stamps across the foreground/background layers
	- Screw this for now. We will anyway want to have locations look unique everywhere
		- We'll later implement streaming

- Well in Construct we have default layers for the object types
	- Still we can change on per-object basis

- Okay why can't we just have arbitrary layers with arbitrary objects inside, fully reorderable?
	- We could then add an aquarium easily as a separate layer
	- Or a group with a tile and object layer
		- aquarium folder would be special? And would have parameters?
- Newly dragged objects just end up on top of everything else
- Firstmost found object layer
	- Though might be a pain if it lands somewhere like in aquarium

- Problem: Default object placement in a layer
	- When we drag and drop from a filesystem
	- Construct has the default layer encoded in the object type 
		- what if it gets deleted?
	- We can later have some elaborate proximity detection
		- Like always place above the hovered object
	- There's nothing wrong with iterating through the folders too
		- Because if we want to add something to the aquarium, why not
	- (Unassigned layer)
		- Always on top, immutable
		- Could be for stuff we didn't automatically find a suitable layer for

- Z order window should, understandably, only show layers from the same sorting layer
	- Is a great help in selecting objects too

- Selecting always navigates to the object in layers

- Prefab bundles? Lol

- If we allow mixing physical and non-physical tiles, it makes little sense to have two separate tabs for ground and solids
	- Well we'll need to split these for the game code anyway

- Structuring
	- Either Tabs or Separators: Ground/Foreground/Solids/Items/Special
		- Flavours?
			- where it makes sense:
				- Items on ground -> BAKA47s
	- What about Lights/Wandering pixels?
		- Their ordering makes little sense
		- User should just see "Particles" instead of wandering pixels/smokes etc
	- All tabs
		- Ground
		- Solids (?)
		- Foreground
	- All special objects
		- Lights
		- Particles
		- Sounds
		- Areas
		- Spots
		- Callouts
	- Each tab has an eye icon for quick hiding
	- Also selecting each category quick-writes a filter in the filesystem
		- Not sure though because it'd make little sense for flavoured layers later perhaps


- Material objects could be used for mulitple tile types
	- Though this would mostly be for the ground invariant
	- Ground material
	- Solid material

- decoration_layer vs render_layer
	- Let's not complicate it further at this point
	- Because this API will anyway be transparent to the user
	- and we'll have some builder's ad-hoc enums for these

- Hovering over a palette highlights the whole logical tile
	- Clicking selects it whole
	- Dragging can select in smaller chunks or just multiple

- Right-clicking tile on scene works similar
	- drag selects many, rightclick selects logical

- Dragging mouse while painting should move the brush by the stamp size and not by just a single tile

- We'll just have category presets like "Glass" that'll initially set those special function flags to false 

- We can show a tile pallette always when a tile layer is selected
	- Conversely, when an object layer is selected, we can show relevant objects to be placed in a filesystem-like structure
		- Perhaps filtered by the layer type too?


- What about layers for complex predefined objects like an aquarium?
	- Well we certainly don't want to expose all those shit with fish to the artist unless they really want to
		- They will NOT want to edit the individual fish positions okay...
	- well can't we just hide it and make uninteractable? Should be no problem
	- but it needs to be an object at the very least
	- Adjustable layer offset
		- Determined from all layers below

- The index is calculated cross-layer because there might be more than one "ground" object layer
		
- Let's think of an example of a layered map
	- Floor tiles
	- On floor tiles

- Perhaps object-based after all? Just that each has a layer and tile objects too have one
	- Though it might be cool to arrange objects into layer-like folders

- So aquarium is a special object
	- What does layer placement mean for it?

- Now we have a problem of whether we want to "unpack" the special objects
	- Well in Unity you have unpackable prefabs right

- Problem: for performance, we really want to imply a specific order of rendering for given layer functionalities
	- Point is we can force a single drawcall on them

- Now what about objects whose render layer is implied like wandering pixels?
	- should it be implied?

- We might allow for unpacking later, why not

- Tabs?
	- Foreground
	- Background
	- Walls

- We can't treat when_born as render order because entities will be created in a single step
	- No matter, we can spare a single int in render component

- But really for some stuff it doesn't even make sense to have render invariants
	- the layer is implied for shootable weapon and all those item entity types

- Maybe create a layer folder for any special object?

- Layers inside Sorting layers could be separated by imgui separators
	- but where does aquarium fit in?
	- do we force it into a ground sorting layer?
		- and if there is no layer in that sorting layer, create one?

- We might just have a "layers" and "objects" view

- For the case where we have tiles with another material on the edge
	- we'll need to be able to arbitrarily place a step sound effect
	- like we currently do in the server room
	- respected by the layers too
	- actually a "step sound" might be one of the special categories with a custom icon

- A universal eraser that clears tiles on every layer under it?
	- Actually just select layers

- Eyedropper tool under "i" or something
	- Pressing LPM shows tiles from all visible layers to the left?
		- Scrolling iterates through the list

- Apart from the eyedropper for single tiles, we should be able to also make a rectangular selection to make a brush

- Multi-layer stamps


# Advanced painting (for later)

- Square/diagonal/cave brushes
- Paint floor polygonally
	- For those diagonal rooms

# Cool polygon editing

https://www.youtube.com/watch?v=elws59R9CrM

# We can "borrow" some icons from Tiled

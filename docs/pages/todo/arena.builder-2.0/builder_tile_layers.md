
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

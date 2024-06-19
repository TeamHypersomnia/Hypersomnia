In the Images GUI, you can:

- Edit properties of imported images.
	- You can check multiple images for editing.
	  The same rules apply as when editing multiple flavours.
- Export properties of images to a neighboring ``.meta.json`` file.
	- Any time that an editor imports an ``path/to/img/image.png`` for its first use, 
	  it will check if a ``path/to/img/image.meta.json`` exists. If it does,
	  the default settings will be loaded from that file.
	- Almost all official images have a ``.meta.json`` file. 
	  Naturally, they will be used whenever you fill your project with a test scene.
- Manually import properties of images from a neighbouring ``.meta.json`` file. 
- Forget the orphaned images - those that are not used anywhere anymore.
	- Simply press "F" next to the orphaned image name.
		- Warning: this will cause the image properties to be deleted from the project.
		  Only the ``.meta.json`` file will, of course, survive.

## Renamed, moved or deleted images

It might happen that you need to move or rename an image that is already imported to the project.
The editor, on the next startup, or after pressing "Re-check existence", 
should detect that some images are missing from their original locations on the disk.
Worry not, no information is lost when this happens. 
A list with all missing images should appear above all other image entries.

On your filesystem, restore the file to its original location.
Then, in the editor, press "Re-check existence" to let the editor notice that the file is already there.
Alternatively, simply point the editor to a new path and you'll be good to go.
The properties will be left untouched.

In any case, the game logic has all the information that it needs to advance, even without the image file.
The only consequence is that the game will have a glitch at places where the missing image would normally be used.

## Using locations

A rather remarkable feature of the editor is the ability to view where in the entire project is a particular asset used.
	If it is not used anywhere, it is called "orphaned" 
	and can then be safely removed from the project.

To use this feature, tick the "Using locations" option.
A new column should appear with the locations printed.

## Current asset dialog

Make sure the "Properties window" is checked and select any image on the list.
A separate window should pop up for editing its properties.

Whenever the current asset dialog or the Images dialog is focused, you can:
- Press Page down to move to the next image on the list.
- Press Page up to move to the previous image on the list.
The Page down and Page up buttons also respect the filter applied to the list.

The reason for having such a separate window is that you can quickly edit 
several similar images in a quick succession.
	Suppose you have just imported a torso animation consisting of several frames.
	You will want to assign proper hand, shoulder, head offsets for each animation frame.

	With PgUp and PgDown, you can simply open the desired offset widget for a single animation frame,
	and then proceed to apply similar placements for the next/previous image on the list,
	without ever having to navigate to the same widget again.

## Extra loadables: Neon maps

Neon maps enable the entities to emit their own light.
First, the input image is searched for pixels of specified colors.
They are then blurred with a strong gaussian-like filter,
and later used in light calculations.
	To specify which colors are to be considered, open Light colors node.

You have to experiment with other parameters for the algorithm to truly understand their effect.

## Extra loadables: Generate desaturation

Should be always seleceted for spell images since these are displayed in grayscale
whenever the players has not enough Personal Electricity to cast the spell.

Should be off otherwise.

## Offsets

Some of the images used for flavours require important metrics.
Examples:
- a torso image might need to have its primary and secondary hand positions specified.
- a gun image might need to have its magazine position specified.

Go to Meta->Offsets of an open image.
Unfold a group of offsets depending on the type of the image.

The controls for choosing an offset are as follows:
- LPM changes the offset position.
- Holding Shift and clicking LPM changes the offset rotation.

## Physical shapes

By default, whenever an entity is created in the physical world,
the shape of its body takes on a rectangle with the dimensions equal to these of its sprite.
This isn't desired for some irregular objects, like pistols.

You can assign arbitrary shapes to images that will be used in physical calculations,
instead of a plain rectangle.

Choose any offset from within Meta->Offsets.
You will be presented with a special-purpose widget for precise vertex placement.

The controls are as follows:

- Always the vertex or the edge closest to the current cursor position is affected.
- Press LPM to shift the closest vertex to the position of the mouse cursor.
- Hold Shift and press LPM to add a new vertex on the closest edge.
- Hold Left Alt and press LPM to remove the closest vertex.
- Hold Left CTRL to see the complete convex partitioning for the object.
	- If the entire shape is convex, the edges will be highlighted in green.
		- Note that it is recommended for shapes of some images to remain convex, 
		  so this is how you verify it.
			- for example, shapes of the bullets (but not those of whole cartridges) should remain convex.
	- Otherwise, all edges will be highlighted in orange.

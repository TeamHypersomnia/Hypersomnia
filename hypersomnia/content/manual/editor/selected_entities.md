You have just selected some entities. 
When you are done reading this section,
press Pause Break to clear the selection and show general help for this view.

The selected entities are highlighted with a bright blue color.
The editor always draws a white bounding bounding box around the selected entities.

If you don't like the colors, you can tweak them in Editor settings (Esc->Settings->Editor->Appearance).

## Selection rectangle

To select many entities, drag the mouse while holding LMB.
A selection rectangle should appear. 

By default, the rectangle will only select entities on the same render layer 
as the first entity hit by the rectangle.

You will change this behaviour on the fly. 
In the order of the most general to the most specific:

- Shift + 1 picks "Everything" mode.
	- All entities touching the selection rectangle will be selected.
- Shift + 2 picks "Layer" mode.
	- After the first entity is selected, only entities having the same render layer will be added to selection.
		- This way you can quickly select all walls, or for example all floors.
- Shift + 3 picks "Flavour" mode.
	- After the first entity is selected, only entities having the same flavour will be added to selection.
		- This way you can quickly select all Electric rapiers, or for example all BILMER2000 magazines.

To see which Rectangle Selection Mode is currently chosen, 
look at the bottom of the screen and find the "RSM: " indicator.

You can press Ctrl+A to select all entities according to the current RSM.

## Combining selections

You can add entities to the current selection,
just like you would files in a file explorer.
Hold CTRL to add or remove entities from your selection.

For example, to select all the Force and PED grenades (the red and the blue ones), do the following:

- Press Shift + 3 to pick "Flavour" RSM.
- Navigate with your mouse to the column of grenades laying on the street, just a little below the origin.
- Select any of the red grenades.
- Press Ctrl+A - the selection will now contain all entities having the same flavour as the last selected entity.
- Start holding left CTRL.
- Select any of the blue grenades.
- Press Ctrl+A to add all blue grenades to the selection.

Done!

## Selection groups

Selection groups exist so that blobs of conceptually related entities 
can always be automatically selected together.

Press Alt+G to open the selection groups dialog. 
The dialog lists all existing selection groups.

- To group currently selected entities, press Ctrl+G.
	- A new group named Group-0 should appear in the dialog.
- To remove currently selected entities from all groups, press Ctrl+U.
	- If a group has no entities left, it will automatically be deleted.

These invoke actual commands,
so make sure to save your work after each grouping or ungrouping operation.

In the "Selection groups" dialog, unfold the chosen group node by clicking exactly at the arrow icon.
There, you can rename the group.

To select all entities belonging to a particular group, click on its name.
You can also filter existing groups by using the text area at the top of the dialog.

## Editing properties

To view the selected entities in a hierarchy and edit their properties,
press Alt+S to open "Selection hierarchy" and follow further instructions.

## Operations: View

- Press Z to place the camera exactly at the center of the selection's bounding box.
	- Press Shift+Z to do the same but to additionally reset zoom.

Press Alt+L to open Layers window and navigate to "Keyboard bindings" section
to see what else you can do with the selected entities.

## Operations: Existential

- Press D or DEL to delete the selected entities.
- Press C to duplicate the selected entities.
	- You will be able to move them right away.
- Press Ctrl + Up/Down/Left/Right arrow to mirror the selection in the respective direction.
	- "Mirroring" here means simply duplicating the selected entities 
	  and flipping them along the axis specified by the chosen arrow button.
	  This is greatly useful for creating symmetrical rooms.

	  Note: while positions and rotations of all entities will be properly mirrored,
	  not all entities will have their sprites flipped.
	  While it may make sense to flip a sprite of a wall or a floor,
	  it makes no sense to flip a laying item's sprite.
	  There are also entities that have no sprites at all, like lights, area markers or wandering pixels.

## Operations: Transformations 

- Press T to start moving the selected entities with your mouse.
- Press R to start rotating the selected entities.
  - You can also press Ctrl+R to rotate the selected entities by exactly -90 degrees.
  - You can also press Shift+R to rotate the selected entities by exactly 90 degrees. 
- Press Shift+H to flip the selected entities horizontally.
- Press Shift+V to flip the selected entities vertically.
- Press E to start resizing the selected entities along a single axis closest to the cursor.
- Press Shift + E to start resizing the selected entities along two axes closest to the cursor.


Remember you can always undo the last performed operation simply by pressing Ctrl+Z.
You can redo operations as well - by pressing Ctrl+Shift+Z.
To see the history of all changes and navigate freely through all revisions, press Alt+H (View->History).

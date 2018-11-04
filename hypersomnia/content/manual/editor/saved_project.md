Perfect! Your work is all safe now.
There are many things that can be done, from here;
Make sure to read carefully through all of these sections, exactly in the order they are presented.

## Autosaves

To minimize the likelihood of losing your progress, the editor comes with an advanced autosaving functionality.
You can exit the editor at any time, and on the next launch, all projects will be restored exactly as you left them,
including things like the history of changes or the camera position.

All open projects will be autosaved:
- Every time you switch window focus.
- Every time you exit the editor.
- Every time the game crashes...
	- ...unless it is due to a segmentation fault or some other catastrophic event.
- Additionally, every single minute. 

To configure autosaving, press Esc, choose Settings and navigate to Editor tab. 
Many other tweaks to editor's behaviour are available there.

The autosave information is stored next inside your project directory, in a folder named ``autosave``.  
This folder should replicate exactly the rest of the project's directory tree.  
The autosave folder can always be safely deleted in order to return to the state at the last explicit save.

## Basic navigation

You should be able to navigate through the world intuitively using your mouse:

- Drag the mouse while holding RMB to move around.
- Use your mouse wheel to zoom in or out.

You can use keyboard to do the same:

- Press your arrow keys to move around.
- Press - to zoom in and = to zoom out.

Whatever movement you make, holding Shift will make it faster.
Holding left Alt will make it slower.

Remember that if you get lost exploring the map, simply press HOME to return to the origin of the map (x = 0, y = 0, zoom = 100%).

## Selecting entities

Using the mouse cursor, you can select entities in the game world.
The currently hovered entity will be highlighted with a bright color. 
If you don't like it, you can tweak the colors in Editor settings (Esc->Settings->Editor).

To select the hovered entity, press LMB. 
To select many entities, drag the mouse while holding LMB - a selection rectangle should appear.

## Marks

You can also use Marks feature to remember some important locations of the map:
Whenever you are looking at a location you want to remember:

- Press M to open the Marks dialog in the marking mode.
- Press another key under which you want to remember (mark) the location.

You have successfully marked a location.
Later, to return to that location, press ' (apostrophe) to open the Marks dialog again - this time, in the jumping mode:

- Press the key under which the location was remembered.

The previously chosen location is at all times assigned to the apostrophe itself. 
Thus, you can quickly jump between two chosen locations by double-tapping the apostrophe. 
This is very useful if you want to move several objects across two distant places - instead of making a lot of mouse movements to navigate between the two locations, 
simply mark both locations once, jump to the first and later to the second location, 
and then, simply double-tap the apostrophe to switch between them.

## Modes

Press Alt+D (View->Modes) for more information.

## Playtesting

You will certainly want to test how your map plays.
To do so, simply press "i" to begin playing your world.

The currently selected mode will start executing its logic on the map.

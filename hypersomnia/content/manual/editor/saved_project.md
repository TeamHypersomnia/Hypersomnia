Perfect! Your work is all safe now.
There are many things that can be done from here.

Make sure to read carefully through the following sections, 
DON'T SKIP ANY, and read EXACTLY in the order they are presented!

## Autosaves

To minimize the likelihood of losing your progress, the editor comes with an advanced autosaving functionality.
You can exit the editor at any time, and on the next launch, all projects will be restored exactly as you left them,
including things like the history of changes or the camera position.

All open projects will be autosaved:
- Every time you switch window focus.
- Every time you exit the editor gracefully.
- Every time the game crashes...
	- ...unless it is due to a segmentation fault or some other catastrophic event.
- Additionally, every single minute. 

If you wish to tweak autosaving behaviour:
- press Escape button,
- choose Settings,
- navigate to Editor tab,
- unfold the "General" section.
You can find many, many other options there that can change how editor behaves.

The autosave state is stored inside your project directory, in a folder named ``autosave``.  
The folder exactly replicates the rest of the project's directory tree (more on that in the next section).  

- On launch, the editor automatically loads the autosaved state if it exists, instead of the files saved explicitly (``Ctrl+S``).
	- When trying to open a folder located at ``C:/Project``, the editor first checks if a folder named ``C:/Project/autosave`` exists.
		- If it is found, the opened tab is populated only with the work from the ``C:/Project/autosave`` folder. 
      	The tab itself still carries the path of the original folder,
	  	so an explicit save (``Ctrl+S``) will properly update the real project folder.

The autosave folder **will be deleted** every time you explicitly save the project.
If you wish to revert the project to the state where you last saved your work explicitly.
you may safely delete the ``autosave`` folder at any time.

## Project directory structure

You will find a bunch of files and folders inside your newly saved project,
the meaning of which is as follows:

- Folder: ``autosave``. 
	- You already know what it is for - if not, return to the "Autosaves" section.

- Folders: ``gfx`` and ``sfx``.
	- This is where you put your project-specific resources.
	  Though initially empty, these two folders are created automatically for your convenience.

	  In your project, you will be able to import images and sounds ONLY from these locations:
		- the project-specific ``gfx`` and ``sfx`` folders,
		- the official collections (content/official/gfx, content/official/sfx).

	  This is to ensure that your project folder has everything it needs to be opened on another machine.

- File: ``Project.int``.
	- A binary blob representing your game world. 
	  ".int" refers to an "intercosm" (a short for an "Interactive cosmos") file format. 
      It MUST exist at the time the editor opens a project folder.
	  You should not delete or otherwise alter it.

- File: ``Project.hist``.
	- A binary blob representing the history of changes. 
	- Can be safely deleted at any time if you don't want to track some old operations anymore and it gets too big.

- File: ``Project.view``.
	- A binary blob holding the camera state, grid settings, marks, hidden layers and others (more on these later).
	- Can be safely deleted at any time to reset those settings.

- File: ``Project.view_ids``.
	- A binary blob holding identificators of selected entities and selection groups (more on these later).
	- Can be safely deleted at any time to reset those settings.

- File: ``Project.rulesets``.
	- A binary blob representing the predefined rulesets of modes to be played on this map (more on these later).
	- You should not delete or otherwise alter it.

- File: ``Project.player``.
	- A binary blob representing the player state (more on that later).
	- You should not delete or otherwise alter it.

To ensure that gameplay recordings are replayed deterministically,
NONE of the above files should be deleted during a playtest (more on that later).

## Basic navigation

You should be able to navigate through the world intuitively using your mouse:

- Drag the mouse while holding RMB to move around.
- Use your mouse wheel to zoom in or out.

You can use keyboard to do the same:

- Press your arrow keys to move around.
- Press - to zoom out and = to zoom in.

Whatever movement you make, holding Shift will make it faster,
whereas holding left Alt will make it slower.

Remember that if you get lost exploring the map,
simply press HOME to return to the origin of the map (x = 0, y = 0, zoom = 100%).
	Press Ctrl+0 to reset the zoom without resetting the camera position.

## Marks

Marks let you remember some important locations in the map.
These are inspired by Vim.
Whenever you are looking at a location you want to remember:

- Press M to open the Marks dialog in the marking mode.
- Press another key under which you want to remember (mark) the location.

You have successfully marked a location.
Later, to return to that location, press ' (apostrophe) to open the Marks dialog again,
but this time in the jumping mode:

- Press the key under which the location was remembered.

The camera should jump to the chosen location.

The previously chosen location is at all times assigned to the apostrophe itself. 
Thus, you can quickly jump between two chosen locations by double-tapping the apostrophe. 
This is very useful if you want to move several objects across two distant places - 
instead of making a lot of mouse movements to navigate between the two locations, 
simply mark both locations once, jump to the first and later to the second location, 
and then, simply double-tap the apostrophe to switch between them.

## Selecting entities

Using the mouse cursor, you can select entities in the game world.
The currently hovered entity will be highlighted with a bright color and outlined with a dashed line.

Select the hovered entity by pressing the Left Mouse Button.
The contents of this window will change when you do so - follow the instructions provided there. 

## Go to entity

Press Slash (/) to start quick search for an entity.

## Assets

Visit these windows to get more information:

## Scene hierarchy

Press Alt+A (View->Scene hierarchy) for more information.

## Modes

Press Alt+D (View->Modes) for more information.

## Playtesting

Naturally, you will want to test how your map plays.
To do so, press HOME to return to the origin,
and then press "i" to begin playing your world.
The currently selected mode will start executing its logic on the map.

If camera is not centered at origin by the time you start the playtest,
it will not follow your character but stay in the same place instead.

## More information



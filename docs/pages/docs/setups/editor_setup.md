---
title: Editor setup
tags: [topics, setups] 
hide_sidebar: true
permalink: editor_setup
tip: This page serves as a manual suitable for both programmers and content designers.
summary: |
    The **editor setup** is a [setup](setup) that allows [authors](author) to work with [intercosm](intercosm) objects.  
    It can read intercosms from files on the disk and perform various operations on them, like create new entities or record and replay simulations.  
---

## Saving and loading your work

The editor supports read and write operations on both ``.int`` and ``.lua`` [intercosm file formats](intercosm#file-formats).  
The "File" menu button provides all supported operations, most with keyboard shortcuts.

### ``.unsaved`` file format

The ``.unsaved`` files always contain the most recent state of the work, even if it hasn't been saved yet to the file originally specified while saving.  
They always reside next to the original file, e.g. having ``D:/works/my_work.int``, there might at some point appear ``D:/my_works/my_work.int.unsaved``.  

These files are created automatically when:
- The editor exits with unsaved changes - either gracefully through the ESC menu or a shortcut like Alt+F4...
- ...and additionally every single minute - this is the default *autosave* behaviour. The interval is configurable.
	- This means that if you've forgotten to save your work for an hour, and your computer suddenly BSODs, you will lose a minute of your work at most!

#### Behaviour

- When trying to open any ``*.int`` or ``*.lua`` file, the editor first checks if there exists a file next to it, named ``*.int.unsaved`` or ``*.lua.unsaved``.  
If it is found, the opened tab is populated with the work from the ``.unsaved`` file, but the tab itself still carries the name of the original file, so saving the work with Ctrl+S will still properly update the real file. 

- **Caution:** suppose you have some old and forgotten ``some_work.int`` and ``some_work.int.unsaved`` next to each other.  
Now you decide to save some other work as ``some_work.int``.  
At this point, the backup file - ``some_work.int.unsaved`` will be considered unnecessary **and will be deleted**.  
If we'd allow the unsaved file - now completely unrelated - to still exist, then if the application would crash several seconds from now,  
the editor would later load that unrelated file instead of your save.
	- Thus you are completely safe if you do not play around files with ``.unsaved`` format. This is an autosave file, meant for use only by the editor.

## Modes

- Normal mode - this is the default mode that the editor is put in when it is launched. 
	- The game world is frozen, allowing to perform precise operations on all entities and other intercosm data.
	- It keeps track of history of changes.
- Gameplay mode - this is when you start playing the game controlling the entity you have earlier chosen for this.
	- Meant primarily for experimenting with what you have just created.
	- Any session of playing gameplay mode can be replayed.
	- An intercosm can store multiple replays for whatever purpose, e.g. for the main menu setup to play an intro scene.

## Dialogs

### Context help

The editor features a context help window which always tries to show tips relevant to the current situation.
It serves as a guide for first-time users. It has unique content every time:

- the user is inside an empty editor with no tabs;
- the user is inside a tab with an empty intercosm;
- the user is inside a tab with a filled intercosm;
- the user hovers over any kind of dialog.

The textual content used in the context menus is found in ``editor/help/*.txt`` files.

### Layers

- Contains a checkbox to toggle visibility of any [render layer](render_layer). 

### Entities
- Has a single tab for each entity.
	- Each tab reveals properties that are modifiable for that entity or its flavour.
- Upon switching to a different tab, the same kind of property is automatically brought to view and selected.
	- Enables one to quickly duplicate one property between two or more entities.
	- It might happen though that the type of the entity in the tab we switched to does not have this kind of property.
		- E.g. a crate might lack a firerate property.

- Tab/ctrl+tab, ctrl+k/j, c+w work
	- conflict with editor tabs

- shift+k/j move the tab?
	- would be best if they could move to a field

### Go to entity

Default shortcut: ``/``.
Pressing Enter opens a new tab with the chosen entity in the [Entities](#Entities) dialog.

### Go to all

Default shortcut: ``ctrl + ,``.
- Goes to any field in the cosmos common significant
- Goes to any viewable
- Going to entities alone will be accomplished via quicksearch or just a window with all entities viewed

#### Future work

- ``Shift + arrows`` let one select multiple entities from that window
- ``Ctrl + enter`` adds to the current selection instead of replacing it

### History

Shows the history of the last *n* [commands](editor_command).
Can click on each command to go back to a given point in time.

### Property editor

### Type editor

See: [entity flavour](entity_flavour).

When creating a new entity flavour, the editor should add some recommended invariants from the start, e.g.
- interpolation (unless we make its existence implied/recommended?)

## Camera behaviour

In **gameplay mode**, the camera always behaves in accordance with the game mechanics,
e.g. it is centered around the character and panned with respect to the crosshair's position.

In **normal mode**, the camera's position can panned by:

- Holding the Right Mouse Button and moving the mouse around.
- Pressing the arrow keys.
	- Holding Ctrl or Shift manipulates the panning amount. 

The camera can additionally be zoomed in or out by:
- Scrolling the wheel button.
- Using + and - keys.
	- Holding Ctrl or Shift manipulates the zooming amount.

## Tokens

An entity token is of the form

## Autosave behaviour
## Multiple selection of entities

## Editor command

&nbsp;&nbsp;&nbsp;&nbsp;*Main article: [Editor command](editor_command)*

The editor keeps a history of changes, thanks to which an author can **undo** or **redo** their changes as they deem fit.

## Inferred state

An editor will most likely need some inferred caches, like entities sorted lexicographically.  
We might always facilitate this by adding some inferred caches for [cosmos common], because it is very rarely reinferred.  
As we will also always trigger cosmos's reinference manually when the editor changes some associated state, we might just reinfer along with the cosmos, never needing to hold additional information in the common inferred.  
 

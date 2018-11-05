---
title: Editor User Manual
tags: [topics, setups] 
hide_sidebar: true
permalink: editor_setup
tip: This page is a manual targeted to content designers, but programmers should as well read it to gain basic understanding of the inner workings.
summary: |
    The **editor setup** is a [setup](setup) that allows [authors](author) to work with [intercosm](intercosm) objects.  
    It can read intercosms from files on the disk and perform various operations on them, like create new entities or record and replay simulations.  
---

## Project structure

The editor considers a **folder to be a project**.  
The **project name** is equivalent to the **folder name**.  

Among other meta file formats, the editor supports read and write operations on both ``.int`` and ``.lua`` [intercosm file formats](intercosm#file-formats).  
The "File" menu button provides all supported operations, most with keyboard shortcuts.

### ``autosave`` subdirectory 

You might notice that, next to your project files, there appears a folder named ``autosave``.  
This folder should replicate exactly the rest of the project's directory tree.  
It always contains the most recent state of your work, even if it hasn't been saved yet to the original files.  

This folder is created automatically when:
- The editor's window loses focus (can be turned off in settings).
- The editor exits with unsaved changes - either gracefully through the menu accessible via ESC button, or a forceful shortcut like Alt+F4.
- Additionally, every single minute. The interval is configurable.
	- This means that if you forget to save your work for a whole hour, and your computer suddenly BSODs, you lose a minute of your work at most!

### Opening a project

- When trying to open a folder named ``Project``, the editor first checks if a folder named ``Project/autosave`` exists.  
	- If it is found, the opened tab is populated only with the work from the ``Project/autosave`` folder. The tab itself still carries the path of the original folder, so saving the work with a quick ``Ctrl + S`` will still properly update the real project folder. 
	- If it is not found, it reads the files inside the ``Project`` directory instead. This will usually happen if, for example, you download work of some other person - they'd have no reason to also send you the autosave files.

- Given a folder named ``Project`` or ``Project/autosave``, the project looks for the following files inside:
	- ``Project.int`` - (**required**) the binary [intercosm file format](intercosm#file-formats). If not found, an error will occur.
	- ``Project.hist`` - (optional) the history file. If not found, the history will appear empty.
	- ``Project.view`` - (optional) state of editor camera (panning + zoom), entity selections and other stuff like last grid density setting. If not found, these will be reset.

### Saving a project

- **Caution:** Every time you save to a ``Project`` folder, the ``Project/autosave`` folder is considered unnecessary **and is deleted** (if it exists).  
If we'd allow the autosave folder - now completely unrelated - to still exist, then if the application would crash several seconds from now,  
the editor would later load that folder instead of your manually saved files.
	- Thus you are completely safe if you do not manually tinker around ther files from ``Project/autosave``. This is an autosave folder, meant for use only by the editor.

## Modes

- Editing mode - this is the default mode that the editor is put in when it is launched. 
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

### Visibility filters

- Contains a checkbox to toggle visibility of any [render layer](render_layer), [type](entity_type) or [flavour](entity_flavour). 

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

### Viewables

- Contains a summary of all paths

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

In **editing mode**, the camera's position can panned by:

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
 

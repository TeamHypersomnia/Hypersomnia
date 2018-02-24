---
title: Editor setup
tags: [topics, setups] 
hide_sidebar: true
permalink: editor_setup
summary: |
    The **editor setup** is a [setup](setup) that allows [authors](author) to work with [intercosm](intercosm) objects. It can read intercosms from files on the disk and perform various operations on them, like create new entities or record and replay simulations.
---

## Writing and reading files

Currently supported:

- Read intercosms in ``.int`` file format.
- Save intercosms to ``.int`` file format.
- Import intercosms from ``.lua`` table files.
- Export intercosms to ``.lua`` table files.

## Tokens

An entity token is of the form

## Modes
## Autosave behaviour
## Multiple selection of entities

## Editor command

&nbsp;&nbsp;&nbsp;&nbsp;*Main article: [Editor command](editor_command)*

The editor keeps a history of changes, thanks to which an author can **undo** or **redo** their changes as they deem fit.

## Dialogs

### Go to all

Default shortcut: ``ctrl + ,``.
- Goes to any entity
- Goes to any field in the cosmos common significant
- Goes to any viewable

### Go to entity

Default shortcut: ``/``.

#### Future work

- ``Shift + arrows`` let one select multiple entities from that window
- ``Ctrl + enter`` adds to the current selection instead of replacing it

### Context help

Serves as a tutorial for first-time users. It has unique content every time:
- the user is inside an empty editor with no tabs;
- the user is inside a tab with an empty intercosm;
- the user is inside a tab with a filled intercosm;
- the user hovers over the history dialog;
- the user hovers over the context help dialog;
- the user hovers over the property editor dialog.

### History

Shows the history of the last *n* [commands](editor_command).
Can click on each command to go back to a given point in time.

### Property editor

### Type editor

See: [entity flavour](entity_flavour).

When creating a new entity flavour, the editor should add some recommended invariants from the start, e.g.
- interpolation (unless we make its existence implied/recommended?)

## Inferred state

An editor will most likely need some inferred caches, like entities sorted lexicographically.  
We might always facilitate this by adding some inferred caches for [cosmos common], because it is very rarely reinferred.  
As we will also always trigger cosmos's reinference manually when the editor changes some associated state, we might just reinfer along with the cosmos, never needing to hold additional information in the common inferred.  
 

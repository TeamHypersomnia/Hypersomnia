---
title: Editor setup
tags: [topics, setups] 
hide_sidebar: true
permalink: editor_setup
summary: |
    The **editor setup** is a [setup](setup) that allows one to work with [intercosm](intercosm) objects. It can read intercosms from files on the disk and perform various operations on them, like create new entities or record and replay simulations.
---

## Writing and reading files

Currently supported:

- Read intercosms in ``.int`` file format.
- Save intercosms to ``.int`` file format.
- Import intercosms from ``.lua`` table files.
- Export intercosms to ``.lua`` table files.

## Autosave behaviour

## Multiple selection of entities


## Editor command

An editor command is an object representing a single, indivisible operation of the user.  
A command can be **undone** or **redone** (executed).

### Change of a value

There are several classes defined for commands that change a value.
Generally, they should follow this format:

- The kind of object that has changed (the [cosmos common state](cosmos_common_state) or the [component](component) type).
- Given the type, an introspective index of the changed field.
- A vector of bytes representing the new value (for execution).
- A vector of bytes representing the old value (for undoing).

Writing chunks of bytes *to* and *from* the [significant state](cosmos#significant) should be entirely deterministic.  
The question is, do we care what happens with the [inferred state](cosmos#inferred) when executing and undoing commands?

#### Smooth GUI sliders

- How do we determine if a new command entry is to be created? We shouldn't create a new command each frame that the slider is moved by the mouse (IMGUI reports that a change occured even during sliding, not just when it is dragged and dropped)
  - The last command entry should always be overridden if the change has been applied to the same field.
- Will performance be acceptable if we perform the change logic every time the slider is moved?
  - Probably yes.

### Delete one or more entities

This might be unsafe if we 

### Considerations
- There is currently little benefit seen in making history of changes be compatible with future releases.
  - Required lifetime of a history of changes is rather short. 
    - An artist probably won't care about the history after closing the editor.
  <!--- - Managing changes in binary format will be significantly more performant and easier to code. -->
  - In extreme case, we may export history of changes to lua.

<!---
If you are not a programmer and only intend to use the editor to author actual content, you can safely skip this section.
-->-

---
title: Editor command
tags: [topics, editor] 
hide_sidebar: true
permalink: editor_command
summary: |
    An **``editor_command``** is an object representing a single, indivisible operation of the [author](author).  
    It can be **undone** or **redone** (executed).
---

## Considerations

As for determinism of the editor commands, it is debatable if it must be as strict as during networking.
 
Those approaches to command implementation have been considered so far:
1. With each command, store a snapshot of both the cosmos's entire [significant state](cosmos#significant) and [inferred state](cosmos#inferred). Additionally, store only the new value for redoing, as undoing is already possible thanks to the snapshot. 
    - Maximum determinism.
    - Easiest to get right without bugs.
    - Unacceptable memory and processing performance.
2. With each command, store a snapshot of the cosmos's entire [significant](cosmos#significant) state. Additionally, store only the new value for redoing, as undoing is already possible thanks to the snapshot. [Reinfer](reinference) on undo.
    - Slightly less determinism.
        - If the author has jumped once 10 commands back, their further actions and recordings might result in a different cosmos than if they would, for example, just repeat undo ten times.
        - Assuming that we always delete redoable history once a new change is made, this will not be noticeable. 
    - Unacceptable memory and processing performance.

### State consistency


### Notes

- There is currently little benefit seen in making history of changes be compatible with future releases.
  - Required lifetime of a history of changes is rather short. 
    - An artist probably won't care about the history after closing the editor.
  <!--- - Managing changes in binary format will be significantly more performant and easier to code. -->
  - In extreme case, we may export history of changes to lua.

<!---
If you are not a programmer and only intend to use the editor to author actual content, you can safely skip this section.
-->-
## Commmand types

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

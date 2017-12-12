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
2. With each command, store a snapshot of the cosmos's entire [significant](cosmos#significant) state. Additionally, store only the bytes of the new value for redoing, as undoing is already possible thanks to the snapshot. [Reinfer](reinference) on undo.
    - Slightly less determinism.
        - If the author has done undo and then redo, their further actions and recordings might result in a different cosmos than if they would stay on the current change.
        - Assuming that we always delete redoable history once a new change is made, this will not be noticeable. 
    - Unacceptable memory and processing performance.
3. With each command, store the bytes of both the new and the old value. If part of sensitive common state, or if part of a synchronized component, [reinfer](reinference).
    - Even less determinism, but the problem is solvable or tolerable equally well as in 2.
        - If the author has jumped once 10 commands back, their further actions and recordings might result in a different cosmos than if they would, for example, just repeat undo ten times.
        - Solved if both redos and undos are reinferred completely.
    - Memory and processing performance is ok. 
    - If we accidentally forget to reinfer something, some state might be corrupted and even result in a crash.

### State consistency

For some components, it is easy to guarantee that any possible combination of member values results in a valid game behaviour,  
or at least that it does not result in crashing the application.
 
For some components, it is considerably harder.

Care must be taken so that whatever field is exposed to the user: 
- There exists no value that would crash the application.
    - In particular, something must be done about processing lists which assume that a relevant component is always existent within an entity.
        - Theoretically, replacing ``get`` with ``find`` should not considerably impair performance.
        - On the other hand, buggy behaviour might be more hard to spot and debug if we can't make basic assumptions in the code.
        - **It might then be advisable to, once a component is removed or added, make proper changes to the processing lists.**
- There exists no value that, shortly after setting it and playing the cosmos, the game becomes unplayable, or the state becomes completely broken.
    - Efforts can be made, but this is virtually impossible to ensure. In any case, the author can always undo the problematic change.

If there exists a value that fails to satisfy the above criteria, the following approaches can be taken:
- On changing to a problematic value, alter some other state (but it should be state invisible to the author) such that the problem no longer exists.
- The bounds for the value prevent the author from setting a problematic value in the first place.

### Summary:

- What do we store?
    - whole significant + inferred - maximum determinism, no bugs, unacceptable performance
    - only whole significant...
        - ...and incrementally reinfer redos - unacceptable performance, slightly less determinism, some bugs possible
        - ...and completely reinfer redos - even more unacceptable performance, maximum determinism, some bugs possible
        - undos have to be completely reinferred either way
    - only the new value and the old value...
        - ...and incrementally reinfer redos, incrementally reinfer undos - best performance, poor determinism, some bugs possible
        - ...and completely reinfer redos, completely reinfer undos - slightly worse processing performance due to reinferences, maximum determinism, less bugs possible
            - note that complete reinference can still be skipped for components that aren't synchronized
        - we could make a flag out of this; both are pretty much acceptable.
            - **we might begin with complete reinferences, as it will be easier.**
        - perhaps mixing complete and incremental reinference makes no sense, as a "slightly worse" assumption about determinism means no assumption


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

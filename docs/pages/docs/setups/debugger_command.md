---
title: Editor command
tags: [topics, editor] 
hide_sidebar: true
permalink: debugger_command
summary: |
    An **``debugger_command``** is an object representing a single, indivisible operation of the [author](author).  
    It can be **undone** or **redone** (executed).
---

## Considerations

As for determinism of the editor commands, it is debatable if it must be as strict as during networking.
 
Those approaches to command implementation have been considered so far:
1. With each command, store a snapshot of both the solvable's entire [significant state](cosmos_solvable#significant) and [inferred state](cosmos_solvable#inferred).
    - Maximum determinism.
    - Easiest to get right without bugs.
    - Unacceptable memory and processing performance.
2. With each command, store a snapshot of the solvable's entire [significant](cosmos_solvable#significant) state. [Reinfer](reinference) on undo.
    - Slightly less determinism.
        - If the author has done undo and then redo, their further actions and recordings might result in a different cosmos than if they would have stayed on the current change.
    - Unacceptable memory and processing performance.
3. (Chosen approach) **With each command, store the bytes of both the new and the old value. If part of associated common state, or if part of a synchronized component, [reinfer](reinference).**
    - Even less determinism, but the problem is solvable or tolerable equally well as in 2.
        - If the author has jumped once 10 commands back, their further actions and recordings might result in a different cosmos than if they would have just repeated undo ten times.
        - Solved if both redos and undos are reinferred completely.
    - Memory and processing performance is ok. 
    - If we accidentally forget to infer something, some state might be corrupted and even result in a crash.
		- (Chosen approach) For now, since the cosmoi will be relatively small, **reinfer the entire cosmos** upon change to any sensitive field.

Further: commands, whether they are redone or executed for the first time, should do so via the same method: redo.
- This wouldn't be acceptable only if the redo performance was for some reason unacceptable.
- This will greatly reduce code duplication.

### State consistency

&nbsp;&nbsp;&nbsp;&nbsp;*Main article: [State consistency](state#consistency)*

There are more things to consider about state consistency when potentially exposing associated fields to the author.

### Problematic values 

For some components, it is easy to guarantee that any possible combination of member values results in a valid game behaviour,  
or at least that it does not result in crashing the application.
 
For other components, it is considerably harder.

Apart from consistency itself, care must be taken so that whatever field is exposed to the user: 

- There exists no value that would crash the application.
    - In particular, something must be done about getters that assume that relevant state will be present, e.g. get_logic_transform.
        - Theoretically, replacing ``get`` with ``find`` should not considerably impair performance.
        - On the other hand, buggy behaviour might be more hard to spot and debug if we can't make basic assumptions in the code.
- There exists no value that, shortly after setting it and playing the cosmos, the game becomes unplayable, or the state becomes completely broken.
    - Efforts can be made, but this is virtually impossible to ensure. In any case, the author can always undo the problematic change.

If there exists a value that fails to satisfy the above criteria, the following approaches can be taken:

- On changing to a problematic value, alter some other state (but it should be state invisible to the author) such that the problem no longer exists.
- The bounds for the value prevent the author from setting a problematic value in the first place.

### Multiplicity

Commands that alter existing entities in any way may be applied to more than just one entity.
Such command types will always be wrapped into an object that additionally specifies a vector of target entities.

No other considerations are needed at this point.

### Summary:

- What do we store?
    - whole significant + inferred - maximum determinism, no bugs, unacceptable performance
    - only whole significant...
        - ...and incrementally infer redos - unacceptable performance, slightly less determinism, some bugs possible
        - ...and completely reinfer redos - even more unacceptable performance, maximum determinism, some bugs possible
        - undos have to be completely reinferred either way
    - only the new value and the old value...
        - ...and incrementally infer redos, incrementally infer undos - best performance, poor determinism, some bugs possible
        - ...and completely reinfer redos, completely reinfer undos - slightly worse processing performance due to reinferences, maximum determinism, less bugs possible
            - note that complete reinference can still be skipped for components that aren't synchronized
        - we could make a flag out of this; both are pretty much acceptable.
            - **we might begin with complete reinferences, as it will be easier.**
        - perhaps mixing complete and incremental inference makes no sense, as a "slightly worse" assumption about determinism means no assumption


### Notes

- There is currently little benefit seen in making history of changes be compatible with future releases.
  - Required lifetime of a history of changes is rather short. 
    - An artist probably won't care about the history after closing the editor.
		- Managing changes in binary format will be significantly more performant and easier to code.

## Commmand types

### Create an entity

- Stores the [entity guid](entity_guid) to be passed to create an entity with ``create_entity_with_specific_guid``.
- On creation, name the entity as "unnamed" so that the name id is valid henceforth. It can be at any time changed, obviously. Maybe even let the focus switch to a textbox with the name.
- Deletes the entity on undo.

### Delete entity

- Stores the previous byte content of all components.

### Paste entities
- Makes sense since we might want to cut something from testbed for example
- Will need to have whole entity content, just like delete command (just the other way around)

### Duplicate entity

- Stores nothing?

### Change of a value

There are several classes defined for commands that change a value.
Generally, they should follow this format:

- The kind of object that has changed (e.g. the type of the component, the [cosmos common significant](cosmos_common#significant)).
- Given the type, an introspective index of the changed field.
- A vector of bytes representing the new value (for execution).
- A vector of bytes representing the old value (for undoing).

Writing chunks of bytes *to* and *from* the solvable's [significant state](cosmos_solvable#significant) should be entirely deterministic.  
What happens with the [inferred state](cosmos_solvable#inferred) on undos and redos is considered [above](#considerations). 

#### Type granularity

We shouldn't make a command type for a change of every component.
Instead just store the type index. Later do constexprs in a generic lambda if necessary.

Types will be separate for:
- A change inside of a common state.
- A change of a name meta (that container will be particularly large so we shouldn't always store whole container).
- A change of a component.
- A change of a viewable.

#### Smooth GUI sliders

- How do we determine if a new command entry is to be created? We shouldn't create a new command each frame that the slider is moved by the mouse (IMGUI reports that a change occured even during sliding, not just when it is dragged and dropped)
  - The last command entry should always be overridden if the change has been applied to the same field.
- Will performance be acceptable if we perform the change logic every time the slider is moved?
  - Probably yes.


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
1. With each command, store a snapshot of both the solvable's entire [significant state](cosmos_solvable#significant) and [inferred state](cosmos_solvable#inferred). Additionally, store only the new value for redoing, as undoing is already possible thanks to the snapshot. 
    - Maximum determinism.
    - Easiest to get right without bugs.
    - Unacceptable memory and processing performance.
2. With each command, store a snapshot of the solvable's entire [significant](cosmos_solvable#significant) state. Additionally, store only the bytes of the new value for redoing, as undoing is already possible thanks to the snapshot. [Reinfer](reinference) on undo.
    - Slightly less determinism.
        - If the author has done undo and then redo, their further actions and recordings might result in a different cosmos than if they would have stayed on the current change.
    - Unacceptable memory and processing performance.
3. (Chosen approach) **With each command, store the bytes of both the new and the old value. If part of sensitive common state, or if part of a synchronized component, [reinfer](reinference).**
    - Even less determinism, but the problem is solvable or tolerable equally well as in 2.
        - If the author has jumped once 10 commands back, their further actions and recordings might result in a different cosmos than if they would have just repeated undo ten times.
        - Solved if both redos and undos are reinferred completely.
    - Memory and processing performance is ok. 
    - If we accidentally forget to reinfer something, some state might be corrupted and even result in a crash.

Further: commands, whether they are redone or executed for the first time, should do so via the same method: redo.
This wouldn't be acceptable only if the redo performance was for some reason unacceptable.
This will greatly reduce code duplication.

### State consistency

Definitions:

- For state to be considered **consistent**, the name(s) of its field(s) must **tell the truth**.  
- If a field implies truths about other fields, or if truths about a field are implied by another field, such field is called **sensitive**.
	- In particular, if a significant field has an inferred cache, both are considered **sensitive**.
	- Requirement: if all sensitive fields in existence were given their **default values**, the state would be **consistent**.

E.g. if there exists a field named ``items_inside`` with regard to some container,  
then it must not lie about some items having set that container as current, when they have really have null identificator (which means - fact dropped on the ground).  
There are more subtle cases:  
For example, if we make an unwritten contract that if a character currently drives a car, it has half the standard linear damping, then, among others, the following lie may occur:  

- The ``driver::owned_vehicle`` points to a correct car entity, but the inferred state of the character body returns the standard linear damping. In this case, the inferred state is inconsistent, because considering the unwritten contract, it lies.

Theoretically, inconsistent state wouldn't always crash the application, but it is nevertheless a requirement at all times.  

It is fairly easy to ensure **state consistency** during [solve](solver), where, upon the need to change some state, we always have helper methods at hand and can write code so as to update all other dependent state. Heck, we can even have FSMs.  
- Still, solver can change type of an entity, which, without proper attention, could cause all sorts of funny bugs. 

It is considerably harder to keep **state consistency** during content creation stage, where, without proper care, significant state might be altered **arbitrarily**. 
In particular, entities might frequently change types with arbitrary differences in enabled definitions (and thus implied components).  
We might as well later enable the author to freely alter, add or remove components themselves.  

#### Strategies for keeping consistency 

- **Know exactly** when the components can only be accessed through safety of entity_handle, and when they can be altered completely arbitrarily due to having access to the aggregate. 
	- This cannot be stressed enough.
- Provide a safe function that deletes all entity's components while preserving its identity.
	- Just call **remove** on all dynamic components and set fundamental components **to default values**. (except guid)
	- It is supposed to move from one consistent state to the next.
	- Let components have "destructors". 
		- Note that for synchronizers it is not necessarily the same as "destroying caches"
- Determine all sensitive fields in the cosmos solvable.
	- **Hide** all significant sensitive fields (no inferred state will be editable anyway) from the author, so that they only ever influence it through specialized methods that move them from one consistent state through the next.
		- Example: though not at all intuivie at first a first glance, the **item component** (as opposed to the **definition**) should be completely immutable to the author. It should not be able to be deleted or added on demand.
			- Only expose a predictable ``perform_transfer`` function.
		- Although, obviously, in case of physics (velocity and others) an author might just see a slider like any else.
- If a significant sensitive field is only sensitive by virtue of having an inferred cache, reinference always makes that field stop breaking consistency.
	- Although if the inferred cache refers to more entities, e.g. it keeps track of all entities having set a particular value as parent, the inferred cache breaks consistency possibly until all entities are reinferred. 
	- If reinference of all entities is to always restore consistency of significant/inferred pairs, care must be taken that all possible reinference orders restore consistency as well.
- The less sensitive state there is ~~in significant~~ at all (no sensitive state in significant implies no sensitive state in inferred):
	- The easier it is to maintain consistency even in the solver.
	- Less components need special care to hide the sensitive fields from the author (but that is not a big concern)
- The more sensitive state is only sensitive by virtue of having an inferred part...
	- the less helper methods need be introduced for when the author wants to change the value. (as only reinference is enough)
		- Logic becomes simpler.
- Strategies to decrease amount of sensitive state:
	- Remove the sensitive state and calculate it from other insensitive state, just where it is needed.
		- Example: the container component does not store the amount of space it's got left, but rather it is calculated any time it is needed, from the existing items and tree of their current slots.
		- Example: make position_copying be considered in get_logic_transform calculations instead of transform being cached in components::transform (this component should probably not exist)
		- Example: instead of having ``apply_base_offsets_to_crosshair_transforms``, let get_logic_transform perform this logic on its own
		- Make density and damping calculations be always dependent on the current driver ownership or sprint state? Performance might suffer, though.
- Strategies to make state sensitive only by virtue of having an inferred part:
	- Create specialized caches.
	- Move more state from significant onto inferred.
	<!-- - Use synchronizers that make efforts to keep state consistent. -->
- Now:
	- Solvers proceed as usual with due care.
	- If the author makes *any* action that somehow alters the sensitive fields, call the safe deleter and, if components are to be readded, make sure their sensitive fields are always default.
		- In particular, if a type of an entity changes, or if enabled definitions change, call safe deleter and re-add intial component values from the definitions. 
			- In particular, the definitions will be guaranteed to always have sensitive fields at their default values.
			- We don't care that if we change an item's type it will be dropped to the ground. Life's harsh.

Existing problems with state consistency:
- processing::processing_subject_categories needs be hidden and updated when components are removed.
	- move basic categories to inferred state
- if a car is deleted but the driver is not, the driver might be left with dangling properties
	- release driver ownership whenever there is a doubt.
- if an item in a container is deleted... then what?
	- always drop on deleting the component to be on the safe side. 

### Problematic values 

For some components, it is easy to guarantee that any possible combination of member values results in a valid game behaviour,  
or at least that it does not result in crashing the application.
 
For other components, it is considerably harder.

Apart from consistency itself, care must be taken so that whatever field is exposed to the user: 

- There exists no value that would crash the application.
    - In particular, something must be done about processing lists which assume that a relevant component is always existent within an entity.
        - Theoretically, replacing ``get`` with ``find`` should not considerably impair performance.
        - On the other hand, buggy behaviour might be more hard to spot and debug if we can't make basic assumptions in the code.
        - **It might then be advisable to, once a component is removed or added, make proper changes to state, e.g. the processing lists.**
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

### Create an entity

- Stores the [entity guid](entity_guid) to be passed to create an entity with ``create_entity_with_specific_guid``.
- On creation, name the entity as "unnamed" so that the name id is valid henceforth. It can be at any time changed, obviously. Maybe even let the focus switch to a textbox with the name.
- Deletes the entity on undo.

### Delete entity

- Stores the previous byte content of all components.

### Duplicate entity

- Stores nothing?

### Add a component

- Stores the component index.
- Removes the component on undo.
    - This must obviously take proper measures to update processing lists and whatnot.

### Remove a component

- Stores the component index and the byte content.
- Adds the component on undo with the previous byte content.

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


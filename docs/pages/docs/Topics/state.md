---
title: State
tags: [topics, ECS] 
hide_sidebar: true
permalink: state
summary: This article deals with several important topics regarding storage, lifetime, kinds of information and their assumptions or implications.
---

## Categories

Great emphasis is put on defining several categories of state, so that there is little to no doubt when deciding where new features should be implemented.  
TODO: Make a graph.

## Consistency

### Definitions

- For state to be considered **consistent**, the name(s) of its field(s) must **tell the truth**.  
- If a field implies truths about other fields, or if truths about a field are implied by another field, such field is called **sensitive**.
	- In particular, if a significant field has an inferred cache, both are considered **sensitive**.
	- Requirement: if all sensitive fields in existence were given their **default values**, the state would be **consistent**.

E.g. if there exists a ``std::vector`` named ``items_inside`` that tracks which items have set this container as current,  
then it must not lie about those items, e.g. amongst its items there cannot be found any which has a null identificator for a ``current_slot``  
(which means that it is dropped to the ground).  

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
	- Accomplished with introduction of the [cosmic functions](cosmic_function).
- Provide a safe function that deletes all entity's components while preserving its identity.
	- Just call **remove** on all dynamic components and set fundamental components **to default values**. (except guid)
	- It is supposed to move from one **consistent state to the next**.
	- Let components have "destructors". 
		- Note that for synchronizers it is not necessarily the same as "destroying caches"
- Determine all sensitive fields in the cosmos solvable.
	- **Hide** all significant sensitive fields (no inferred state will be editable anyway) from the author, so that they only ever influence it through specialized methods that move them from one consistent state through the next.
		- Example: though not at all intuitive at a first glance, the **item component** (as opposed to the **definition**) should be completely immutable to the author. It should not be able to be deleted or added on demand.
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

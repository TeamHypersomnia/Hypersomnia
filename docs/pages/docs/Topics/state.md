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
- If all **sensitive** fields in existence were given their respective **default values**, the state would be **consistent**.
	- If inference of a sensitive field may possibly yield failure, such default value shall be given by the default constructor.
	- Otherwise if inference cannot yield failure, the default constructor is free to set any value that is just intuitive to have.
		- E.g. the rigid body might have a field defined as ``bool activated = true;`` for simplicity.
	- Therefore, if a field at any time requires an inferred cache, there shall exist "default value" that does not require an inferred cache.
	- A sensitive field with default value cannot lie.
	- A requirement on the programmer, rather than absolute truth.
- A state that at any point is **consistent**, keeps being consistent after complete [reinference](reinference).
	- Additionally, it is assumed that any significant state that is given to the solvable anew (e.g. due to network transfer and later copy-assignment), was previously consistent as well.
		- In practice, this means that any destruction/inference cycle moves the game from one consistent state to the next. 
- T: A state that at any point is **consistent**, can only be made **inconsistent** by:
	- Incorrect alteration of an existing sensitive field that already tells the truth.
		- Prevented by encapsulation of fields and exposing it only to functions that move it from one consistent state to the next.
			- Thus it might only be caused by an erroneous implementation of such functions.
				- In particular, if the state is sensitive by virtue of having an inferred cache, there should exist:
					- A helper function that predicts that the reinference with such altered field won't succeed...
					- ...and the modifier function itself that will notify the client that it would fail and then does nothing at all (does not even destroy the existing cache).
	- Incorrect deletion of an existing sensitive field that is already spoken truth about.
		- Always present components don't suffer from this.
			- In fact they can only suffer from alteration, not from creation or deletion
		- Can only happen with deletion of a component.
			- A component, on removal, shall either destroy the relevant caches or call its own destructor if the sensitive state is inside significant.
	- Creation of a new sensitive field that introduces a lie. 
		- If, for a sensitive field inside a component, the other corresponding sensitive field exists within significant or if reinference of that field may end in failure
			- solver shall only be able to add it with default values of these sensitive fields.
				- then on adding the component, nothing should be done and work should be offloaded to special methods.
					- assign car ownership
					- perform transfer
			- In particular, default values for sensitive fields shall not be provided by entity type...
				- **...let alone mutable by the author.**
			- Therefore, on changing the entity's type, state should remain consistent.
		- If a field is sensitive only by virtue of being inferred, and reinference always succeeds, regardless of the initial value...
			- ...let the entity type specify the initial value.
			- On adding, reinfer.
- Examples of conduct with sensitive fields:
	- At **no point in time** can an entity have destroyed caches or only partially inferred, except as a part of reinference cycle.
	- The only moment that an entity is reinferred completely is when it changes a type.
		- Removing all components should imply complete destruction of inferred state.
			- Note: for the best intuition, there relation between caches and synchronized components shall be 1:1.
				- In particular, the relational cache shall be split into several classes.
					- There is always a component that implies parenthood, and component that implies childhood.
				- In particular, the physics world cache shall be split into several classes. 
					- And the class that holds b2World itself, which the caches share, shall do nothing on inference/destruction of entities
				- Additionally, parent and children trackers may want to be held separate?
					- Why do we need caches for children in the first place?
						- Alternative naming: parent_of_fixtures cache, child_of_body_cache?
							- child_of_body_cache needs not store data, only provide a callback
							- is_parent_set can just iterate through the parent cache while knowing the parent from significant
							- significant will anyways only ever be altered with safe functions
	- Existence of components might be part of sensitive state itself.
		- but only for processing lists.
		- processing::processing_subject_categories needs be hidden and updated when components are removed.
			- move basic categories to inferred state and always reinfer processing lists cache on component existence manipulation 
	- Alteration of some inferred cache might require reinference of other caches.
		- Car might be deleted while still being driven.
			- Car destructor releases driver ownership or just destruction of its cache later reinfers the driver.
		- Container can be deleted while it still holds items
			- the items' current_slot fields will now point to a dead entity, so it always means they are dropped
			- but for this to take effect, all items need to be reinferred before destroying all caches of the container (we'll need its relational cache to know the held items)
			- otherwise we will  
			- if an item in a container is deleted
				- incrementally update the relational cache of the container 
	- Remember that item and container can both be components, but that should not change much in the behaviour.
		- Currently, friction fields can be deleted while still being pointed to by m_ownerFrictionGround.
			- Relational cache should keep track of entities that have such pointer and reset pointers to null properly once the friction field gets deleted.
				- For now we'll just disallow removal? 
				- For now let's just iterate through all bodies and unset the pointers to the deleted body.

E.g. if there exists a ``std::vector`` named ``items_inside`` that tracks which items have set this container as current,  
then it must not lie about those items, e.g. amongst its items there cannot be found any which has a null identificator for a ``current_slot``  
(which means that it is dropped to the ground).  

There are more subtle cases:  
For example, if we make an unwritten contract that if a character currently drives a car, it has half the standard linear damping, then, among others, the following lie may occur:  

- The ``driver::owned_vehicle`` points to a correct car entity, but the inferred state of the character body returns the standard linear damping. In this case, the inferred state is inconsistent, because considering the unwritten contract, it lies.

Theoretically, inconsistent state wouldn't always crash the application, but it is nevertheless a requirement at all times.  

**Most of the time**, it is fairly easy to ensure **state consistency**, since upon the need to change some state, we always have helper methods at hand and can write code so as to update all other dependent state. Heck, we can even have FSMs.  
- Still, [solver](solver) can change type of an entity, which, without proper attention, could cause all sorts of funny bugs. 
	- E.g. the hand fuse logic changes type of the grenade to a released counterpart.
- During content creation stage, without proper care, sensitive significant state might be altered **arbitrarily**
	- In particular, entities might frequently change types with arbitrary differences in enabled definitions (and thus implied components).  
	- We might as well later enable the author to freely alter, add or remove components themselves.  

#### Strategies for keeping consistency 

- Determine all sensitive fields in the cosmos solvable.
	- **Hide** all significant sensitive fields (no inferred state will be editable anyway) from the author, so that they only ever influence it through specialized methods that move them from one consistent state to the next.
		- Example: though not at all intuitive at a first glance, the **item component** (as opposed to the **definition**) should be completely immutable to the author. It should not be able to be deleted or added on demand.
			- Only expose a predictable ``perform_transfer`` function.
		- Although, obviously, in case of physics (velocity and others) an author might just see a slider like any else.
- **Know exactly** what code can directly and arbitrarily write to the sensitive fields.
	- Farily easy now with introduction of the [cosmic functions](cosmic_function).
	- **Know exactly** what entity handle lets us do and when.
- Provide a safe function that deletes all entity's components while preserving its identity.
	- Just call **remove** on all dynamic components and set always_present components **to default values**. **(except guid)**
	- Implies complete destruction of caches for an entity.
	- It is, however, required to move from one **consistent state to the next**.
		- If all sensitive state would only be sensitive by virtue of having an inferred cache...
			-  ...this would be as simple as destroying all caches related to that component.
		- Otherwise...
			- ...let components have "destructors". 
			- We'll probably go with this first for sensitives that will take a lot of effort to move to the sensitive state.
- If a significant sensitive field is only sensitive by virtue of having an inferred cache, reinference always makes that field stop breaking consistency.
	- Except if it is possible for the reinference to not lead to a logical success.
		- E.g. item component might be reinferred with a current slot that is not viable.
		- In this case, there is no other way than only let such sensitive fields be written to by a helper method that can predict the outcome.
	- Although if the inferred cache refers to more entities, e.g. it keeps track of all entities having set a particular value as parent, the inferred cache breaks consistency possibly until all entities are reinferred. 
	- If reinference of all entities is to always restore consistency of significant/inferred pairs, care must be taken that all possible reinference orders restore consistency as well.
- The less sensitive state there is ~~in significant~~ at all (no sensitive state in significant implies no sensitive state in inferred):
	- The easier it is to maintain consistency **even in the solver**.
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
- special physics component holds owner friction grounds, which could be made inferred

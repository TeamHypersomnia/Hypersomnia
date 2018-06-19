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

E.g. if, for a container entity **C**, there exists a ``std::vector<entity_id>`` named ``items_inside`` (that tracks which items have set **C** as their *current container*),  
then all entities referred to by the ids in this vector must have an [item component](item_component) whose ``current_slot`` field points to **C**. If one of those items had, for example, ``current_slot`` set to null identificator, then the ``items_inside`` field would introduce a lie, effectively breaking consistency of state.

There are more subtle cases:  

For example, if we make an unwritten contract that if a character currently drives a car, it has **half its usual linear damping**, then, for example, the following lie may occur:  
- The ``driver::owned_vehicle`` points to a correct car entity, but the ``b2Body`` of the character reports that it has its usual linear damping set. In this case, the ``b2Body`` is inconsistent, because considering the unwritten contract, it lies.

Theoretically, inconsistent state wouldn't always crash the application, but it is nevertheless a requirement at all times.  

- If a field implies truths about other fields, or if truths about a field are implied by another field, such field is called **associated**.
	- In particular, if a significant field has an inferred cache, both are considered **associated**.
- A field that is not associated is called **unassociated**.
- If a field in significant has an inferred counterpart and this inference may fail for some value(s), such **associated** field is called **sensitive**.
	- Sensitive fields are protected:
		- By inferrers throwing [cosmos inconsistent error](cosmos inconsistent error) if the sensitive field(s) lie(s).
		- Similarly as the rest of associated fields, by only being writable by special functions; those will additionally try and catch the inconsistency exception. Upon getting one, they shall revert to the original state and reinfer again.
		- By carefully determining that we do not care if it is inconsistent.
			- E.g. it won't be much of a catastrophy if we allow too much items in a container, e.g. while editing.
- If all **associated** fields in existence were given their respective **default values**, the state would be **consistent**.
	- Default values for **sensitive** fields shall be carefully chosen by the default constructor so that they meet this criterion.
	- For the rest of **associated** fields, the default constructor is free to set any value that is just intuitive to have.
		- E.g. the rigid body might set some sensible linear damping for simplicity.
	- A requirement on the programmer, rather than absolute truth.
- A state that at any point is **consistent**, keeps being consistent after complete [reinference](reinference).
	- Additionally, it is assumed that any significant state that is given to the solvable anew (e.g. due to network transfer and later copy-assignment), was previously consistent as well.
		- In practice, this means that any destruction/inference cycle moves the game from one consistent state to the next. 
- A state that at any point is **consistent**, can only be made **inconsistent** by:
	- Incorrect alteration of an existing associated field that already tells the truth.
		- Prevented by encapsulation of fields and exposing it only to functions that move it from one consistent state to the next.
			- Thus it might only be caused by an erroneous implementation of such functions.
				- In particular, if the state is associated by virtue of having an inferred cache, there should exist:
					- A helper function that predicts that the reinference with such altered field won't succeed...
					- ...and the modifier function itself that will notify the client that it would fail and then does nothing at all (does not even destroy the existing cache).
	- Incorrect deletion of an existing associated field that is already spoken truth about.
		- Can only happen with deletion of an entity.
			- An entity, on deletion, shall either destroy and/or update the relevant caches.
	- Creation of a new associated field that introduces a lie. 
		- If, for a associated field inside a component, the other corresponding associated field exists within significant or if reinference of that field may end in failure
			- solver shall only be able to add it with default values of these associated fields.
				- then on adding the component, nothing should be done and work should be offloaded to special methods.
					- assign car ownership
					- perform transfer
			- In particular, default values for associated fields shall not be provided by entity flavour...
				- **...let alone mutable by the author.**
			- Therefore, on changing the entity's type, state should remain consistent.
		- If a field is associated only by virtue of being inferred, and reinference always succeeds, regardless of the initial value...
			- ...let the entity flavour specify the initial value.
			- On adding, reinfer.

### Requirements

We will list here all possible corner cases and what code is expected to correctly do.

- Car's hand brake may be altered, requiring a change in physical properties of the car.
	- Required by: solver
	- **Expected:** update the damping and/or density of the car without resetting the body.
		- Reinfer rigid body cache of the car incrementally, only resetting what's changed.
- Item can change a slot.
	- Required by: solver
	- **Expected:** this entity, as well as all its children items need to be reinferred recursively with regard to their owner bodies. 
		- Since the complete reinference must be able to rebuild these caches from scratch, it makes sense that we would just overwrite the field, reinfer the dependent caches and catch potential inconsistency exceptions.
			- The GUI will have predictors so that an erroneous transfer will not happen in the first place.
- Item can change a slot requiring the physical b2Body to start functioning.
	- Which means we need to reinfer the rigid body cache.
		- Which means we will need to reinfer caches that depend on b2Body.
- A fixture of an item in item deposit (with deactivated body) gets reinferred.
	- Expected: determines that the body cache is inferred but b2Body does not exist, therefore does not create b2Fixtures; still, ends up considered inferred. 
- Car gets deleted while still being driven.
	- Required by: editor, possibly some logic?
	- **Expected:** Car destructor releases driver ownership or just destruction of its cache later reinfers the driver.
- The body may be deleted while it is still referred to by other entities via custom_rigid_body.
	- Reinfers all fixtures_of_bodies effectively making them standalone bodies.
- Container gets deleted while it still holds items.
	- Needs to reinfer completely fixture and joint caches of children items. 
	- the items' current_slot fields will now point to a dead entity, so it always means they are dropped
	- but for this to take effect, all items need to be reinferred before destroying all caches of the container (we'll need its relational cache to know the held items)
	- if an item in a container is deleted
		- incrementally update the relational cache of the container 
- Currently, friction fields can be deleted while still being pointed to by m_ownerFrictionGround.
	- Required by: editor, possibly some logic?
		- Solution: a separate relational cache keeps track of entities that have such pointer and reset pointers to null properly once the friction field gets deleted.
			- For now we'll just disallow removal? 
			- For now let's just iterate through all bodies and unset the pointers to the deleted body.
- Complete inference can encounter:
	- fixtures first, only later bodies to which they belong.
	- engines first, only later the body car to which they belong.
	- items first, only later containers to which they belong.
	- ~~**Expected:** infer the parent, then infer the children, and don't later infer children.~~
	- **Expected:** NOTHING NEEDS BE DONE EXCEPT FOR PERFORMANCE. Since **inferrers themselves will be designed to always traverse the dependency tree** and infer the root first, the complete reinference should get the information from the inferrer which caches have been already inferred, to improve performance.
- During complete reinference, container can be found to have been assigned less space than it had and can no longer contain items.
	- Required by: editor
	- **Expected:** throw cosmos inconsistent state when, during complete reinference, items of an entity are found to occupy too much space. 

#### Strategies for keeping consistency 

- Provide a separate function that determines dependencies of caches? 

- While caches can be made to only depend on significant, dependencies of caches might need other caches to be calculated.

- Observation: If a cache ascribed to entity **E** depends on exclusively significant fields of one or more entities, among them can be found only **E** or parents of **E**.
	- That is because we never store children information in the significant state.
	- Even if intuitively an id is not a "parent" it could be made conceptually so
- If calculation of a cache starts depending on some children tracker, then caches that depend on a certain significant field could be found wherever.
- T: if an associated significant information (or identity) is a dependency of a cache, then all caches that depend on it are found:
	- either in this entity,
	- or in one or more entities in children vector tracker(s) in the inferred state.
	- Example: slot physical behaviour is dependency of children attachments only, but not the parent inventory nodes

- Determine all associated fields in the cosmos solvable.
	- This state, not even sensitive fields, needs not be hidden from author because inferrers are required to throw cosmos inconsistent error and the editor is supposed to catch it.
		- Otherwise we would need to hide container's capacity as well, which is unreasonable.
- **Know exactly** what code can directly and arbitrarily write to the associated fields.
	- Farily easy now with introduction of the [cosmic functions](cosmic_function).
	- **Know exactly** what entity handle lets us do and when.
- If a significant associated field is only associated by virtue of having an inferred cache, reinference always makes that field stop breaking consistency.
	- Except if it is possible for the reinference to not lead to a logical success.
		- E.g. item component might be reinferred with a current slot that is not viable.
		- In this case, there is no other way than only let such associated fields be written to by a helper method that can predict the outcome.
	- Although if the inferred cache refers to more entities, e.g. it keeps track of all entities having set a particular value as parent, the inferred cache breaks consistency possibly until all entities are reinferred. 
	- If reinference of all entities is to always restore consistency of significant/inferred pairs, care must be taken that all possible reinference orders restore consistency as well.
- The less associated state there is ~~in significant~~ at all (no associated state in significant implies no associated state in inferred):
	- The easier it is to maintain consistency **even in the solver**.
	- Less components need special care to hide the associated fields from the author (but that is not a big concern)
- The more associated state is only associated by virtue of having an inferred part...
	- the less helper methods need be introduced for when the author wants to change the value. (as only reinference is enough)
		- Logic becomes simpler.
- Strategies to decrease amount of associated state:
	- Remove the associated state and calculate it from other unassociated state, just where it is needed.
		- Example: the container component does not store the amount of space it's got left, but rather it is calculated any time it is needed, from the existing items and tree of their current slots.
		- Example: make position_copying be considered in get_logic_transform calculations instead of transform being cached in transformr (this component should probably not exist)
		- Example: instead of having ``apply_base_offsets_to_crosshair_transforms``, let get_logic_transform perform this logic on its own
		- Make density and damping calculations be always dependent on the current driver ownership or sprint state? Performance might suffer, though.
- Strategies to make state associated only by virtue of having an inferred part:
	- Create specialized caches.
	- Move more state from significant onto inferred.
	<!-- - Use synchronizers that make efforts to keep state consistent. -->
- Now:
	- Solvers proceed as usual with due care.
	- If the author makes *any* action that somehow alters the associated fields, call the safe deleter and, if components are to be readded, make sure their associated fields are always default.
		- In particular, if a type of an entity changes, or if enabled invariants change, call safe deleter and re-add intial component values from the invariants. 
			- In particular, the invariants will be guaranteed to always have associated fields at their default values.
			- We don't care that if we change an item's type it will be dropped to the ground. Life's harsh.

Existing problems with state consistency:
- special physics component holds owner friction grounds, which could be made inferred

## Getter ecosystem

- Retrieving values that may or may not exist at the moment.
	- Examples
		- Logic transforms.
			- Only finder.
			- Since an item in a container takes transform of its container, the transform will almost always exist.
			- Might not exist due to not having necessary components.
				- Some entities representing processes.
				- An item in a container that does not have a transform on its own? 
					- Possible but unlikely.
		- AABBs.
			- Existence similar to that of logic transform.
			- Only finder.
		- Colliders connections.
			- May very well stop existing at arbitrary times. We must thus be defensive here.
			- Finder and calculator.
		- b2Body-gettable properties.
			- Only found after finding cache.
			- TestPoint.
	- This is a case of defensive programming?
	- Ultimately, we should probably move away from any kind of "get" even if it serves brevity.
		- Theoretically, we can sometimes expect that the entity will ALWAYS have this logic transform or other value
			- e.g. a character
		- But we can never be sure.
			- Unless we prove at compile time that an entity will have a transform.
		- And if the value does not exist, it means a crash.
				- One could argue that functions like this could always be implemented without a need for an existing cache
	- Operation types:
		- Finds actual value in the cache - fast, works only when constructed - always?
			- Preffix: find_
			- If cache exists, return the value from the cache.
				- The thing is, we might at some time decide that it is only upon destruction of cache that we write back to the significant.
					- Because of possible bottleneck in e.g. step and set new transforms.
				- Otherwise	recalculate it.
			- If it becomes too slow, we can make the synchronizer cache the cache pointer.
			- const-valued caches should also be gettable for special kinds of processors.
		- Obsolete: assumes that the value will be found through find and dereferences it right away for brevity
			- Preffix: get_
			- We should stop using it right away.
		- Operations that both do find and calculate at some point.
			- Named "find", because finders anyway calculate if they do not find the existing cached value.
		- Calculates the value to be passed to cache - slow, works always
			- Preffix: calc_
			- Some calculators might provide overloads taking a needed invariant/component ref
				- So that the inferrers can pass the one that they've already got
		- Requests for a certain field to be recalculated - e.g. we know that a driver will only need a correction to damping and not entire body
			- ONLY IN CASE OF EXTREME BOTTLENECK INSIDE INFERRERS.
			- Notice that the same can be accomplished by just incrementally inferring the whole related domain.
			- We thus reduce code duplication.

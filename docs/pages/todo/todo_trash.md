---
title: ToDo trash
hide_sidebar: true
permalink: todo_trash
summary: Just a hidden scratchpad.
---

	- ~~thus let on_add and on_remove be called when adding or removing components.~~
	- ~~proposed solution: if the only moment that we add or remove components is when creating an entity and when changing its type...~~
		- ~~we could ditch partial reinferences and implement only whole-entity destruction and inference~~
			- not quite, because we will still need partial reinference
			- if we do partial reinference we can as well enable it when components are added or removed

		- ~~as for dependencies, specify them in components/component synchronizers~~
		- adders and removers should always explicitly uphold the consistency of state
<!--
		- entity handle calls on_remove/on_add cycle on all dependencies after calling the subject's on_add or on_remove. Examples:
			- removing/adding an item calls on_remove/on_add for rigid body component
				- since item ownership could change
				- custom body owner is a stronger body overrider than a container
			- removing/adding a custom body owner calls on_remove/on_add for rigid body component
			- removing/adding item  
		- items_of_slots -> fixtures_of_bodies -> physics_world_cache
--!>
- replace "reinfer all caches of" calls in entity handle with something else
	- existence of components should imply these things
		- basic processing categories
		- existence of correspondent definitions
			- in particular, it should be unsafe to access definitions if component has not been checked beforehand 
		- existence of caches
			- as for always_present components which need caches...
				- ...the cache should be inferred right with creation of the entity,
				- and destroyed only upon its death.
					- or at otherwise special circumstances?
					- changing the type is pretty much special
				- makes sense for guid and type components.


	- problem: if container definition was enabled, container behaviour will be present even if we destroy all components, thus breaking our assumptions that destroying all caches via removing all components.
		- provide a container component with activated flag?
			- no activation flag. Just an empty component.
			- even if there is no circumstantial data that we need now, it may change in the future
		- provide per-entity cache destroyer that takes definitions into account?
		- make type dynamic but statically allocated and let it handle that?
	- especially since a component removed in one entity may trigger change of caches for other entities, dependencies will not be as easy to determine as specifying some types
		- thus each callback should do it on their own. Examples:
			- remover of container shall reinfer all its children items. 
			- remover of rigid body shall reinfer all its children fixtures and joints. 
			- remover of custom rigid body owner shall reinfer its body cache. its children fixtures. 
	- offload work to cache classes where it is reasonable
		- doesnt make sense for children vector trackers as they are in class of their own
	- pass inferred access keys to adders and removers
	- handle calls it either if the member function is present or if component is synchronized
		- so required in the latter case

<!--
Commented out because we've ditched the concept of activationness
	- Therefore, if a field at any time requires an inferred cache, there shall exist "default value" that does not require an inferred cache.
	- A sensitive field with default value cannot lie.
-->

	~~- If cache is constructed, **it is guaranteed to be consistent**, therefore return.~~
		- **Since we're going to allow incremental inferences of cache, it should always check if the cache is consistent and incrementally update it.**
			- And if it does not exist at all, probably use a fast function.
		- That is because if it is ever incrementally updated, it is done controllably.
		- When we know that this cache is no longer consistent due to being dependent on other cache, we always destroy it before calling inference.


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
<!-- 
		- If cache depends on data from a single instance of a component, it shall trivially belong to one entity
-->
<!--
			- We *might*, for example, say that the children caches of one sort are dependent on other kind of cache of entity that they specify as their parent via some significant field.
				- Because their pointing to this entity as their parent implies some truths about caches.
-->
#### Notes

Note that now on_add and on_remove become obsolete because helper methods can decide on their own what should be reinferred.

- T: ~~changing an entity's type or thereof content and reinfering the whole cosmos right away shall never break consistency~~
	- WRONG: less space could've been specified for a container definition.
	- this is because associated fields will not be settable for definition's initial values; the associated fields will then be set to **defaults** which will always lead to reinference success
	- and we may require the solver to uphold the rule of never changing the type of any entity
		- theoretically we could let the solver change the type but it would be one more place to screw things up
		- it makes little practical difference for the author, actually none except that identity is broken but identity issues should be handled by the solver
			- basically the only place where we need it is the fuse?
- Therefore, we don't have to worry about changed types
	- However remember that complete reinference should infer identity and that which is inferred by the type; so it should do somewhat the same thing that explicit creation and adding components would do
- an entity's identity may also have a cache
	- all possible mutables/fluxing input to the simulation shall be determined at definition and creation stage
	- let an entity's type imply existence of some caches
	- solver shall not mutate component existence
- note that author can also mutate variables of definitions which would theoretically need reinference
	- currently we say that we reinfer the whole cosmos so not a problem and associateds will be private 
- existence of components is directly implied by definitions
	- if anything is implied by existence of components, it is then implied by definitions
	- dependencies
		- any function that changes associated state is on its own responsible for upholding consistency
- Make damping, density and owner bodies always possible to be calculated from the significant state.
	- damping/density values are only default data in rigid body
	- custom body owner is just an override
- Calculate activated field on the go, like damping and density. An author has completely no reason to specify a deactivated body from the start, and activationness should otherwise be calculated from the circumstances, e.g. from belonging to an item deposit.  
- separate "relational cache" into several classes
	- introduce relational_cache_mixin that provides reservation callback, set_relation and and stores the tracker.
	- concerns, are, I guess, separate: mapping of list of children is separate from mapping of body pointers
	- rigid body on_remove calls delete potential parents from joints of bodies and fixtures of bodies
	- container component on remove calls delete potential parent from items of slots
- determine EXACTLY dependencies between caches (or just for now make them in proper order and reinfer all on adding a associated component)
	- always refer to a proper visual graph of state dependencies
	- e.g. fixtures of bodies would depend on items of slots
	- each cache defines ``using depends_on = type_list<items_of_bodies, ...>`` etc.
		- on inference or destruction, iterate through all caches and reinfer them if they depend on the currently modified one  
- use augs::introspect to reserve entities on all caches and later optionally reinfer all caches 
- get_owner_body should return value from the inferred cache as technically an entity should always be inferred
	- the inferred cache should provide actual calculation from circumstances
- separate a "custom_rigid_body_owner" component so that caches/components have 1:1 ratio
	- so that fixtures cache does not need to construct both the relational and fixtural caches
- make operator= for synchronized components verify if there is a need of complete reinference, as just a single field could have changed

Assumptions that are the most relaxing:
- The author can only alter the first step of the simulation. 
	- Then they can as well be forbidden from possibly changing the type as we could step back and create entity with a different type in the first place. 
	- The editor data then becomes just a recipe for creating the simulation.
	- Then we can assume that the type of the entity stays the same throughout its lifetime.
		- What does it give?


- ~~associated significant with a counterpart in significant~~
	- should not exist, except in extreme circumstances where inference is so expensive, or if the code is not yet refactored
- associated significant with a counterpart in inferred...
	- ...whose any value in the significant ends in successful reinference.
	- ...for whom exist some values in the significant whose inference may fail
		- called **sensitive**.

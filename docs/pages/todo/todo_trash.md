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
- get_owner_of_colliders should return value from the inferred cache as technically an entity should always be inferred
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


#### Consistency upholders

These are the only functions that ever actually touch the associated (and thus sensitive) fields when the state is already consistent.  
They are callable from the cosmic functions and it is their requirement to uphold consistency of state.  
- ~~Change type of an entity~~. We will forbid this for simplicity. Later we might just facilitate this in the editor somehow.
	- It would be dangerous to change type of an existing entity without invalidating its identity.
		- That is because, if it was a container, and it changed to a container with less space, and its identity cache is left intact (e.g. its parenthood cache wasn't destroyed), then the state becomes inconsistent.
	- Destroys all caches of the existent entity, ~~except identity cache (e.g. does not destroy parenthood)~~ (poses a risk of breaking consistency).
<!--- T: Creating, cloning or destroying an entity does never break consistency.
- T: Changing an entity's type may possibly breaking
-->

- Examples of conduct with associated fields:
	- At **no point in time** can an entity have destroyed caches or only partially inferred, except as a part of reinference cycle.
	- Existence of components might be part of associated state itself.
		- but only for processing categories.
<!--
	- **Hide** all significant associated fields (no inferred state will be editable anyway) from the author, so that they only ever influence it through specialized methods that move them from one consistent state to the next.
		- Example: though not at all intuitive at a first glance, the **item component** (as opposed to the **definition**) should be completely immutable to the author. It should not be able to be deleted or added on demand.
			- Only expose a predictable ``perform_transfer`` function.
		- Although, obviously, in case of physics (velocity and others) an author might just see a slider like any else.
-->
- Provide a safe function that deletes all entity's components while preserving its identity.
	- Just call **remove** on all dynamic components and set always_present components **to default values**. **(except guid)**
	- Implies complete destruction of caches for an entity.
	- It is, however, required to move from one **consistent state to the next**.
		- If all associated state would only be associated by virtue of having an inferred cache...
			-  ...this would be as simple as destroying all caches related to that component.
		- Otherwise...
			- ...let components have "destructors". 
			- We'll probably go with this first for associateds that will take a lot of effort to move to the associated state.

- ~~with the current architecture, get_fixture_entities will only be needed in resolve_density_of_associated_fixtures; as dependencies will be well managed during inferences, the rigid body inferrer won't need to query the children~~
	- ~~thus we will be able to do away with fixtures_of_bodies cache?~~
	- Notice that when an item is hidden in deposit, then even though the body cache might be considered inferred, the b2Body will not exist for performance. Then when the current slot for this hidden item changes, it will need to iterate the children fixtures to reinfer them too as their caches are order-dependent on the body.

#### Detail cache functions

Domains in direct need of creating cache:
- Creating entity (whether by a clone, a delta or a genuinely new entity)
	- Should always easily succeed, problem starts with incremental alteration
- Complete reinference of the cosmos
- Methods for incremental alteration?
	- What is the largest alteration possible at a time?

Domains in direct need of destruction of some cache:
- Deleting entity
- Methods for incremental alteration?
- ~~Complete reinference of the cosmos~~
	- Complete reinference will just use class destructors.

- **Domains where inference and destruction of caches could happen arbitrarily:**
	- Complete reinference of the cosmos
	- Methods that change sensitive state while upholding consistency may need to arbitrarily reinfer some caches.
	- **This is especially important implication when dealing with parent/child relationships.**

- Component synchronizer's method that alters sensitive state and updates the caches to uphold consistency.
	- Calls responsible inferrer that will update incrementally.
	- If the associated field is sensitive
		- Catch [cosmos inconsistent error](cosmos_inconsistent_error) in case of failure.
			- Now the cache is destroyed.
			- In this case, restore old value and reinfer again with guarantee of success.

- What if complete reinference could be implemented in terms of entity creations/deletions?
	- Corner cases could theoretically be mitigated by enforcing order of construction?	

## ENTITY TYPE TRASH

<!--
- Entity can change the type during its lifetime, in which case it should be completely reinferred.
	- That will be helpful when we want to make a change to entity (that is impossible by just changing the components) while preserving correctness of all identificators that point to it.
	- Example: a grenade that changes from a normal body to a bullet body on being thrown (its sprite might change as well).
		- Most ``child_entity_id`` types will anyway be replaced with a entity_type_id. For other cases, it makes little sense to also enforce deletion of those entities just because they are meant to be related.
-->

<!--
		- if a definition implies more than one component, perhaps they should be merged?
		- That ensures that each component has a corresponding definition.
		- missile will imply both missile and sender?
			- we'll just add the sender component where necessary and where it wasn't yet added.
			- the sender, child components will anyway be hidden from the author as they are detail.~~
-->


<!--
- ~~Entity should be reinferred if it changes a type (during content creation).~~ Type id for an entity should stay constant until its death.
	- If at all, changing the type should be implemented as an editor feature. Let us not make this an actual feature in the game code and let us always assume that an entity's type stays constant throughout its lifetime.
	- It will be easier in the beginning if we disallow this and require the author to just create a new entity.
-->
<!--
### Components considerations
An author should not be concerned with adding components to a new entity, that properly correspond to what they've already set in the definitions.  
Regardless of the fact that the definitions are statically allocated (and thus always "present"), the author should be able to specify which definitions they are interested in.  

Additionally, the author might want to specify initial values for the component that is added, that corresponds to the given definition type.
For example, some physical bodies might want to have some particular initial velocity set.  

However, the logic should, for the sake of performance and simplicity of code, always assume that a correspondent definition is present, to avoid noise.  
Thus the logic should derive usage of definitions from the presence of components.  
However, the presence of components will be initially derived from the presence of definitions.
It will thus be always required to define at least an empty type that specifies the correspondent component type,  
event if the component type itself does not need any definition-like data.

A concern could be raised because with this design, performance of serializing types could suffer.  
That is because, if we serialize linearly without regard for which definitions are set, we serialize possibly a lot of unneeded data.  
On the other hand, if we do serialize only that which was set, we may suffer from instruction cache misses as the code will not degrade to a simple memcpy anymore.

However, the type information:
	- Will not at all be that large;
	- Will not at all be copied around frequently (pretty much never, just the cosmos) 
	- Should have the topmost access performance, which would suffer from facilitating the structure for better serialization performance.

-->

<!--
### Performance

Some concern could be falsely raised to the fact that an entity's type id first need to be obtained, and only later the type information;  
as, compared to just a single component lookup, this could potentially be slower.

However, at the time of having an entity handle, the type id is already available, possibly in cache.  
The only thing left is to perform the lookup in the type information map.
Then, the only thing to compare performance-wise is a lookup in an ``unordered_map`` vs a [``pool``](pool).
If the container was optimized for deterministic identificators, it could possibly be faster;
Note that types won't be altered during logic, usually only during the creation of content.  
So, they could be stored in a fixed-sized container. **There can even be a vast performance improvement**. 
-->

<!--
		- if a definition implies more than one component, perhaps they should be merged?
		- That ensures that each component has a corresponding definition.
		- missile will imply both missile and sender?
			- we'll just add the sender component where necessary and where it wasn't yet added.
			- the sender, child components will anyway be hidden from the author as they are detail.~~
-->


<!--
	- Mutable or not, by the solver.
		- Preferably not, but it might be useful when partitioning polygons into destructed pieces.
	- They would take precedence over what would possibly be found in the correspondent definitions in the type.
	- When we will need to copy the game world, how do we determine if a shape is owned by an override or a pre-inferred definition? 
		- we need to take proper measures so as to never copy the shapes that are indeed flyweights; as opposed to the custom shapes.
	- The problem will not be frequent besides names and polygons, so it is a viable solution.
	- **Solution**: for each such case, create a dynamic component that, on its own, will not consume memory when not used, but can be named for example "custom polygon shape"
		- additionally, the dynamic component might have different assumptions about storage, since it will not be so common; it could even have plain ``std::vector``s instead of fixed-size containers.
			- although I would not advocate this.
		- Least memory wasted.
		- Best performance as the override checks will be limited to several specific domains.
		- Medium-hard conceptually.
	## ENTITY TYPE TRAS		- Additionally, it will be pretty much known from the get-go which data is possibly altered.
			- Nobody will also be surprised if the solver needs to alter that state, because they are plain components.
		- Good flexibility, until something unexpected needs to change.
	- **Solution**: create overrides for definitions and let an entity have its own set of definitions, if so is required.
		- Most memory wasted.
			- An entity with an override costs the size of all the definitions, unless we make each definition dynamically allocated, hindering overridden-entities performance all the more.
			- Additionally, as the type is the same, we must assume the same storage rules as with the definitions. In particular, we won't be able to use a dynamically allocated vector of vertices if the definition object already uses a constant size vector.
		- Worst performance as upon getting any definition we check for an existing override.
			- The only impact is on non-overridden entities.
			- Overidden entities would otherwise still need to get a component.
			- The impact might not be so great for the non-overriden entities as we might make the definiton id to already reside in cache once the type id is in cache.
		- Hardest conceptually.
			- Is it predictable when modifying any definition field? Some state might need to be updated to be consistent.
			- For example, rigid body parameters like type (static or dynamic, if it is a bullet etc.).
				- If we anyway allow for changing a type, then reinference should be enough.
		- Most flexibility in altering the game's behaviour.
	- The memory waste calculations for these two solutions actually assume that there will be a lot of overridden entities. 
		- If there are just a few, than having a dynamic component id for each possible override actually costs more in terms of aggregate size.
		- If there are many, then yes, per-entity overrides are worse. 
		- The actual serialized size should stay pretty much the same.
-->

<!--
### Simple per-field overrides

Consider this: usually, we specify damping values once for a given type of physical entity.  
However, the logic itself may want to alter the damping value due to some special circumstances (being inert, riding a motorcycle, sprinting, etc.).
Two approaches can be taken:

- ``std::optional<field_type> overridden_field;``
- ``field_type field_scalar;``

We'll probably use scalars most of the time.

COMMENTED OUT: Such things will be calculated statelessly.
-->


<!-- **We should disregard tailoring the assignment operator until we get to networking, where we'll probably switch to another physics engine with the features we need.** -->

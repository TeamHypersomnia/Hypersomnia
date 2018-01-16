---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Contents


### Microplanned implementation order:  

- Now definitionize renders for entities created in testbed

- Implement entity types to some degree.
	- That is to not repeat the code when we'll be writing improvements to inferences 
	- Order:
		- Implement passively the architecture.
		- Begin by, for example, definitionizing a gun component, so slowly making it use the definitions.
			- firing engine sound and muzzle particles are "persistent child entities"
				- thus they should become group entities.
			- magic missile def is just a child entity to be cloned around as needed.
				- thus it only becomes type id.
		- At some point, remove the definition data from the component.
		- Build and test.
		- Repeat with all components.
	- Get rid of component adders and deleters.
- Remove owner_body from fixtures component and create a component named "custom rigid body owner" that overrides anything else
- Current slot should stay in the item component as other fields of item will anyway be a private
- **moving disabled processing lists out of the significant state**
	- for now ditch the setter of rotation copying when switching driver ownership
	- most stateless, thence easiest is when we just easily determine when not to process an entity without relying on having proper fields set in processing lists
		- we should then only ever optimize when it is needed
	- or it can be part of inferred state which will complicate things a little
- force joint components should be "custom" in a sense that they can be added and they do not override anything else
	- there should be one cache per joint type, e.g. attachment joint cache which is inferred from current inventory state 

#### Needing confirmation:

- parenthood caches design:  
Current design with relational caches is correct.  
Parenthood requested by current values in signi should be tracked even if it is determined that, for a while, the actual functionality of the parent should be disabled (e.g. rigid body hidden in a backpack).  
Parenthood can be invalidated only with death of the parent itself, as even though only correct parents can be set in the first place,  
it is irrelevant for the relational cache whether the parents pointed to by ids do indeed fulfill parenthood conditions.  
Since possibility of parenthood is implied at definition stage, it makes sense to destroy that cache with death of the entity.  
	- Other than a memory optimization, we don't at all need to remove caches for a parent once it gets destroyed.  
Once children get destroyed, the cache will be freed automatically.
Additionally, the children will anyway be destroyed.
When we do "get_children_of" in a relational mixin, we can ensure that the entity is alive.
Memory is somewhat safe because it can only grow as far as the children grow.
		- Concern could be raised becasue that would mean that, after reinference, that cache would be drastically different. However, we make no guarantee of 0% reinference error. Functionally, the parent cache with dead parent id is equal to no cache. The code, however, will be simpler.
			- We will save that correction for later though.

#### Cosmic functions that **always move from one consistent state to the next**:

- Create an entity with a specific type.
	- Without side effects, allocates and adds data needed by the simulation to store intermediate results (aka components).
		- All sensitive fields of components are set to default, therefore reinference must always succeed.
	- Inference pass
		- Infers caches for lifetime-immutable data.
			- The type id cache.
			- The processing category cache, if determined to be needed separately from processing lists cache. (component set will be immutable)
			- Initializes any identity-related cache.
				- At this time, only the parenthood cache (but this is initialization is only theoretical. In practice nothing is done in this regard until the first child appears.)
		- Calls all other dynamic inferrers.
- Create a group with a specific type.
	- Allocates identificators to all entities in the group.
	- Unpacks the group by creating components with correspondent identificators.
		- Fields whose inference can fail are guaranteed to be set to defaults.
	- Performs inference passes on one entity after the other. 
- Delete an entity.
	- First deletes recursively all entities that specify "existential_child" component that points to the entity being now deleted. 
		- Gets that information from a existential_children_cache.
	- Searches for all existent caches for this entity, ~~without help of current significant state~~, and destroys those existent caches.
		- Actually, since associated state will be controllably modified, **it makes sense to use information from current significant**.
			- E.g. for type cache, as it is a map from type id to vector of entities, we don't immediately know where in the cache the relevant entry to be destroyed is.
		- Clears reference to this entity in inferred caches.
		- Destroys lifetime-immutable caches.
			- Identity caches.
			- The type id cache.
- Clone an entity.
	- First clones recursively all entities that specify "existential_child" component that points to the entity being now cloned. 
	- Creates an entity of the same type.
- Get a component whose fields are all unassociated and write directly to it.
- Get a component and do an operation on an associated, insensitive field through a method.
	- The method is responsible for upholding the consistency of state.
- Get a component and do an operation on a sensitive field through a method.
	- The method is responsible for upholding the consistency of state.
- Set a new common significant state and reinfer the whole cosmos.
	- WARNING: Reinference of some existent entities may fail (e.g. less space was specified for a container).
		- Throw on reinference error? Then the editor could catch it and revert the change.
		- May throw [cosmos inconsistent error](cosmos_inconsistent_error).
- Set a new solvable siginficant state and reinfer the cosmos solvable.
	- WARNING: Reinference of some existent entities may fail (e.g. more items were specified for a container).
		- May throw [cosmos inconsistent error](cosmos_inconsistent_error).
- T: Except inside the methods responsible for creating, destroying and cloning entities, at no point in time is any existing entity *partially inferred*.
- T: Except inside the methods responsible for creating, destroying and cloning entities, changing the entity's type and methods that deal with complicated reading (e.g. cosmic delta), any existing entity is at all times *fully inferred*.

#### Editor operations with regard to cosmos that move from one consistent state to the next:

- Write directly to an unassociated field in the cosmos common/solvable significant.
- Write directly to an associated field in the cosmos common/solvable significant. 
	- Reinfer the whole cosmos.
- Write directly to a sensitive field in the cosmos common/solvable significant.
	- WARNING: We should hide as much sensitive state from the author as is possible.
		- Despite space available in the containers being basically sensitive state, 
	- Reinfer the whole cosmos.
	- Catch cosmos inconsistent state exception and revert if it occurred.

- Inferrer of cache.
	- If the cache exists, update it incrementally.
	- If the cache does not exist, quickly update it fully.
	- For domains that don't need much performance, incremental update might always do the same thing as the full update.
	- If we provide incremental update logic, it might as well be used by the full update logic, without much of loss.
		- Be wary though of comparing to default values.
	- Dependent caches
		- Caches can depend on calculated information that **does not physically exist in significant state**.
			- Inference of some cache depends on an already inferred value of some other cache(s).
				- Happens when: 
					- The inference of cache depends on mere existence of the other cache (a cache that is order-dependent).
						- An address of a cache is an inferred value of cache that changes on each reinference, despite the significant input being the same.
						- A fixture is dependent on the body's address in memory so that it might be internally assigned a correct pointer. 
							- However, it is determined entirely from the significant upon which body-entity it is dependent, if at all dependent.
					- Calculation of some information from significant is very, very costly; thus it is inferred once, stored once and later used by some other inferences.
						- In practice we don't have such a case. (probably?)
				- In particular, the caches-dependencies might not necessarily belong to the same entity.
					- In some cases, determining the entities that have caches-dependencies is not necessarily a trivial task.
						- In particular, the part of logic in the item transfer that upholds consistency of state, might still be a complex one.
			- Inference of some cache depends on a value that is calculated at the time of inference, from one or more significant fields (possibly from different domains)
				- The fields **always belong to the one entity that owns the relevant components**. 
					- Architecturally identical to the cache that depends on just one existent field in significant.
					- The only difference is in the final expression that writes to the cache and the fact that all significant fields become associated state and thus only accessible by helper methods.
				- The fields **might dynamically belong to one or more different entities**.
					- ~~In such case, care would need to be taken so that when the associated significant state is erased, the cache that by chance has read from it during inference is now reinferred.~~
					- The *dynamics* might only come from the fields that define a parent-like entity. In such case, we establish another children vector tracker in inferred state. 
					- Or we do some handwriting? The caches will anyway not be assigned to components, much less to the parent-like id fields.
		- Simplest case: inference of cache does not depend upon any other cache and inference of no caches depend on this cache.
			- Type cache. 
			- Tree of NPO cache.
			- Processing lists cache.
		- The hardest case is probably the fixtures cache which depends both on an existent cache (body cache) as well as some diffused significant state (for determining body ownership).
		- The fact alone that we track children of a specific entity does not mean that any caches of the children are dependent on any cache of the parent. 
			- E.g. existential children.
	- If possibility of being a parent is implied by the type (though we might specifically check for component's existence)
		- Detail: if called on entity creation, since only viable parents can be set in the first place, there is no way that with creating this entity some children could have started pointing to this entity.
		- Possibility: There might be no parent cache due to no children being yet assigned.		
		- Possibility: There might be a parent cache with ids of existing children that point to this parent.
			- At that point, the children might be inferred or not.
				- A child might be intuitively "inactive"; Even though a child might already be inferred, its 
	- Calls all other inferrers that depend on it.
		- Using declarations with type lists might still come in handy for this.
- Destroyer of cache.
	- Only destroys the single cache from that single domain that belongs to this particular entity.
	- Since on reinference, inference usually follows after destruction, it is the job of the inference to notify the dependent caches of arisen change.
- Destructor of cache.
	- As opposed to destroyer, this will also notify all caches dependent on this cache to 

#### Later
- replace child_entity_id with entity_id and let child component be synchronized
- child component should be always_present + synchronized
- strip children vector tracker of children caches as we'll take that data from signi
	- was anyway used only for ensuring
- remove all_inferred_state as it is essentially pointless once we have types implemented
- fix car component to calculate damping statelessly, it will need to be synchronized
	- in practice car won't ever be an item or a driver or have movement... 
- implement instances of cooldowns for casts statelessly when there is such a need
- describe concept: quick caches
	- stored directly in the aggregate
	- will be used to store copies of definitions for super quick access
		- e.g. render
	- will be used for super quick inferred caches
		- e.g. if we ever need a transform or sprite cache if they become too costly to calculate (either computationally or because of fetches)
	- will be serialized in binary as well to improve memcpy speed 
		- will need manual rebuild of that data
	- but will be omitted from lua serialization to decrease script size


### Plans

current slot will also have index in container
drivers dont need one as there's always one
solver should never let this state be correupted and it will never be visible to author

order here will be important, in particular, caches should also be ordered well so that:
circumstantial_fixtures_owner_cache
circumstantial_damping_cache
circumstantial_density_cache
are before, for example
physics_world_cache
which puts everything together.

what about AI?
- let pathfinding be a circumstantial component?
- we don't care much about that AI alters some state because virtually only ai will use that pathfinding

shouldnt clone entity use a safe entity handle interface?

Currently, cloning performance of cosmos solvable with static allocation may suffer due to the fact that constant size vector uses default array's operator=, (as component aggregate is trivially copyable).
This means that all slots for entities will get cloned, not just allocated ones.
On the other hand, it is good that some components use default operator= so that all can be cloned in a single pass.

**We will just need to create an operator=(cosmos_solvable_significant&) for the solvable signi that takes note of that and clones the pools manually.**

move "game/common_state" files to a more proper location

WHEN ORDER OF ITEMS IN THE CONTAINER BECOMES RELEVANT, 
This procedure should be fixed or otherwise the reinference might break the order of items!

fixtures can form scene graph as they have relative transforms.
	position copyings could be calculated statelessly in get_logic_transform, and only changed when performance so requires
transform logic might need refactoring for statelessness?

components should be templatized by id
	lets us define groups in definitions 
	synchronizers will anyway work with entity_id specialization thus not much more will be templatized
	introspectors could static assert against usage of entity ids in non-template components


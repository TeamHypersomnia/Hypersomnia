---
title: ToDo performance
hide_sidebar: true
permalink: todo_perf
summary: Just a hidden scratchpad.
---

- Optimize fish neighborhood calculation

- Let render layers be tied to native entity types so that we don't have to dispatch each time
	- Actually all visible entities could just be array of vectors per native type

- Implement std::vector<byte_no_init>
	- Some casts will need fixing then

- Implement constrained handles and constrained entity ids
	- Actually the generic handle could just take the required components.
		- The entity handle and const entity handle would be typedefs for handles without constraints.
	- because inventory slot handle item getters should return handles that guarantee presence of an item
	- thanks to that we can avoid problems with having many entity types and enlarging the dispatch code

- Performance of flavour ids
	- right now they are just regular pool ids
	- they are actually quite performance critical
	- would id relinking pool actually be useful here?
		- the for each id structure should be quite easy here
			- literally just common and common/solvable signis
	- they can be sparse though
		- since we care about performance the flavours will be anyway statically allocated
		- and allocation/deallocation speed won't be that important here
			- could still iterate over all ids and serialize only existing
		- we can always easily check if the flavour exists, so no relinking needed?
		- actually relinking still needed if after removing a flavour we allocate a new one

- sparse_pool implementation that avoids indirection?
	- can have IDENTICAL interface as the pool
		- even the pooled object ids can stay the same really
			- just that the indirection index will actually be used as a real index
	- existence of versioning determines whether we need to perform for_eaches
	- versioning could still be there so we can easily undo/redo without for eaches
		- we can let those several bytes slide
	- **we should always be wary of pessimistic cases of memory usage, anyway**
	- for now we can use pools for everything and incrementally introduce sparse_pool
	- once we have sparse_pool, the loaded caches and images in atlas can just be sparse pools as well?
		- though the effect is ultimately the same and it's more container agnostic

- neon map generation could perhaps be parallelized
	- but it's not really necessary for now

- audiovisual/inferred caches and reservation
	- if it so happens that std::unordered_map is too slow, we can always introduce constant-sized vectors/maps under STATICALLY_ALLOCATE_ENTITIES
		- each type will specify how many to statically allocate 
		- we can also make caches only for the types that fulfill given conditions of invariants/components existence

- destroy_all_caches -> clear_all_caches and use clears

- separate rigid body and static rigid body
	- so that the static rigid body does not store velocities and dampings needlessly
	- and so that we only add interpolation component for dynamic rigid bodies
- it is quite possible that step_and_set_new_transforms might become a bottleneck. In this case, it would be beneficial to:
	- allow for destroy_cache_of to accept entity_handle. 
		- Then, upon reinference of entity, significant state will be written with the new data.
			- care must be taken so that reinference happens BEFORE network transfer of any sort.
		- **Then, upon complete destruction of the cosmos, caches will call their "clear" method that will also reuse any memory.**
			- It is better than calling class destructor because later inferences will be quicker.
	- transform, sweep and velocity fields shall be immutable by the solver, only when:
		- the author wants to displace an object after which shall come complete reinference
			- ~~first, complete deinference~~
			- the author sets new value
			- then complete reinference
		- destroy cache wants to update sweep transform and velocities for consistency 
	- otherwise settransform and others will be called directly

- describe concept: quick caches
	- stored directly in the aggregate
	- will be used to store copies of invariants for super quick access
		- e.g. render
	- will be used for super quick inferred caches
		- e.g. if we ever need a transform or sprite cache if they become too costly to calculate (either computationally or because of fetches)
	- will be serialized in binary as well to improve memcpy speed 
		- will need manual rebuild of that data
	- but will be omitted from lua serialization to decrease script size


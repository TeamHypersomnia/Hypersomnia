---
title: ToDo performance
hide_sidebar: true
permalink: todo_perf
summary: Just a hidden scratchpad.
---

- neon map generation could perhaps be parallelized
	- but it's not really necessary for now

- audiovisual/inferred caches and reservation
	- if it so happens that std::unordered_map is too slow, we can always introduce constant-sized vectors/maps under STATICALLY_ALLOCATE_ENTITIES
		- each type will specify how many to statically allocate 
		- we can also make caches only for the types that fulfill given conditions of invariants/components existence

- destroy_all_caches -> clear_all_caches and use clears

- arrange components in such an order that the fundamentals go first, and frequently used go next to each other
	- so that component ids are cached more frequently
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
- for better cache coherency, we might, inside systematic functions, iterate components and keep track in some cache to which entities they belong
	- so that we always have a perfect coherency at least along a single component 
- describe concept: quick caches
	- stored directly in the aggregate
	- will be used to store copies of invariants for super quick access
		- e.g. render
	- will be used for super quick inferred caches
		- e.g. if we ever need a transform or sprite cache if they become too costly to calculate (either computationally or because of fetches)
	- will be serialized in binary as well to improve memcpy speed 
		- will need manual rebuild of that data
	- but will be omitted from lua serialization to decrease script size


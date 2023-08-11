---
title: ToDo performance
hide_sidebar: true
permalink: todo_perf
summary: Just a hidden scratchpad.
---

- Cull/fade sound sources that are obviously too far away
	- we need to fade them if they're playing so they delete
		- actually just stop them abruptly since they'll be inaudible at that point anyway
	- this is about more than just performance because sounds are simply missed

- Fix the problem with massive performance hit when something lands between walls
	- Could this be a problem with posting collision sounds?
		- shouldn't be as there are cooldowns
	- Probably substepping



- Move these to cosmos global solvable:
	real32 teleport_progress = 0.0f;
	real32 teleport_progress_falloff_speed = 0.0f;
	signi_entity_id inside_portal;
	vec2 saved_velocity;
	real32 saved_angular_velocity = 0.f;
- Only hold a float for modified alpha inside rigid bodies

- TIL Box2D uses ToI by default against static bodies.
	- Maybe we could let mapper disable it for improved performance?
	- Only glitch I could think of is grenades flying over thin walls
		- but flying objects should have bullet flag set anyway
		- And they actually do in calc_needs_analytic_physics! So it's not a problem.
			- as for players, they already clip sometimes anyway lol
			- so the mapper would ensure there are no too-thin static walls which is anyway a problem with the clipping/shoot-through glitch

- Use callgrind to see what happens when the physics slows down due to hard calculations
	- e.g. when character is stuck inside two colliders
	- can e.g. spawn it in the red room on cyberaqua for a test

- Debug the performance difference between native de_cyberaqua and the new de_cyberaqua
	- around 1200-1400 fps on CT spawn on the old one, 800-1000 on the new one

- Optimizing post-culling so Ad hoc atlas only holds resized down images (except the one previewed in some color/shape picker)
	- ad_hoc_in_atlas_map can have get_unscaled_size instead of tarnishing the augs::atlas_entry
		- But wait, what would we really use this method for?
		- Just for resetting in sprite resource inspector? Really? That'd be overengineering


- Rebuild scene quickly becomes the bottleneck even in release
	- Let's profile it

- OpenAL begins to miss sound effects when there is a lot of ambient sounds in the background
	- even if they are completely far away!

- In debug mode, You could trigger a drastic performance hit in physics
	-  By simply switching weapon from nothing to zamiec for example
	- Only works if there are two neighboring static bodies instead of a single continuous one, and something is put inside the notch between them
		- happened with shells too
	- for now let's disable the interior collider, better to just teleport inside the aquarium rather than hit the 
		- we could also have an invisible, soundless player-only collider
- This will eventually be fixed by having an additional fov-aligned player-only collider for static bodies that stretches to infinity
- We could anyway diagnose what's causing the performance hit in physics because it's a security concern
	- not so much security but someone could still do some griefing
	- esp since it can also happen to shells, so this might still apply after we do the fov feature

- Cosmos should really hold a reference to flavors instead of a copy so that we can have e.g. two cosmoi without copying the same data and so that the second correctly points to the first one

- Why does the performance drop dramatically in debug after spawning just a few firearms?
	- Doesn't happen with normal entities too
	- Maybe check with physical objects too?
	- Possibly because of physical reinference, it slows a bit with other physical objects
		- I think contacts might be getting resolved at that stage too
			- Maybe this is because weapons with attachments suffer all the more

- Area markers and point markers should have their own tonpo (tree of non-physical object)
	- for now the bruteforce will work
- search for TODO: hits in editor sources later because some important temporary fixes were applied that are not necessarily optimal
	- e.g. bugfix to point/area marker icons being unhoverable when zoomed out. This is a temporary measure
 
- maybe dont load editor icons for actual gameplay?
	- but for now leave it at that, easier to playtest too


- Legacy editor slowed down when looking at a grid far away from the center

- Profile interpolation rewrite

- Looks like we have drastically less FPS with character on screen compared to no character
	- Would it be only sound logic?
- Determine how could we possibly have over 1000 fps on linux in the past
	- Really just movement paths?
	- More debug details?
	- Why does having a character on screen decrease the fps so much?
		- Test it by going to spectator

- Perhaps conditionally remove the logs for deletions in relwithdebinfo?
- Perhaps conditionally remove the checking if the inventory slots are valid?

- only serialize the server setp once and multicast the same buffer to all clients!
	- we can use global server's allocator
	- set the reference count to num connected clients + 1
		- so it never gets released by any other allocator than the one we chose - global allocator
	- when the refcount is 1, manually free memory

- Performance problems on Windows...
	- Rendering script takes too much time, it randomly takes long
		- allocations? but where are we allocating during render?
		- dashed lines?
			- plausible since each single segment invokes a make_sprite_triangles so possibly sines and cosines

- Looks like the operator== for viewables paths in load_all shows up in profiler as well.
	- It would be good if the setup simply set "dirty" flag on all viewables that need reloading, 
	  instead of dummily checking each time if the input has changed.
		- this is even more flexible.
	- The sounds could be incrementally loaded/unloaded based on paths
	- But images anyway have to be atlased

- Idea: to improve physics readback performance,
  we could somehow only update non-sleeping bodies, whilst acquiring this information directly from box2d?

- Destroy b2Bodies when they are not needed, e.g. they are in inventory
	- Mandatory for MMO

- Sound should be loaded from the smallest to the biggest
	- So that effects are loaded first
	- New synchronization
		- store std::atomic<bool> next to sound_buffer in loaded_sounds_map
			- set it to false whenever definition changes or it is to be loaded
			- check if future is implemented same way

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


---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order:  


- Audiovisual caches should always check if the transform exist because sometimes the transform might be lost even due to having been put into a backpack.
	- especially the interpolation system.
		- **it should crash on transfer to backpack when we correct the transfers **
	- just use find transform
	- use clear dead entities as well because it minimizes sampling errors if e.g. the solve wouldn't run between steps
		- though I think we always run audiovisual advance after step

- There was a crash due to dead entity in driver setup finding a component
	- possible cause is that the gun system destroys item in chamber, but technically they should not trigger END_CONTACT
	as they are devoid of... hey, actually that might be because we still see those entities even if they are in backpack. 
	In that case, if the owner body of an item chamber is badly determined, it might make sense that it crashes
	- ensure that the posted subject in contact listener is alive, but that is most likely the case since we get physical components from them anyway
		- but ensures won't hurt
- Local setup should record session live
	- This is really important in the face of bugs.
	- Or just get rid of test scene setup for now and let it by default launch a scene in editor that records inputs

- Later, bullet trace sounds should as be calculated statelessly
- Sound system should probably assume the same strategy as an inferred cache:

- PROBLEM: entity-chasing sound/particle streams might start chasing wrong entities if a cosmos gets reassigned from prediction correction!
	- Streams will either be very, very short in which case it won't matter (and it would suck to replay a sfx or move it in time randomly)
	- Or they will be continuous in which case they will be statelessly calculated.
		- Car engines and decorations.

- Discriminate sound/particle effect types and treat them accordingly. Might turn out that some caches might need to be copied, some not.
	- Think of explosions and how unpredicted explosions shall be handled, because it is important for the future.
		- Just played from the beginning, I believe.
	- Actually, no caches will need to be copied around.
	- Very short streams of sound or particles (e.g. sound on damage, but also its particle stream effect!) should be treated exactly like fire-and-forgets, despite the name.
	- E.g. it might make sense to make continuous stream of messages for continuous streams or enablable streams, because it is important for logic to catch up on them quickly.
	- Continuous streams in case of sound system might be music, for example.
		- Car engine and gun engine sounds as well!
	- Continuous streams might even be handled completely without messages, but statelessly.
	- So don't worry, we'll reuse what was written and add new things later for continuous objects. 
	- You've actually written about those stateless things...

<!--
	- On prediction correction, we could:
		- disable all streams?
		- if each cosmos timeline held its copy of audiovisual caches... we could reassign along it.
		- In any case, logic is presumed to catch up quickly with sending messages for e.g. continuous streams 
			- because a cosmos might be at any time read from disk in which case it will have no audiovisual state

- Fire and forget streams should never last long.
- For continuous streams (e.g. decorations), we could make it so the logic is required to send a message once in a while so that the invalid caches "burn out" and the valid caches quickly start
	- Best if it only happens on startup and we keep copies of audiovisual caches for each cosmos and don't worry that they're not synchronized

- Particles are easy, can we pause all sounds if we want to keep a copy of audiovisual state for later revival?
	- Might anyway be implemented later.
-->

- Messages for particles and sounds are the best compromise because this state is then not synchronized and server might just opt out of processing messages itself with some compilation flag.
	- It's easier for audiovisual state not to iterate through all entities.
	- Startup catching up will not be that much of an issue
	- We can always do some "circumstantial" magic when overwriting cosmos signi because we have original input/start values saved
		- wouldn't anyways be perfect with particle existences, though at least you have times of births there
		- but storing audiovisual caches will provide the same flexibility

- Message intervals vs message every step
	- Message every step amortizes side-effects and would theoretically remove the need to store and reassign visual caches
		- as every time we can rebuild from messages
		- pro: don't have to come up with a sensible interval
			- no interval at all, actually, just persist caches through entire step and clear it on next post solve
		- pro: caches quickly catch up on any change
		- con: many messages posted 
			- but server can anyway opt out
		- con: has to clear audiovisual caches every time and prepare it anew? 
			- but ONLY for continuous streams
			- or just compare message contents to some last one so that it finds a suitable resource and determine whether some of it should be continued
	- Message per some interval
		- con: has to come up with nice interval for every effect 	
			- important only on startup
			- perhaps a stateless method that determines initial state of continuous streams?
				- decorations would have their own existential component with particle effect id
				- thus they would be somewhat similar to particle existences
		- con: has to cache audiovisual state for every timeline to avoid glitches

- sentience component should have integrated crosshair
	- provide a get crosshair transform function

- when neither crosshair, nor shared particle/sound objects, nor car engines at all need to be tracked as children, we can basically remove child component
	- except we can't, because we need crosshair's recoil body

- position copying becomes the only way to change the entity's logic transform beside the rigid body and the wandering pixels
	- thus the only one with the problem of properly updating NPO 
		- We don't need NPO to track particle existences for now as there really won't be that many. 200 static ones would be so, so much already.
	- we can make it so that static particle existences don't even have the position copying component
- the only current use: particles and sound existences that chase after the entity
	- **solution**: 
	store a relevant field in each component that potentially needs it, 
	statelessly calculate transform when needed in particles simulation system,
	**statelessly determine whether particles should be activated (due to car engine or others) in particles simulation system** (that info could be stored in child component of chased entity)
	(not just activationness, the amount itself could be calculated)
		- pro: **quality of statelessness**
			- simplicity of code
			- less state transferred through the network
		- pro: particles and sounds can specify different chasings in the same entity
		- con: minimal duplication of code
		- con: worst performance as many WITH_PARTICLES_EXISTENCE need to be iterated through
			- can be optimized later via synchronizers and caches, without breaking content
			- as it's audiovisual mostly, this kind of thing can even be multithreaded
			- **and it moves load off the server**
			- if we're really furious, we can even make one thread continuously regenerate an npo from which then a renderer reads every frame
				- otherwise we would anyway need to update npo tree every frame which would be too big of a load for server
			- it can even be faster to iterate all particles existences while being cache-coherent.
	- additionally, expose a "get_particles_transform" to disambiguate get_logic_transform - the latter should not check for particles existence.
		- because a fixtural entity might additionally have a particles existence that chases upon itself.
			- e.g. truck engine

- Thoughts about native types
	- We can make the entity_type's populators in ingredients just templated functions
		- For the sake of not repeating test scene flavour logic for some different native types
	- We will totally get rid of processing component and calculate statelessly which that which needs to be calculated.
		- We anyway very rarely ever disabled something in processing subjects and we must always account for the worst case.
	- Refer to typed_entity_handle for draft.
	- Rename "entity_type" to "entity_flavour" and use "entity type" to represent the natively assembled aggregate of invariants and its counterpart
		- entity_flavour_type vs entity_type 
	- Since the architecture will be corrected to the point where the solvers won't add any component,
	we could actually introduce native entity types.
		- dynamic_obstacle = sprite + render + rigid body + fixtures 
	- The authors should not be concerned with customizing invariant sets but with choosing native, well-understood presets.
	- This could be even introduced incrementally, e.g. entity handle would still perform type erasure to not compile everything in headers.
		- For now the authors may also see just presets without being able to add or remove invariants
		- Specialized handles would be returned by for_each for processing_subjects in the cosmos
		
	- Storage
		- Array of structs vs struct of arrays
			- Storage will be transparent to the logic, even if we don't introduce native types.
				- typed_entity_handle<character> will have perhaps cosmos& and character* 
				- general entity handle will have more than now, as it will have to perform type erasure. It will have type id and void* that will be reinterpret-cast. 
			- Changing ingredients to native types won't require more work than just adding a type specifier to get test scene type. 
				- meta.set will still apply as always
				- enums will also apply because many entity types might share the same native type
			- So we don't have to do it now.
			- We will specify storage for native types in tuples, thus we will be able to change SoA to AoS and back with just one compilation flag. 
	
- Rename "fixtures" to "colliders"

- Instead of having "force joint" at all, make it so that while processing the cars, they additionally apply forces to drivers to keep them
- Make it clear which functions get cache content and which actually calculate from the significant
	- Three? kinds of operations:
		- Gets actual value in the cache - fast, works only when constructed - always?
			- **should be a member function of the cache for fast access.**
				- then the logic can just get the cache once.
			- const-valued caches should be gettable.
			- some funny special logic will probably use it to do some additional physics calculations?
			- will probably be nice to pass around the invariant ref to avoid repeated getting
				- or just do so once we determine the bottleneck
		- Calculates the value to be passed to cache - slow, works always
		- Requests for a certain field to be recalculated
			- We know that a driver will only need a correction to damping and not entire body
	- Notice that there is no point in having fixtures component

- Implement entity types incrementally.
	- We do it now to not repeat the code when we'll be writing improvements to inferences 
		- gun component 
			- firing engine sound and muzzle particles are "persistent child entities"
				- thus they should become group entities.
					- That will be only possible once we invariantize practically everything.
			- magic missile def is just a child entity to be cloned around as needed.
				- thus it only becomes type id.
		- catridge component
			- whole component should become invariants and its child entity id fields should just become type ids
	- Get rid of component adders and deleters.
		- That will be only possible once we invariantize practically everything in the test scenes.
		- We'll thus need to make sender and others "always present".
- Current slot should stay in the item component as other fields of item will anyway be a private
- **moving disabled processing lists out of the significant state**
	- for now ditch the setter of rotation copying when switching driver ownership
	- most stateless, thence easiest is when we just easily determine when not to process an entity without relying on having proper fields set in processing lists
		- we should then only ever optimize when it is needed
	- or it can be part of inferred state which will complicate things a little
- force joint components should be "custom" in a sense that they can be added and they do not override anything else
	- there should be one cache per joint type, e.g. attachment joint cache which is inferred from current inventory state 
- Describe two kinds of state: constant-divergent and exponentially-divergent
	- tree of NPO, sprites, polygons, renders: constant divergence
	- sentience, fixtures, rigid body: exponential divergence

## Later
- replace child_entity_id with entity_id and let child component be synchronized
- child component should be always_present + synchronized
- strip children vector tracker of children caches as we'll take that data from signi
	- was anyway used only for ensuring
- remove all_inferred_state as it is essentially pointless once we have types implemented
- fix car component to calculate damping statelessly, it will need to be synchronized
	- in practice car won't ever be an item or a driver or have movement... 
- implement instances of cooldowns for casts statelessly when there is such a need
- calculate trace component statelessly
	- fix interpolation issue or just customize it when editor is ready
		- should really be done once we have that facility in editor because debugging will be a LOT easier when we have the internals to tweak and inspect
	- should be fast enough
	- otherwise make it a super quick cache?
	- chosen_lengthening_duration_ms should be randomized statelessly with help of guid
	- store just stepped timestamp of when the trace was fired instead of incrementing the passed time 
- fix the feel of traces (maybe shrink them only horizontally?)
	- **only do this after we have editor**, obviously


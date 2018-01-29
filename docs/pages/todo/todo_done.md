
- Solutions for "children entities": entities that are cloned with some other entity and deleted with it
	- Stateless calculation of a parent entity and storing results in a parenthood cache
		- Makes some other potentially unrelated state now associated
		- From what do we calculate this?
	- **Chosen solution:** delete_with component that is a synchronized component
		- Most flexibility and separation of concerns; childhood is not really something that will change rapidly, as opposed to damping or density
		- Groups could then easily specify initial values for entities in the group
			- will just be a common practice to set the delete_with to the root member of the group
				- Currently would anyway be only used for crosshair
	- crosshair has id to its parent that is used on deletion
		- we'd still need to cache this value and make more state associated

### Microplanned implementation order (done):  

- Should rigid body synchronizer check whether the cache is constructed or always assume that it is?
	- **We might later implement it so that a body might be absent under certain circumstances to save memory.**
		- A single check does not hurt so we'll leave it as it is.

- rename fundamental to always_present
	- so that it is intuitive that there are no callbacks for adding and removing

Reiteration #1:

- Getting information about success from reinference
Some configurations of solvable significant do not allow for an inferred counterpart that is consistent.  
E.g. if vector of items inside a container is inferred, and one would like to perform a transfer by 
Here's where helper functions might be necessary. A perform_transfer function should only do as much as predict the outcome of reinferring the item component with newly set current slot,
and if it is successful, do set it and reinfer the item.

we consider whole type overrides too complex architeciturally:
- the assumptions about inferred state for entity flavours are gone; previously it was just generated once and set in stone, always correct
- noise while getting those invariants
- we should discourage solver from ever modifying that state
	- otherwise to look good we would need to always completely destroy that entity and recreate it with components with default associated values

- fix trace interpolation problems (possibly others) in release
	- observation: debug works
		- divergence is consistent after clearing the build folder
	- numerical problem perhaps?
		- "looks like" the interpolation cache may be getting reset too often
	- fixed: a temporary for the version was invalidated

- Currently, logic transform might be ambiguous if:
	- The entity has a rigid body and also fixtures that belong to some remote rigid body.
		- We will always prefer fixtures if they exist.

- If we want to destroy b2Body data once all fixtures are destroyed, it would be a case without precedent where the state of the parent cache depends on some of its children.
	- Let's just allow bodies to stay without fixtures for now as it will be easier. We'll see what happens.
	- Thus the former owner body won't have to be reinferred once a remote fixture entity gets destroyed.
	- In any case, we could just implement it in the physics engine side that the body gets removed from the world list if the last fixture was removed.

- Get logic transform will always acquire values using caches for speed.
	- **If it doesnt destroy performance, we can check if cache exists and fall back to value in the component.**
	- But such a thing would anyway probably be needed only in constructor?
		- not if we allow b2body to be destroyed
	- For now let's just leave it as it is so that we hit assert when we acquire logic transform at a suspicious time

- Invariantizing rigid body
	- velocity and transform should always be able to be set directly as they are temporary parameters of the simulation, not statelessly calculated on any state.
		- better wording: these variables are accumulators

- Remove owner_body from fixtures component and create a component named "custom rigid body owner" that overrides anything else

- change path of #include "game/messages/message.h"

- ditch stream displacement for a while
- remove completely the particles existence and sound existence component
	- logic will post relevant messages to create bursts
		- fire and forget bursts:
			- no owner assigned, just starting world transform
			- separate vector in particles simulation system
		- stream can be created in the name of an entity/offset
			- don't use purpose, just differentiate by orbital transforms
			- car will be able to stop engine's stream by looking up its orbital transform
				- bytewise comparisons should work after just assigning			
		- in which case the stream will be reset to the new value instead of creating additional stream
			- removes the need to hold child entity ids in sentience	
		- burst can be stopped in the name of an entity
			- used by the car
		- continuous streams could be queried statelessly by the simulation system

- should find_logic_transform return nullopt if entity is dead?
	- perhaps, for simplicity. we don't gain much by ensuring there

- Implement entity flavours incrementally.
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

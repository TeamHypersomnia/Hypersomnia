A particle effect is an unpathed asset.
It can be explicitly created or duplicated.

## Emissions 

A particle effect is composed of one or more "emissions".
This introduces variety - we can spawn multiple completely different particle emissions 
for a single event for which the particle effect was chosen.

For example, the item pickup effect has an emission that produces sparkles,
and an emission that produces smoke.
Both have completely different particle lifetimes, sizes and speeds,
yet they are spawned and processed at the same time.

You should not create too many emissions for a single particle effect,
as particles can quickly get quite costly to compute.

## Creating a new particle effect

It is highly recommended to duplicate an existing particle effect
that most closely resembles your intended end result and simply tweak its parameters until you are satisfied.
In the same way, you can also duplicate the particle definitions already existing in duplicated effects.

**Note:** The GUI for particle definitions is not yet perfect as it mixes input to the calculations 
with the mutable state like positions or velocities.
There's also a need to specify sizes on your own instead of setting them to image dimensions.
Therefore, instead of creating your own effect from scratch, you should stick with duplication for now.

When you duplicate or create a particle effect by pressing "Create" button, 
it starts as being Orphaned, since it is not yet used anywhere.

When you are done setting up the parameters for your emission,
you'll have to add one or more particle definitions in the emission's Particle definitions->All.

Particles are split into several distinct types depending on their capabilities.
The parameters that can be specified greatly differ among types.
This is to improve the processing performance.
Currently, we have three types:
- The general_particle that is arguably the least capable.
- The animated_particle that displays an animation instead of a static sprite.
- The homing_animated_particle that, apart from displaying an animation, also seeks the target 
  that was specified by the game logic upon spawning the effect.

Whenever a particle emission determines it is the time to spawn a particle,
it will pick a random particle definition among all definitions of a given type.
It will do this once per EACH particle type for whom definitions exist.
	Therefore, if an emission has definitions for three types of particles, 
	it will emit 3 times as many particles as an emission that only has definitions of one type.

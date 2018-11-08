The flavour and entity hierarchies let you tweak innumerable properties 
of the objects residing within the game world.

This window has the most potential for customization.

## Theory: Flavour vs Entity

Summarizing from http://wiki.hypersomnia.xyz/entity_flavour

So that working with content is easier, and so that less memory is wasted,  
each game object is divided into two kinds of data:

1. Flavour - state that should never change and is supposed to be shared between entities.
   For example: the muzzle velocity and fire rate of a gun.
2. Entity - state whose instance is needed by EACH and EVERY discernible object, since it is constantly in flux.
   For example: the object's position or a flag signifying whether a gun's trigger is pressed, or the volume of its heat engine.

In short, a flavour contains information that is stored ONCE and is later shared by one or more entities,
whereas an entity is an instance of some flavour.

A game "object" is an entity and its flavour seen as a whole.

As a mapper, you will only be concerned with flavours 99% of the time.
Viewing entity properties is mostly useful for in-game debugging.

## Theory: Invariants vs Components

Invariants are constituent elements of a flavour.
Components are constituent elements of an entity.
That is the only difference.
Invariants and components of the corresponding types will always exist at the same time in a game object,
e.g. a sentience component will always be present alongside a sentience invariant.

## Theory: Entity type

The memory layout of all entities and flavours is hardcoded, and thus, immutable.  
It means that there are only several possible combinations of invariants and components,
and you can't manually add, remove or disable components/invariants at will.  

This is for several reasons:
- Performance. These assumptions let us employ some massive hacks to improve cache coherency,
  e.g. we already iterate all entities linearly, 
  but we could even make each component type reside in its own dedicated array.
- It is less error-prone. It would be very hard to ensure that each possible combination
  of invariants and components - the number of which grows exponentially - does not crash the game 
  or make it otherwise unplayable.
  
  We don't see much benefit in letting mappers create "camera objects that shoot bullets",
  or "spawn point markers that can also explode", because we're more pragmatic than idealistic.

  That doesn't mean that you can't introduce another type like this, as a C++ developer!
- It was way, way simpler and faster to code.

An entity type is, for example, a "Controlled character".
It is used to create playable characters. 
It will have components like a sentience, a rigid body, a fixture (which is another name for a "collider") 
and a crosshair.

Some kind of a wall, on the other hand, will have a "Plain sprited body" type.
It will have a rigid body and a collider, but no crosshair or sentience.

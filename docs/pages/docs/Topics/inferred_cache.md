---
title: Inferred cache
tags: [topics, ECS] 
hide_sidebar: true
permalink: inferred_cache
summary: An **inferred cache** is a portion of state that only ever exists at [run time](https://en.wikipedia.org/wiki/Run_time_(program_lifecycle_phase)) *and* can always be completely generated from some [significant state](significant_state).
---

## Etymology

The term is composed of two words, each of which describes an important property of the state that it refers to:
- *inferred* means that it can always be calculated based on the current contents of significant state;
- *cache* means that its destruction does not imply loss of data in any way meaningful to the application user; which, in some way, is already implied by the capability of being *inferred*.

Caches can be:
- [reinferred](reinference);
- inferred fully from their initial state;
- inferred incrementally from their existing state.

## Purpose

An inferred cache is a classical example of a [time–memory trade-off](https://en.wikipedia.org/wiki/Space%E2%80%93time_tradeoff).
If our computers had infinite processing speed, existence of inferred caches would be pretty much pointless.

### Example

Consider how you would manage names for your game objects.
Say that the state for your game looks like this:

```cpp
struct my_entity {
	std::string name;
	int my_super_cool_health;
};

std::vector<my_entity> my_entities;

````

Let's also assume that two or more entities may have the same names.  
At some point, you might want to have a function that lets you get all entities having a particular name. So:

```cpp
std::vector<my_entity*> get_entities_by_name(const std::string& searched_name);
````

And how do you implement that function?
Obviously, you could just iterate through the entire ``my_entities`` and populate the output vector with entries that match:  

```cpp
std::vector<my_entity*> get_entities_by_name(const std::string& searched_name) {
	std::vector<my_entity*> result;

	for (auto& e : my_entities) {
		if (e.name == searched_name) {
			result.push_back(&e);
		}
	}

	return result;
}
````

This approach scales rather poorly if you are going to frequently call the ``get_entities_by_name`` and you have lots of entities.  

What you are more likely to do is to introduce a helper map:

```cpp
std::unordered_map<std::string, std::vector<my_entity*>> entities_by_name;
````

**You would keep ``entities_by_name`` up to date** each time an entity gets deleted, created, or if name of some entity changes.  
Then, the definition for ``get_entities_by_name`` would pretty much boil down to:

```cpp
std::vector<my_entity*> get_entities_by_name(const std::string& searched_name) {
	return entities_by_name.at(name);
}
````

Notice that, as long as you have ``my_entities`` intact, you can always safely destroy ``entities_by_name`` and recalculate its contents again, just iterating through all entities and recreating the relevant entries.

Now, here is where our distinction comes in.

- ``my_entities`` is a part of *significant state*, because this is the state that you write to disk, read from disk and synchronize through the network. If you lost that data, it would have some rather *significant* consequences.
- ``entities_by_name`` is an *inferred cache*. As its state can be *inferred* from the significant state, it only makes sense for that state to exist at runtime, and it makes no sense to serialize it or send through the network, as each client can infer it from the significant state that is assumed to have been already synchronized.

## Reinference error

Assume that an inferred cache is continually kept up to date with the current significant state, every time the latter changes even the slightest bit.  
Say that you now completely [destroy the caches and infer](reinference) them again from the current significant state.  
One would naturally expect the inferred cache to remain completely unchanged, maybe even identical down to every single byte.  
Unfortunately, that is not always the case.  

Assume the [previous example](#explanation).
Assume also, that, at some point in your code, you do this:

```cpp
// Prints all existent names of entities
for (const auto& e : entities_by_name) {
	std::cout << e.first.name << std::endl;
}
```
It may print something like:

```
Road
Tree
BILMER2000
Motorcycle
```

If, however, you now completely [reinfer](reinference) ``entities_by_name``, the same code may produce:

```
BILMER2000
Tree
Motorcycle
Road
```

Iterating two distinct ``std::unordered_map``s and expecting the order to be the same is only reasonable when the elements are identical and inserted in the same order (this, however, [*is not* enforced by the standard](https://stackoverflow.com/a/13623172/503776), but can be easily verified empirically).
However, once we reinfer the map, we pass it elements (entity names) in a possibly completely different order from when it was incrementally kept up to date - entity creations, deletions and renames might have happened arbitrarily. Which means that our inferred cache is not quite identical to the one before reinference, even though they were generated from the same significant state. 

This is an important implication when dealing with simulation's [determinism](determinism).  
For example, if one client needs to completely [reinfer](reinference) from their significant state, all others **should reinfer too**, so that the simulation does not diverge across machines.  
This is particularly important when dealing with [``physics_cache``](physics_cache), which also keeps all contact information in a ``b2World``.

### Children tracker

Some inferred caches are simple enough that, if they are defined per-object, there is always a single cache per-object to generate or destroy.  
That is not necessarily the case with caches that 
There exists a special case of an inferred cache.

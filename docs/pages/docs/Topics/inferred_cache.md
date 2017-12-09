---
title: Inferred cache
tags: [topics, ECS] 
hide_sidebar: true
permalink: inferred_cache
summary: An **inferred cache** is a portion of state that only ever exists at [run time](https://en.wikipedia.org/wiki/Run_time_(program_lifecycle_phase)) and can always be completely regenerated from some [significant state](significant_state).
---

## Etymology

The term is composed of two words, each of which describes an important property of the state that it refers to:
- *inferred* means that it can always be recalculated based on the current contents of significant state;
- *cache* means that its destruction does not imply loss of data in any way meaningful to the application user; which, in some way, is already implied by the capability of being *inferred*.

## Purpose

An inferred cache is a classical example of a [time–memory trade-off](https://en.wikipedia.org/wiki/Space%E2%80%93time_tradeoff).
If our computers had infinite processing speed, existence of inferred caches would be pretty much pointless.

### Explanation

Consider how you would manage names for your game objects.
Say that the state for your game looks like this:

```cpp
struct my_entity {
	std::string my_cool_name;
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

You would keep ``entities_by_name`` up to update each time an entity gets deleted, created, or if name of some entity changes.  
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

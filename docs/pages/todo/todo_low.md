---
title: Low priority ToDo
hide_sidebar: true
permalink: todo_low
---

separate guids from the cosmos, they are rarely needed
consider if entities without guids are at all addressed by guids;
in any case, we can always iterate if we do not find an entry within the map,
and the guids will nicely fit into the inferred cache scheme. 

has/find templates should fail for always_present components
We might leave the find so that we can conveniently switch between a component being always_present or not.
Has is anyway a bad pattern.

rename world, cosmos to csm

- Describe two kinds of state: constant-divergent and exponentially-divergent
	- tree of NPO, sprites, polygons, renders: constant divergence
	- sentience, fixtures, rigid body: exponential divergence

- implement instances of cooldowns for casts statelessly when there is such a need

- introduce cosmos::retick function that can change delta while preserving timings by updating all stepped timestamps according to lifetimes found in other places

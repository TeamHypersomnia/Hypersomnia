---
title: Entity type
tags: [topics, ECS]
hide_sidebar: true
permalink: entity_type
summary: |
  An **entity type** is a C++ class that hard-codes a specific collection of invariants and components for entities instantiated from this type.
---

<!--
	- An existent invariant implies that the entity needs a component of type ``invariant_type::implied_component`` (if specified) for the invariant to be ever used by the logic.  
		- Thus if implied_component type is specified, it additionally stores an **initial value** for the component.
-->

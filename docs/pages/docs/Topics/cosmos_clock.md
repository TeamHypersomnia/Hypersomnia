---
title: Cosmos clock
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos_clock
summary: |
    The **``cosmos_clock``** is an object holding the current step number, the fixed delta time and the [entity guid](entity_guid) to be assigned to the next created entity.  
    It is a part of the [cosmos's significant state](cosmos#significant).
---

## Fixed delta

The field ``cosmos_clock::delta`` is used by [solvers](solver) to determine the amount of time by which to move the game state forward in time.  
The behaviour is undefined if the delta is less or equal than ``0`` (in other words, equal to ``delta::zero``).

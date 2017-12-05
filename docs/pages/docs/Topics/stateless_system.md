---
title: Stateless system
tags: [topics, ECS] 
hide_sidebar: true
summary: |
    A [system](system) is called *stateless* if the side effects of all of its [systematic functions](systematic_function) are limited to the fields of the [logic step](logic_step) or just the [cosmos](cosmos) that each takes as input.
permalink: stateless_system
---

## Conventions

A stateless system is usually an empty class without member fields, only member [systematic functions](systematic_function).  
Thus the name *stateless*.  
In this case these member functions could as well be free-standing functions in the global scope.  

If a stateless system needed to initialize some heavy data to be used once by all of its member functions, having those functions as members allows us to do so transparently to the system's client; though to date, no stateless system needs that.

---
title: Audiovisual system
tags: [topics, ECS] 
hide_sidebar: true
summary: |
    A [system](system) is called *audiovisual* if all of its [systematic functions](systematic_function) take only immutable variants of [logic step](logic_step) or [cosmos](cosmos), (``const_logic_step`` or ``const cosmos&``) and hence, whose side effects are limited to the private fields of the system.
permalink: audiovisual_system
---

## Conventions

Since the definition of an *audiovisual* system effectively prohibits any alteration to our "application model", it makes literally no sense to use it for anything else than for the "view" part of the application.  
Thence the name *audiovisual* (sound is a part of view, too!).

For example, a particle system 

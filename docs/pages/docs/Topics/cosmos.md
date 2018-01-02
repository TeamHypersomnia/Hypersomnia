---
title: Cosmos
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos
summary: |
    The **cosmos** stores [cosmos common](cosmos_common) and [cosmos solvable](cosmos_solvable).  
    The class itself only exists to tie these two functionalities into a single, comfortable object;  
    it provides many handy helper functions to perform common operations that require both of these objects.  

    The entire state of the cosmos is considered [replicated](replicated_state).
---

## Overview

The cosmos is arguably the most complex structure in the entire codebase.  

The reason that the cosmos couples so much functionality into what is a single object, is because if one were to try to separate one from any other, they would only accomplish so much as to introduce unnecessary noise, in the form of twice as many references passed across the entire game's logic; not only would it be far less readable, it could also hinder performance in the long run.  

Methods of the **cosmos** are mostly handy getters. Examples:
- get a handle to an entity referenced by an [id](entity_id) or [guid](entity_guid);
- get a [processing list](processing_lists_cache) of a given kind;
- get all entities matching a given name. 


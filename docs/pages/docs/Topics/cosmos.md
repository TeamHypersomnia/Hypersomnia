---
title: Cosmos
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmos
summary: |
    The **cosmos** stores [cosmos common](cosmos_common) and [cosmos solvable](cosmos_solvable).  
    The class itself only exists to tie these two functionalities into a single, comfortable object;  
    it provides many handy helper functions to perform common operations that require both of these objects.  
---

## Overview

The cosmos is arguably the most complex structure in the entire codebase.  

The reason that the cosmos couples so much functionality into what is a single object, is because if one were to try to separate one from any other, they would only accomplish so much as to introduce unnecessary noise, in the form of twice as many references passed across the entire game's logic; not only would it be far less readable, it could also hinder performance in the long run.  

Methods of the **cosmos** allow to, for example, create entities, access and modify them via returned [handles](entity_handle), clone or delete them.  
There are also many other helper methods: e.g. getting a handle to an entity referenced by an [id](entity_id), [guid](entity_guid), getting a [processing list](processing_lists_cache) of a given kind, or getting all entities matching a given name. While, under the principle of separation of concerns, it would make sense to distribute such functions across multiple source files or even classes, there is currently no benefit seen in doing that.


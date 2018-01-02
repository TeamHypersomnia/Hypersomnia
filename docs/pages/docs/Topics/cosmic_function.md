---
title: Cosmic function
tags: [topics, ECS] 
hide_sidebar: true
permalink: cosmic_function
summary: |
    A function is called **cosmic** if it has direct write access to the entire [cosmos solvable](cosmos_solvable). 
    
    Except for methods of [entity handle](entity_handle) and [cosmic delta](cosmic_delta), all cosmic functions are declared statically under the class named *cosmic*.
---

## Overview

Methods of the **cosmic** class allow to, for example, create [entities](entity), access and modify them via returned [handles](entity_handle), clone, delete them or [reinfer](reinference) their caches.

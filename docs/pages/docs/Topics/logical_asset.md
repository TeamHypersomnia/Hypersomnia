---
title: Logical asset
tags: [topics, flyweights]
hide_sidebar: true
permalink: logical_asset
summary: | 
  A **logical asset** is a piece of information that is stored once and may be referenced by ids throughout the entire [cosmos](cosmos).  
  Formally, it is a [flyweight](flyweight) whose instance is an integer identifier.  
  
  While [entity types](entity_type) exist to be shared between entities, a **logical asset** can be shared even by entity types themselves.
  
---

## Overview

Examples of a logical asset:
- a physical material, containing a matrix of sound identificators for collisions.
- a recoil player, containing a vector of forces to be applied to a gun when it is firing. 

There also exist logical assets that are not directly defined by the [author](author), but are generated automatically from a given [viewable](viewable).  
In this case, such a logical asset is called a [logical viewable](logical_viewable).
